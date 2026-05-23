// ADXL345_Module.cpp
// Acelerometro ADXL345 via I2C directo
// Calcula pitch y roll con filtro paso bajo
// MONTAJE: eje Y del ADXL apunta hacia adelante del barco

#include "ADXL345_Module.h"
#include "Utilidades.h"
#include <Wire.h>
#include <math.h>

// ---------- REGISTROS ADXL345 ----------
#define ADXL_REG_DEVID      0x00  // Device ID = 0xE5
#define ADXL_REG_POWER_CTL  0x2D
#define ADXL_REG_DATA_FORMAT 0x31
#define ADXL_REG_BW_RATE    0x2C
#define ADXL_REG_DATAX0     0x32  // 6 bytes: X0,X1,Y0,Y1,Z0,Z1

// ---------- VARIABLES EXPORTADAS ----------
float adxl_pitch = 0.0f;
float adxl_roll  = 0.0f;
bool  adxlOK     = false;

// ---------- FILTRO PASO BAJO ----------
// Alpha bajo = mas suavizado (menos sensible a vibraciones del motor)
static const float LP_ALPHA = 0.15f;
static float ax_f = 0, ay_f = 0, az_f = 1;  // aceleracion filtrada

// ---------- I2C HELPERS ----------
static void adxlWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static uint8_t adxlRead8(uint8_t reg) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345_ADDR, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
}

static bool adxlReadRaw(int16_t& x, int16_t& y, int16_t& z) {
    Wire.beginTransmission(ADXL345_ADDR);
    Wire.write(ADXL_REG_DATAX0);
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345_ADDR, (uint8_t)6);
    if (Wire.available() < 6) return false;
    x = (int16_t)(Wire.read() | (Wire.read() << 8));
    y = (int16_t)(Wire.read() | (Wire.read() << 8));
    z = (int16_t)(Wire.read() | (Wire.read() << 8));
    return true;
}

// ---------- SETUP ----------
void SetupADXL345() {
    delay(10);

    // Verificar Device ID
    uint8_t id = adxlRead8(ADXL_REG_DEVID);
    if (id != 0xE5) {
        Serial.printf("ADXL345: NO encontrado (ID=0x%02X, esperado 0xE5)\n", id);
        Serial.printf("         Prueba con direccion 0x1D si SDO esta a GND\n");
        adxlOK = false;
        return;
    }

    // BW_RATE: 100Hz output data rate (0x0A)
    adxlWrite(ADXL_REG_BW_RATE, 0x0A);

    // DATA_FORMAT: rango ±4g, full resolution
    adxlWrite(ADXL_REG_DATA_FORMAT, 0x01);

    // POWER_CTL: modo medicion continua
    adxlWrite(ADXL_REG_POWER_CTL, 0x08);
    delay(10);

    // Lectura inicial para inicializar filtro
    int16_t rx, ry, rz;
    if (adxlReadRaw(rx, ry, rz)) {
        // ADXL345 en rango ±4g full res: 7.8mg/LSB = 0.0078 g/LSB
        ax_f = rx * 0.0078f;
        ay_f = ry * 0.0078f;
        az_f = rz * 0.0078f;
    }

    adxlOK = true;
    Serial.printf("ADXL345: OK (ID=0xE5) addr=0x%02X\n", ADXL345_ADDR);
}

// ---------- LOOP ----------
void LoopADXL345() {
    if (!adxlOK) return;

    int16_t rx, ry, rz;
    if (!adxlReadRaw(rx, ry, rz)) return;

    // Convertir a g (±4g full res: 7.8mg/LSB)
    float ax = rx * 0.0078f;
    float ay = ry * 0.0078f;
    float az = rz * 0.0078f;

    // Filtro paso bajo para eliminar vibraciones del motor
    ax_f = LP_ALPHA * ax + (1.0f - LP_ALPHA) * ax_f;
    ay_f = LP_ALPHA * ay + (1.0f - LP_ALPHA) * ay_f;
    az_f = LP_ALPHA * az + (1.0f - LP_ALPHA) * az_f;

    // REMAPEO segun montaje: Y_ADXL=adelante(pitch), X_ADXL=lateral(roll)
    adxl_pitch = atan2(-ay_f, az_f);   // Y adelante: inclinacion proa/popa
    adxl_roll  = atan2( ax_f, az_f);   // X lateral:  inclinacion babor/estribor

    // Log cada 2 segundos
    static uint32_t lastLog = 0;
    if (millis() - lastLog > 2000) {
        lastLog = millis();
        Serial.printf("[ADXL345] pitch=%.1f° roll=%.1f°  ax=%.3f ay=%.3f az=%.3f\n",
                      adxl_pitch * 180.0f / M_PI,
                      adxl_roll  * 180.0f / M_PI,
                      ax_f, ay_f, az_f);
    }
}
