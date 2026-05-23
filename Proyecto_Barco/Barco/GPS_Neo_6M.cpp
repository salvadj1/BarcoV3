// GPS_Neo_6M.cpp
// Lectura GPS, filtro de posicion y configuracion para maxima precision

#include "GPS_Neo_6M.h"
#include "Viajes_Logica.h"
#include "Servos.h"

#include <HardwareSerial.h>
#include <SPIFFS.h>

// NEO-6M: TX->GPIO16, RX->GPIO17
HardwareSerial gpsSerial(2);
TinyGPSPlus    gps;

// ---------- FILTRO MEDIA MOVIL POSICION (20 muestras) ----------
double filterLat[FILTER_SIZE] = { 0 };
double filterLng[FILTER_SIZE] = { 0 };
int    filterIdx  = 0;
bool   filterFull = false;
double smoothLat  = 0, smoothLng = 0;

static void updateFilter(double lat, double lng) {
    filterLat[filterIdx] = lat;
    filterLng[filterIdx] = lng;
    filterIdx = (filterIdx + 1) % FILTER_SIZE;
    if (filterIdx == 0) filterFull = true;
    int n = filterFull ? FILTER_SIZE : filterIdx;
    double sLat = 0, sLng = 0;
    for (int i = 0; i < n; i++) { sLat += filterLat[i]; sLng += filterLng[i]; }
    smoothLat = sLat / n;
    smoothLng = sLng / n;
}

// ---------- CONFIGURACION UBX ----------
static void sendUBX(const uint8_t* msg, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) gpsSerial.write(msg[i]);
    delay(10);
}

// 10 Hz
static void setGPS_10Hz() {
    const uint8_t rate[] = {
        0xB5,0x62, 0x06,0x08, 0x06,0x00,
        0x64,0x00,          // 100ms -> 10Hz
        0x01,0x00, 0x01,0x00,
        0x7A,0x12
    };
    sendUBX(rate, sizeof(rate));
    Serial.println("GPS: 10 Hz");
}

// Modo Automotive (mejor filtrado de movimiento en vehiculo lento)
static void setGPS_ModeAutomotive() {
    const uint8_t mode[] = {
        0xB5,0x62, 0x06,0x24, 0x24,0x00,
        0xFF,0xFF,            // mask: aplicar todos
        0x04,                 // dynModel: 4=Automotive
        0x03,                 // fixMode: 3=auto 2D/3D
        0x00,0x00,0x00,0x00,
        0x10,0x27,0x00,0x00,
        0x05,0x00,0xFA,0x00,
        0xFA,0x00,0x64,0x00,
        0x2C,0x01,0x00,0x3C,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,
        0x4F,0x82
    };
    sendUBX(mode, sizeof(mode));
    Serial.println("GPS: modo Automotive");
}

// Aumentar baudrate a 115200 para evitar overflow al leer 10Hz
static void setGPS_Baud115200() {
    const uint8_t baud[] = {
        0xB5,0x62, 0x06,0x00, 0x14,0x00,
        0x01,0x00,0x00,0x00,
        0xD0,0x08,0x00,0x00,
        0x00,0xC2,0x01,0x00,  // 115200
        0x07,0x00,0x03,0x00,
        0x00,0x00,0x00,0x00,
        0xC0,0x7E
    };
    sendUBX(baud, sizeof(baud));
    delay(100);
    gpsSerial.begin(115200, SERIAL_8N1, 16, 17);
    Serial.println("GPS: 115200 baud");
}

// ---------- SETUP ----------
void SetupGPS() {
    // Arrancar a 9600 para enviar comandos de configuracion
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
    delay(500);

    setGPS_Baud115200();    // subir baud primero
    setGPS_10Hz();          // luego 10Hz
    setGPS_ModeAutomotive();// y modo vehiculo

    Serial.println("GPS listo");
}

// ---------- LOOP ----------
void LoopGPS() {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());

    if (gps.location.isValid()) {
        updateFilter(gps.location.lat(), gps.location.lng());
        LoopViajesLogica();
    }


}
