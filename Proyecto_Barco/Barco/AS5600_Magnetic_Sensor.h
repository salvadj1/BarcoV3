// AS5600_Magnetic_Sensor.h
// Sensor magnetico AS5600 via I2C - sustituye a PotenciometroB10K.h
// I2C: SDA=21, SCL=22 - Wire.begin() en Barco.ino
// Resolucion: 12 bits (4096 pasos = 360 grados) -> 0.088 grados/paso
//
// CALIBRACION: centrar el timon fisicamente y pulsar CTR en la web.
// El angulo de centro se guarda en SPIFFS y se recupera en cada arranque.
// Si existe calibracion en SPIFFS, timonReferenciada arranca en true
// gracias al flag as5600CentroValido que SetupTimon() consulta.

#ifndef AS5600_MAGNETIC_SENSOR_H
#define AS5600_MAGNETIC_SENSOR_H

#include <Arduino.h>

// ─── I2C ─────────────────────────────────────────────
#define AS5600_ADDR      0x36

// ─── REGISTROS AS5600 ────────────────────────────────
#define AS5600_REG_ANGLE_H 0x0E   // angulo filtrado high byte
#define AS5600_REG_ANGLE_L 0x0F   // angulo filtrado low byte

// ─── AMPLITUD DE GIRO ────────────────────────────────
// Grados desde el centro hasta cada tope fisico
// Se asume simetria: izquierda = centro - AMPLITUD, derecha = centro + AMPLITUD
#define AMPLITUD_GIRO_DEG  90.0f

// ─── SPIFFS ──────────────────────────────────────────
#define AS5600_CALIB_FILE  "/as5600_centro.dat"

// ─── ESTADO EXPORTADO ────────────────────────────────
extern float as5600CentroGrados;     // angulo magnetico del centro fisico (0.0-360.0)
extern float as5600IzquierdaGrados;  // tope izquierdo en grados magneticos
extern float as5600DerechaGrados;    // tope derecho en grados magneticos
extern bool  as5600CentroValido;     // true si hay centro guardado (SPIFFS o CTR pulsado)
extern bool  as5600SensorOK;         // true si el sensor respondio en setup

// ─── API PUBLICA (misma que PotenciometroB10K) ────────
void setupHW040Encoder();    // inicializa I2C y carga centro de SPIFFS
void loopHW040Encoder();     // log periodico

int  encoderGetDegrees();    // devuelve grados 90-270 (centro dinamico)
void encoderSetCentro();     // memoriza angulo actual como centro y guarda en SPIFFS
void encoderBorrarCentro();  // borra centro de RAM y SPIFFS (segundo CTR)
int32_t encoderGetSteps();   // devuelve 0 (compatibilidad, no usado)
void encoderReset();         // compatibilidad
bool encoderButtonPressed(); // devuelve false (sin boton fisico)

#endif
