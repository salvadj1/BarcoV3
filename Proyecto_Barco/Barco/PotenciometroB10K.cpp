// PotenciometroB10K.cpp
// Potenciometro B10K via ADC - sustituye a HW040Encoder.cpp
// GPIO 34 -> ADC1_CH6 (input-only, no necesita pinMode OUTPUT)
// Filtro: promedio movil de 10 muestras para eliminar ruido ADC
//
// CALIBRACION: centrar el timon fisicamente y pulsar CTR en la web.
// Esto fija el centro ADC y calcula los topes como centro ± AMPLITUD_GIRO.
// Para ver el ADC actual: observar el log [POT] adc=XXXX

#include "PotenciometroB10K.h"
#include "Utilidades.h"

// ─── CENTRO DINAMICO Y TOPES CALCULADOS ──────────────
int potAdcCentro    = 0;
int potAdcIzquierda = 0;
int potAdcDerecha   = 0;

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
    Serial.println("[POT] Centra el timon y pulsa CTR para fijar referencia");
}

// ─── LOOP ────────────────────────────────────────────
void loopHW040Encoder() {
    if (timer_log_HW040Encoder.listo(1000)) {
        int adc = filteredADC();
        Serial.printf("[POT] adc=%d grados=%d centroADC=%d izq=%d der=%d\n",
                      adc, encoderGetDegrees(),
                      potAdcCentro, potAdcIzquierda, potAdcDerecha);
    }
}

// ─── FIJAR CENTRO Y CALCULAR TOPES ───────────────────
void encoderSetCentro() {
    potAdcCentro    = filteredADC();
    potAdcIzquierda = potAdcCentro - AMPLITUD_GIRO;
    potAdcDerecha   = potAdcCentro + AMPLITUD_GIRO;
    Serial.printf("[POT] Centro fijado: ADC=%d  izq=%d  der=%d\n",
                  potAdcCentro, potAdcIzquierda, potAdcDerecha);
}

// ─── API ─────────────────────────────────────────────
int encoderGetDegrees() {
    // Sin referencia: devolver 180 (centro) para no mover el timon
    if (potAdcCentro == 0) return 180;

    int adc = filteredADC();
    int deg;
    if (adc <= potAdcCentro) {
        // Tramo izquierdo: potAdcIzquierda..potAdcCentro -> 90..180 grados
        deg = (int)map(adc, potAdcIzquierda, potAdcCentro, 90, 180);
    } else {
        // Tramo derecho: potAdcCentro..potAdcDerecha -> 180..270 grados
        deg = (int)map(adc, potAdcCentro, potAdcDerecha, 180, 270);
    }
    return constrain(deg, 90, 270);
}

int32_t encoderGetSteps() {
    // No aplica con potenciometro - devuelve 0 por compatibilidad
    return 0;
}

void encoderReset() {
    // Compatibilidad - la referencia se gestiona via timonReferenciada en TimonSistema
}

bool encoderButtonPressed() {
    // Sin boton fisico - devuelve false por compatibilidad
    return false;
}
