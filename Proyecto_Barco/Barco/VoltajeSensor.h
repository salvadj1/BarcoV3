// VoltajeSensor.h
// Sensor de voltaje DC 0-25V (divisor x5 interno)
// GPIO 34 - ADC1 canal 6 - compatible con WiFi/ESP-NOW
// Conexión: "+" a 3.3V, "-" a GND, "S" a GPIO 34
// Con 3.3V el rango seguro es 0 - 16.5V

#ifndef _VOLTAJE_SENSOR_h
#define _VOLTAJE_SENSOR_h

#include <Arduino.h>

// ─── PIN ─────────────────────────────────────────────
#define VOLTAJE_PIN       34

// ─── CONFIGURACIÓN BATERÍA ───────────────────────────
// Ajusta según tu batería:
// LiPo 2S: min=6.4  max=8.4
// LiPo 3S: min=9.6  max=12.6
// Pb 12V:  min=10.5 max=12.6
#define VOLTAJE_BAT_MIN   9.9f
#define VOLTAJE_BAT_MAX   12.6f
#define VOLTAJE_ALARMA_V  10.1f   // umbral batería baja (0 = desactivado)

// ─── ESTADO EXPORTADO ────────────────────────────────
extern float voltajeActual;   // voltios leídos
extern float voltajePct;      // porcentaje batería 0-100
extern bool  voltajeAlarma;   // true si voltaje < VOLTAJE_ALARMA_V

// ─── FUNCIONES ───────────────────────────────────────
void SetupVoltajeSensor();
void LoopVoltajeSensor();

#endif
