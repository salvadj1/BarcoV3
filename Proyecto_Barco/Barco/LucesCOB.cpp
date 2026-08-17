// LucesCOB.cpp
// LEDs COB laterales - babor GPIO 13 / estribor GPIO 14
// Todos los efectos son NO BLOQUEANTES

#include "LucesCOB.h"

// ─── ESTADO EXPORTADO ────────────────────────────────
ModoCOB cobModo[2]   = { COB_APAGADO, COB_APAGADO };
uint8_t cobBrillo[2] = { 255, 255 };

// ─── ESTADO INTERNO POR CANAL ────────────────────────
static uint8_t  _ledc[2]     = { COB_LEDC_BABOR, COB_LEDC_ESTRIBOR };
static uint8_t  _pin[2]      = { COB_PIN_BABOR,  COB_PIN_ESTRIBOR  };
static uint16_t _periodo[2]  = { 500, 500 };   // ms ciclo completo
static uint32_t _tUltimo[2]  = { 0, 0 };       // millis último cambio
static bool     _estadoOn[2] = { false, false };

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

// ─── LOOP ────────────────────────────────────────────
void LoopLucesCOB() {
    uint32_t ahora = millis();

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
