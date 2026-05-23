// ADXL345_Module.h
// Acelerometro para compensacion de tilt del GY273
// I2C: SDA=21, SCL=22 - Wire.begin() en Barco.ino
// Direccion: 0x53 (SDO=3.3V) o 0x1D (SDO=GND)
//
// MONTAJE: eje Y del ADXL apunta hacia adelante del barco
//          (misma direccion que eje X del GY273)

#ifndef _ADXL345_MODULE_h
#define _ADXL345_MODULE_h

#include <Arduino.h>

// Direccion I2C - cambiar a 0x1D si SDO esta a GND
#define ADXL345_ADDR  0x53

// Ejes remapeados segun montaje fisico:
// Y_ADXL -> adelante del barco (pitch)
// X_ADXL -> babor/estribor     (roll)
// Z_ADXL -> vertical

// Angulos exportados en radianes - usados por GY273 para tilt compensation
extern float adxl_pitch;   // inclinacion proa/popa (rad)
extern float adxl_roll;    // inclinacion lateral   (rad)
extern bool  adxlOK;

void SetupADXL345();
void LoopADXL345();

#endif
