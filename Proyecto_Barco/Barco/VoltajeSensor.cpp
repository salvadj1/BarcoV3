// VoltajeSensor.cpp
// Sensor de voltaje DC 0-25V - GPIO 34

#include "VoltajeSensor.h"
#include "Utilidades.h"

// ─── VARIABLES EXPORTADAS ────────────────────────────
float voltajeActual = 0.0f;
float voltajePct    = 0.0f;
bool  voltajeAlarma = false;

// ─── CONSTANTES INTERNAS ─────────────────────────────
// Factor divisor del módulo (R1=30kΩ, R2=7.5kΩ -> x5)
static const float FACTOR   = 5.53f;
static const float VREF     = 3.3f;
static const float ADC_MAX  = 4095.0f;
static const int   MUESTRAS = 20;

// ─── LECTURA CON PROMEDIO ────────────────────────────
static float leerVoltaje() {
    long suma = 0;
    for (int i = 0; i < MUESTRAS; i++) {
        suma += analogRead(VOLTAJE_PIN);
        delayMicroseconds(200);
    }
    float vPin = (suma / (float)MUESTRAS / ADC_MAX) * VREF;
    return vPin * FACTOR;
}

// ─── SETUP ───────────────────────────────────────────
void SetupVoltajeSensor() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    voltajeActual = leerVoltaje();
    Serial.printf("[VOLTAJE] OK - %.2fV  rango bateria %.1f-%.1fV  alarma %.1fV\n",
                  voltajeActual, VOLTAJE_BAT_MIN, VOLTAJE_BAT_MAX, VOLTAJE_ALARMA_V);
}

// ─── LOOP ────────────────────────────────────────────
void LoopVoltajeSensor() {
    voltajeActual = leerVoltaje();

    // Porcentaje de batería
    float pct = (voltajeActual - VOLTAJE_BAT_MIN) / (VOLTAJE_BAT_MAX - VOLTAJE_BAT_MIN) * 100.0f;
    voltajePct = constrain(pct, 0.0f, 100.0f);

    // Alarma batería baja
    voltajeAlarma = (VOLTAJE_ALARMA_V > 0.0f) && (voltajeActual < VOLTAJE_ALARMA_V);

    // Log cada 5 segundos
    static uint32_t lastLog = 0;
    if (millis() - lastLog > 5000) {
        lastLog = millis();
        Serial.printf("[VOLTAJE] %.2fV  %.0f%%  %s\n",
                      voltajeActual, voltajePct,
                      voltajeAlarma ? "!BATERIA BAJA!" : "OK");
    }
}
