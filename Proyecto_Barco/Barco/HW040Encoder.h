#ifndef HW040ENCODER_H
#define HW040ENCODER_H

#include <Arduino.h>

extern int Pasos_por_revolucion;

// ─── API PÚBLICA SIMPLE ─────────────────────────────
void setupHW040Encoder();
void loopHW040Encoder();

int encoderGetDegrees();
int32_t encoderGetSteps();
void encoderReset();
bool encoderButtonPressed();

#endif