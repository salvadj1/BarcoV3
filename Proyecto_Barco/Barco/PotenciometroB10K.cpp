// PotenciometroB10K.cpp
// Potenciometro B10K via ADC - sustituye a HW040Encoder.cpp
// GPIO 34 -> ADC1_CH6 (input-only, no necesita pinMode OUTPUT)
// Filtro: promedio movil de 10 muestras para eliminar ruido ADC
//
// CALIBRACION: medir ADC con el timon en cada tope fisico y ajustar:
//   POT_ADC_IZQUIERDA -> timon en tope izquierdo  (90 grados)
//   POT_ADC_DERECHA   -> timon en tope derecho     (270 grados)
// El centro (180 grados) queda automaticamente en el punto medio entre ambos.
// Para obtener los valores: observar el log [POT] adc=XXXX con el timon en cada tope.

#include "PotenciometroB10K.h"
#include "Utilidades.h"

// ─── FILTRO PROMEDIO MOVIL ────────────────────────────
#define FILTER_SAMPLES 10
static int filterBuf[FILTER_SAMPLES] = { 0 };
static int filterIdx = 0;
static bool filterFull = false;

static int filteredADC() {
    int raw = analogRead(POT_PIN);
    filterBuf[filterIdx] = raw;
    filterIdx = (filterIdx + 1) % FILTER_SAMPLES;
    if (filterIdx == 0) filterFull = true;
    int n = filterFull ? FILTER_SAMPLES : filterIdx;
    long sum = 0;
    for (int i = 0; i < n; i++) sum += filterBuf[i];
    return (int)(sum / n);
}

// ─── SETUP ───────────────────────────────────────────
void setupHW040Encoder() {
    // GPIO 34 es input-only en ESP32, no requiere pinMode
    // Precalentar el filtro con lecturas iniciales
    for (int i = 0; i < FILTER_SAMPLES; i++) {
        filterBuf[i] = analogRead(POT_PIN);
        delay(2);
    }
    filterFull = true;
    Serial.printf("[POT] Potenciometro B10K OK - GPIO %d - lectura inicial: %d\n",
                  POT_PIN, analogRead(POT_PIN));
}

// ─── LOOP ────────────────────────────────────────────
void loopHW040Encoder() {
    if (timer_log_HW040Encoder.listo(1000)) {
        Serial.printf("[POT] adc=%d grados=%d\n", analogRead(POT_PIN), encoderGetDegrees());
    }
}

// ─── API ─────────────────────────────────────────────
int encoderGetDegrees() {
    int adc = filteredADC();
    // Mapear entre los topes fisicos calibrados -> 90 a 270 grados
    // Fuera de rango se clampea para evitar valores absurdos
    int deg = (int)map(adc, POT_ADC_IZQUIERDA, POT_ADC_DERECHA, 90, 270);
    return constrain(deg, 90, 270);
}

int32_t encoderGetSteps() {
    // No aplica con potenciometro - devuelve 0 por compatibilidad
    return 0;
}

void encoderReset() {
    // Con potenciometro no hay pasos que resetear.
    // La referencia se confirma externamente via timonReferenciada en TimonSistema.
    // Esta funcion se mantiene por compatibilidad con ResetearTimon()
}

bool encoderButtonPressed() {
    // Sin boton fisico - devuelve false por compatibilidad
    return false;
}
