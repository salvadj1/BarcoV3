// ESPNow_Playa.h

#ifndef _ESPNOW_PLAYA_h
#define _ESPNOW_PLAYA_h

#include <Arduino.h>
#include "Shared_Types.h"

// MAC del ESP32 del Barco - CAMBIAR POR LA MAC REAL en ESPNow_Playa.cpp
// Para obtenerla: Serial.println(WiFi.macAddress()) en el ESP32 Barco
extern uint8_t macBarco[6];

// Telemetria mas reciente recibida del barco
extern TelemetriaBarco telemetria;
extern bool telemetriaRecibida;
extern unsigned long ultimaTelemetriaMs;

void SetupESPNowPlaya();
void EnviarComando(ComandoPlaya& cmd);

#endif
