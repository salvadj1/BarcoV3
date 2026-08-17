#include "esp32-hal.h"
#include "Wifi_Config.h"

#include <WiFi.h>

// ---------- CONFIG AP ----------
const char* ssid = "Barco_Solo_Para_Expertos";

void SetupWifi() {
    // WIFI_AP_STA: SoftAP para el movil + STA necesario para ESP-NOW
    WiFi.mode(WIFI_AP_STA);
    delay(1000);
   WiFi.softAP(ssid, "12345678");
    delay(1000);
    Serial.println("AP: " + WiFi.softAPIP().toString());
}
