#include "TimonSistema.h"
#include "HW040Encoder.h"
#include "ESCMotor.h"
#include "Utilidades.h"
#include "TB6612FNG.h"

// ============================================================
//  VARIABLES EXPORTADAS
// ============================================================

float trimTimon = 0.0f;
bool invertirTimon = false;
double targetBearing = 0;
bool modoManual = false;

int joySteer = 0;

int timonTargetSteps = 0;

bool timonReferenciada = false;

int timonEnGrados = 180;
int TIMON_OBJETIVO_EN_GRADOS = 180;
int Timon_ultimo_objetivo = 180;

static const int ZONA_MUERTA = 10;
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
    }
    Serial.printf("Trim timon cargado: %.1f\n", trimTimon);
  }
  if (SPIFFS.exists(INVERT_FILE)) {
    File f = SPIFFS.open(INVERT_FILE, FILE_READ);
    if (f) {
      invertirTimon = (bool)f.readString().toInt();
      f.close();
    }
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
  double x = cos(radians(lat1)) * sin(radians(lat2)) - sin(radians(lat1)) * cos(radians(lat2)) * cos(dLng);
  return fmod(degrees(atan2(y, x)) + 360.0, 360.0);
}

// ============================================================
//  PARAMETROS AUTONOMO
// ============================================================
static const double Kp_heading = 0.40;



/*int MoverMotorHaciaDestinoEnGrados(int grados) {
  return map(grados, 0, Pasos_por_revolucion, 0, 360);
}*/

// ============================================================
//  RESET REFERENCIA (centro fisico)
// ============================================================
void ResetearTimon() {

  noInterrupts();

  encoderReset();

  interrupts();

  timonEnGrados = 180;  // actualizar inmediatamente sin esperar moverHacia
  timonTargetSteps = 0;
  timonReferenciada = true;
  Serial.println("Timon: referencia reseteada - centro=0");
}

// ============================================================
//  UPDATE TIMON — llamar a 50 Hz
// ============================================================


