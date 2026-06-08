#include "Wifi_Config.h"
#include "ESPNow_Playa.h"
#include "Servidor_Web.h"

void setup() {
    delay(2000);
    Serial.begin(115200);
    SetupWifi();         // WIFI_AP_STA - debe ir ANTES de ESP-NOW
    SetupESPNowPlaya();
    SetupServidorWeb();
    Serial.println("PLAYA listo");
}

void loop() {
    LoopServidorWeb();
}
