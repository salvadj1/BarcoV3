// Utilidades.h

#ifndef _UTILIDADES_h
#define _UTILIDADES_h

#include <Arduino.h>

// ===== CLASE TIMER ============================================================
class Timer {
public:
	unsigned long last = 0;  // Ultima vez que se ejecuto
	unsigned long reps = 0;  // Conteo de repeticiones
	bool listo(unsigned long intervalo, unsigned long maxReps = 0) {
		if (maxReps > 0 && reps >= maxReps) return false;
		if (millis() - last >= intervalo) {
			last = millis();
			reps++;
			return true;
		}
		return false;
	}
	void reset() {
		last = 0;
		reps = 0;
	}
};

extern Timer timer_log_Timon;
extern Timer timer_procesarTimon;

extern Timer timer_lectura_GY273;
extern Timer timer_log_GY273;
extern Timer timer_lectura_ADXL345;

extern Timer timer_lectura_MPU6050;
extern Timer timer_log_MPU6050;
extern Timer timer_log_HW040Encoder;

extern Timer timer_telemetria;
// ===== FUNCIONES ==============================================================
void SetupUtilidades();
void parpadearLed(uint8_t pin, uint16_t veces, uint32_t duracionEncendido, uint32_t esperaEntreParpadeos);
void encenderLed(uint8_t pin, uint32_t delayDespuesDeEncender);
void apagarLed(uint8_t pin, uint32_t delayDespuesDeApagar);


#endif
