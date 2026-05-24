// ESCMotor.h
// Control del ESC y motor principal
// Wire-up: PIN_ESC (GPIO 25), rango 1000-1900 µs

#ifndef _ESCMOTOR_h
#define _ESCMOTOR_h

#include <Arduino.h>
#include <ESP32Servo.h>

// ---------- PIN ----------
#define PIN_ESC 25

// ---------- LIMITES ESC (microsegundos) ----------
#define ESC_STOP 1000
#define ESC_MAX 1900

// ---------- ESTADO EXPORTADO ----------
extern int throttleMax;     // velocidad maxima de crucero (0-100%)
extern int throttleMin;     // velocidad minima durante freno progresivo (0-50%), default 6
extern bool motorRunning;   // true si el motor esta en marcha
extern int motorPctActual;  // velocidad real actual del motor 0-100%

// ---------- FUNCIONES ----------
void SetupESC();
void setMotorPct(int pct);
int pctToUs(int pct);

#endif
