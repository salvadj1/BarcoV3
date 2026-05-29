#include "ESPNow_Barco.h"

#include <WiFi.h>
#include <esp_now.h>

// MAC del ESP32 de la Playa - CAMBIAR POR LA MAC REAL
uint8_t macPlaya[6] = { 0x40, 0x22, 0xD8, 0x05, 0x05, 0xAC };

#include "GPS_Neo_6M.h"
#include "GY273_Module.h"
#include "Viajes_Logica.h"
#include "ESCMotor.h"
#include "TimonSistema.h"
#include "Servos.h"

// ---------- CALLBACK: COMANDO RECIBIDO DESDE PLAYA ----------
void onComandoRecibido(const uint8_t* mac, const uint8_t* data, int len) {
  if (len != sizeof(ComandoPlaya)) return;
  ComandoPlaya cmd;
  memcpy(&cmd, data, sizeof(ComandoPlaya));

  Serial.printf("CMD recibido: len=%d tipo=%d throttle=%d rumbo=%d\n",
                len, ((ComandoPlaya*)data)->tipo,
                ((ComandoPlaya*)data)->throttle,
                ((ComandoPlaya*)data)->rumbo);


  switch (cmd.tipo) {

    case CMD_JOYSTICK:
      // rumbo: -1=izquierda, 0=centro, +1=derecha
      // rumbo=0 (soltar boton) se procesa SIEMPRE para garantizar vuelta al centro
      if (cmd.rumbo == 0) {
        //updateJoyTimestamp();
        joySteer = 0;
        modoManual = true;
        setMotorPct(cmd.throttle);
        motorRunning = (cmd.throttle > 0);
        break;
      }
      if (navState == IDLE || navState == ARRIVED) {
        if (navState == ARRIVED) {
          navState = IDLE;
          motorRunning = false;
        }
        //updateJoyTimestamp();
        joySteer = cmd.rumbo;
        modoManual = true;
        setMotorPct(cmd.throttle);
        motorRunning = (cmd.throttle > 0);
      }
      break;

    case CMD_CENTER_TIMON:
      ResetearTimon();
      break;

    case CMD_GUARDAR_PUNTO:
      if (!gps.location.isValid()) break;
      if (cmd.punto == 0) home = { smoothLat, smoothLng, true };
      if (cmd.punto == 1) cebo1 = { smoothLat, smoothLng, true };
      if (cmd.punto == 2) cebo2 = { smoothLat, smoothLng, true };
      savePointsToSPIFFS();
      break;

    case CMD_CEBO:
      if (cmd.numeroCebo == 1) setCebo1(cmd.abrirCebo);
      if (cmd.numeroCebo == 2) setCebo2(cmd.abrirCebo);
      break;

    case CMD_START_TRIP:
      if (!home.valid || !cebo1.valid || !cebo2.valid) break;
      modoManual = false;
      joySteer = 0;
      pausandoEn = 0;
      navState = GOING_CEBO1;
      histCount = 0;
      totalDist = 0;
      maxSpeed = 0;
      sumSpeed = 0;
      speedSamples = 0;
      lastLat = smoothLat;
      lastLng = smoothLng;
      tripStartMs = millis();
      cebo1Disparado = false;
      cebo2Disparado = false;
      setCebo1(false);
      setCebo2(false);
      targetBearing = calcBearing(smoothLat, smoothLng, cebo1.lat, cebo1.lng);
      motorRunning = true;
      setMotorPct(throttleMax);
      break;

    case CMD_STOP_TRIP:
      if (histCount > 2) saveCurrentTrip();
      pausandoEn = 0;
      setCebo1(false);
      setCebo2(false);
      navState = IDLE;
      motorRunning = false;
      modoManual = false;
      joySteer = 0;
      setMotorPct(0);
      break;

    case CMD_THROTTLE:
      throttleMax = constrain(cmd.nuevoThrottle, 0, 100);
      if (motorRunning) setMotorPct(throttleMax);
      break;

    case CMD_TRIM:
      //trimTimon = constrain(cmd.trimTimon / 10.0f, -15.0f, 15.0f); //15 grados
      trimTimon = constrain(cmd.trimTimon / 10.0f, -45.0f, 45.0f); //45 grados
      saveTrim();
      break;

    case CMD_INVERT_TIMON:
      invertirTimon = cmd.invertirTimon;
      saveInvert();
      break;

    case CMD_CALIB_GY273:
      IniciarCalibracionGY273();
      break;

    case CMD_THROTTLE_MIN:
      throttleMin = constrain(cmd.nuevoThrottleMin, 0, 50);
      break;

    case CMD_PROXIMIDAD:
      distProximidad = constrain(cmd.distProximidad, 1, 30);
      saveParamsViaje();
      break;

    case CMD_PAUSA:
      pausaMotor = constrain(cmd.pausaMotor, 0, 30);
      saveParamsViaje();
      break;
  }
}

