#include "ESPNow_Playa.h"

#include <WiFi.h>
#include <esp_now.h>

// MAC del ESP32 del Barco - CAMBIAR POR LA MAC REAL
uint8_t macBarco[6] = { 0x68, 0x25, 0xDD, 0xEF, 0x20, 0x68 };

// ---------- ESTADO GLOBAL RECIBIDO ----------
TelemetriaBarco telemetria = {};
bool            telemetriaRecibida = false;
unsigned long   ultimaTelemetriaMs = 0;

// ---------- CALLBACK: TELEMETRIA RECIBIDA DESDE BARCO ----------
void onTelemetriaRecibida(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(TelemetriaBarco)) return;
    memcpy(&telemetria, data, sizeof(TelemetriaBarco));
    telemetriaRecibida = true;
    ultimaTelemetriaMs = millis();
}

// ---------- SETUP ----------
void SetupESPNowPlaya() {
    // ESP-NOW necesita WiFi en modo STA para funcionar junto con SoftAP
    // WiFi.mode(WIFI_AP_STA) se llama en Wifi_Config.cpp
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init PLAYA error");
        return;
    }

    // Registrar callback de recepcion
    esp_now_register_recv_cb(onTelemetriaRecibida);

    // Registrar peer (ESP32 Barco)
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, macBarco, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    Serial.println("ESP-NOW PLAYA OK");
    Serial.print("MAC Playa: "); Serial.println(WiFi.macAddress());
}

// ---------- ENVIAR COMANDO AL BARCO ----------
void EnviarComando(ComandoPlaya& cmd) {
    esp_now_send(macBarco, (uint8_t*)&cmd, sizeof(ComandoPlaya));
}
