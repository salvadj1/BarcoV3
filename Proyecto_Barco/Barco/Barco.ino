#include "AS5600_Magnetic_Sensor.h"
#include "TB6612FNG.h"
#include "VoltajeSensor.h"
#include "LucesCOB.h"

#include "Utilidades.h"
#include "GPS_Neo_6M.h"
#include "ADXL345_Module.h"
#include "GY273_Module.h"
#include "I2C_Scanner.h"
#include "ESCMotor.h"
#include "TimonSistema.h"
#include "Servos.h"
#include "ESPNow_Barco.h"
#include "Viajes_Logica.h"
#include <Wire.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <esp_now.h>

// Intervalos de actualizacion
#define TELEMETRIA_INTERVAL 200  // 5 Hz
#define GY273_INTERVAL 50        // 20 Hz
#define ADXL_INTERVAL 20         // 50 Hz
#define TIMON_INTERVAL 20        // 50 Hz
#define VOLTAJE_INTERVAL  500   // 2 Hz
// LucesCOB se llama sin timer (como LoopADXL345 en su versión interna)
// o con intervalo muy corto si prefieres controlarlo:
#define COB_INTERVAL      20    // 50 Hz - necesario para efectos suaves

void setup() {
  SetupUtilidades();

  // SPIFFS primero: todos los modulos que leen/escriben archivos lo necesitan
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS: error al montar - formateando...");
    SPIFFS.format();
    SPIFFS.begin(true);
  }
  Serial.println("SPIFFS OK");

  Wire.begin(21, 22);  // I2C: SDA=21, SCL=22
  EscanearI2C();        // lista por Serial los sensores detectados (util para verificar cableado/direccion)

  SetupESC();    // ESC / motor principal (GPIO 25)
  SetupTimon();  // Motor DC timon + encoder (GPIO 26,4,32,33,34,35)
  SetupCebos();  // Servos cebo 1 y 2 (GPIO 12, 27)

  SetupGPS();
  loadParamsViaje();  // carga distProximidad, pausaMotor y puntos guardados
  SetupADXL345();
  SetupGY273();
  setupTB6612FNG();
  setupHW040Encoder();

  SetupVoltajeSensor();
SetupLucesCOB();


  SetupESPNowBarco();

  Serial.println("BARCO listo");
}

void loop() {

  //LoopGPS(); //Desde aqui se llama a LoopViajesLogica();

  loopHW040Encoder();

  if (LoopGPS()) {
    LoopViajesLogica();
  }

  // ADXL345 a 50 Hz — antes del GY273 para que el tilt este actualizado
  if (timer_lectura_ADXL345.listo(ADXL_INTERVAL)) {
    LoopADXL345();
  }

  // GY273 a 20 Hz
 if (timer_lectura_GY273.listo(GY273_INTERVAL)) {
    LoopGY273();
  }

  // Timon a 50 Hz — bucle proporcional sobre encoder
  if (timer_procesarTimon.listo(TIMON_INTERVAL)) {
    updateTimon();
  }

  if (timer_lectura_Voltaje.listo(VOLTAJE_INTERVAL)) {
    LoopVoltajeSensor();
}
if (timer_lectura_LucesCOB.listo(COB_INTERVAL)) {
    LoopLucesCOB();
}

  // Telemetria a 5 Hz
  if (timer_telemetria.listo(TELEMETRIA_INTERVAL)) {
    EnviarTelemetria();
  }
}
