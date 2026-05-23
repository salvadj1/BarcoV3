// Shared_Types.h
// ESTE ARCHIVO ES IDENTICO EN AMBOS PROYECTOS (Barco y Playa)
// Structs de comunicacion ESP-NOW

#ifndef _SHARED_TYPES_h
#define _SHARED_TYPES_h

#include <stdint.h>

// ---------- BARCO -> PLAYA : Telemetria ----------
struct TelemetriaBarco {
    float    lat;
    float    lng;
    float    speed;
    float    course;
    float    alt;
    uint8_t  sats;
    float    hdop;
    bool     fix;

    uint8_t  navState;

    bool     cebo1Abierto;
    bool     cebo2Abierto;
    bool     cebo1Disparado;
    bool     cebo2Disparado;

    float    totalDist;
    float    maxSpeed;
    float    avgSpeed;
    uint32_t tripSecs;

    float    homeLat;
    float    homeLng;
    bool     homeValid;
    float    c1Lat;
    float    c1Lng;
    bool     c1Valid;
    float    c2Lat;
    float    c2Lng;
    bool     c2Valid;

    float    smoothLat;
    float    smoothLng;

    uint8_t  timeH;
    uint8_t  timeM;
    uint8_t  timeS;

    int      throttleMax;
    int      throttleMin;
    bool     motorRunning;
    int      motorPctActual;
    int      timonAngle;         // grados reales desde encoder (negativo=izq, positivo=der)
    bool     invertirTimon;
    int      trimTimon;          // decimas de grado (-150..150)
    float    targetBearing;
    int      distProximidad;
    int      pausaMotor;

    bool     timonReferenciada;  // true si el encoder tiene referencia valida

    // GY273 calibracion
    uint8_t  calibProgress;
    bool     calibOK;
    float    calibOffsetX;
    float    calibOffsetY;
    float    calibScaleX;
    float    calibScaleY;
};

// ---------- PLAYA -> BARCO : Comandos ----------
#define CMD_JOYSTICK      0
#define CMD_GUARDAR_PUNTO 1
#define CMD_CEBO          2
#define CMD_START_TRIP    3
#define CMD_STOP_TRIP     4
#define CMD_THROTTLE      5
#define CMD_TRIM          6
#define CMD_INVERT_TIMON  7
#define CMD_CALIB_GY273   8
#define CMD_PROXIMIDAD    9
#define CMD_PAUSA        10
#define CMD_THROTTLE_MIN 11
#define CMD_CENTER_TIMON 12   // resetea encSteps=0, fija referencia de centro

struct ComandoPlaya {
    uint8_t tipo;

    // CMD_JOYSTICK
    int     rumbo;           // -1=izquierda, 0=centro, +1=derecha
    int     throttle;        // 0 a 100

    // CMD_GUARDAR_PUNTO
    uint8_t punto;

    // CMD_CEBO
    uint8_t numeroCebo;
    bool    abrirCebo;

    // CMD_THROTTLE
    int     nuevoThrottle;

    // CMD_TRIM
    int     trimTimon;

    // CMD_INVERT_TIMON
    bool    invertirTimon;

    // CMD_PROXIMIDAD
    int     distProximidad;

    // CMD_PAUSA
    int     pausaMotor;

    // CMD_THROTTLE_MIN
    int     nuevoThrottleMin;
};

#endif
