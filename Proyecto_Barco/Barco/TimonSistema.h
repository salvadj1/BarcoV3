// TimonSistema.h
// Control del timon: motor DC reductor + encoder HW040
// Driver: TB6612FNG  |  Encoder: HW040 (cuadratura + boton SW)
//
// PINES:
//   GPIO 26 -> TIMON_PWMA
//   GPIO  4 -> TIMON_STBY
//   GPIO 32 -> TIMON_AIN1
//   GPIO 33 -> TIMON_AIN2
//   GPIO 13 -> ENC_CLK
//   GPIO 23 -> ENC_DT
//   GPIO 18 -> ENC_SW  (boton reset referencia - INPUT_PULLUP)
//
// ESCALA: tope a tope = 30 pasos = 180 grados -> 6 grados/paso
// Centro fisico: encSteps=0 (se fija pulsando SW o botón CTR web)
//
// SPIFFS debe estar montado antes de llamar a SetupTimon()

#ifndef _TIMONSISTEMA_h
#define _TIMONSISTEMA_h

#include <Arduino.h>
#include <SPIFFS.h>
#include "GY273_Module.h"  // currentCourse, courseValid


// ---------- PWM ESP32 (LEDC) ----------
//#define TIMON_PWM_CH 4
//#define TIMON_PWM_FREQ 1000
//#define TIMON_PWM_RES 8  // 0-255


// ---------- PARAMETROS DE CONTROL ----------
#define TIMON_PWM_STOP 0
#define TIMON_PWM_MIN 100  // mínimo para vencer fricción — ajusta si no llega
#define TIMON_PWM_MAX 255



// ---------- ESTADO EXPORTADO ----------
extern double targetBearing;
extern bool modoManual;
extern int joySteer;     // -100..100 desde la web (solo autonomo)
extern float trimTimon;  // offset alineacion grados (-15..+15)
extern bool invertirTimon;

extern int timonTargetSteps;
extern bool timonReferenciada;  // true si el encoder tiene referencia valida

extern int timonEnGrados;

// ---------- FUNCIONES ----------
void SetupTimon();
void updateTimon();

void saveTrim();
void loadTrim();
void saveInvert();
void ResetearTimon();  // pone encoderReset(), llamar cuando timon este centrado
//void updateJoyTimestamp();    // llamar en cada CMD_JOYSTICK recibido

double calcBearing(double lat1, double lng1, double lat2, double lng2);

// Convierte pasos a grados
/*inline float timonStepsToDeg(int steps) {
  return steps * TIMON_GRADOS_POR_PASO;
}

// Para telemetria: grados en formato 0-360 con centro=180
inline int timonAngleDeg() {
  int deg = (int)timonStepsToDeg(currentTimonSteps);
  return ((deg + 180) + 360) % 360;  // centro=180, izq<180, der>180
}*/

#endif
