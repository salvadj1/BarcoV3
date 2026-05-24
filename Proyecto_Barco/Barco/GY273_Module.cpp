// GY273_Module.cpp
// HMC5883L via Adafruit_HMC5883_U
// Calibracion no bloqueante via SPIFFS
// Filtro Kalman + promedio circular

#include "GY273_Module.h"
#include "Utilidades.h"
#include "ADXL345_Module.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <SPIFFS.h>
#include <math.h>

static Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12344);
static bool sensorOK = false;

// ---------- VARIABLES EXPORTADAS ----------
double currentCourse = 0;
bool courseValid = false;
bool gy273CalibOK = false;
uint8_t gy273CalibProgress = 255;
float gy273OffsetX = 0, gy273OffsetY = 0;
float gy273ScaleX = 1, gy273ScaleY = 1;

// ---------- CALIBRACION ----------
static float offsetX = 0, offsetY = 0, offsetZ = 0;
static float scaleX = 1, scaleY = 1;
static float xMin = 10000, xMax = -10000;
static float yMin = 10000, yMax = -10000;
static float zMin = 10000, zMax = -10000;
static bool calibrando = false;
static bool calibCompleta = false;
static int calibSamples = 0;

// ---------- FILTRO KALMAN ----------
static float kalman_heading = 0.0f;
static float kalman_uncertainty = 4.0f;
static const float processNoise = 0.01f;
static const float measurementNoise = 0.5f;

static float kalmanFilter(float z) {
  kalman_uncertainty += processNoise;
  float K = kalman_uncertainty / (kalman_uncertainty + measurementNoise);
  float diff = z - kalman_heading;
  if (diff > M_PI) diff -= 2 * M_PI;
  if (diff < -M_PI) diff += 2 * M_PI;
  kalman_heading += K * diff;
  kalman_uncertainty *= (1 - K);
  if (kalman_heading < 0) kalman_heading += 2 * M_PI;
  if (kalman_heading >= 2 * M_PI) kalman_heading -= 2 * M_PI;
  return kalman_heading;
}

// ---------- PROMEDIO CIRCULAR ----------
#define BUF_SIZE 5
static float headingBuf[BUF_SIZE] = { 0 };
static int bufIdx = 0;

static float circularAverage(float newVal) {
  headingBuf[bufIdx] = newVal;
  bufIdx = (bufIdx + 1) % BUF_SIZE;
  float sumSin = 0, sumCos = 0;
  for (int i = 0; i < BUF_SIZE; i++) {
    sumSin += sin(headingBuf[i]);
    sumCos += cos(headingBuf[i]);
  }
  float avg = atan2(sumSin / BUF_SIZE, sumCos / BUF_SIZE);
  if (avg < 0) avg += 2 * M_PI;
  return avg;
}

// ---------- SPIFFS ----------
struct CalibData {
  float oX, oY, oZ, sX, sY;
};

static bool loadCalib() {
  if (!SPIFFS.exists(GY273_CALIB_FILE)) return false;
  File f = SPIFFS.open(GY273_CALIB_FILE, FILE_READ);
  if (!f) return false;
  CalibData d;
  f.read((uint8_t*)&d, sizeof(d));
  f.close();
  if (isnan(d.oX) || isnan(d.oY) || isnan(d.sX) || isnan(d.sY)) return false;
  if (fabs(d.oX) < 0.01f && fabs(d.oY) < 0.01f) return false;
  offsetX = d.oX;
  offsetY = d.oY;
  offsetZ = d.oZ;
  scaleX = d.sX;
  scaleY = d.sY;
  gy273OffsetX = offsetX;
  gy273OffsetY = offsetY;
  gy273ScaleX = scaleX;
  gy273ScaleY = scaleY;
  Serial.printf("GY273: calib cargada  oX=%.2f oY=%.2f oZ=%.2f sX=%.4f sY=%.4f\n",
                offsetX, offsetY, offsetZ, scaleX, scaleY);
  return true;
}

static void saveCalib() {
  CalibData d = { offsetX, offsetY, offsetZ, scaleX, scaleY };
  File f = SPIFFS.open(GY273_CALIB_FILE, FILE_WRITE);
  if (!f) {
    Serial.println("GY273: error guardando calib");
    return;
  }
  f.write((uint8_t*)&d, sizeof(d));
  f.close();
  gy273OffsetX = offsetX;
  gy273OffsetY = offsetY;
  gy273ScaleX = scaleX;
  gy273ScaleY = scaleY;
  Serial.printf("GY273: calib guardada  oX=%.2f oY=%.2f oZ=%.2f sX=%.4f sY=%.4f\n",
                offsetX, offsetY, offsetZ, scaleX, scaleY);
}

