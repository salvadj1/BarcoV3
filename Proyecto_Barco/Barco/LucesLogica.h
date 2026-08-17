// LucesLogica.h
// Decide que patron de LucesCOB mostrar segun el estado del barco
// (navegacion, bateria, señal, cebos, manual). No toca hardware directamente,
// solo llama a la API de LucesCOB.h.

#ifndef _LUCES_LOGICA_h
#define _LUCES_LOGICA_h

#include <Arduino.h>

// Llamar periodicamente (p.ej. junto a LoopLucesCOB, cada COB_INTERVAL ms)
void LoopLucesLogica();

#endif
