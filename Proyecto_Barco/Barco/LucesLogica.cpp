// LucesLogica.cpp
// Traduce el estado del barco (navegacion / bateria / señal / cebos / manual)
// a un patron de LucesCOB. Aplica el patron UNA sola vez cuando cambia
// (para no reiniciar animaciones como la respiracion o la sirena en cada tick),
// salvo el "boost" de giro en modo autonomo, que se recalcula cada llamada.

#include "LucesLogica.h"
#include "LucesCOB.h"
#include "Viajes_Logica.h"    // navState, targetBearing
#include "TimonSistema.h"     // modoManual
#include "GY273_Module.h"     // currentCourse, courseValid
#include "ESCMotor.h"         // (no imprescindible, dejado por si se usa motorRunning)
#include "VoltajeSensor.h"    // voltajeAlarma
#include "ESPNow_Barco.h"     // senalPerdida
#include "Servos.h"           // cebo1PulsoFin, cebo2PulsoFin

// ─── PARAMETROS AJUSTABLES ────────────────────────────
static const uint8_t  BRILLO_BASE_AUTONOMO = 160;   // brillo "fijo" normal en viaje
static const uint8_t  BRILLO_BOOST_GIRO    = 255;   // brillo del lado que corrige fuerte
static const double   UMBRAL_GIRO_FUERTE   = 25.0;  // grados de error de rumbo para "corrige fuerte"
static const uint8_t  BRILLO_MANUAL        = 200;   // brillo fijo en modo manual

// ─── MODOS LOGICOS (uno por cada "situacion" del barco) ───────────────
enum ModoLuces {
    LUCES_NONE = -1,   // sin aplicar todavia / recien invalidado
    LUCES_SIN_SENAL,
    LUCES_BATERIA_BAJA,
    LUCES_MANUAL,
    LUCES_ARRIVED,
    LUCES_AUTONOMO,
    LUCES_IDLE
};

static ModoLuces _modoActual = LUCES_NONE;

// ─── DECIDIR MODO SEGUN ESTADO (orden = prioridad) ────────────────────
static ModoLuces calcularModoDeseado() {
    if (senalPerdida)   return LUCES_SIN_SENAL;     // seguridad: verse a distancia
    if (voltajeAlarma)  return LUCES_BATERIA_BAJA;  // seguridad: bateria critica
    if (modoManual)      return LUCES_MANUAL;        // control manual anula lo automatico
    if (navState == ARRIVED) return LUCES_ARRIVED;
    if (navState == GOING_CEBO1 || navState == GOING_CEBO2 || navState == RETURNING)
        return LUCES_AUTONOMO;
    return LUCES_IDLE;
}

// ─── BOOST DE GIRO (solo en autonomo, se recalcula cada tick) ─────────
// Escribe directamente cobBrillo[] (sin pasar por cobSet) para no reiniciar
// el temporizador del modo FIJO, que no lo necesita.
static void aplicarBoostGiro() {
    if (!courseValid) {
        cobBrillo[COB_BABOR]    = BRILLO_BASE_AUTONOMO;
        cobBrillo[COB_ESTRIBOR] = BRILLO_BASE_AUTONOMO;
        return;
    }

    double errorRumbo = targetBearing - currentCourse;
    if (errorRumbo > 180.0)  errorRumbo -= 360.0;
    if (errorRumbo < -180.0) errorRumbo += 360.0;

    if (errorRumbo > UMBRAL_GIRO_FUERTE) {
        // Corrigiendo hacia estribor (derecha)
        cobBrillo[COB_ESTRIBOR] = BRILLO_BOOST_GIRO;
        cobBrillo[COB_BABOR]    = BRILLO_BASE_AUTONOMO;
    } else if (errorRumbo < -UMBRAL_GIRO_FUERTE) {
        // Corrigiendo hacia babor (izquierda)
        cobBrillo[COB_BABOR]    = BRILLO_BOOST_GIRO;
        cobBrillo[COB_ESTRIBOR] = BRILLO_BASE_AUTONOMO;
    } else {
        cobBrillo[COB_BABOR]    = BRILLO_BASE_AUTONOMO;
        cobBrillo[COB_ESTRIBOR] = BRILLO_BASE_AUTONOMO;
    }
}

// ─── APLICAR UN MODO (solo se llama cuando el modo CAMBIA) ────────────
static void aplicarModo(ModoLuces modo) {
    switch (modo) {
        case LUCES_SIN_SENAL:
            cobDestello(COB_AMBOS, 2000, 255);           // faro lento sincronizado
            break;
        case LUCES_BATERIA_BAJA:
            cobSirena(300, 255);                          // alterna tipo sirena
            break;
        case LUCES_MANUAL:
            cobFijo(COB_AMBOS, BRILLO_MANUAL);             // fijo, anula lo automatico
            break;
        case LUCES_ARRIVED:
            cobPersecucion(4, 150, 255);                   // 4 barridos babor<->estribor
            break;
        case LUCES_AUTONOMO:
            cobFijo(COB_AMBOS, BRILLO_BASE_AUTONOMO);       // fijo; el boost se ajusta aparte
            break;
        case LUCES_IDLE:
        default:
            cobRespira(3000, 255);                         // respirar en espejo
            break;
    }
}

// ─── LOOP PRINCIPAL ────────────────────────────────────
void LoopLucesLogica() {
    unsigned long ahora = millis();

    // Si hay un destello de cebo en marcha en cualquiera de los dos lados,
    // no tocar las luces: dejar que termine sus 2-3 pulsos sin interferencias.
    // Al invalidar _modoActual, en cuanto acabe el pulso se reaplica el modo
    // de fondo entero (asi el lado que parpadeo vuelve a quedar sincronizado).
    if (ahora < cebo1PulsoFin || ahora < cebo2PulsoFin) {
        _modoActual = LUCES_NONE;
        return;
    }

    ModoLuces deseado = calcularModoDeseado();

    if (deseado == LUCES_AUTONOMO) {
        if (_modoActual != LUCES_AUTONOMO) {
            aplicarModo(LUCES_AUTONOMO);
            _modoActual = LUCES_AUTONOMO;
        }
        aplicarBoostGiro();   // se recalcula siempre, sin reiniciar el modo FIJO
        return;
    }

    if (deseado == _modoActual) return;   // ya aplicado, no reiniciar la animacion

    aplicarModo(deseado);
    _modoActual = deseado;
}
