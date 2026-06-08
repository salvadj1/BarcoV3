// AS5600_Magnetic_Sensor.cpp
// Sensor magnetico AS5600 via I2C - sustituye a PotenciometroB10K.cpp
// GPIO SDA=21, SCL=22 - Wire.begin() en Barco.ino
//
// CALIBRACION: centrar el timon fisicamente y pulsar CTR en la web.
// El angulo de centro se guarda en SPIFFS y se recupera en cada arranque.
// Para ver el angulo raw actual: observar el log [AS5600] angle=XXX.X

#include "AS5600_Magnetic_Sensor.h"
#include "Utilidades.h"
#include <Wire.h>
#include <SPIFFS.h>
#include <math.h>

// ─── ESTADO EXPORTADO ────────────────────────────────
float as5600CentroGrados    = 0.0f;
float as5600IzquierdaGrados = 0.0f;
float as5600DerechaGrados   = 0.0f;
bool  as5600CentroValido    = false;
bool  as5600SensorOK        = false;

// ─── I2C HELPER ──────────────────────────────────────
static float leerAnguloRaw() {
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(AS5600_REG_ANGLE_H);
    if (Wire.endTransmission(false) != 0) return -1.0f;
    Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
    if (Wire.available() < 2) return -1.0f;
    uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();
    raw &= 0x0FFF;  // 12 bits validos
    return raw * 360.0f / 4096.0f;
}

// ─── DIFERENCIA ANGULAR CIRCULAR ─────────────────────
// Devuelve diferencia entre dos angulos (0-360) en rango -180..+180
// Necesario para manejar el cruce por 0/360 correctamente
static float diffAngular(float a, float b) {
    float d = a - b;
    if (d >  180.0f) d -= 360.0f;
    if (d < -180.0f) d += 360.0f;
    return d;
}

// ─── SPIFFS ──────────────────────────────────────────
static void guardarCentro() {
    File f = SPIFFS.open(AS5600_CALIB_FILE, FILE_WRITE);
    if (!f) {
        Serial.println("[AS5600] Error guardando centro en SPIFFS");
        return;
    }
    f.write((uint8_t*)&as5600CentroGrados, sizeof(float));
    f.close();
    Serial.printf("[AS5600] Centro guardado en SPIFFS: %.2f grados\n", as5600CentroGrados);
}

static bool cargarCentro() {
    if (!SPIFFS.exists(AS5600_CALIB_FILE)) return false;
    File f = SPIFFS.open(AS5600_CALIB_FILE, FILE_READ);
    if (!f) return false;
    float val = 0.0f;
    if (f.read((uint8_t*)&val, sizeof(float)) != sizeof(float)) { f.close(); return false; }
    f.close();
    // Validar rango — cualquier float valido 0-360 es aceptable incluido 0.0
    if (isnan(val) || val < 0.0f || val > 360.0f) return false;
    as5600CentroGrados    = val;
    as5600IzquierdaGrados = as5600CentroGrados - AMPLITUD_GIRO_DEG;
    as5600DerechaGrados   = as5600CentroGrados + AMPLITUD_GIRO_DEG;
    as5600CentroValido    = true;
    Serial.printf("[AS5600] Centro cargado de SPIFFS: %.2f grados  izq=%.2f  der=%.2f\n",
                  as5600CentroGrados, as5600IzquierdaGrados, as5600DerechaGrados);
    return true;
}

// ─── SETUP ───────────────────────────────────────────
void setupHW040Encoder() {
    delay(10);

    // Verificar presencia del sensor en bus I2C
    Wire.beginTransmission(AS5600_ADDR);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("[AS5600] NO encontrado en 0x%02X (err=%d) - revisa cableado SDA=21 SCL=22\n",
                      AS5600_ADDR, err);
        as5600SensorOK = false;
        return;
    }

    // Lectura de prueba para confirmar comunicacion
    float angle = leerAnguloRaw();
    if (angle < 0.0f) {
        Serial.println("[AS5600] Error en lectura inicial - sensor no responde");
        as5600SensorOK = false;
        return;
    }

    as5600SensorOK = true;
    Serial.printf("[AS5600] OK en 0x%02X - angulo inicial: %.2f grados\n", AS5600_ADDR, angle);

    // Intentar cargar centro guardado de SPIFFS
    // Si lo carga, as5600CentroValido=true y TimonSistema lo detecta en SetupTimon()
    if (cargarCentro()) {
        Serial.println("[AS5600] Referencia cargada - CTR no necesario en este arranque");
    } else {
        Serial.println("[AS5600] Sin referencia guardada - centra el timon y pulsa CTR");
    }
}

