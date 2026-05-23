// TimonSistema.cpp
// Control del timon: motor DC reductor + encoder HW040
//
// MODO MANUAL (joySteer):
//   joySteer > 0  -> ir a +TIMON_MANUAL_MAX_DEG (derecha)
//   joySteer < 0  -> ir a -TIMON_MANUAL_MAX_DEG (izquierda)
//   joySteer == 0 -> volver al centro (trim)
//
// REFERENCIA:
//   Pulsar SW del encoder fisico O comando CTR web -> encSteps=0=centro
//
// updateTimon() debe llamarse a 50 Hz desde Barco.ino

#include "TimonSistema.h"
#include "ESCMotor.h"

// ============================================================
//  ENCODER HW040 — cuadratura por ISR
// ============================================================
static volatile int32_t encSteps = 0;

static volatile uint8_t  encLast = 0;
static volatile int8_t   encAcc  = 0;

static const int8_t ENC_TABLE[16] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

static void IRAM_ATTR isrEncoder() {
    uint8_t clk = digitalRead(TIMON_ENC_CLK);
    uint8_t dt  = digitalRead(TIMON_ENC_DT);
    uint8_t ns  = (clk << 1) | dt;
    encAcc += ENC_TABLE[(encLast << 2) | ns];
    encLast = ns;
    if (encAcc >= 2) { encSteps++; encAcc = 0; }
    if (encAcc <= -2){ encSteps--; encAcc = 0; }
}

// ISR boton SW: resetea referencia (centro fisico)
static volatile bool swPressed = false;
static void IRAM_ATTR isrSW() {
    swPressed = true;
}

static int32_t readEnc() {
    noInterrupts();
    int32_t v = encSteps;
    interrupts();
    return v;
}

// ============================================================
//  VARIABLES EXPORTADAS
// ============================================================
double targetBearing    = 0;
bool   modoManual       = false;
int    joySteer         = 0;
float  trimTimon        = 0.0f;
bool   invertirTimon    = false;
int    currentTimonSteps = 0;
int    timonTargetSteps  = 0;
bool   timonReferenciada = false;

// ============================================================
//  WATCHDOG JOYSTICK
//  Si no llega CMD_JOYSTICK en TIMON_JOY_TIMEOUT ms -> joySteer=0
// ============================================================
static unsigned long lastJoyMs = 0;
#define TIMON_JOY_TIMEOUT  600   // ms sin comando -> parar (mayor que intervalo web 100ms)

void updateJoyTimestamp() {
    lastJoyMs = millis();
}

// ============================================================
//  RESET REFERENCIA (centro fisico)
// ============================================================
void resetTimonReferencia() {
    noInterrupts();
    encSteps = 0;
    encAcc   = 0;
    interrupts();
    currentTimonSteps = 0;   // actualizar inmediatamente sin esperar moverHacia
    timonTargetSteps  = 0;
    timonReferenciada = true;
    Serial.println("Timon: referencia reseteada - centro=0");
}

// ============================================================
//  DRIVER TB6612FNG
// ============================================================
static void motorTimon(int pwm) {
    pwm = constrain(pwm, -255, 255);
    if (invertirTimon) pwm = -pwm;

    if (pwm == 0) {
        digitalWrite(TIMON_AIN1, HIGH);
        digitalWrite(TIMON_AIN2, HIGH);
        ledcWrite(TIMON_PWM_CH, 0);
    } else if (pwm > 0) {
        digitalWrite(TIMON_AIN1, HIGH);
        digitalWrite(TIMON_AIN2, LOW);
        ledcWrite(TIMON_PWM_CH, (uint8_t)pwm);
    } else {
        digitalWrite(TIMON_AIN1, LOW);
        digitalWrite(TIMON_AIN2, HIGH);
        ledcWrite(TIMON_PWM_CH, (uint8_t)(-pwm));
    }
}

static void frenarTimon() {
    motorTimon(0);
}

// ============================================================
//  BUCLE PROPORCIONAL DE POSICION
// ============================================================
static void moverHacia(int targetSteps) {
    // currentTimonSteps ya actualizado en updateTimon()

    // Seguridad absoluta: si supera el limite fisico, parar motor inmediatamente
    if (currentTimonSteps >  TIMON_STEPS_FULL) { frenarTimon(); return; }
    if (currentTimonSteps < -TIMON_STEPS_FULL) { frenarTimon(); return; }

    targetSteps = constrain(targetSteps, -TIMON_STEPS_FULL, TIMON_STEPS_FULL);
    timonTargetSteps = targetSteps;

    int error = targetSteps - currentTimonSteps;

    if (abs(error) <= TIMON_DEAD_ZONE) { frenarTimon(); return; }

    // Maxima velocidad siempre
    motorTimon(error > 0 ? 255 : -255);
}

// ============================================================
//  PERSISTENCIA SPIFFS
// ============================================================
#define TRIM_FILE   "/trim.txt"
#define INVERT_FILE "/invert.dat"

void loadTrim() {
    if (SPIFFS.exists(TRIM_FILE)) {
        File f = SPIFFS.open(TRIM_FILE, FILE_READ);
        if (f) { trimTimon = constrain(f.readString().toFloat(), -15.0f, 15.0f); f.close(); }
        Serial.printf("Trim timon cargado: %.1f\n", trimTimon);
    }
    if (SPIFFS.exists(INVERT_FILE)) {
        File f = SPIFFS.open(INVERT_FILE, FILE_READ);
        if (f) { invertirTimon = (bool)f.readString().toInt(); f.close(); }
        Serial.printf("Inversion timon: %d\n", (int)invertirTimon);
    }
}

