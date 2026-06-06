// PotenciometroB10K.h
// Potenciometro B10K para lectura de posicion del timon
// ADC: GPIO 34 (input-only, ADC1_CH6)

#ifndef POTENCIOMETROB10K_H
#define POTENCIOMETROB10K_H

#include <Arduino.h>

// ─── PIN ─────────────────────────────────────────────
#define POT_PIN 34

// ─── AMPLITUD DE GIRO ────────────────────────────────
// Unidades ADC desde el centro hasta cada tope fisico
// Se asume simetria: izquierda = centro - AMPLITUD, derecha = centro + AMPLITUD
#define AMPLITUD_GIRO 800

// ─── CENTRO DINAMICO Y TOPES CALCULADOS ──────────────
// Se calculan al pulsar CTR con el timon centrado
extern int potAdcCentro;
extern int potAdcIzquierda;
extern int potAdcDerecha;

// ─── API PUBLICA (misma que HW040Encoder) ────────────
void setupHW040Encoder();    // inicializa ADC
void loopHW040Encoder();     // log periodico

int  encoderGetDegrees();    // devuelve grados 90-270 (con filtro, centro dinamico)
void encoderSetCentro();     // memoriza ADC actual como centro y calcula topes
int32_t encoderGetSteps();   // devuelve 0 (compatibilidad, no usado)
void encoderReset();         // compatibilidad
bool encoderButtonPressed(); // devuelve false (sin boton fisico)

#endif
