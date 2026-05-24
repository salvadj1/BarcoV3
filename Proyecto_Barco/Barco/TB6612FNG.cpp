#include "TB6612FNG.h"

// ================= PINES =================
// Motor A (TB6612FNG)
#define AIN1 32
#define AIN2 33
#define PWMA 26
#define STBY 4  // Standby del driver

// ================= PWM ESP32 =================
const int pwmChannel = 4;
const int pwmFreq = 1000;
const int pwmResolution = 8;  // 0-255

void setupTB6612FNG() {

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Activar driver
  digitalWrite(STBY, HIGH);

  // Configurar PWM
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(PWMA, pwmChannel);

  // Motor parado inicialmente
  ledcWrite(pwmChannel, 0);
}

void movermotor(int velocidad, bool sentidoHorario) {

  // Limitar velocidad
  velocidad = constrain(velocidad, 0, 255);

  if (sentidoHorario) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  }

  ledcWrite(pwmChannel, velocidad);
}

void loopTB6612FNG() {
  // Ejemplo simple de uso
  /* movermotor(180, true);
  delay(2000);

  movermotor(0, true);
  delay(1000);

  movermotor(180, false);
  delay(2000);

  movermotor(0, false);
  delay(1000);*/
}