void saveTrim() {
    File f = SPIFFS.open(TRIM_FILE, FILE_WRITE);
    if (!f) return;
    f.print(trimTimon, 1);
    f.close();
}

void saveInvert() {
    File f = SPIFFS.open(INVERT_FILE, FILE_WRITE);
    if (!f) return;
    f.print(invertirTimon ? 1 : 0);
    f.close();
}

// ============================================================
//  CALCULO BEARING
// ============================================================
double calcBearing(double lat1, double lng1, double lat2, double lng2) {
    double dLng = radians(lng2 - lng1);
    double y = sin(dLng) * cos(radians(lat2));
    double x = cos(radians(lat1)) * sin(radians(lat2)) -
               sin(radians(lat1)) * cos(radians(lat2)) * cos(dLng);
    return fmod(degrees(atan2(y, x)) + 360.0, 360.0);
}

// ============================================================
//  PARAMETROS AUTONOMO
// ============================================================
static const double Kp_heading = 0.40;
static const double ZONA_MUERTA = 3.0;

// ============================================================
//  UPDATE TIMON — llamar a 50 Hz
// ============================================================
void updateTimon() {

    // --- Comprobar boton SW fisico del encoder ---
    if (swPressed) {
        swPressed = false;
        // Debounce simple por software
        static uint32_t lastSW = 0;
        if (millis() - lastSW > 300) {
            lastSW = millis();
            resetTimonReferencia();
        }
    }

    // Actualizar posicion actual del encoder SIEMPRE (para telemetria correcta)
    currentTimonSteps = (int)readEnc();

    // --- WATCHDOG: sin comando joystick en TIMON_JOY_TIMEOUT ms -> parar ---
    if (modoManual && (millis() - lastJoyMs > TIMON_JOY_TIMEOUT)) {
        frenarTimon();
        return;
    }

    // Trim en pasos
    int trimSteps = (int)(trimTimon * TIMON_PASOS_POR_GRADO);

    // ---- MODO MANUAL ----
    if (modoManual) {
        if (joySteer == 0) {
            moverHacia(trimSteps);   // volver al centro
        } else if (joySteer > 0) {
            moverHacia(TIMON_STEPS_MAX + trimSteps);
        } else {
            moverHacia(-TIMON_STEPS_MAX + trimSteps);
        }
        return;
    }

    // ---- IDLE: motor parado o sin rumbo valido ----
    if (!motorRunning || !courseValid) {
        moverHacia(trimSteps);
        return;
    }

    // ---- AUTONOMO: proporcional sobre error de rumbo ----
    double error = targetBearing - currentCourse;
    while (error >  180.0) error -= 360.0;
    while (error < -180.0) error += 360.0;

    int objetivo;
    if (fabs(error) < ZONA_MUERTA) {
        objetivo = trimSteps;
    } else {
        double angDeg = constrain(Kp_heading * error, -35.0, 35.0);
        objetivo = (int)(angDeg * TIMON_PASOS_POR_GRADO) + trimSteps;
    }

    moverHacia(objetivo);
}

// ============================================================
//  SETUP
// ============================================================
void SetupTimon() {
    // Driver TB6612FNG
    pinMode(TIMON_AIN1, OUTPUT);
    pinMode(TIMON_AIN2, OUTPUT);
    pinMode(TIMON_STBY, OUTPUT);
    digitalWrite(TIMON_STBY, HIGH);

    ledcSetup(TIMON_PWM_CH, TIMON_PWM_FREQ, TIMON_PWM_RES);
    ledcAttachPin(TIMON_PWMA, TIMON_PWM_CH);
    ledcWrite(TIMON_PWM_CH, 0);

    // Encoder HW040 - CLK y DT
    pinMode(TIMON_ENC_CLK, INPUT_PULLUP);
    pinMode(TIMON_ENC_DT,  INPUT_PULLUP);
    encLast = (digitalRead(TIMON_ENC_CLK) << 1) | digitalRead(TIMON_ENC_DT);
    attachInterrupt(digitalPinToInterrupt(TIMON_ENC_CLK), isrEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(TIMON_ENC_DT),  isrEncoder, CHANGE);

    // Encoder SW - boton reset referencia
    pinMode(TIMON_ENC_SW, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TIMON_ENC_SW), isrSW, FALLING);

    frenarTimon();

    // SPIFFS ya montado por Barco.ino
    loadTrim();

    lastJoyMs = millis();   // inicializar watchdog

    Serial.println("TimonSistema: OK");
    Serial.printf("  PWMA=GPIO%d  STBY=GPIO%d  AIN1=GPIO%d  AIN2=GPIO%d\n",
                  TIMON_PWMA, TIMON_STBY, TIMON_AIN1, TIMON_AIN2);
    Serial.printf("  ENC_CLK=GPIO%d  ENC_DT=GPIO%d  ENC_SW=GPIO%d\n",
                  TIMON_ENC_CLK, TIMON_ENC_DT, TIMON_ENC_SW);
    Serial.printf("  Manual max: %d pasos = %.0f grados  Escala: %.1f grados/paso\n",
                  TIMON_STEPS_MAX, TIMON_STEPS_MAX * TIMON_GRADOS_POR_PASO, TIMON_GRADOS_POR_PASO);
    Serial.println("  >> Centra el timon y pulsa SW o CTR en la web para fijar referencia");
}