// ---------- SETUP ----------
void SetupESPNowBarco() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init BARCO error");
    return;
  }

  esp_now_register_recv_cb(onComandoRecibido);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, macPlaya, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("ESP-NOW BARCO OK");
  Serial.print("MAC Barco: ");
  Serial.println(WiFi.macAddress());
}

// ---------- ENVIAR TELEMETRIA ----------
void EnviarTelemetria() {
  TelemetriaBarco t = {};

  bool fix = gps.location.isValid();
  t.fix = fix;
  t.lat = (float)smoothLat;
  t.lng = (float)smoothLng;
  t.speed = gps.speed.isValid() ? (float)gps.speed.kmph() : 0;
  t.course = courseValid ? (float)currentCourse : 0;
  t.alt = gps.altitude.isValid() ? (float)gps.altitude.meters() : 0;
  t.sats = gps.satellites.isValid() ? (uint8_t)gps.satellites.value() : 0;
  t.hdop = gps.hdop.isValid() ? (float)gps.hdop.hdop() : 99.9f;

  t.navState = (uint8_t)navState;
  t.cebo1Abierto = cebo1Abierto;
  t.cebo2Abierto = cebo2Abierto;
  t.cebo1Disparado = cebo1Disparado;
  t.cebo2Disparado = cebo2Disparado;

  t.totalDist = (float)totalDist;
  t.maxSpeed = (float)maxSpeed;
  t.avgSpeed = speedSamples > 0 ? (float)(sumSpeed / speedSamples) : 0;
  t.tripSecs = tripStartMs > 0 ? (millis() - tripStartMs) / 1000 : 0;

  t.homeLat = (float)home.lat;
  t.homeLng = (float)home.lng;
  t.homeValid = home.valid;
  t.c1Lat = (float)cebo1.lat;
  t.c1Lng = (float)cebo1.lng;
  t.c1Valid = cebo1.valid;
  t.c2Lat = (float)cebo2.lat;
  t.c2Lng = (float)cebo2.lng;
  t.c2Valid = cebo2.valid;

  t.smoothLat = (float)smoothLat;
  t.smoothLng = (float)smoothLng;

  if (gps.time.isValid()) {
    t.timeH = gps.time.hour();
    t.timeM = gps.time.minute();
    t.timeS = gps.time.second();
  }

  t.throttleMax = throttleMax;
  t.throttleMin = throttleMin;
  t.motorRunning = motorRunning;
  t.motorPctActual = motorPctActual;
  t.timonAngle = timonEnGrados;  // grados reales desde encoder
  t.invertirTimon = invertirTimon;
  t.trimTimon = (int)(trimTimon * 10.0f);
  t.targetBearing = (float)targetBearing;
  t.distProximidad = distProximidad;
  t.pausaMotor = pausaMotor;
  t.timonReferenciada = timonReferenciada;
  t.calibProgress = gy273CalibProgress;
  t.calibOK = gy273CalibOK;
  t.calibOffsetX = gy273OffsetX;
  t.calibOffsetY = gy273OffsetY;
  t.calibScaleX = gy273ScaleX;
  t.calibScaleY = gy273ScaleY;

  esp_now_send(macPlaya, (uint8_t*)&t, sizeof(TelemetriaBarco));
}
