// Servos.h
// Unicamente control de los servos de cebo 1 y cebo 2
// Wire-up: PIN_CEBO1 (GPIO 27), PIN_CEBO2 (GPIO 12)

#ifndef _SERVOS_h
#define _SERVOS_h

#include <Arduino.h>
#include <ESP32Servo.h>

// ---------- PINES ----------
#define PIN_CEBO1  27
#define PIN_CEBO2  12

// ---------- ANGULOS SERVO ----------
#define CEBO_CERRADO   0
#define CEBO_ABIERTO  90

// ---------- ESTADO EXPORTADO ----------
extern bool cebo1Abierto;
extern bool cebo2Abierto;
extern bool cebo1Disparado;
extern bool cebo2Disparado;

// Instante (millis) hasta el que dura el destello de luces al soltar cada cebo.
// Mientras "millis() < cebo1PulsoFin" el destello esta en marcha y LucesLogica
// no debe tocar el lado correspondiente.
extern unsigned long cebo1PulsoFin;
extern unsigned long cebo2PulsoFin;

// ---------- FUNCIONES ----------
void SetupCebos();
void setCebo1(bool abrir);
void setCebo2(bool abrir);

#endif