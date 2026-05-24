// Utilidades.cpp

#include "Utilidades.h"

Timer timer_log_Timon;
Timer timer_procesarTimon;

Timer timer_lectura_GY273;
Timer timer_log_GY273;
Timer timer_lectura_ADXL345;

Timer timer_lectura_MPU6050;
Timer timer_log_MPU6050;
Timer timer_log_Radio;

Timer timer_telemetria;

void SetupUtilidades() {
	Serial.begin(115200);

	// pinMode(LED_BUILTIN, OUTPUT);
}

void parpadearLed(uint8_t pin, uint16_t veces, uint32_t duracionEncendido, uint32_t esperaEntreParpadeos) {
	/*  for (uint16_t i = 0; i < veces; i++) {
		  digitalWrite(pin, HIGH);
		  delay(duracionEncendido);
		  digitalWrite(pin, LOW);
		  delay(esperaEntreParpadeos);
	  }*/
}

void encenderLed(uint8_t pin, uint32_t delayDespuesDeEncender) {
	/*digitalWrite(pin, HIGH);
	delay(delayDespuesDeEncender);*/
}

void apagarLed(uint8_t pin, uint32_t delayDespuesDeApagar) {
	/*digitalWrite(pin, LOW);
	delay(delayDespuesDeApagar);*/
}
