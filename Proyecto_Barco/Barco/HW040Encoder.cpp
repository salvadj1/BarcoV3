#include "HW040Encoder.h"
#include "Utilidades.h"

// ─── CONFIGURACIÓN ───────────────────────────────────
#define ENCODER_CLK 13
#define ENCODER_DT 23
#define ENCODER_SW 18
#define DEBOUNCE_MS 50

// ─── ESTADO GLOBAL ───────────────────────────────────
static volatile int32_t encoderSteps = 15;
static volatile int8_t accumulator = 0;
static volatile uint8_t lastState = 0;
static volatile bool buttonPressed = false;
static volatile uint32_t lastDebounce = 0;
int Pasos_por_revolucion = 30;

// ─── TABLA DE ESTADOS ────────────────────────────────
static const int8_t stateTable[16] = {
  0, -1, 1, 0,
  1, 0, 0, -1,
  -1, 0, 0, 1,
  0, 1, -1, 0
};

// ─── ISR ENCODER ─────────────────────────────────────
static void IRAM_ATTR isrEncoder() {
  uint8_t clk = digitalRead(ENCODER_CLK);
  uint8_t dt = digitalRead(ENCODER_DT);

  uint8_t newState = (clk << 1) | dt;

  accumulator += stateTable[(lastState << 2) | newState];
  lastState = newState;

  if (accumulator >= 2) {
    encoderSteps++;
    accumulator = 0;
  }
  if (accumulator <= -2) {
    encoderSteps--;
    accumulator = 0;
  }
}

// ─── ISR BOTÓN ───────────────────────────────────────
static void IRAM_ATTR isrButton() {
  uint32_t now = millis();
  if ((now - lastDebounce) > DEBOUNCE_MS) {
    buttonPressed = true;
    lastDebounce = now;
  }
}

// ─── SETUP ───────────────────────────────────────────
void setupHW040Encoder() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  lastState = (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SW), isrButton, FALLING);
}


void loopHW040Encoder() {
  if (timer_log_HW040Encoder.listo(1000)) {
    Serial.printf("[ENC] steps=%d\n", encoderGetSteps());
  }
}

// ─── API ─────────────────────────────────────────────
int encoderGetDegrees() {
 return map(encoderSteps, 0, Pasos_por_revolucion, 0, 360);
}

int32_t encoderGetSteps() {
  return encoderSteps;
}

void encoderReset() {
  encoderSteps = Pasos_por_revolucion/2;
}

bool encoderButtonPressed() {
  if (buttonPressed) {
    buttonPressed = false;
    return true;
  }
  return false;
}