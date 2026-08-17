// Servos.cpp
// Unicamente control de los servos de cebo 1 y cebo 2

#include "Servos.h"
#include "LucesCOB.h"

// Duracion del destello de luces al soltar un cebo (2-3 pulsos de 300ms)
static const unsigned long CEBO_PULSO_DURACION_MS = 900;

// ---------- OBJETOS SERVO ----------
static Servo servoCebo1;
static Servo servoCebo2;

// ---------- VARIABLES EXPORTADAS ----------
bool cebo1Abierto   = false;
bool cebo2Abierto   = false;
bool cebo1Disparado = false;
bool cebo2Disparado = false;
unsigned long cebo1PulsoFin = 0;
unsigned long cebo2PulsoFin = 0;

// ---------- CONTROL ----------
void setCebo1(bool abrir) {
    // Flanco de apertura (manual o automatico) -> destello rapido en babor
    if (abrir && !cebo1Abierto) {
        cobDestello(COB_BABOR, 300, 255);
        cebo1PulsoFin = millis() + CEBO_PULSO_DURACION_MS;
    }
    cebo1Abierto = abrir;
    servoCebo1.write(abrir ? CEBO_ABIERTO : CEBO_CERRADO);
}

void setCebo2(bool abrir) {
    // Flanco de apertura (manual o automatico) -> destello rapido en estribor
    if (abrir && !cebo2Abierto) {
        cobDestello(COB_ESTRIBOR, 300, 255);
        cebo2PulsoFin = millis() + CEBO_PULSO_DURACION_MS;
    }
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