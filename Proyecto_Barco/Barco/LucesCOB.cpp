// LucesCOB.cpp
// LEDs COB laterales - babor GPIO 13 / estribor GPIO 14
// Todos los efectos son NO BLOQUEANTES

#include "LucesCOB.h"
#include <math.h>   // sinf(), PI (definido en Arduino.h, pero por claridad)

// ─── ESTADO EXPORTADO ────────────────────────────────
ModoCOB cobModo[2]   = { COB_APAGADO, COB_APAGADO };
uint8_t cobBrillo[2] = { 255, 255 };

// ─── ESTADO INTERNO POR CANAL ────────────────────────
static uint8_t  _ledc[2]     = { COB_LEDC_BABOR, COB_LEDC_ESTRIBOR };
static uint8_t  _pin[2]      = { COB_PIN_BABOR,  COB_PIN_ESTRIBOR  };
static uint16_t _periodo[2]  = { 500, 500 };   // ms ciclo completo
static uint32_t _tUltimo[2]  = { 0, 0 };       // millis último cambio
static bool     _estadoOn[2] = { false, false };

// ─── ESTADO INTERNO PERSECUCION (compartido entre los 2 canales) ─────
static uint8_t  _chasePos     = 0;   // 0=babor iluminado, 1=estribor iluminado
static int8_t   _chaseDir     = 1;   // 1 = va hacia estribor, -1 = vuelve hacia babor
static uint8_t  _chaseCiclos  = 0;   // ciclos ida+vuelta que quedan por hacer
static uint16_t _chasePaso    = 150; // ms entre cada paso del barrido
static uint32_t _chaseUltimo  = 0;
bool cobPersecucionActiva = false;

// ─── HELPERS ─────────────────────────────────────────
static void escribir(uint8_t i, uint8_t valor) {
    ledcWrite(_ledc[i], valor);
}

// ─── SETUP ───────────────────────────────────────────
void SetupLucesCOB() {
    for (int i = 0; i < 2; i++) {
        ledcSetup(_ledc[i], COB_LEDC_FREQ, COB_LEDC_BITS);
        ledcAttachPin(_pin[i], _ledc[i]);
        escribir(i, 0);
    }
    Serial.println("[COB] Luces OK - babor=GPIO13  estribor=GPIO14");
}

// ─── HELPER: PASO DE PERSECUCION (estado compartido babor/estribor) ──
static void actualizarPersecucion(uint32_t ahora) {
    if (ahora - _chaseUltimo < _chasePaso) return;
    _chaseUltimo = ahora;

    // Encender solo el canal que toca, apagar el otro
    escribir(0, _chasePos == 0 ? cobBrillo[0] : 0);
    escribir(1, _chasePos == 1 ? cobBrillo[1] : 0);

    if (_chaseDir == 1) {
        if (_chasePos == 0) { _chasePos = 1; }
        else                { _chaseDir = -1; }
    } else {
        if (_chasePos == 1) { _chasePos = 0; }
        else {
            _chaseDir = 1;
            if (_chaseCiclos > 0) _chaseCiclos--;
            if (_chaseCiclos == 0) {
                // Fin de la persecucion: dejar ambos lados fijos
                cobPersecucionActiva = false;
                cobFijo(COB_AMBOS, cobBrillo[0]);
            }
        }
    }
}

