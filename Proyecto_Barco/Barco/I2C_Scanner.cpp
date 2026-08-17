// I2C_Scanner.cpp
#include "I2C_Scanner.h"
#include <Wire.h>

// ---------- TABLA DE DIRECCIONES CONOCIDAS DEL PROYECTO ----------
// Ampliar libremente: es solo informativa, no afecta al escaneo.
struct I2CKnownDevice {
    uint8_t     addr;
    const char* nombre;
};

static const I2CKnownDevice DISPOSITIVOS_CONOCIDOS[] = {
    { 0x53, "ADXL345 (SDO=3.3V)" },
    { 0x1D, "ADXL345 (SDO=GND)" },
    { 0x68, "MPU6050 (AD0=GND) / DS1307" },
    { 0x69, "MPU6050 (AD0=3.3V)" },
    { 0x1E, "HMC5883L / GY-273" },
    { 0x0D, "QMC5883L (clon GY-273)" },
    { 0x36, "AS5600" },
};
static const uint8_t N_CONOCIDOS = sizeof(DISPOSITIVOS_CONOCIDOS) / sizeof(DISPOSITIVOS_CONOCIDOS[0]);

static const char* nombreConocido(uint8_t addr) {
    for (uint8_t i = 0; i < N_CONOCIDOS; i++) {
        if (DISPOSITIVOS_CONOCIDOS[i].addr == addr) return DISPOSITIVOS_CONOCIDOS[i].nombre;
    }
    return nullptr;
}

uint8_t EscanearI2C(bool rapido) {
    uint32_t clockOriginal = 100000; // valor por defecto tipico de Wire
    if (rapido) {
        clockOriginal = Wire.getClock();
        Wire.setClock(400000); // 400kHz: reduce el tiempo por direccion sin perder fiabilidad
    }

    Serial.println("\n[I2C] Escaneando bus...");
    uint32_t t0 = millis();
    uint8_t encontrados = 0;

    // Solo direcciones validas de 7 bits: 0x08-0x77
    // (0x00-0x07 y 0x78-0x7F estan reservadas por la especificacion I2C
    //  y casi ningun periferico real las usa; saltarlas ahorra ~15 intentos)
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            encontrados++;
            const char* nombre = nombreConocido(addr);
            Serial.printf("[I2C]  -> 0x%02X  %s\n", addr, nombre ? nombre : "(desconocido)");
        }
        // error==4 -> error de bus real; el resto (2,3) son "no ACK", esperado y se ignora
    }

    uint32_t dt = millis() - t0;
    Serial.printf("[I2C] %d dispositivo(s) encontrado(s) en %lu ms\n\n", encontrados, dt);

    if (rapido) Wire.setClock(clockOriginal); // restaura el reloj original del proyecto
    return encontrados;
}
