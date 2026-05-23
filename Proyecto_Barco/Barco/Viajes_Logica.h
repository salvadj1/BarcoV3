// Viajes_Logica.h

#ifndef _VIAJES_LOGICA_h
#define _VIAJES_LOGICA_h

#include <Arduino.h>
#include "GPS_Neo_6M.h"

// ---------- COORDENADAS Y ESTADO ----------
extern Coordenada home;
extern Coordenada cebo1;
extern Coordenada cebo2;

extern EstadoNavegacion navState;

// ---------- HISTORIAL ----------
extern HistPoint     history[MAX_HIST];
extern int           histCount;

// ---------- ESTADISTICAS ----------
extern double        totalDist;
extern double        maxSpeed;
extern double        sumSpeed;
extern int           speedSamples;
extern double        lastLat;
extern double        lastLng;
extern unsigned long tripStartMs;

// ---------- FUNCIONES ----------
double haversine(double lat1, double lng1, double lat2, double lng2);
void   saveCurrentTrip();
String loadTrip(const char* path);
void   LoopViajesLogica();

// ---------- PARAMETROS CONFIGURABLES ----------
extern int distProximidad;   // metros de proximidad para llegar (1-30)
extern int pausaMotor;       // segundos de pausa al llegar a cada punto (0-30)
void saveParamsViaje();
void loadParamsViaje();

// ---------- PERSISTENCIA DE PUNTOS ----------
void savePointsToSPIFFS();
void loadPointsFromSPIFFS();

// ---------- PAUSA NO BLOQUEANTE ----------
extern uint8_t       pausandoEn;
extern unsigned long pausaInicioMs;

#endif