// ─── LOOP ────────────────────────────────────────────
void LoopLucesCOB() {
    uint32_t ahora = millis();

    // La persecucion usa estado compartido entre canales: se gestiona aparte
    if (cobModo[COB_BABOR] == COB_PERSECUCION || cobModo[COB_ESTRIBOR] == COB_PERSECUCION) {
        actualizarPersecucion(ahora);
        return;
    }

    for (int i = 0; i < 2; i++) {
        uint32_t dt = ahora - _tUltimo[i];

        switch (cobModo[i]) {

            case COB_APAGADO:
                escribir(i, 0);
                break;

            case COB_FIJO:
                escribir(i, cobBrillo[i]);
                break;

            // On/off rítmico: mitad del periodo encendido, mitad apagado
            case COB_PARPADEO:
                if (dt >= _periodo[i] / 2) {
                    _tUltimo[i]  = ahora;
                    _estadoOn[i] = !_estadoOn[i];
                    escribir(i, _estadoOn[i] ? cobBrillo[i] : 0);
                }
                break;

            // Pulso corto (100ms) cada periodo ms
            case COB_DESTELLO:
                if (!_estadoOn[i] && dt >= _periodo[i]) {
                    // Apagado -> encender (pulso corto)
                    _tUltimo[i]  = ahora;
                    _estadoOn[i] = true;
                    escribir(i, cobBrillo[i]);
                } else if (_estadoOn[i] && dt >= 100) {
                    // Encendido -> apagar tras 100ms
                    _tUltimo[i]  = ahora;
                    _estadoOn[i] = false;
                    escribir(i, 0);
                }
                break;

            // Parpadeo rápido fijo 80ms
            case COB_ALERTA:
                if (dt >= 80) {
                    _tUltimo[i]  = ahora;
                    _estadoOn[i] = !_estadoOn[i];
                    escribir(i, _estadoOn[i] ? cobBrillo[i] : 0);
                }
                break;

            // Respiracion en espejo: brillo senoidal, estribor desfasado 180° respecto a babor
            case COB_RESPIRA: {
                float fase = (float)(dt % _periodo[i]) / (float)_periodo[i];      // 0..1
                float ang  = fase * 2.0f * PI + (i == 1 ? PI : 0.0f);             // espejo
                float s    = (sinf(ang) + 1.0f) * 0.5f;                          // 0..1
                escribir(i, (uint8_t)(s * cobBrillo[i]));
                break;
            }

            // Sirena: cuadrada alterna, estribor desfasado medio periodo respecto a babor
            case COB_SIRENA: {
                uint32_t offset = (i == 1) ? _periodo[i] / 2 : 0;
                uint32_t fase   = (dt + offset) % _periodo[i];
                escribir(i, (fase < _periodo[i] / 2) ? cobBrillo[i] : 0);
                break;
            }
        }
    }
}

// ─── API ─────────────────────────────────────────────
void cobSet(uint8_t lado, ModoCOB modo, uint16_t periodoMs, uint8_t brillo) {
    auto aplicar = [&](uint8_t i) {
        cobModo[i]   = modo;
        cobBrillo[i] = brillo;
        _periodo[i]  = periodoMs;
        _tUltimo[i]  = millis();
        _estadoOn[i] = false;
    };
    if (lado == COB_BABOR    || lado == COB_AMBOS) aplicar(0);
    if (lado == COB_ESTRIBOR || lado == COB_AMBOS) aplicar(1);
}

void cobApagar(uint8_t lado)                                           { cobSet(lado, COB_APAGADO);                           }
void cobFijo(uint8_t lado, uint8_t brillo)                             { cobSet(lado, COB_FIJO,     0,         brillo);        }
void cobParpadeo(uint8_t lado, uint16_t periodoMs, uint8_t brillo)     { cobSet(lado, COB_PARPADEO, periodoMs, brillo);        }
void cobDestello(uint8_t lado, uint16_t periodoMs, uint8_t brillo)     { cobSet(lado, COB_DESTELLO, periodoMs, brillo);        }
void cobAlerta(uint8_t lado)                                           { cobSet(lado, COB_ALERTA,   0,         255);           }

void cobRespira(uint16_t periodoMs, uint8_t brillo) {
    cobSet(COB_AMBOS, COB_RESPIRA, periodoMs, brillo);
}

void cobSirena(uint16_t periodoMs, uint8_t brillo) {
    cobSet(COB_AMBOS, COB_SIRENA, periodoMs, brillo);
}

void cobPersecucion(uint8_t ciclos, uint16_t pasoMs, uint8_t brillo) {
    cobModo[0]   = COB_PERSECUCION;
    cobModo[1]   = COB_PERSECUCION;
    cobBrillo[0] = brillo;
    cobBrillo[1] = brillo;
    _chasePos    = 0;
    _chaseDir    = 1;
    _chaseCiclos = ciclos;
    _chasePaso   = pasoMs;
    _chaseUltimo = millis();
    cobPersecucionActiva = true;
}