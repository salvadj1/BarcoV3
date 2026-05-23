// TimonSistema.h
// Control del timon: motor DC reductor + encoder HW040
// Driver: TB6612FNG  |  Encoder: HW040 (cuadratura)
//
// PINES (conflictos resueltos):
//   GPIO 26 -> TIMON_PWMA  (libre tras eliminar servo timon)
//   GPIO  4 -> TIMON_STBY  (libre, sin funcion especial en ESP32)
//   GPIO 32 -> TIMON_AIN1
//   GPIO 33 -> TIMON_AIN2
//   GPIO 34 -> ENC_CLK     (input-only, sin boot-glitch)
//   GPIO 35 -> ENC_DT      (input-only, sin boot-glitch)
//   NOTA: GPIO 34/35 no tienen pullup interno -> añadir 10kΩ externo a 3.3V en CLK y DT
//
// LEDC canal 4: ESP32Servo ocupa canales 0-3 automaticamente; canal 4 queda libre
//
// SPIFFS debe estar montado antes de llamar a SetupTimon()

#ifndef _TIMONSISTEMA_h
#define _TIMONSISTEMA_h

#include <Arduino.h>
#include <SPIFFS.h>
#include "GY273_Module.h"   // currentCourse, courseValid

// ---------- PINES TB6612FNG ----------
#define TIMON_AIN1    32   // direccion A
#define TIMON_AIN2    33   // direccion B
#define TIMON_PWMA    26   // PWM velocidad  (GPIO 26: libre, era el servo timon)
#define TIMON_STBY     4   // standby driver (HIGH = activo)

// ---------- PWM ESP32 (LEDC) ----------
#define TIMON_PWM_CH   4   // canal 4: fuera del rango que usa ESP32Servo (0-3)
#define TIMON_PWM_FREQ 1000
#define TIMON_PWM_RES  8   // 8 bits -> 0-255

// ---------- ENCODER HW040 ----------
//#define TIMON_ENC_CLK  34  // input-only, interrupcion OK, necesita 10kΩ externo a 3.3V
//#define TIMON_ENC_DT   35  // input-only, interrupcion OK, necesita 10kΩ externo a 3.3V
#define TIMON_ENC_CLK  13  
#define TIMON_ENC_DT   23  

// ---------- LIMITES MECANICOS (en pasos de encoder) ----------
// Calibrar TIMON_STEPS_MAX en banco: mover timon hasta el tope fisico y leer encSteps.
#define TIMON_STEPS_MAX   30   // pasos hasta el maximo desvio (ajustar en banco)
#define TIMON_STEPS_FULL  35   // limite de seguridad absoluto antes del tope mecanico

// ---------- PARAMETROS DE CONTROL ----------
#define TIMON_KP         6.5f  // ganancia proporcional pasos->PWM
#define TIMON_DEAD_ZONE  1     // pasos de zona muerta del bucle de posicion
#define TIMON_PWM_MIN    60    // PWM minimo para vencer estatica del reductor
#define TIMON_PWM_MAX    220   // PWM maximo

// ---------- ESTADO EXPORTADO ----------
extern double targetBearing;     // rumbo objetivo (0-360), fijado por Viajes_Logica
extern bool   modoManual;        // true cuando el joystick esta activo
extern int    joySteer;          // -100..100 desde la web
extern float  trimTimon;         // offset de alineacion en grados (-15..+15)
extern bool   invertirTimon;     // invierte direccion del motor
extern int    currentTimonSteps; // posicion real en pasos (neg=izq, pos=der)
extern int    timonTargetSteps;  // objetivo actual en pasos

// ---------- FUNCIONES ----------
void SetupTimon();
void updateTimon();   // llamar a 50 Hz desde Barco.ino

void saveTrim();
void loadTrim();
void saveInvert();

double calcBearing(double lat1, double lng1, double lat2, double lng2);

// Convierte pasos encoder a grados aproximados para telemetria
inline int timonAngleDeg() {
    return (int)((float)currentTimonSteps * 35.0f / (float)TIMON_STEPS_MAX);
}

#endif
