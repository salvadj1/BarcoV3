// ESCMotor.cpp
// Control del ESC y motor principal

#include "ESCMotor.h"

// ---------- OBJETO SERVO (ESC) ----------
static Servo escMotor;

// ---------- VARIABLES EXPORTADAS ----------
int  throttleMax   = 50;
int  throttleMin   = 6;
bool motorRunning  = false;
int  motorPctActual = 0;

// ---------- HELPERS ----------
int pctToUs(int pct) {
    return ESC_STOP + (int)((ESC_MAX - ESC_STOP) * pct / 100.0);
}

// ---------- CONTROL ----------
void setMotorPct(int pct) {
    pct = constrain(pct, 0, 100);
    motorPctActual = pct;
    escMotor.writeMicroseconds(pctToUs(pct));
}

// ---------- SETUP ----------
void SetupESC() {
    escMotor.attach(PIN_ESC, 1000, 1900);
    escMotor.writeMicroseconds(ESC_STOP);
}
