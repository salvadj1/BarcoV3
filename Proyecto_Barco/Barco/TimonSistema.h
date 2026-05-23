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
#include "GY273_Module.h"   // currentCourse, courseValid

// ---------- PINES TB6612FNG ----------
#define TIMON_AIN1    32
#define TIMON_AIN2    33
#define TIMON_PWMA    26
#define TIMON_STBY     4

// ---------- PWM ESP32 (LEDC) ----------
#define TIMON_PWM_CH   4
#define TIMON_PWM_FREQ 1000
#define TIMON_PWM_RES  8   // 0-255

// ---------- ENCODER HW040 ----------
#define TIMON_ENC_CLK  13
#define TIMON_ENC_DT   23
#define TIMON_ENC_SW   18  // boton SW del encoder - resetea referencia

// ---------- ESCALA GRADOS ----------
// Tope a tope: 30 pasos = 360 grados fisicos
// Cada paso = 12 grados
#define TIMON_GRADOS_POR_PASO   12.0f
#define TIMON_PASOS_POR_GRADO   (1.0f / TIMON_GRADOS_POR_PASO)

// ---------- LIMITES MECANICOS ----------
#define TIMON_STEPS_MAX   15   // maximo cada lado (15 * 12 = 180 grados)
#define TIMON_STEPS_FULL  16   // limite seguridad absoluto — para motor si se supera

// ---------- PARAMETROS DE CONTROL ----------
#define TIMON_KP         6.5f
#define TIMON_DEAD_ZONE  1
#define TIMON_PWM_MIN    255
#define TIMON_PWM_MAX    255

// ---------- ESTADO EXPORTADO ----------
extern double targetBearing;
extern bool   modoManual;
extern int    joySteer;          // -100..100 desde la web (solo autonomo)
extern float  trimTimon;         // offset alineacion grados (-15..+15)
extern bool   invertirTimon;
extern int    currentTimonSteps;
extern int    timonTargetSteps;
extern bool   timonReferenciada; // true si el encoder tiene referencia valida

// ---------- FUNCIONES ----------
void SetupTimon();
void updateTimon();

void saveTrim();
void loadTrim();
void saveInvert();
void resetTimonReferencia();   // pone encSteps=0, llamar cuando timon este centrado
void updateJoyTimestamp();     // llamar en cada CMD_JOYSTICK recibido

double calcBearing(double lat1, double lng1, double lat2, double lng2);

// Convierte pasos a grados
inline float timonStepsToDeg(int steps) {
    return steps * TIMON_GRADOS_POR_PASO;
}

// Para telemetria: grados en formato 0-360 con centro=180
inline int timonAngleDeg() {
    int deg = (int)timonStepsToDeg(currentTimonSteps);
    return ((deg + 180) + 360) % 360;   // centro=180, izq<180, der>180
}

#endif
