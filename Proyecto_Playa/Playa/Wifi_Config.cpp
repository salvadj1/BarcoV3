#include "Wifi_Config.h"

#include <WiFi.h>

// ---------- CONFIG AP ----------
const char* ssid = "Barco_Solo_Para_Expertos";

void SetupWifi() {
    // WIFI_AP_STA: SoftAP para el movil + STA necesario para ESP-NOW
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid);
    Serial.println("AP: " + WiFi.softAPIP().toString());
}
