// TimonSistema.cpp
// Control del timon: motor DC reductor + encoder HW040
//
// COMPORTAMIENTO:
//   Manual   -> joySteer != 0 : mueve motor hacia angulo pedido
//               joySteer == 0 : objetivo = centro + trim (al soltar el boton)
//   Autonomo -> objetivo en pasos segun error de rumbo (Kp_heading)
//               dentro de zona muerta -> objetivo = centro + trim
//   IDLE / motor parado -> objetivo = centro + trim
//
// updateTimon() debe llamarse a 50 Hz desde Barco.ino

#include "TimonSistema.h"
#include "ESCMotor.h"  // motorRunning

// ============================================================
//  ENCODER HW040 — cuadratura por ISR
//  GPIO 34/35: input-only, sin pullup interno
//  -> Añadir resistencias 10kΩ externas a 3.3V en CLK y DT
// ============================================================
static volatile int32_t encSteps = 0;

static const int8_t ENC_TABLE[16] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};
static volatile uint8_t encLast = 0;
static volatile int8_t encAcc = 0;

static void IRAM_ATTR isrEncoder() {
  uint8_t clk = digitalRead(TIMON_ENC_CLK);
  uint8_t dt = digitalRead(TIMON_ENC_DT);
  uint8_t ns = (clk << 1) | dt;
  encAcc += ENC_TABLE[(encLast << 2) | ns];
  encLast = ns;
  if (encAcc >= 2) {
    encSteps++;
    encAcc = 0;
  }
  if (encAcc <= -2) {
    encSteps--;
    encAcc = 0;
  }
}

// Lectura thread-safe del encoder
static int32_t readEnc() {
  noInterrupts();
  int32_t v = encSteps;
  interrupts();
  return v;
}

// ============================================================
//  VARIABLES EXPORTADAS
// ============================================================
double targetBearing = 0;
bool modoManual = false;
int joySteer = 0;
float trimTimon = 0.0f;
bool invertirTimon = false;
int currentTimonSteps = 0;
int timonTargetSteps = 0;

