// I2C_Scanner.h
// Escaner I2C generico y reutilizable (cualquier proyecto ESP32/Arduino).
// Recorre solo el rango de direcciones validas (0x08-0x77, las reservadas
// 0x00-0x07 y 0x78-0x7F quedan excluidas por spec I2C) y, si el bus lo
// permite, sube el reloj a 400kHz durante el escaneo para acortar el
// timeout de cada intento fallido. Requiere Wire.begin() ya llamado antes.

#ifndef _I2C_SCANNER_h
#define _I2C_SCANNER_h

#include <Arduino.h>

// Escanea el bus I2C y muestra por Serial cada direccion que responde,
// junto con el nombre del sensor si coincide con la tabla de conocidos.
// rapido: si true, sube el reloj I2C a 400kHz solo durante el escaneo
//         y lo restaura al finalizar (recomendado, mas veloz y no
//         invasivo para el resto del proyecto).
// Devuelve el numero de dispositivos encontrados.
uint8_t EscanearI2C(bool rapido = true);

#endif
