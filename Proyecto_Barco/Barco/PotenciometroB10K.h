// PotenciometroB10K.h
// Potenciometro B10K para lectura de posicion del timon
// ADC: GPIO 34 (input-only, ADC1_CH6)
// Rango fisico: 0-270 grados, centro=180 grados
// Sustituye a HW040Encoder.h - mantiene la misma API publica

#ifndef POTENCIOMETROB10K_H
#define POTENCIOMETROB10K_H

#include <Arduino.h>

// ─── PIN ─────────────────────────────────────────────
#define POT_PIN 34

// ─── CALIBRACION DE TOPES FISICOS ────────────────────
// Mover el timon a cada tope, leer "adc=XXXX" en el log serial y ajustar:
#define POT_ADC_IZQUIERDA  1200   // ADC con timon en tope izquierdo  (90 grados)
#define POT_ADC_DERECHA    2900   // ADC con timon en tope derecho     (270 grados)

// ─── RANGO ANGULAR ───────────────────────────────────
#define POT_GRADOS_MAX 270   // recorrido total del potenciometro
#define POT_CENTRO     180   // grados en posicion central del timon

// ─── API PUBLICA (misma que HW040Encoder) ────────────
void setupHW040Encoder();    // inicializa ADC
void loopHW040Encoder();     // log periodico

int  encoderGetDegrees();    // devuelve grados 0-270 (con filtro)
int32_t encoderGetSteps();   // devuelve 0 (compatibilidad, no usado)
void encoderReset();         // marca timonReferenciada=true
bool encoderButtonPressed(); // devuelve false (sin boton fisico)

#endif
