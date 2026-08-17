// Gamepad.h
// Modulo reutilizable para leer un gamepad casero (potenciometros + botones)
// Pines: Throttle=GPIO34(ADC), Trim=GPIO35(ADC), Izq=GPIO25, Der=GPIO26, Cebo1=GPIO27

#ifndef _GAMEPAD_h
#define _GAMEPAD_h

#include <Arduino.h>

// Configura pinModes de los botones (llamar en setup())
void SetupGamepad();

// Lee todos los inputs (llamar en loop())
void UpdateGamepad();

// ---------- GETTERS ----------
int  GetThrottle();     // 0-100
int  GetTrim();         // -150..150 (decimas de grado, igual que ComandoPlaya)
int  GetRumbo();        // -1=izquierda, 0=centro, +1=derecha
bool GetCebo1Pressed(); // true si boton cebo1 esta pulsado

#endif
