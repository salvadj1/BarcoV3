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


void updateTimon() {

  timonEnGrados = encoderGetDegrees();

  if (timer_log_Timon.listo(1000)) {
    Serial.printf("[TIMON] grados=%d objetivo=%d\n", timonEnGrados, TIMON_OBJETIVO_EN_GRADOS);
  }



  // --- Comprobar boton SW fisico del encoder ---
  /*if (encoderButtonPressed) {
    ResetearTimon();
  }*/


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




  int velocidad = 0;
  bool sentido = false;

  if (timonEnGrados > TIMON_OBJETIVO_EN_GRADOS) {
    velocidad = TIMON_PWM_MAX;
    sentido = false;
  } else {
    velocidad = TIMON_PWM_MAX;
    sentido = true;
  }


  if (timonEnGrados <= 0 || timonEnGrados >= 360) {
    velocidad = 0;
  }

  if (abs(timonEnGrados - TIMON_OBJETIVO_EN_GRADOS) <= ZONA_MUERTA) {
    velocidad = 0;
  }

  

  movermotor(velocidad, sentido);


  // ---- MODO MANUAL ----
  if (modoManual) {

    return;
  }

  // ---- IDLE: motor parado o sin rumbo valido ----
  if (!motorRunning || !courseValid) {

    return;
  }

  // ---- AUTONOMO: proporcional sobre error de rumbo ----
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
