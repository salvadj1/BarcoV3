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

    uint8_t  navState;       // 0=IDLE, 1=GOING_CEBO1, 2=GOING_CEBO2, 3=RETURNING, 4=ARRIVED

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
    int      throttleMin;   // velocidad minima freno progresivo (0-50)
    bool     motorRunning;
    int      motorPctActual;  // velocidad real actual del motor 0-100%
    int      timonAngle;
    bool     invertirTimon;  // estado actual del eje del timon
    int      trimTimon;      // trim en decimas de grado (-150..150 = -15.0..+15.0)
    float    targetBearing;  // rumbo hacia el destino actual (0-360)
    int      distProximidad; // metros de proximidad para llegar (1-30)
    int      pausaMotor;     // segundos de pausa al llegar a cada punto (0-30)

    // GY273 calibracion
    uint8_t  calibProgress;  // 0-100 % durante calibracion, 255=idle
    bool     calibOK;        // true si hay calibracion guardada
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
#define CMD_PROXIMIDAD    9   // radio de llegada en metros (1-30)
#define CMD_PAUSA        10   // segundos de pausa en cada punto (0-30)
#define CMD_THROTTLE_MIN 11   // velocidad minima durante freno progresivo (0-50)

struct ComandoPlaya {
    uint8_t tipo;

    // CMD_JOYSTICK
    int     rumbo;           // -100 a 100 (izq/der)
    int     throttle;        // 0 a 100

    // CMD_GUARDAR_PUNTO
    uint8_t punto;           // 0=home, 1=cebo1, 2=cebo2

    // CMD_CEBO
    uint8_t numeroCebo;      // 1 o 2
    bool    abrirCebo;

    // CMD_THROTTLE
    int     nuevoThrottle;

    // CMD_TRIM
    int     trimTimon;       // decimas de grado (-150..150)

    // CMD_INVERT_TIMON
    bool    invertirTimon;   // true=invertir eje, false=normal

    // CMD_PROXIMIDAD
    int     distProximidad;  // metros (1-30)

    // CMD_PAUSA
    int     pausaMotor;      // segundos (0-30)

    // CMD_THROTTLE_MIN
    int     nuevoThrottleMin; // velocidad minima freno progresivo (0-50)
};

#endif
