// Viajes_Logica.cpp
// Logica de navegacion autonoma, cebos, historial de ruta y SPIFFS

#include "Viajes_Logica.h"
#include "GPS_Neo_6M.h"
#include "ESCMotor.h"
#include "TimonSistema.h"
#include "Servos.h"

#include <SPIFFS.h>
#include <math.h>

// ---------- COORDENADAS Y ESTADO ----------
Coordenada home  = { 0, 0, false };
Coordenada cebo1 = { 0, 0, false };
Coordenada cebo2 = { 0, 0, false };

EstadoNavegacion navState = IDLE;

// ---------- PARAMETROS CONFIGURABLES ----------
int distProximidad = 3;   // metros de proximidad para dar punto por llegado
int pausaMotor     = 0;   // segundos de pausa en cada punto

#define PARAMS_FILE "/viaje_params.dat"
struct ViajeParams { int dist; int pausa; };

// ---------- PERSISTENCIA DE PUNTOS ----------
struct PointsData {
    double homeLat, homeLng; bool homeValid;
    double c1Lat,   c1Lng;   bool c1Valid;
    double c2Lat,   c2Lng;   bool c2Valid;
};
#define POINTS_FILE "/points.dat"

void savePointsToSPIFFS() {
    PointsData p = {
        home.lat,  home.lng,  home.valid,
        cebo1.lat, cebo1.lng, cebo1.valid,
        cebo2.lat, cebo2.lng, cebo2.valid
    };
    File f = SPIFFS.open(POINTS_FILE, FILE_WRITE);
    if (!f) return;
    f.write((uint8_t*)&p, sizeof(p));
    f.close();
    Serial.println("Puntos guardados en SPIFFS");
}

void loadPointsFromSPIFFS() {
    if (!SPIFFS.exists(POINTS_FILE)) return;
    File f = SPIFFS.open(POINTS_FILE, FILE_READ);
    if (!f) return;
    PointsData p;
    if (f.read((uint8_t*)&p, sizeof(p)) != sizeof(p)) { f.close(); return; }
    f.close();
    home.lat  = p.homeLat;  home.lng  = p.homeLng;  home.valid  = p.homeValid;
    cebo1.lat = p.c1Lat;    cebo1.lng = p.c1Lng;    cebo1.valid = p.c1Valid;
    cebo2.lat = p.c2Lat;    cebo2.lng = p.c2Lng;    cebo2.valid = p.c2Valid;
    Serial.printf("Puntos cargados: home=%d c1=%d c2=%d\n",
                  home.valid, cebo1.valid, cebo2.valid);
}

void saveParamsViaje() {
    ViajeParams p = { distProximidad, pausaMotor };
    File f = SPIFFS.open(PARAMS_FILE, FILE_WRITE);
    if (!f) return;
    f.write((uint8_t*)&p, sizeof(p));
    f.close();
    Serial.printf("Params viaje guardados: dist=%dm pausa=%ds\n",
                  distProximidad, pausaMotor);
}

void loadParamsViaje() {
    if (!SPIFFS.exists(PARAMS_FILE)) return;
    File f = SPIFFS.open(PARAMS_FILE, FILE_READ);
    if (!f) return;
    ViajeParams p;
    f.read((uint8_t*)&p, sizeof(p));
    f.close();
    distProximidad = constrain(p.dist,  1, 30);
    pausaMotor     = constrain(p.pausa, 0, 30);
    Serial.printf("Params viaje cargados: dist=%dm pausa=%ds\n",
                  distProximidad, pausaMotor);
    loadPointsFromSPIFFS();
}

// ---------- HISTORIAL ----------
HistPoint history[MAX_HIST];
int       histCount = 0;

#define HIST_MIN_DIST_M  3.0
#define HIST_MIN_SPEED   0.3

static void addHistoryPoint() {
    if (!gps.location.isValid()) return;
    double spd = gps.speed.isValid() ? gps.speed.kmph() : 0;
    if (spd < HIST_MIN_SPEED && histCount > 0) return;
    if (histCount > 0) {
        double dist = haversine(smoothLat, smoothLng,
                                history[histCount-1].lat,
                                history[histCount-1].lng);
        if (dist < HIST_MIN_DIST_M) return;
    }
    HistPoint p = { smoothLat, smoothLng };
    if (histCount < MAX_HIST) {
        history[histCount++] = p;
    } else {
        memmove(history, history + 1, (MAX_HIST - 1) * sizeof(HistPoint));
        history[MAX_HIST - 1] = p;
    }
}

// ---------- ESTADISTICAS ----------
double        totalDist    = 0;
double        maxSpeed     = 0;
double        sumSpeed     = 0;
int           speedSamples = 0;
double        lastLat      = 0, lastLng = 0;
unsigned long tripStartMs  = 0;

