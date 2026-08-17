#include "Wifi_Config.h"
#include "ESPNow_Playa.h"
#include "Servidor_Web.h"
#include "Gamepad.h"

void setup() {
    delay(1000);
    Serial.begin(115200);
     delay(1000);
    SetupWifi();         // WIFI_AP_STA - debe ir ANTES de ESP-NOW
     delay(1000);
    SetupESPNowPlaya();
    SetupServidorWeb();
    SetupGamepad();
    Serial.println("PLAYA listo");
}

unsigned long ultimoUpdateGamepad = 0;
const unsigned long intervaloGamepad = 100; // 100 ms
void loop() {
    LoopServidorWeb();
    //UpdateGamepad();

        if (millis() - ultimoUpdateGamepad >= intervaloGamepad) {
        ultimoUpdateGamepad = millis();
        UpdateGamepad();
    }
}
