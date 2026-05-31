// TimonSistema.h

// SPIFFS debe estar montado antes de llamar a SetupTimon()

#ifndef _TIMONSISTEMA_h
#define _TIMONSISTEMA_h

#include <Arduino.h>
#include <SPIFFS.h>
#include "GY273_Module.h"  // currentCourse, courseValid

// ---------- PARAMETROS DE CONTROL ----------
#define TIMON_PWM_STOP 0
#define TIMON_PWM_MIN 100  // mínimo para vencer fricción — ajusta si no llega
#define TIMON_PWM_MAX 125

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


double calcBearing(double lat1, double lng1, double lat2, double lng2);

#endif