// ---------- HAVERSINE ----------
double haversine(double lat1, double lng1, double lat2, double lng2) {
    const double R = 6371000.0;
    double dLat = radians(lat2 - lat1);
    double dLng = radians(lng2 - lng1);
    double a = sin(dLat/2)*sin(dLat/2) +
               cos(radians(lat1))*cos(radians(lat2))*
               sin(dLng/2)*sin(dLng/2);
    return R * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

// ---------- GUARDAR VIAJE EN SPIFFS ----------
void saveCurrentTrip() {
    if (SPIFFS.exists("/viaje1.json")) SPIFFS.remove("/viaje1.json");
    if (SPIFFS.exists("/viaje0.json")) SPIFFS.rename("/viaje0.json", "/viaje1.json");
    File f = SPIFFS.open("/viaje0.json", FILE_WRITE);
    if (!f) return;
    unsigned long dur    = (millis() - tripStartMs) / 1000;
    double        avgSpd = speedSamples > 0 ? sumSpeed / speedSamples : 0;
    f.print("{\"dur\":");    f.print(dur);
    f.print(",\"dist\":");   f.print(totalDist, 1);
    f.print(",\"maxSpd\":"); f.print(maxSpeed, 1);
    f.print(",\"avgSpd\":"); f.print(avgSpd, 1);
    f.print(",\"pts\":[");
    for (int i = 0; i < histCount; i++) {
        if (i) f.print(",");
        f.print("{\"la\":"); f.print(history[i].lat, 6);
        f.print(",\"ln\":"); f.print(history[i].lng, 6);
        f.print("}");
    }
    f.print("]}");
    f.close();
}

String loadTrip(const char* path) {
    if (!SPIFFS.exists(path)) return "null";
    File f = SPIFFS.open(path, FILE_READ);
    if (!f) return "null";
    String s = f.readString();
    f.close();
    return s;
}

// ---------- PAUSA NO BLOQUEANTE ----------
uint8_t       pausandoEn    = 0;
unsigned long pausaInicioMs = 0;

// Centra el timon sin servo:
// joySteer=0 + modoManual ya false en viaje -> updateTimon() lo lleva al centro solo
static inline void centrarTimon() {
    joySteer = 0;
}

// ---------- LOOP PRINCIPAL DE VIAJE ----------
void LoopViajesLogica() {
    double spd = gps.speed.isValid() ? gps.speed.kmph() : 0;

    // Acumular historial y estadisticas si hay viaje activo
    if (navState != IDLE && navState != ARRIVED) {
        addHistoryPoint();
        if (spd > 0.3) {
            double d = haversine(smoothLat, smoothLng, lastLat, lastLng);
            totalDist += d;
            lastLat = smoothLat; lastLng = smoothLng;
            if (spd > maxSpeed) maxSpeed = spd;
            sumSpeed += spd; speedSamples++;
        }
    }

    // --- PAUSA NO BLOQUEANTE ---
    if (pausandoEn > 0) {
        unsigned long pausaMs = (unsigned long)pausaMotor * 1000UL;
        if (millis() - pausaInicioMs < pausaMs) {
            // En pausa: motor parado, timon al centro
            setMotorPct(0);
            centrarTimon();
            return;
        }
        // Pausa terminada
        uint8_t dondeParamos = pausandoEn;
        pausandoEn = 0;

        if (dondeParamos == 1) {
            setCebo1(true);
            navState      = GOING_CEBO2;
            targetBearing = calcBearing(smoothLat, smoothLng, cebo2.lat, cebo2.lng);
            setMotorPct(throttleMax);
        } else if (dondeParamos == 2) {
            setCebo2(true);
            navState      = RETURNING;
            targetBearing = calcBearing(smoothLat, smoothLng, home.lat, home.lng);
            setMotorPct(throttleMax);
        } else if (dondeParamos == 3) {
            navState     = ARRIVED;
            motorRunning = false;
            setMotorPct(0);
            centrarTimon();
            saveCurrentTrip();
        }
        return;
    }

    // --- MAQUINA DE ESTADOS ---
    if (navState == GOING_CEBO1 && cebo1.valid) {
        targetBearing = calcBearing(smoothLat, smoothLng, cebo1.lat, cebo1.lng);
        double dist   = haversine(smoothLat, smoothLng, cebo1.lat, cebo1.lng);
        if (dist < 50) setMotorPct(max(throttleMin, (int)(throttleMax * dist / 50.0)));
        if (dist <= distProximidad && !cebo1Disparado) {
            cebo1Disparado = true;
            setMotorPct(0);
            centrarTimon();
            pausandoEn    = 1;
            pausaInicioMs = millis();
        }
    }
    else if (navState == GOING_CEBO2 && cebo2.valid) {
        targetBearing = calcBearing(smoothLat, smoothLng, cebo2.lat, cebo2.lng);
        double dist   = haversine(smoothLat, smoothLng, cebo2.lat, cebo2.lng);
        if (dist < 50) setMotorPct(max(throttleMin, (int)(throttleMax * dist / 50.0)));
        if (dist <= distProximidad && !cebo2Disparado) {
            cebo2Disparado = true;
            setMotorPct(0);
            centrarTimon();
            pausandoEn    = 2;
            pausaInicioMs = millis();
        }
    }
    else if (navState == RETURNING && home.valid) {
        targetBearing = calcBearing(smoothLat, smoothLng, home.lat, home.lng);
        double dist   = haversine(smoothLat, smoothLng, home.lat, home.lng);
        if (dist < 50) setMotorPct(max(throttleMin, (int)(throttleMax * dist / 50.0)));
        if (dist <= distProximidad) {
            setMotorPct(0);
            centrarTimon();
            pausandoEn    = 3;
            pausaInicioMs = millis();
        }
    }
    // NOTA: updateTimon() lo gestiona el timer de 50 Hz en Barco.ino, no se llama aqui
}