/*void updateTimon() {

  timonEnGrados = encoderGetDegrees();

  if (timer_log_Timon.listo(1000)) {
    Serial.printf("[TIMON] grados=%d objetivo=%d\n", timonEnGrados, TIMON_OBJETIVO_EN_GRADOS);
  }



  // --- Comprobar boton SW fisico del encoder ---
  //if (encoderButtonPressed) {
  //  ResetearTimon();
  //}


  // ---- MODO MANUAL: joySteer controla el objetivo ----
  if (modoManual) {
    switch (joySteer) {
      case 0:
        TIMON_OBJETIVO_EN_GRADOS = 180;
        break;
      case -1:  //izquierda
        TIMON_OBJETIVO_EN_GRADOS = 90;
        break;
      case 1:  //derecha
        TIMON_OBJETIVO_EN_GRADOS = 270;
        break;
    }
  }
  // ---- AUTONOMO: proporcional sobre error de rumbo ----
  else if (motorRunning && courseValid) {
    // Error angular normalizado a -180..+180 (camino mas corto)
    double errorRumbo = targetBearing - currentCourse;
    if (errorRumbo >  180.0) errorRumbo -= 360.0;
    if (errorRumbo < -180.0) errorRumbo += 360.0;

    // Aplicar trim de alineacion
    errorRumbo += (double)trimTimon;

    // Invertir si el montaje del encoder esta invertido
    if (invertirTimon) errorRumbo = -errorRumbo;

    // Correccion proporcional -> grados de timon (centro=180)
    double correccion = errorRumbo * Kp_heading;

    // Clamp: maximo giro fisico es 90 grados a cada lado
    correccion = constrain(correccion, -90.0, 90.0);

    TIMON_OBJETIVO_EN_GRADOS = constrain((int)(180.0 + correccion), 90, 270);
  }
  // ---- IDLE: sin viaje activo o sin GPS -> timon al centro ----
  else {
    TIMON_OBJETIVO_EN_GRADOS = 180;
  }

  int velocidad = 0;
  bool sentido = false;

  if (timonEnGrados > TIMON_OBJETIVO_EN_GRADOS) {
    velocidad = TIMON_PWM_MAX;
    sentido = false;
  } else {
    velocidad = TIMON_PWM_MAX;
    sentido = true;
  }

  // Proteccion topes fisicos
  if (timonEnGrados <= 0 || timonEnGrados >= 360) {
    velocidad = 0;
  }

  // Zona muerta: no mover si ya estamos suficientemente cerca
  if (abs(timonEnGrados - TIMON_OBJETIVO_EN_GRADOS) <= ZONA_MUERTA) {
    velocidad = 0;
  }

  // Bloquear motor si no hay referencia valida
  if (!timonReferenciada) {
    velocidad = 0;
  }

  movermotor(velocidad, sentido);
}
*/
void updateTimon() {

  // Leer posicion actual del encoder y convertirla a grados (0-360)
  timonEnGrados = encoderGetDegrees();

  // Log de depuracion a 1 Hz: muestra posicion actual y objetivo
  if (timer_log_Timon.listo(1000)) {
    Serial.printf("[TIMON] grados=%d objetivo=%d\n", timonEnGrados, TIMON_OBJETIVO_EN_GRADOS);
  }

  // --- Comprobar boton SW fisico del encoder ---
  /*if (encoderButtonPressed) {
    ResetearTimon();
  }*/

  // ---- MODO MANUAL: joySteer controla el objetivo ----
  if (modoManual) {
    // Si el timon esta montado al reves, invertir la direccion del joystick
    int joyEfectivo = invertirTimon ? -joySteer : joySteer;

    // Traducir joystick (-1/0/1) a grados de timon, aplicando trim como offset
    switch (joyEfectivo) {
      case 0:  TIMON_OBJETIVO_EN_GRADOS = constrain(180 + (int)trimTimon, 90, 270); break; // recto
      case -1: TIMON_OBJETIVO_EN_GRADOS = constrain(90  + (int)trimTimon, 90, 270); break; // izquierda
      case 1:  TIMON_OBJETIVO_EN_GRADOS = constrain(270 + (int)trimTimon, 90, 270); break; // derecha
    }
  }
  // ---- AUTONOMO: proporcional sobre error de rumbo ----
  else if (motorRunning && courseValid) {
    // Diferencia entre rumbo deseado y rumbo actual
    double errorRumbo = targetBearing - currentCourse;

    // Normalizar a -180..+180 para tomar siempre el camino mas corto
    if (errorRumbo >  180.0) errorRumbo -= 360.0;
    if (errorRumbo < -180.0) errorRumbo += 360.0;

    // Aplicar trim: corrige desviacion sistematica del barco
    errorRumbo += (double)trimTimon;

    // Si el timon esta montado al reves, invertir la correccion
    if (invertirTimon) errorRumbo = -errorRumbo;

    // Multiplicar error por Kp para obtener angulo de correccion (proporcional)
    double correccion = errorRumbo * Kp_heading;

    // Limitar la correccion al rango fisico del timon (±90 grados)
    correccion = constrain(correccion, -90.0, 90.0);

    // Centro=180: sumar correccion y clampear a limites fisicos
    TIMON_OBJETIVO_EN_GRADOS = constrain((int)(180.0 + correccion), 90, 270);
  }
  // ---- IDLE: sin viaje activo o sin GPS ----
  else {
    // Volver al centro aplicando trim
    TIMON_OBJETIVO_EN_GRADOS = constrain(180 + (int)trimTimon, 90, 270);
  }

  int velocidad = 0;
  bool sentido = false;

  // Determinar direccion de giro segun si estamos por encima o por debajo del objetivo
  if (timonEnGrados > TIMON_OBJETIVO_EN_GRADOS) {
    velocidad = TIMON_PWM_MAX;
    sentido = false; // girar hacia menor grado
  } else {
    velocidad = TIMON_PWM_MAX;
    sentido = true;  // girar hacia mayor grado
  }

  // Proteccion topes fisicos: si el encoder llega a 0 o 360 parar el motor
  if (timonEnGrados <= 0 || timonEnGrados >= 360) velocidad = 0;

  // Zona muerta: no mover el motor si ya estamos suficientemente cerca del objetivo
  if (abs(timonEnGrados - TIMON_OBJETIVO_EN_GRADOS) <= ZONA_MUERTA) velocidad = 0;

  // Sin referencia valida: permitir movimiento lento con joystick para poder centrar
  // el timon antes de hacer homing, pero sin logica de objetivo ni retorno automatico
  if (!timonReferenciada) {
    if (joySteer == -1) { movermotor(TIMON_PWM_MIN, false); return; }
    if (joySteer ==  1) { movermotor(TIMON_PWM_MIN, true);  return; }
    movermotor(0, false);
    return;
  }

  // Ejecutar movimiento con la velocidad y sentido calculados
  movermotor(velocidad, sentido);
}


// ============================================================
//  SETUP
// ============================================================
void SetupTimon() {
  // SPIFFS ya montado por Barco.ino
  loadTrim();

  Serial.println("TimonSistema: OK");

  Serial.println("  >> Centra el timon y pulsa SW o CTR en la web para fijar referencia");
}
