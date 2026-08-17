#include "Gamepad.h"
#include "ESPNow_Playa.h"
#include "Shared_Types.h"

// ---------- PINES ----------
const uint8_t pinThrottle = 34;
const uint8_t pinTrim     = 35;
const uint8_t pinIzq      = 25;
const uint8_t pinDer      = 26;
const uint8_t pinCebo1    = 27;

// ---------- ESTADO ----------
int  throttle = 0;
int  trim     = 0;
int  rumbo    = 0;
bool cebo1    = false;

// ---------- ESTADO PREVIO (para detectar cambios y enviar solo si varian) ----------
static int  lastThrottle = -1;
static int  lastTrim     = -9999;
static int  lastRumbo    = -9999;
static bool lastCebo1    = false;

// Convierte lectura ADC (0-4095) a rango de salida
static int MapADC(int pin, int outMin, int outMax) {
    int raw = analogRead(pin); // 0-4095 (ADC 12 bits ESP32)
    return map(raw, 0, 4095, outMin, outMax);
}

void SetupGamepad() {
    // Pines ADC no necesitan pinMode
    pinMode(pinIzq, INPUT_PULLUP);
    pinMode(pinDer, INPUT_PULLUP);
    pinMode(pinCebo1, INPUT_PULLUP);
}

void UpdateGamepad() {
    //Serial.printf("[GAMEPAD-DEBUG] raw throttle (pin %d) = %d\n", pinThrottle, analogRead(pinThrottle));
    throttle = MapADC(pinThrottle, 50, 0);
    trim     = MapADC(pinTrim, -150, 150);

    // Botones con pull-up: LOW = pulsado
    bool izq = (digitalRead(pinIzq) == LOW);
    bool der = (digitalRead(pinDer) == LOW);

    if (izq && !der)      rumbo = -1;
    else if (der && !izq) rumbo = 1;
    else                    rumbo = 0;

    cebo1 = (digitalRead(pinCebo1) == LOW);

    // ---------- ENVIAR SOLO LO QUE CAMBIO (mismo formato que usan los handlers web) ----------
    // Banda muerta en el throttle: el ADC oscila +-1/2 por ruido en reposo,
    // y sin banda muerta eso generaria envios constantes que pisarian el
    // throttle fijado desde la web aunque nadie toque el gatillo del mando.
    const int THROTTLE_DEADBAND = 2;
    if (abs(throttle - lastThrottle) > THROTTLE_DEADBAND || rumbo != lastRumbo) {
        ComandoPlaya cmd = {};
        cmd.tipo     = CMD_JOYSTICK;
        cmd.rumbo    = rumbo;
        cmd.throttle = throttle;
        EnviarComando(cmd);
        Serial.printf("[GAMEPAD] Joystick -> throttle=%d rumbo=%d\n", throttle, rumbo);
        lastThrottle = throttle;
        lastRumbo    = rumbo;
    }

   /* if (trim != lastTrim) {
        ComandoPlaya cmd = {};
        cmd.tipo      = CMD_TRIM;
        cmd.trimTimon = trim;
        EnviarComando(cmd);
        Serial.printf("[GAMEPAD] Trim -> %d\n", trim);
        lastTrim = trim;
    }*/

    if (cebo1 != lastCebo1) {
        ComandoPlaya cmd = {};
        cmd.tipo       = CMD_CEBO;
        cmd.numeroCebo = 1;
        cmd.abrirCebo  = cebo1;
        EnviarComando(cmd);
        Serial.printf("[GAMEPAD] Cebo1 -> %s\n", cebo1 ? "PULSADO" : "SOLTADO");
        lastCebo1 = cebo1;
    }
}

int  GetThrottle()     { return throttle; }
int  GetTrim()         { return trim; }
int  GetRumbo()        { return rumbo; }
bool GetCebo1Pressed() { return cebo1; }