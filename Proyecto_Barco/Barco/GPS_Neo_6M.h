// GPS_Neo_6M.h
// Solo posicion GPS y filtro de suavizado
// El rumbo (currentCourse, courseValid) viene de GY273_Module.h

#ifndef _GPS_NEO_6M_h
#define _GPS_NEO_6M_h

#include <Arduino.h>
#include <TinyGPS++.h>

#define FILTER_SIZE 20
#define MAX_HIST    300

extern TinyGPSPlus gps;

// ---------- FILTRO MEDIA MOVIL ----------
extern double filterLat[FILTER_SIZE];
extern double filterLng[FILTER_SIZE];
extern int    filterIdx;
extern bool   filterFull;
extern double smoothLat;
extern double smoothLng;

// ---------- TIPOS COMPARTIDOS ----------
struct Coordenada {
    double lat;
    double lng;
    bool   valid;
};

struct HistPoint {
    double lat;
    double lng;
};

enum EstadoNavegacion {
    IDLE,
    GOING_CEBO1,
    GOING_CEBO2,
    RETURNING,
    ARRIVED
};

void SetupGPS();
//void LoopGPS();
bool LoopGPS();

#endif
