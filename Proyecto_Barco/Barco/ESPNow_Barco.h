// ESPNow_Barco.h

#ifndef _ESPNOW_BARCO_h
#define _ESPNOW_BARCO_h

#include <Arduino.h>
#include "Shared_Types.h"

// MAC del ESP32 de la Playa - CAMBIAR POR LA MAC REAL en ESPNow_Barco.cpp
// Para obtenerla: Serial.println(WiFi.macAddress()) en el ESP32 Playa
extern uint8_t macPlaya[6];

void SetupESPNowBarco();
void EnviarTelemetria();

#endif