// ============================================================
//  DRIVER TB6612FNG
//  pwm: -255..+255  (negativo = izquierda, positivo = derecha)
//  freno activo: AIN1=HIGH, AIN2=HIGH, PWM=0
// ============================================================
static void motorTimon(int pwm) {
  pwm = constrain(pwm, -255, 255);
  if (invertirTimon) pwm = -pwm;

  if (pwm == 0) {
    // Freno activo: mantiene posicion contra la corriente
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
//  Lleva el encoder al targetSteps con PWM proporcional al error
// ============================================================
static void moverHacia(int targetSteps) {
  currentTimonSteps = (int)readEnc();

  // Acotar objetivo a los limites de seguridad
  targetSteps = constrain(targetSteps, -TIMON_STEPS_FULL, TIMON_STEPS_FULL);
  timonTargetSteps = targetSteps;

  int error = targetSteps - currentTimonSteps;

  // Seguridad hardware: no seguir empujando si ya paso el limite fisico
  if (currentTimonSteps > TIMON_STEPS_FULL && error > 0) {
    frenarTimon();
    return;
  }
  if (currentTimonSteps < -TIMON_STEPS_FULL && error < 0) {
    frenarTimon();
    return;
  }

  if (abs(error) <= TIMON_DEAD_ZONE) {
    frenarTimon();
    return;
  }

  int pwm = (int)(TIMON_KP * abs(error));
  pwm = constrain(pwm, TIMON_PWM_MIN, TIMON_PWM_MAX);
  motorTimon(error > 0 ? pwm : -pwm);
}

// ============================================================
//  PERSISTENCIA SPIFFS
// ============================================================
#define TRIM_FILE "/trim.txt"
#define INVERT_FILE "/invert.dat"

void loadTrim() {
  if (SPIFFS.exists(TRIM_FILE)) {
    File f = SPIFFS.open(TRIM_FILE, FILE_READ);
    if (f) {
      trimTimon = constrain(f.readString().toFloat(), -15.0f, 15.0f);
      f.close();
      Serial.printf("Trim timon cargado: %.1f\n", trimTimon);
    }
  }
  if (SPIFFS.exists(INVERT_FILE)) {
    File f = SPIFFS.open(INVERT_FILE, FILE_READ);
    if (f) {
      invertirTimon = (bool)f.readString().toInt();
      f.close();
      Serial.printf("Inversion timon: %d\n", (int)invertirTimon);
    }
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
//  CALCULO DE RUMBO (bearing) entre dos coordenadas GPS
// ============================================================
double calcBearing(double lat1, double lng1, double lat2, double lng2) {
  double dLng = radians(lng2 - lng1);
  double y = sin(dLng) * cos(radians(lat2));
  double x = cos(radians(lat1)) * sin(radians(lat2)) - sin(radians(lat1)) * cos(radians(lat2)) * cos(dLng);
  return fmod(degrees(atan2(y, x)) + 360.0, 360.0);
}

// ============================================================
//  PARAMETROS AUTONOMO
// ============================================================
static const double Kp_heading = 0.40;  // error de rumbo (grados) -> angulo timon (grados)
static const double ZONA_MUERTA = 3.0;  // grados de error de rumbo para ignorar

// ============================================================
//  UPDATE TIMON — llamar a 50 Hz
// ============================================================
void updateTimon() {
  // Trim convertido a pasos de encoder
  int trimSteps = (int)(trimTimon * (float)TIMON_STEPS_MAX / 35.0f);

  // ---- MODO MANUAL ----
  if (modoManual) {
    if (joySteer == 0) {
      // Boton suelto: volver al centro + trim
      moverHacia(trimSteps);
    } else {
      // Boton pulsado: mover al angulo pedido
      int objetivo = (int)((float)joySteer * (float)TIMON_STEPS_MAX / 100.0f) + trimSteps;
      moverHacia(objetivo);
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
  while (error > 180.0) error -= 360.0;
  while (error < -180.0) error += 360.0;

  int objetivo;
  if (fabs(error) < ZONA_MUERTA) {
    objetivo = trimSteps;
  } else {
    double angDeg = constrain(Kp_heading * error, -35.0, 35.0);
    objetivo = (int)(angDeg * (float)TIMON_STEPS_MAX / 35.0f) + trimSteps;
  }

  moverHacia(objetivo);
}

// ============================================================
//  SETUP
// ============================================================
void SetupTimon() {
  // --- Driver TB6612FNG ---
  pinMode(TIMON_AIN1, OUTPUT);
  pinMode(TIMON_AIN2, OUTPUT);
  pinMode(TIMON_STBY, OUTPUT);
  digitalWrite(TIMON_STBY, HIGH);  // activar driver

  // Canal LEDC 4: fuera del rango de ESP32Servo (0-3)
  ledcSetup(TIMON_PWM_CH, TIMON_PWM_FREQ, TIMON_PWM_RES);
  ledcAttachPin(TIMON_PWMA, TIMON_PWM_CH);
  ledcWrite(TIMON_PWM_CH, 0);

  /*
    // --- Encoder HW040 ---
    // GPIO 34/35 son input-only: no usar OUTPUT ni pullup de SW
    // Resistencias 10kΩ externas a 3.3V obligatorias en CLK y DT
    pinMode(TIMON_ENC_CLK, INPUT);
    pinMode(TIMON_ENC_DT,  INPUT);
 */
                                                                                                                                                                    
  // --- Encoder HW040 ---
  pinMode(TIMON_ENC_CLK, INPUT_PULLUP);
  pinMode(TIMON_ENC_DT, INPUT_PULLUP);
  encLast = (digitalRead(TIMON_ENC_CLK) << 1) | digitalRead(TIMON_ENC_DT);
  attachInterrupt(digitalPinToInterrupt(TIMON_ENC_CLK), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(TIMON_ENC_DT), isrEncoder, CHANGE);

  frenarTimon();

  // SPIFFS ya montado por Barco.ino
  loadTrim();

  Serial.println("TimonSistema: TB6612FNG + HW040 OK");
  Serial.printf("  PWMA=GPIO%d  STBY=GPIO%d  AIN1=GPIO%d  AIN2=GPIO%d\n",
                TIMON_PWMA, TIMON_STBY, TIMON_AIN1, TIMON_AIN2);
  Serial.printf("  ENC_CLK=GPIO%d  ENC_DT=GPIO%d  LEDC_CH=%d\n",
                TIMON_ENC_CLK, TIMON_ENC_DT, TIMON_PWM_CH);
}