// ---------- SETUP ----------
void SetupGY273() {
  // Escanear bus I2C para diagnostico
  Serial.println("GY273: escaneando I2C...");
  bool found = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Dispositivo en: 0x%02X\n", addr);
      found = true;
    }
  }
  if (!found) {
    Serial.println("  Ningun dispositivo I2C - revisa cableado SDA=21 SCL=22");
    sensorOK = false;
    return;
  }

  if (!mag.begin()) {
    Serial.println("GY273: HMC5883L NO detectado en 0x1E");
    sensorOK = false;
    return;
  }

  sensorOK = true;
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.printf("GY273: '%s' OK (ID:%d)\n", sensor.name, sensor.sensor_id);

  // Forzar modo continuo via I2C directo tras mag.begin()
  Wire.beginTransmission(0x1E);
  Wire.write(0x00);
  Wire.write(0x78);  // CRA: 8avg, 75Hz, normal
  Wire.endTransmission();
  delay(5);
  Wire.beginTransmission(0x1E);
  Wire.write(0x01);
  Wire.write(0x20);  // CRB: gain 1
  Wire.endTransmission();
  delay(5);
  Wire.beginTransmission(0x1E);
  Wire.write(0x02);
  Wire.write(0x00);  // MODE: continuo
  Wire.endTransmission();
  delay(15);
  Serial.println("GY273: modo continuo forzado OK");

  for (int i = 0; i < BUF_SIZE; i++) headingBuf[i] = 0;

  gy273CalibOK = loadCalib();
  if (!gy273CalibOK)
    Serial.println("GY273: sin calibracion - usa boton CAL en la web");
}

// ---------- CALIBRACION NO BLOQUEANTE ----------
void IniciarCalibracionGY273() {
  if (!sensorOK) return;
  calibrando = true;
  calibCompleta = false;
  calibSamples = 0;
  gy273CalibProgress = 0;
  xMin = yMin = zMin = 10000;
  xMax = yMax = zMax = -10000;
  Serial.println("GY273: CALIBRANDO - gira 360 grados lentamente...");
}

bool CalibracionGY273Completa() {
  return calibCompleta;
}

// ---------- LOOP ----------
void LoopGY273() {
  if (!sensorOK) {
    // Sin sensor: enviar heading 0 igualmente para que la web no se quede sin datos
    courseValid = false;
    return;
  }


  // Log GY273 cada 1 segundo
  if (timer_log_GY273.listo(1000)) {
    //Serial.printf("[GY273] heading=%.1f  valid=%s  calibOK=%s  tilt=%s\n",
    //              (float)currentCourse,
    //              courseValid ? "SI" : "NO",
    //              gy273CalibOK ? "SI" : "NO",
    //              adxlOK ? "SI" : "NO");
    //Serial.printf("[GY273 RAW] x=%.3f y=%.3f z=%.3f\n", mx, my, ev.magnetic.z);
  }


  sensors_event_t ev;
  mag.getEvent(&ev);
  float mx = ev.magnetic.x;
  float my = ev.magnetic.y;

  // Debug RAW cada 500ms
  //static uint32_t lastRawLog = 0;
  //if (millis() - lastRawLog > 500) {
 //   lastRawLog = millis();
  //  Serial.printf("[GY273 RAW] x=%.3f y=%.3f z=%.3f\n", mx, my, ev.magnetic.z);
 // }

  // ---- CALIBRACION ----
  if (calibrando) {
    if (mx < xMin) xMin = mx;
    if (mx > xMax) xMax = mx;
    if (my < yMin) yMin = my;
    if (my > yMax) yMax = my;
    float mz = ev.magnetic.z;
    if (mz < zMin) zMin = mz;
    if (mz > zMax) zMax = mz;
    calibSamples++;
    gy273CalibProgress = (uint8_t)((calibSamples * 100) / GY273_CALIB_SAMPLES);

    if (calibSamples >= GY273_CALIB_SAMPLES) {
      offsetX = (xMax + xMin) / 2.0f;
      offsetY = (yMax + yMin) / 2.0f;
      offsetZ = (zMax + zMin) / 2.0f;
      float rX = (xMax - xMin) / 2.0f;
      float rY = (yMax - yMin) / 2.0f;
      scaleX = (rX > 0) ? rY / rX : 1.0f;
      scaleY = (rY > 0) ? rX / rY : 1.0f;
      saveCalib();
      calibrando = false;
      calibCompleta = true;
      gy273CalibOK = true;
      gy273CalibProgress = 255;
      Serial.println("GY273: calibracion completada!");
    }
    return;
  }

  // ---- HEADING CON COMPENSACION DE TILT ----
  // GY273: eje X adelante
  // ADXL:  eje Y adelante (pitch), eje X lateral (roll)
  float cx = (mx - offsetX) * scaleX;
  float cy = (my - offsetY) * scaleY;
  float cz = (ev.magnetic.z - offsetZ);  // Z calibrado

  float rawH;

  if (adxlOK) {
    float pitch = adxl_pitch;  // atan2(-ay, az) - Y adelante
    float roll = adxl_roll;    // atan2( ax, az) - X lateral

    float cp = cos(pitch), sp = sin(pitch);
    float cr = cos(roll), sr = sin(roll);

    // Matriz de rotacion completa - compensa pitch Y roll
    // GY273 eje X adelante
    float Xh = cx * cp + cy * sr * sp + cz * cr * sp;
    float Yh = cy * cr + cz * sr;

    rawH = atan2(Yh, Xh);
  } else {
    // Sin ADXL: heading plano - igual que antes de introducir tilt
    rawH = atan2(cy, cx);
  }

  rawH += GY273_DECLINATION_RAD;
  if (rawH < 0) rawH += 2 * M_PI;
  if (rawH >= 2 * M_PI) rawH -= 2 * M_PI;

  float smoothH = kalmanFilter(rawH);
  float finalH = circularAverage(smoothH);

  currentCourse = finalH * 180.0 / M_PI;
  courseValid = true;
}