// ─── LOOP ────────────────────────────────────────────
void loopHW040Encoder() {
    if (timer_log_HW040Encoder.listo(1000)) {
        float angle = as5600SensorOK ? leerAnguloRaw() : -1.0f;
        Serial.printf("[AS5600] angle=%.1f grados=%d centroGrad=%.1f izq=%.1f der=%.1f\n",
                      angle, encoderGetDegrees(),
                      as5600CentroGrados, as5600IzquierdaGrados, as5600DerechaGrados);
    }
}

// ─── FIJAR CENTRO Y CALCULAR TOPES ───────────────────
void encoderSetCentro() {
    if (!as5600SensorOK) {
        Serial.println("[AS5600] encoderSetCentro: sensor no disponible");
        return;
    }
    float angle = leerAnguloRaw();
    if (angle < 0.0f) {
        Serial.println("[AS5600] encoderSetCentro: error de lectura");
        return;
    }
    as5600CentroGrados    = angle;
    as5600IzquierdaGrados = as5600CentroGrados - AMPLITUD_GIRO_DEG;
    as5600DerechaGrados   = as5600CentroGrados + AMPLITUD_GIRO_DEG;
    as5600CentroValido    = true;
    guardarCentro();
    Serial.printf("[AS5600] Centro fijado: %.2f grados  izq=%.2f  der=%.2f\n",
                  as5600CentroGrados, as5600IzquierdaGrados, as5600DerechaGrados);
}

// ─── BORRAR CENTRO ────────────────────────────────────
// Llamado por ResetearTimon() en el segundo CTR para quitar la referencia
void encoderBorrarCentro() {
    as5600CentroGrados    = 0.0f;
    as5600IzquierdaGrados = 0.0f;
    as5600DerechaGrados   = 0.0f;
    as5600CentroValido    = false;
    if (SPIFFS.exists(AS5600_CALIB_FILE)) {
        SPIFFS.remove(AS5600_CALIB_FILE);
        Serial.println("[AS5600] Centro borrado de SPIFFS");
    }
    Serial.println("[AS5600] Referencia borrada - centra y pulsa CTR de nuevo");
}

// ─── API ─────────────────────────────────────────────
int encoderGetDegrees() {
    // Sin referencia o sin sensor: devolver 180 (centro) para no mover el timon
    if (!as5600CentroValido || !as5600SensorOK) return 180;

    float angle = leerAnguloRaw();
    if (angle < 0.0f) return 180;  // error de lectura: no mover

    // Diferencia angular circular respecto al centro (-180..+180)
    // diffAngular maneja correctamente el cruce por 0/360
    float diff = diffAngular(angle, as5600CentroGrados);

    // Clampear diff al rango fisico antes de mapear
    diff = constrain(diff, -AMPLITUD_GIRO_DEG, AMPLITUD_GIRO_DEG);

    // Mapear diff (-AMPLITUD..+AMPLITUD) -> grados sistema (90..270)
    // diff negativa = izquierda = 90
    // diff cero     = centro    = 180
    // diff positiva = derecha   = 270
    float mapped = 180.0f + (diff / AMPLITUD_GIRO_DEG) * 90.0f;

    return (int)constrain(mapped, 90.0f, 270.0f);
}

int32_t encoderGetSteps() {
    // No aplica con AS5600 - devuelve 0 por compatibilidad
    return 0;
}

void encoderReset() {
    // Compatibilidad - la referencia se gestiona via timonReferenciada en TimonSistema
}

bool encoderButtonPressed() {
    // Sin boton fisico - devuelve false por compatibilidad
    return false;
}
