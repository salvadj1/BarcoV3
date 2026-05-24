#ifndef TB6612FNG_H
#define TB6612FNG_H

#include <Arduino.h>

// Inicialización del driver
void setupTB6612FNG();

// Loop opcional de ejemplo
void loopTB6612FNG();

// Control del motor
void movermotor(int velocidad, bool sentidoHorario);

#endif