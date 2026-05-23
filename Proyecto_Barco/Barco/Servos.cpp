// Servos.cpp
// Unicamente control de los servos de cebo 1 y cebo 2

#include "Servos.h"

// ---------- OBJETOS SERVO ----------
static Servo servoCebo1;
static Servo servoCebo2;

// ---------- VARIABLES EXPORTADAS ----------
bool cebo1Abierto   = false;
bool cebo2Abierto   = false;
bool cebo1Disparado = false;
bool cebo2Disparado = false;

// ---------- CONTROL ----------
void setCebo1(bool abrir) {
    cebo1Abierto = abrir;
    servoCebo1.write(abrir ? CEBO_ABIERTO : CEBO_CERRADO);
}

void setCebo2(bool abrir) {
    cebo2Abierto = abrir;
    servoCebo2.write(abrir ? CEBO_ABIERTO : CEBO_CERRADO);
}

// ---------- SETUP ----------
void SetupCebos() {
    servoCebo1.attach(PIN_CEBO1, 500, 2500);
    servoCebo2.attach(PIN_CEBO2, 500, 2500);
    servoCebo1.write(CEBO_CERRADO);
    servoCebo2.write(CEBO_CERRADO);
}
