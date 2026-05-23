// GY273_Module.h
// HMC5883L via Adafruit_HMC5883_U
// Wire.begin() se llama en Barco.ino antes de SetupGY273()

#ifndef _GY273_MODULE_h
#define _GY273_MODULE_h

#include <Arduino.h>

// ---------- CALIBRACION SPIFFS ----------
#define GY273_CALIB_FILE     "/gy273_calib.dat"
#define GY273_CALIB_SAMPLES   500

// ---------- DECLINACION MAGNETICA ----------
// Torrevieja, Espana: +1deg17' = +0.022 rad
#define GY273_DECLINATION_RAD  0.022f

// ---------- ESTADO EXPORTADO ----------
extern double  currentCourse;       // heading final 0-360 grados
extern bool    courseValid;

extern bool    gy273CalibOK;
extern uint8_t gy273CalibProgress;  // 0-100 durante calib, 255=idle
extern float   gy273OffsetX, gy273OffsetY;
extern float   gy273ScaleX,  gy273ScaleY;

// ---------- FUNCIONES ----------
void SetupGY273();
void LoopGY273();
void IniciarCalibracionGY273();
bool CalibracionGY273Completa();

#endif
