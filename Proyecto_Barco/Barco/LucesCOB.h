// LucesCOB.h
// LEDs COB laterales - babor (GPIO 13) y estribor (GPIO 14)
// Requiere MOSFET/transistor entre GPIO y LED
//
// MODOS:
//   COB_APAGADO      - apagado
//   COB_FIJO         - encendido continuo
//   COB_PARPADEO     - on/off rítmico
//   COB_DESTELLO     - pulso corto periódico (tipo faro náutico)
//   COB_ALERTA       - parpadeo muy rápido

#ifndef _LUCES_COB_h
#define _LUCES_COB_h

#include <Arduino.h>

// ─── PINES ───────────────────────────────────────────
#define COB_PIN_BABOR     13
#define COB_PIN_ESTRIBOR  14

// ─── CANALES LEDC ────────────────────────────────────
// TB6612FNG usa canal 4 -> usamos 5 y 6
#define COB_LEDC_BABOR    5
#define COB_LEDC_ESTRIBOR 6
#define COB_LEDC_FREQ     1000
#define COB_LEDC_BITS     8     // 0-255

// ─── LADO ────────────────────────────────────────────
#define COB_BABOR    0
#define COB_ESTRIBOR 1
#define COB_AMBOS    2

// ─── MODOS ───────────────────────────────────────────
enum ModoCOB : uint8_t {
    COB_APAGADO  = 0,
    COB_FIJO     = 1,
    COB_PARPADEO = 2,   // on/off cada periodoMs ms
    COB_DESTELLO = 3,   // pulso corto cada periodoMs ms
    COB_ALERTA   = 4    // parpadeo rápido fijo 80ms
};

// ─── ESTADO EXPORTADO (para telemetría/web) ──────────
extern ModoCOB cobModo[2];     // cobModo[0]=babor  cobModo[1]=estribor
extern uint8_t cobBrillo[2];   // 0-255

// ─── FUNCIONES ───────────────────────────────────────
void SetupLucesCOB();
void LoopLucesCOB();

// Fijar modo en uno o ambos lados
// lado: COB_BABOR(0), COB_ESTRIBOR(1), COB_AMBOS(2)
// periodoMs: duración del ciclo en parpadeo/destello (ignorado en fijo/alerta)
// brillo: 0-255
void cobSet(uint8_t lado, ModoCOB modo, uint16_t periodoMs = 500, uint8_t brillo = 255);

// Atajos rápidos
void cobApagar(uint8_t lado = COB_AMBOS);
void cobFijo(uint8_t lado = COB_AMBOS, uint8_t brillo = 255);
void cobParpadeo(uint8_t lado = COB_AMBOS, uint16_t periodoMs = 500, uint8_t brillo = 255);
void cobDestello(uint8_t lado = COB_AMBOS, uint16_t periodoMs = 1500, uint8_t brillo = 255);
void cobAlerta(uint8_t lado = COB_AMBOS);

#endif
