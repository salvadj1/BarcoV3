#include "Servidor_Web.h"
#include <WebServer.h>
#include <DNSServer.h>
#include "ESPNow_Playa.h"

WebServer server(80);
DNSServer  dns;

const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Barco</title>
<style>
:root{
  --bg:#03080d;--panel:#071118;--border:#0d2d45;
  --accent:#00d4ff;--accent2:#00ff88;--accent3:#ff6b35;
  --warn:#ffcc00;--text:#c8e6f0;--dim:#3a6a84;
  --going:#00ff88;--ret:#ff6b35;--idle:#3a6a84;
  --red:#ff3b3b;--green:#00ff88;--map-bg:#010d14;
}
body.light{
  --bg:#f0f4f8;--panel:#ffffff;--border:#c8d8e8;
  --accent:#005f8a;--accent2:#006638;--accent3:#cc5500;
  --warn:#a06600;--text:#0d1a26;--dim:#4a6a80;
  --going:#00884a;--ret:#cc5500;--idle:#7a9ab0;
  --red:#aa1111;--green:#006638;--map-bg:#e8f0f8;
}
*{box-sizing:border-box;margin:0;padding:0;}
body{background:var(--bg);color:var(--text);font-family:'Courier New',monospace;font-size:.85rem;min-height:100vh;transition:background .3s,color .3s;}
.top-bar{display:flex;align-items:center;justify-content:space-between;padding:5px 12px;background:var(--panel);border-bottom:1px solid var(--border);}
.logo{color:var(--accent);font-size:.85rem;font-weight:700;letter-spacing:.12em;}
.top-right{display:flex;align-items:center;gap:7px;flex-wrap:nowrap;}
.gps-badge{display:flex;align-items:center;gap:4px;font-size:.72rem;}
.dot{width:7px;height:7px;border-radius:50%;background:var(--dim);flex-shrink:0;}
.dot.on{background:var(--accent2);box-shadow:0 0 6px var(--accent2);}
#gps-time{color:var(--dim);font-size:.7rem;}
.theme-btn{background:transparent;border:1px solid var(--border);border-radius:12px;padding:2px 8px;color:var(--dim);font-family:inherit;font-size:.7rem;cursor:pointer;white-space:nowrap;}
.theme-btn:hover{border-color:var(--accent);color:var(--accent);}
.link-status{font-size:.7rem;padding:2px 7px;border-radius:10px;white-space:nowrap;}
.link-ok{background:rgba(0,255,136,.1);color:var(--green);border:1px solid var(--green);}
.link-lost{background:rgba(255,59,59,.1);color:var(--red);border:1px solid var(--red);}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:7px;padding:7px;}
@media(max-width:480px){.grid{grid-template-columns:1fr;}}
.panel{background:var(--panel);border:1px solid var(--border);border-radius:6px;padding:10px;}
.panel-title{color:var(--dim);font-size:.68rem;letter-spacing:.1em;margin-bottom:8px;text-transform:uppercase;}
.coord-lbl{color:var(--dim);font-size:.78rem;}
.coord-val{color:var(--accent2);font-size:.85rem;font-weight:700;}
.stat-row{display:flex;justify-content:space-between;align-items:center;padding:4px 0;border-bottom:1px solid var(--border);}
.stat-lbl{color:var(--dim);font-size:.78rem;}
.trip-lbl{font-size:.85rem;font-weight:700;letter-spacing:.06em;margin-bottom:5px;}
.sat-grid{display:flex;flex-wrap:wrap;gap:3px;}
.sb{width:9px;height:9px;border-radius:2px;background:var(--border);}
.sb.on{background:var(--accent2);}
.compass-wrap{width:90px;height:90px;flex-shrink:0;}
.tele-wrap{display:flex;gap:10px;align-items:center;}
.tele-datos{flex:1;display:flex;flex-direction:column;gap:3px;}
.tele-row{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px solid var(--border);font-size:.82rem;}
.tele-lbl{color:var(--dim);}
.tele-val{font-weight:700;}
.btn{width:100%;padding:9px;margin-top:5px;background:transparent;border:1px solid var(--dim);border-radius:4px;color:var(--text);font-family:inherit;font-size:.8rem;letter-spacing:.07em;cursor:pointer;transition:.2s;}
.btn:hover{border-color:var(--accent);color:var(--accent);}
.btn:disabled{opacity:.3;cursor:default;}
.btn-go{border-color:var(--going);color:var(--going);}
.btn-stop-trip{border-color:var(--red);color:var(--red);}
.cebo-row{display:flex;gap:7px;margin-top:6px;}
.btn-cebo{flex:1;font-size:.72rem;padding:9px 6px;background:transparent;border-radius:4px;cursor:pointer;font-family:inherit;border:1px solid;}
.btn-cebo.cargado{color:var(--green);border-color:var(--green);}
.btn-cebo.descargado{color:var(--red);border-color:var(--red);}
.map-tabs{display:flex;gap:5px;margin-bottom:6px;}
.map-tab{flex:1;padding:4px;text-align:center;border:1px solid var(--border);border-radius:4px;cursor:pointer;font-size:.72rem;}
.map-tab.active{border-color:var(--accent);color:var(--accent);}
canvas{width:100%;display:block;border-radius:4px;background:var(--map-bg);}
.point-coord{color:var(--dim);font-size:.72rem;}

/* ===== CONTROL MANUAL ===== */
.ctrl-section{margin-top:4px;}
.ctrl-lbl-row{display:flex;justify-content:space-between;align-items:baseline;margin-bottom:4px;margin-top:10px;}
.ctrl-lbl{font-size:.7rem;color:var(--dim);letter-spacing:.08em;text-transform:uppercase;}
.ctrl-val{font-size:.95rem;font-weight:700;}
.ctrl-track{position:relative;width:100%;height:52px;background:var(--map-bg);border:1px solid var(--border);border-radius:26px;overflow:hidden;user-select:none;}
.ctrl-fill{position:absolute;left:0;top:0;height:100%;border-radius:26px;pointer-events:none;}
.ctrl-center{position:absolute;left:50%;top:9px;bottom:9px;width:1.5px;background:var(--border);pointer-events:none;}
.ctrl-knob{position:absolute;top:50%;width:46px;height:46px;border-radius:50%;transform:translate(-50%,-50%);cursor:grab;display:flex;align-items:center;justify-content:center;touch-action:none;}
.ctrl-knob:active{cursor:grabbing;}
.ctrl-knob-inner{width:33px;height:33px;border-radius:50%;border:2.5px solid currentColor;background:rgba(0,0,0,.15);transition:box-shadow .1s;}
.ctrl-knob.dragging .ctrl-knob-inner{box-shadow:0 0 16px currentColor;}
.ctrl-knob.sm{width:38px;height:38px;}
.ctrl-knob.sm .ctrl-knob-inner{width:27px;height:27px;}
.ctrl-track.sm{height:42px;}

/* Botones timon */
.timon-btn-wrap{display:flex;align-items:center;gap:8px;margin-top:4px;}
.timon-btn{flex:1;height:64px;background:var(--map-bg);border:2px solid var(--border);border-radius:8px;color:var(--accent);font-size:1.8rem;cursor:pointer;user-select:none;touch-action:none;display:flex;align-items:center;justify-content:center;transition:background .1s,border-color .1s;}
.timon-btn:active,.timon-btn.pressed{background:rgba(0,212,255,.18);border-color:var(--accent);}
.timon-center-display{flex:0 0 80px;text-align:center;}

/* Badge referencia timon */
.ref-badge{display:inline-block;font-size:.65rem;padding:2px 7px;border-radius:10px;margin-left:6px;}
.ref-ok{background:rgba(0,255,136,.1);color:var(--green);border:1px solid var(--green);}
.ref-nok{background:rgba(255,59,59,.1);color:var(--red);border:1px solid var(--red);}

.ctrl-stop{width:100%;margin-top:10px;padding:10px;background:rgba(255,59,59,.08);border:1.5px solid var(--red);border-radius:5px;color:var(--red);font-family:inherit;font-size:.8rem;letter-spacing:.1em;cursor:pointer;}
.ctrl-stop:active{background:rgba(255,59,59,.22);}
</style>
</head>
<body>

<div class="top-bar">
  <span class="logo">&#9875; BARCO</span>
  <div class="top-right">
    <button class="theme-btn" id="theme-btn" onclick="toggleTheme()">&#9788; CLARO</button>
    <span id="calib-progress-badge" style="display:none;font-size:.7rem;padding:2px 8px;border-radius:10px;background:rgba(255,107,53,.1);color:var(--accent3);border:1px solid var(--accent3);">CAL 0%</span>
    <span id="link-badge" class="link-status link-lost">SIN SE&#209;AL</span>
    <div class="gps-badge">
      <div class="dot" id="gps-dot"></div>
      <span id="gps-txt">SIN FIX</span>
    </div>
    <span id="gps-time">--:--:--</span>
  </div>
</div>

<div class="grid">

  <!-- 1. POSICION GPS -->
  <div class="panel">
    <div class="panel-title">&#128225; Posici&#243;n GPS</div>
    <div style="display:flex;justify-content:space-between;margin-bottom:5px;">
      <span><span class="coord-lbl">LAT </span><span class="coord-val" id="lat-val">--</span></span>
      <span><span class="coord-lbl">LNG </span><span class="coord-val" id="lng-val">--</span></span>
    </div>
    <div style="display:flex;align-items:center;gap:8px;margin-top:8px;">
      <span class="coord-lbl">SATS&nbsp;<span id="sat-num" class="coord-val">0</span></span>
      <div class="sat-grid" style="flex:1;">
        <div class="sb" id="sb0"></div><div class="sb" id="sb1"></div><div class="sb" id="sb2"></div>
        <div class="sb" id="sb3"></div><div class="sb" id="sb4"></div><div class="sb" id="sb5"></div>
        <div class="sb" id="sb6"></div><div class="sb" id="sb7"></div><div class="sb" id="sb8"></div>
        <div class="sb" id="sb9"></div><div class="sb" id="sb10"></div><div class="sb" id="sb11"></div>
      </div>
      <span class="coord-lbl">HDOP&nbsp;<span id="hdop-val" class="coord-val">--</span></span>
    </div>
  </div>

  <!-- 2. TELEMETRIA -->
  <div class="panel">
    <div class="panel-title" style="display:flex;justify-content:space-between;align-items:center;">&#128268; Telemetr&#237;a<button onclick="window.location='/calib'" style="background:transparent;border:1px solid var(--border);border-radius:4px;color:var(--dim);font-family:inherit;font-size:.65rem;padding:2px 8px;cursor:pointer;letter-spacing:.05em;" id="cal-btn">CAL</button></div>
    <div class="tele-wrap">
      <div class="tele-datos">
        <div class="tele-row"><span class="tele-lbl">RUMBO</span><span class="tele-val" style="color:var(--accent);"><span id="rumbo-deg">0</span>&#176; <span id="rumbo-cardinal">N</span></span></div>
        <div class="tele-row"><span class="tele-lbl">VELOC.</span><span class="tele-val" style="color:var(--accent2);"><span id="tele-speed">0</span> km/h</span></div>
        <div class="tele-row"><span class="tele-lbl">ALTIT.</span><span class="tele-val" style="color:var(--accent2);"><span id="tele-alt">0</span> m</span></div>
        <div class="tele-row"><span class="tele-lbl">MOTOR</span><span class="tele-val" style="color:var(--accent2);" id="motor-pct">0%</span></div>
        <div class="tele-row" style="border-bottom:none;"><span class="tele-lbl">TIM&#211;N</span><span class="tele-val" style="color:var(--accent);" id="timon-val">--&#176;</span></div>
      </div>
      <div class="compass-wrap" style="width:126px;height:126px;">
        <svg viewBox="0 0 90 90" width="126" height="126">
          <circle cx="45" cy="45" r="42" fill="var(--map-bg)" stroke="var(--border)" stroke-width="1.5"/>
          <text x="45" y="12" text-anchor="middle" fill="var(--dim)" font-size="8" font-family="Courier New">N</text>
          <text x="45" y="85" text-anchor="middle" fill="var(--dim)" font-size="8" font-family="Courier New">S</text>
          <text x="82" y="49" text-anchor="middle" fill="var(--dim)" font-size="8" font-family="Courier New">E</text>
          <text x="8" y="49" text-anchor="middle" fill="var(--dim)" font-size="8" font-family="Courier New">O</text>
          <g id="dest-needle" transform="rotate(0,45,45)" opacity="0">
            <polygon points="45,16 43,45 47,45" fill="var(--accent3)"/>
          </g>
          <g id="compass-needle" transform="rotate(0,45,45)">
            <polygon points="45,14 42,45 48,45" fill="var(--accent)"/>
            <polygon points="45,76 42,45 48,45" fill="var(--dim)"/>
          </g>
          <circle cx="45" cy="45" r="3" fill="var(--border)"/>
          <rect x="2" y="78" width="7" height="4" fill="var(--accent)" rx="1"/>
          <text x="11" y="82" font-size="5" fill="var(--dim)" font-family="Courier New">RUMBO</text>
          <rect x="2" y="84" width="7" height="4" fill="var(--accent3)" rx="1"/>
          <text x="11" y="88" font-size="5" fill="var(--dim)" font-family="Courier New">DESTINO</text>
        </svg>
      </div>
    </div>
  </div>

  <!-- 3. CONTROL MANUAL -->
  <div class="panel" style="grid-column:1/-1;">
    <div class="panel-title">&#127917; Control Manual <span id="joy-mode-badge" style="font-size:.68rem;color:var(--dim);">(solo en IDLE)</span></div>
    <div class="ctrl-section">

      <!-- MOTOR -->
      <div class="ctrl-lbl-row">
        <span class="ctrl-lbl">MOTOR</span>
        <span class="ctrl-val" id="motor-ctrl-val" style="color:#00ff88;">0%</span>
      </div>
      <div class="ctrl-track" id="motor-track">
        <div class="ctrl-fill" id="motor-fill" style="width:0%;background:#00ff88;opacity:.2;"></div>
        <div class="ctrl-knob" id="motor-knob" style="color:#00ff88;">
          <div class="ctrl-knob-inner"></div>
        </div>
      </div>

      <!-- TIMON -->
      <div class="ctrl-lbl-row">
        <span class="ctrl-lbl">
          TIM&#211;N
          <span id="ref-badge" class="ref-badge ref-nok">SIN REF</span>
        </span>
        <span style="display:flex;align-items:center;gap:8px;">
          <label style="font-size:.7rem;color:var(--dim);display:flex;align-items:center;gap:4px;cursor:pointer;">
            <input type="checkbox" id="chk-invert-timon" onchange="toggleInvert(this.checked)" style="accent-color:var(--accent3);width:14px;height:14px;">
            INVERTIR
          </label>
          <span class="ctrl-val" id="timon-ctrl-val" style="color:var(--accent);">--&#176;</span>
        </span>
      </div>

      <!-- Botones timon: SIMPLE, sin acumulacion -->
      <div class="timon-btn-wrap">
        <button class="timon-btn" id="timon-btn-izq"
          ontouchstart="timonPress(-1);event.preventDefault()"
          ontouchend="timonRelease();event.preventDefault()"
          onmousedown="timonPress(-1)"
          onmouseup="timonRelease()"
          onmouseleave="timonRelease()">&#9664;</button>

        <div class="timon-center-display">
          <div style="font-size:.6rem;color:var(--dim);letter-spacing:.08em;">POSICION</div>
          <div style="font-size:1rem;font-weight:700;color:var(--accent);" id="timon-pos-val">--&#176;</div>
          <!-- CTR: centra fisicamente Y fija referencia -->
          <button onclick="centerTimon()" style="background:transparent;border:1px solid var(--accent2);border-radius:3px;color:var(--accent2);font-family:inherit;font-size:.6rem;padding:2px 7px;cursor:pointer;margin-top:4px;">&#8226; CTR</button>
        </div>

        <button class="timon-btn" id="timon-btn-der"
          ontouchstart="timonPress(1);event.preventDefault()"
          ontouchend="timonRelease();event.preventDefault()"
          onmousedown="timonPress(1)"
          onmouseup="timonRelease()"
          onmouseleave="timonRelease()">&#9654;</button>
      </div>

      <!-- TRIM -->
      <div class="ctrl-lbl-row">
        <span class="ctrl-lbl">&#9668; TRIM &#9658;</span>
        <span class="ctrl-val" id="trim-ctrl-val" style="color:var(--accent3);">0.0&#176;</span>
      </div>
      <div class="ctrl-track sm" id="trim-track">
        <div class="ctrl-center"></div>
        <div class="ctrl-knob sm" id="trim-knob" style="color:var(--accent3);">
          <div class="ctrl-knob-inner"></div>
        </div>
      </div>
      <div style="display:flex;justify-content:flex-end;margin-top:3px;">
        <button onclick="resetTrim()" style="background:transparent;border:1px solid var(--border);border-radius:4px;color:var(--dim);font-family:inherit;font-size:.7rem;padding:3px 10px;cursor:pointer;">&#8635; CENTRAR TRIM</button>
      </div>

      <button class="ctrl-stop" onclick="joyStop()">&#9632; STOP MOTOR</button>

      <!-- CEBOS -->
      <div class="cebo-row">
        <button class="btn-cebo cargado" id="btn-cebo1" onclick="toggleCebo(1)">&#9679; CEBO 1<br><span style="font-size:.65rem" id="cebo1-st">CARGADO</span></button>
        <button class="btn-cebo cargado" id="btn-cebo2" onclick="toggleCebo(2)">&#9679; CEBO 2<br><span style="font-size:.65rem" id="cebo2-st">CARGADO</span></button>
      </div>
    </div>
  </div>

  <!-- 4. VIAJE AUTONOMO -->
  <div class="panel" style="grid-column:1/-1;">
    <div class="panel-title">&#128674; Viaje Aut&#243;nomo</div>
    <div class="trip-lbl" id="trip-lbl" style="color:var(--idle);">EN ESPERA</div>
    <div id="dist-val" style="display:none;"></div>
    <div id="dist-u" style="display:none;"></div>
    <div class="stat-row"><span class="stat-lbl">ETA</span><span id="eta-val">--:--</span></div>
    <div class="stat-row" style="align-items:center;">
      <span class="stat-lbl">VEL M&#193;X</span>
      <div style="display:flex;align-items:center;gap:5px;">
        <input type="range" id="thr-slider" min="0" max="100" value="50" oninput="setThrottle(this.value)" style="width:80px;accent-color:var(--accent);">
        <span id="thr-val" style="color:var(--accent);font-weight:700;min-width:26px;">50%</span>
      </div>
    </div>
    <div class="stat-row" style="align-items:center;">
      <span class="stat-lbl">VEL M&#205;N</span>
      <div style="display:flex;align-items:center;gap:5px;">
        <input type="range" id="thrmin-slider" min="0" max="50" value="6" oninput="setThrottleMin(this.value)" style="width:80px;accent-color:var(--warn);">
        <span id="thrmin-val" style="color:var(--warn);font-weight:700;min-width:26px;">6%</span>
      </div>
    </div>
    <div class="stat-row" style="align-items:center;">
      <span class="stat-lbl">PROXIMIDAD</span>
      <div style="display:flex;align-items:center;gap:5px;">
        <input type="range" id="prox-slider" min="1" max="30" value="3" oninput="setProximidad(this.value)" style="width:80px;accent-color:var(--accent2);">
        <span id="prox-val" style="color:var(--accent2);font-weight:700;min-width:26px;">3m</span>
      </div>
    </div>
    <div class="stat-row" style="align-items:center;">
      <span class="stat-lbl">PAUSA</span>
      <div style="display:flex;align-items:center;gap:5px;">
        <input type="range" id="pausa-slider" min="0" max="30" value="0" oninput="setPausa(this.value)" style="width:80px;accent-color:var(--accent3);">
        <span id="pausa-val" style="color:var(--accent3);font-weight:700;min-width:26px;">0s</span>
      </div>
    </div>
    <button class="btn btn-go" id="btn-trip" onclick="startTrip()">&#9654; INICIAR VIAJE</button>
    <button class="btn btn-stop-trip" id="btn-stop" onclick="stopTrip()" disabled>&#9632; DETENER</button>
    <div style="margin-top:8px;border-top:1px solid var(--border);padding-top:6px;">
      <div class="stat-row"><span class="stat-lbl">DISTANCIA</span><span id="stat-dist">0</span> m</div>
      <div class="stat-row"><span class="stat-lbl">VEL MAX</span><span id="stat-maxspd">0.0</span> km/h</div>
      <div class="stat-row"><span class="stat-lbl">VEL MED</span><span id="stat-avgspd">0.0</span> km/h</div>
      <div class="stat-row" style="border-bottom:none;"><span class="stat-lbl">TIEMPO</span><span id="stat-time">00:00</span></div>
    </div>
  </div>

  <!-- 5. MAPA -->
  <div class="panel" style="grid-column:1/-1;">
    <div class="panel-title">&#128506; Mapa</div>
    <div class="map-tabs">
      <div class="map-tab active" id="tab0" onclick="switchTab(0)"><span class="zona-name" id="zona-name-0" ondblclick="editZonaName(0)">ZONA 1</span></div>
      <div class="map-tab" id="tab1" onclick="switchTab(1)"><span class="zona-name" id="zona-name-1" ondblclick="editZonaName(1)">ZONA 2</span></div>
      <div class="map-tab" id="tab2" onclick="switchTab(2)"><span class="zona-name" id="zona-name-2" ondblclick="editZonaName(2)">ZONA 3</span></div>
    </div>
    <canvas id="map-canvas"></canvas>
  </div>

  <!-- 6. PUNTOS -->
  <div class="panel" style="grid-column:1/-1;">
    <div class="panel-title">&#128205; Puntos &mdash; <span id="zona-lbl" style="color:var(--accent);">ZONA 1</span></div>
    <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:7px;">
      <div><div class="stat-lbl" style="font-size:.68rem;">&#127968; HOME</div><div class="point-coord" id="h-coord">--</div></div>
      <div><div class="stat-lbl" style="font-size:.68rem;">&#127919; CEBO 1</div><div class="point-coord" id="c1-coord">--</div></div>
      <div><div class="stat-lbl" style="font-size:.68rem;">&#127919; CEBO 2</div><div class="point-coord" id="c2-coord">--</div></div>
    </div>
    <div style="display:flex;gap:7px;">
      <button class="btn" style="border-color:var(--accent);color:var(--accent);margin-top:0;" onclick="savePoint('home')">&#9632; Home</button>
      <button class="btn" style="border-color:#a78bfa;color:#a78bfa;margin-top:0;" onclick="savePoint('cebo1')">&#9711; Cebo 1</button>
      <button class="btn" style="border-color:#fb923c;color:#fb923c;margin-top:0;" onclick="savePoint('cebo2')">&#9711; Cebo 2</button>
    </div>
  </div>

</div>

<script>
const canvas=document.getElementById('map-canvas');
const ctx=canvas.getContext('2d');
let mapTab=0;
let live={history:[],home:null,c1:null,c2:null,pos:null};

function toggleTheme(){
  const l=document.body.classList.toggle('light');
  localStorage.setItem('tema',l?'light':'dark');
  document.getElementById('theme-btn').textContent=l?'\u263C OSCURO':'\u263C CLARO';
  drawMap();
}
(function(){if(localStorage.getItem('tema')==='light'){document.body.classList.add('light');document.getElementById('theme-btn').textContent='\u263C OSCURO';}})();

function degToCardinal(d){return['N','NE','E','SE','S','SO','O','NO'][Math.round(d/45)%8];}

function zonaNameKey(i){return 'zona_nombre_'+i;}
function loadZonaNames(){
  for(let i=0;i<3;i++){
    const n=localStorage.getItem(zonaNameKey(i))||(i===0?'ZONA 1':i===1?'ZONA 2':'ZONA 3');
    document.getElementById('zona-name-'+i).textContent=n;
  }
}
function editZonaName(i){
  event.stopPropagation();
  const span=document.getElementById('zona-name-'+i);
  const old=span.textContent;
  const inp=document.createElement('input');
  inp.value=old;
  inp.style.cssText='background:transparent;border:none;border-bottom:1px solid var(--accent);color:var(--accent);font-family:inherit;font-size:.72rem;width:60px;outline:none;text-align:center;';
  span.replaceWith(inp);
  inp.focus();inp.select();
  function guardar(){
    const val=(inp.value.trim().toUpperCase()||old).substring(0,12);
    localStorage.setItem(zonaNameKey(i),val);
    const sp=document.createElement('span');
    sp.className='zona-name';sp.id='zona-name-'+i;
    sp.title='Doble clic para renombrar';sp.textContent=val;
    sp.ondblclick=()=>editZonaName(i);
    inp.replaceWith(sp);
    if(i===mapTab)document.getElementById('zona-lbl').textContent=val;
  }
  inp.addEventListener('blur',guardar);
  inp.addEventListener('keydown',e=>{if(e.key==='Enter')inp.blur();if(e.key==='Escape'){inp.value=old;inp.blur();}});
}
loadZonaNames();

function presetKey(zona,punto){return 'zona'+zona+'_'+punto;}
function loadPreset(zona){
  const h =JSON.parse(localStorage.getItem(presetKey(zona,'home'))||'null');
  const c1=JSON.parse(localStorage.getItem(presetKey(zona,'cebo1'))||'null');
  const c2=JSON.parse(localStorage.getItem(presetKey(zona,'cebo2'))||'null');
  document.getElementById('h-coord').textContent =h ?h.lat.toFixed(5)+', '+h.lng.toFixed(5):'--';
  document.getElementById('c1-coord').textContent=c1?c1.lat.toFixed(5)+', '+c1.lng.toFixed(5):'--';
  document.getElementById('c2-coord').textContent=c2?c2.lat.toFixed(5)+', '+c2.lng.toFixed(5):'--';
  live.home=h;live.c1=c1;live.c2=c2;
  const nombre=localStorage.getItem(zonaNameKey(zona))||('ZONA '+(zona+1));
  document.getElementById('zona-lbl').textContent=nombre;
  drawMap();
}
function savePoint(p){
  if(!live.pos){alert('Sin posicion GPS');return;}
  const pt={lat:live.pos.lat,lng:live.pos.lng};
  localStorage.setItem(presetKey(mapTab,p),JSON.stringify(pt));
  fetch('/save?p='+p).then(r=>r.json()).then(d=>{
    if(!d.ok)alert('Sin fix GPS en el barco');
    loadPreset(mapTab);
  });
}
function switchTab(t){
  mapTab=t;
  ['tab0','tab1','tab2'].forEach((id,i)=>document.getElementById(id).className='map-tab'+(i===t?' active':''));
  loadPreset(t);
}

// ===== MAPA =====
function latToM(dLat){return dLat*111320;}
function lngToM(dLng,lat){return dLng*111320*Math.cos(lat*Math.PI/180);}
function gridStep(rangeM){
  const steps=[1,2,5,10,25,50,100,250,500,1000,2500,5000];
  const target=rangeM/5;
  for(const s of steps){if(s>=target)return s;}
  return steps[steps.length-1];
}
function speedColor(spd,maxSpd){
  if(!maxSpd||maxSpd===0)return '#00ff88';
  const t=Math.min(spd/maxSpd,1);
  let r,g,b;
  if(t<0.5){const u=t*2;r=0;g=Math.round(136+u*119);b=Math.round(255-u*255);}
  else{const u=(t-0.5)*2;r=Math.round(u*255);g=Math.round(255+u*(59-255));b=0;}
  return`rgb(${r},${g},${b})`;
}
function drawMap(){
  const W=canvas.width,H=canvas.height;
  ctx.clearRect(0,0,W,H);
  const lgt=document.body.classList.contains('light');
  ctx.fillStyle=lgt?'#e8f0f8':'#010d14';ctx.fillRect(0,0,W,H);
  const {history:hist,home,c1,c2,pos}=live;
  if(!pos){ctx.fillStyle=lgt?'#c8d8e8':'#0d2d45';ctx.font='13px Courier New';ctx.textAlign='center';ctx.fillText('Sin posicion GPS',W/2,H/2);return;}
  const centerLat=pos.lat,centerLng=pos.lng;
  const pad=44,usableW=W-pad*2,usableH=H-pad*2;
  const extraPts=[];
  if(home)extraPts.push(home);if(c1)extraPts.push(c1);if(c2)extraPts.push(c2);
  hist.forEach(p=>extraPts.push(p));
  let maxDLat=0.0001,maxDLng=0.0001;
  extraPts.forEach(p=>{maxDLat=Math.max(maxDLat,Math.abs(p.lat-centerLat));maxDLng=Math.max(maxDLng,Math.abs(p.lng-centerLng));});
  maxDLat*=1.25;maxDLng*=1.25;
  const scLat=usableH/(2*maxDLat),scLng=usableW/(2*maxDLng),sc=Math.min(scLat,scLng);
  function xy(la,ln){return{x:W/2+(ln-centerLng)*sc,y:H/2-(la-centerLat)*sc};}
  const rLa=maxDLat*2,mPerPx=111320/sc,rangeM=latToM(rLa),step=gridStep(rangeM),stepDeg=step/111320;
  const mnLa=centerLat-maxDLat,mxLa=centerLat+maxDLat,mnLn=centerLng-maxDLng,mxLn=centerLng+maxDLng;
  const goLa=Math.floor(mnLa/stepDeg)*stepDeg,goLn=Math.floor(mnLn/stepDeg)*stepDeg;
  ctx.lineWidth=0.5;ctx.font='8px Courier New';ctx.textAlign='right';
  for(let la=goLa;la<=mxLa+stepDeg;la+=stepDeg){const q=xy(la,centerLng);if(q.y<pad||q.y>H-pad+2)continue;ctx.strokeStyle=lgt?'rgba(255,255,255,.7)':'rgba(255,255,255,.2)';ctx.beginPath();ctx.moveTo(pad,q.y);ctx.lineTo(W-pad,q.y);ctx.stroke();const distM=Math.round(latToM(la-centerLat));ctx.fillStyle=lgt?'#444':'#aaa';ctx.fillText((distM>=0?'+':'')+distM+'m',pad-2,q.y+3);}
  ctx.textAlign='center';
  for(let ln=goLn;ln<=mxLn+stepDeg;ln+=stepDeg){const q=xy(centerLat,ln);if(q.x<pad||q.x>W-pad+2)continue;ctx.strokeStyle=lgt?'rgba(255,255,255,.7)':'rgba(255,255,255,.2)';ctx.beginPath();ctx.moveTo(q.x,pad);ctx.lineTo(q.x,H-pad);ctx.stroke();const distM=Math.round(lngToM(ln-centerLng,centerLat));ctx.fillStyle=lgt?'#444':'#aaa';ctx.fillText((distM>=0?'+':'')+distM+'m',q.x,H-pad+12);}
  const scalePx=step/mPerPx,sx=pad,sy=H-14;
  ctx.strokeStyle=lgt?'#666':'#aaa';ctx.lineWidth=2;
  ctx.beginPath();ctx.moveTo(sx,sy);ctx.lineTo(sx+scalePx,sy);ctx.stroke();
  ctx.beginPath();ctx.moveTo(sx,sy-4);ctx.lineTo(sx,sy+4);ctx.stroke();
  ctx.beginPath();ctx.moveTo(sx+scalePx,sy-4);ctx.lineTo(sx+scalePx,sy+4);ctx.stroke();
  ctx.fillStyle=lgt?'#555':'#bbb';ctx.font='9px Courier New';ctx.textAlign='left';
  ctx.fillText(step>=1000?(step/1000)+'km':step+'m',sx+scalePx+4,sy+3);
  if(hist.length>1){for(let i=1;i<hist.length;i++){const a=xy(hist[i-1].lat,hist[i-1].lng),b=xy(hist[i].lat,hist[i].lng);ctx.strokeStyle=speedColor(i/hist.length,1);ctx.lineWidth=3;ctx.shadowColor=ctx.strokeStyle;ctx.shadowBlur=4;ctx.beginPath();ctx.moveTo(a.x,a.y);ctx.lineTo(b.x,b.y);ctx.stroke();}ctx.shadowBlur=0;}
  function marker(p,col,lbl){const q=xy(p.lat,p.lng);ctx.beginPath();ctx.arc(q.x,q.y,7,0,Math.PI*2);ctx.fillStyle=col;ctx.shadowColor=col;ctx.shadowBlur=10;ctx.fill();ctx.shadowBlur=0;ctx.fillStyle='#fff';ctx.font='bold 9px Courier New';ctx.textAlign='center';ctx.fillText(lbl,q.x,q.y+3);}
  if(home)marker(home,'#ff6b35','H');if(c1)marker(c1,'#a78bfa','1');if(c2)marker(c2,'#fb923c','2');
  const qp=xy(pos.lat,pos.lng),course=(pos.course||0)*Math.PI/180,ar=14;
  ctx.strokeStyle=lgt?'rgba(0,0,0,.15)':'rgba(255,255,255,.12)';ctx.lineWidth=1;
  ctx.beginPath();ctx.moveTo(W/2-20,H/2);ctx.lineTo(W/2+20,H/2);ctx.stroke();
  ctx.beginPath();ctx.moveTo(W/2,H/2-20);ctx.lineTo(W/2,H/2+20);ctx.stroke();
  ctx.beginPath();ctx.arc(qp.x,qp.y,7,0,Math.PI*2);ctx.fillStyle=lgt?'#005f8a':'#00d4ff';ctx.shadowColor=lgt?'#005f8a':'#00d4ff';ctx.shadowBlur=14;ctx.fill();ctx.shadowBlur=0;
  const fx=qp.x+Math.sin(course)*ar,fy=qp.y-Math.cos(course)*ar;
  ctx.strokeStyle='#fff';ctx.lineWidth=2.5;ctx.shadowColor='#fff';ctx.shadowBlur=6;
  ctx.beginPath();ctx.moveTo(qp.x,qp.y);ctx.lineTo(fx,fy);ctx.stroke();
  const al=6,aa=0.45;ctx.beginPath();ctx.moveTo(fx,fy);ctx.lineTo(fx-Math.sin(course-aa)*al,fy+Math.cos(course-aa)*al);ctx.lineTo(fx-Math.sin(course+aa)*al,fy+Math.cos(course+aa)*al);ctx.closePath();ctx.fillStyle='#fff';ctx.fill();ctx.shadowBlur=0;
}
function resize(){canvas.width=canvas.offsetWidth;canvas.height=290;drawMap();}
window.addEventListener('resize',resize);resize();
loadPreset(0);

// ===== VIAJE =====
function startTrip(){fetch('/startTrip').then(r=>r.json()).then(d=>{if(!d.ok)alert(d.msg);});}
function stopTrip(){fetch('/stopTrip').then(r=>r.json());}
function setThrottle(v){document.getElementById('thr-val').textContent=v+'%';fetch('/throttle?v='+v);}
function setThrottleMin(v){document.getElementById('thrmin-val').textContent=v+'%';fetch('/throttleMin?v='+v);}
function setProximidad(v){document.getElementById('prox-val').textContent=v+'m';fetch('/proximidad?v='+v);}
function setPausa(v){document.getElementById('pausa-val').textContent=v+'s';fetch('/pausa?v='+v);}
function toggleCebo(n){fetch('/cebo?n='+n);}
function toggleInvert(v){fetch('/invertTimon?v='+(v?1:0));}

// ===== TIMON: LOGICA SIMPLE =====
// Mantener pulsado -> envia dirección (-1 o +1) a 10Hz
// Soltar -> envia 0 (vuelve al centro)
let timonDir=0;
let timonInterval=null;
let motorPctActual=0; // actualizado por poll

function timonPress(dir){
  if(timonDir===dir)return;
  timonDir=dir;
  document.getElementById(dir<0?'timon-btn-izq':'timon-btn-der').classList.add('pressed');
  // Enviar inmediatamente
  fetch('/joystick?t='+motorPctActual+'&s='+dir);
  // Y repetir a 10Hz mientras se mantiene
  if(timonInterval)clearInterval(timonInterval);
  timonInterval=setInterval(()=>{
    fetch('/joystick?t='+motorPctActual+'&s='+dir);
  },100);
}

function timonRelease(){
  if(timonDir===0)return;
  timonDir=0;
  if(timonInterval){clearInterval(timonInterval);timonInterval=null;}
  document.getElementById('timon-btn-izq').classList.remove('pressed');
  document.getElementById('timon-btn-der').classList.remove('pressed');
  fetch('/joystick?t='+motorPctActual+'&s=0');
}

// CTR: centra fisicamente el timon Y fija referencia de encoder
function centerTimon(){
  fetch('/centerTimon').then(r=>r.json()).then(d=>{
    if(d.ok) console.log('Referencia timon reseteada');
  });
}

function joyStop(){
  timonRelease();
  fetch('/joystick?t=0&s=0');
}

// ===== COLOR MOTOR =====
function motorColor(p){
  let r,g,b;
  if(p<=50){const t=p/50;r=Math.round(t*255);g=Math.round(255+t*(204-255));b=Math.round(136+t*(-136));}
  else{const t=(p-50)/50;r=255;g=Math.round(204+t*(59-204));b=Math.round(t*59);}
  return`rgb(${r},${g},${b})`;
}

// ===== CONTROL MOTOR (slider) =====
const KW=46,KW_SM=38;
(function(){
  const track=document.getElementById('motor-track');
  const knob=document.getElementById('motor-knob');
  const kw=KW;
  let pct=0,dragging=false,activeTouchId=null,lastSend=0;
  function setPct(p,render){
    pct=Math.max(0,Math.min(100,p));
    motorPctActual=Math.round(pct);
    const px=pct/100*(track.offsetWidth-kw)+kw/2;
    knob.style.left=px+'px';
    const col=motorColor(pct);
    document.getElementById('motor-ctrl-val').textContent=Math.round(pct)+'%';
    document.getElementById('motor-ctrl-val').style.color=col;
    knob.style.color=col;
    document.getElementById('motor-fill').style.width=pct+'%';
    document.getElementById('motor-fill').style.background=col;
    knob.querySelector('.ctrl-knob-inner').style.boxShadow=pct>0?`0 0 14px ${col}`:'none';
    if(!render){const now=Date.now();if(now-lastSend<80)return;lastSend=now;fetch('/joystick?t='+Math.round(pct)+'&s='+timonDir);}
  }
  function pctFromEvent(e){
    const rect=track.getBoundingClientRect();
    let touch=e;
    if(e.touches){for(let t of(e.changedTouches||e.touches)){if(t.identifier===activeTouchId){touch=t;break;}}if(touch===e)touch=e.touches[0]||e.changedTouches[0];}
    const x=Math.max(kw/2,Math.min(rect.width-kw/2,touch.clientX-rect.left));
    return(x-kw/2)/(rect.width-kw)*100;
  }
  function startDrag(e){e.preventDefault();if(e.touches)activeTouchId=e.touches[0].identifier;dragging=true;knob.classList.add('dragging');setPct(pctFromEvent(e),false);}
  function moveDrag(e){if(!dragging)return;e.preventDefault();if(e.touches){let f=null;for(let t of e.touches){if(t.identifier===activeTouchId){f=t;break;}}if(!f)return;}setPct(pctFromEvent(e),false);}
  function endDrag(e){if(!dragging)return;if(e&&e.type&&e.type.startsWith('touch')){const ts=e.changedTouches||[];let f=false;for(let t of ts){if(t.identifier===activeTouchId)f=true;}if(!f)return;}dragging=false;activeTouchId=null;knob.classList.remove('dragging');}
  knob.addEventListener('mousedown',startDrag);document.addEventListener('mousemove',moveDrag);document.addEventListener('mouseup',endDrag);
  knob.addEventListener('touchstart',startDrag,{passive:false});document.addEventListener('touchmove',moveDrag,{passive:false});
  document.addEventListener('touchend',endDrag);document.addEventListener('touchcancel',endDrag);
  requestAnimationFrame(()=>setPct(0,true));
})();

// ===== TRIM =====
const trimCtrl=(function(){
  const track=document.getElementById('trim-track');
  const knob=document.getElementById('trim-knob');
  const kw=KW_SM;
  let pct=0,dragging=false,activeTouchId=null,lastSend=0;
  function setPct(p,render){
    pct=Math.max(-100,Math.min(100,p));
    const norm=(pct+100)/200;
    const px=norm*(track.offsetWidth-kw)+kw/2;
    knob.style.left=px+'px';
    const deg=(pct*15/100).toFixed(1);
    document.getElementById('trim-ctrl-val').textContent=(parseFloat(deg)>=0?'+':'')+deg+'\u00b0';
    knob.querySelector('.ctrl-knob-inner').style.boxShadow=pct!==0?'0 0 10px var(--accent3)':'none';
    if(!render){const now=Date.now();if(now-lastSend<100)return;lastSend=now;fetch('/trim?v='+Math.round(pct*1.5));}
  }
  function pctFromEvent(e){
    const rect=track.getBoundingClientRect();
    let touch=e;
    if(e.touches){for(let t of(e.changedTouches||e.touches)){if(t.identifier===activeTouchId){touch=t;break;}}if(touch===e)touch=e.touches[0]||e.changedTouches[0];}
    const x=Math.max(kw/2,Math.min(rect.width-kw/2,touch.clientX-rect.left));
    return((x-kw/2)/(rect.width-kw))*200-100;
  }
  function startDrag(e){e.preventDefault();if(e.touches)activeTouchId=e.touches[0].identifier;dragging=true;knob.classList.add('dragging');setPct(pctFromEvent(e),false);}
  function moveDrag(e){if(!dragging)return;e.preventDefault();if(e.touches){let f=null;for(let t of e.touches){if(t.identifier===activeTouchId){f=t;break;}}if(!f)return;}setPct(pctFromEvent(e),false);}
  function endDrag(e){if(!dragging)return;if(e&&e.type&&e.type.startsWith('touch')){const ts=e.changedTouches||[];let f=false;for(let t of ts){if(t.identifier===activeTouchId)f=true;}if(!f)return;}dragging=false;activeTouchId=null;knob.classList.remove('dragging');}
  knob.addEventListener('mousedown',startDrag);document.addEventListener('mousemove',moveDrag);document.addEventListener('mouseup',endDrag);
  knob.addEventListener('touchstart',startDrag,{passive:false});document.addEventListener('touchmove',moveDrag,{passive:false});
  document.addEventListener('touchend',endDrag);document.addEventListener('touchcancel',endDrag);
  requestAnimationFrame(()=>setPct(0,true));
  return{get dragging(){return dragging;},set:(p,r=true)=>setPct(p,r)};
})();
window.resetTrim=function(){trimCtrl.set(0);fetch('/trim?v=0');};

// ===== ESTADOS VIAJE =====
const scol={IDLE:'var(--idle)',GOING_CEBO1:'var(--going)',GOING_CEBO2:'var(--going)',RETURNING:'var(--ret)',ARRIVED:'var(--warn)'};
const slbl={IDLE:'EN ESPERA',GOING_CEBO1:'\u2191 HACIA CEBO 1',GOING_CEBO2:'\u2191 HACIA CEBO 2',RETURNING:'\u2193 REGRESANDO',ARRIVED:'\u2713 LLEGADO'};
function fmt(s){return String(Math.floor(s/60)).padStart(2,'0')+':'+String(s%60).padStart(2,'0');}
function fmtETA(s){return(!s||s>86400)?'--:--':fmt(Math.round(s));}

// ===== POLL =====
function poll(){
  fetch('/data').then(r=>r.json()).then(d=>{
    const badge=document.getElementById('link-badge');
    if(d.linkOk){badge.className='link-status link-ok';badge.textContent='ENLACE OK';}
    else{badge.className='link-status link-lost';badge.textContent='SIN SE\u00d1AL';}
    const dot=document.getElementById('gps-dot');
    if(d.fix){dot.className='dot on';document.getElementById('gps-txt').textContent='GPS FIX';}
    else{dot.className='dot';document.getElementById('gps-txt').textContent='SIN FIX';}
    if(d.timeStr)document.getElementById('gps-time').textContent=d.timeStr;
    document.getElementById('lat-val').textContent=d.lat.toFixed(6);
    document.getElementById('lng-val').textContent=d.lng.toFixed(6);
    document.getElementById('sat-num').textContent=d.sats;
    document.getElementById('hdop-val').textContent=(d.hdop||0).toFixed(1);
    const course=d.course||0;
    document.getElementById('compass-needle').setAttribute('transform','rotate('+course+',45,45)');
    const destNeedle=document.getElementById('dest-needle');
    const inTripForDest=(d.navState!=='IDLE'&&d.navState!=='ARRIVED');
    if(inTripForDest&&d.targetBearing!==undefined){destNeedle.setAttribute('transform','rotate('+d.targetBearing+',45,45)');destNeedle.setAttribute('opacity','1');}
    else destNeedle.setAttribute('opacity','0');
    document.getElementById('rumbo-deg').textContent=Math.round(course);
    document.getElementById('rumbo-cardinal').textContent=degToCardinal(course);
    document.getElementById('tele-speed').textContent=(d.speed||0).toFixed(1);
    document.getElementById('tele-alt').textContent=Math.round(d.alt);
    for(let i=0;i<12;i++)document.getElementById('sb'+i).className='sb'+(i<d.sats?' on':'');
    document.getElementById('motor-pct').textContent=d.linkOk?(d.motorPct+'%'):'--';

    // Timon: mostrar grados reales y badge referencia
    const refBadge=document.getElementById('ref-badge');
    if(d.timonReferenciada){
      refBadge.className='ref-badge ref-ok';refBadge.textContent='REF OK';
      const ta=d.timonAngle||0;
      const taStr=(ta>=0?'+':'')+ta+'\u00b0';
      document.getElementById('timon-val').textContent=taStr;
      document.getElementById('timon-ctrl-val').textContent=taStr;
      document.getElementById('timon-pos-val').textContent=taStr;
    } else {
      refBadge.className='ref-badge ref-nok';refBadge.textContent='SIN REF';
      document.getElementById('timon-val').textContent='--\u00b0';
      document.getElementById('timon-ctrl-val').textContent='--\u00b0';
      document.getElementById('timon-pos-val').textContent='--\u00b0';
    }

    const sl=document.getElementById('trip-lbl');sl.textContent=slbl[d.navState]||d.navState;sl.style.color=scol[d.navState]||'var(--idle)';
    const dv=document.getElementById('dist-val'),du=document.getElementById('dist-u');
    const inTrip=(d.navState!=='IDLE'&&d.navState!=='ARRIVED');
    if(inTrip){dv.style.display='block';du.style.display='block';const m=d.distRemaining;dv.textContent=m>=1000?(m/1000).toFixed(2):Math.round(m);du.textContent=m>=1000?'KM RESTANTES':'METROS RESTANTES';}
    else{dv.style.display='none';du.style.display='none';}
    document.getElementById('eta-val').textContent=fmtETA(d.eta);
    document.getElementById('stat-dist').textContent=Math.round(d.totalDist);
    document.getElementById('stat-maxspd').textContent=(d.maxSpeed||0).toFixed(1);
    document.getElementById('stat-avgspd').textContent=(d.avgSpeed||0).toFixed(1);
    document.getElementById('stat-time').textContent=fmt(d.tripSecs);
    function updCebo(n,o){document.getElementById('btn-cebo'+n).className='btn-cebo '+(o?'descargado':'cargado');document.getElementById('cebo'+n+'-st').textContent=o?'DESCARGADO':'CARGADO';}
    updCebo(1,d.cebo1Open);updCebo(2,d.cebo2Open);
    if(d.homeValid){live.home={lat:d.homeLat,lng:d.homeLng};document.getElementById('h-coord').textContent=d.homeLat.toFixed(5)+', '+d.homeLng.toFixed(5);}
    if(d.c1Valid){live.c1={lat:d.c1Lat,lng:d.c1Lng};document.getElementById('c1-coord').textContent=d.c1Lat.toFixed(5)+', '+d.c1Lng.toFixed(5);}
    if(d.c2Valid){live.c2={lat:d.c2Lat,lng:d.c2Lng};document.getElementById('c2-coord').textContent=d.c2Lat.toFixed(5)+', '+d.c2Lng.toFixed(5);}
    if(d.fix)live.pos={lat:d.lat,lng:d.lng,course:d.course||0};
    live.history=d.history;
    drawMap();
    if(document.activeElement!==document.getElementById('thr-slider')){document.getElementById('thr-slider').value=d.throttleMax;document.getElementById('thr-val').textContent=d.throttleMax+'%';}
    if(document.activeElement!==document.getElementById('thrmin-slider')&&d.throttleMin!==undefined){document.getElementById('thrmin-slider').value=d.throttleMin;document.getElementById('thrmin-val').textContent=d.throttleMin+'%';}
    if(document.activeElement!==document.getElementById('prox-slider')&&d.distProximidad!==undefined){document.getElementById('prox-slider').value=d.distProximidad;document.getElementById('prox-val').textContent=d.distProximidad+'m';}
    if(document.activeElement!==document.getElementById('pausa-slider')&&d.pausaMotor!==undefined){document.getElementById('pausa-slider').value=d.pausaMotor;document.getElementById('pausa-val').textContent=d.pausaMotor+'s';}
    if(!trimCtrl.dragging&&d.trimTimon!==undefined){trimCtrl.set((d.trimTimon/150)*100,true);}
    const chkInv=document.getElementById('chk-invert-timon');
    if(chkInv&&!chkInv._touching)chkInv.checked=!!(d.invertirTimon);
    const calBtn=document.getElementById('cal-btn');
    if(calBtn){calBtn.style.borderColor=d.calibOK?'var(--green)':'var(--red)';calBtn.style.color=d.calibOK?'var(--green)':'var(--red)';}
    const cpBadge=document.getElementById('calib-progress-badge');
    if(cpBadge){const cp=d.calibProgress;if(cp!==undefined&&cp!==255&&cp!==null){cpBadge.style.display='inline';cpBadge.textContent='CAL '+cp+'%';}else cpBadge.style.display='none';}
    document.getElementById('btn-trip').disabled=inTrip;
    document.getElementById('btn-stop').disabled=!inTrip;
    document.getElementById('btn-trip').textContent=d.navState==='ARRIVED'?'\u25b6 NUEVO VIAJE':'\u25b6 INICIAR VIAJE';
  }).catch(()=>{});
}
setInterval(poll,200);poll();
</script>
</body>
</html>
)rawhtml";

// ============================================================
//  HELPERS
// ============================================================
static String navStateStr(uint8_t ns) {
    switch (ns) {
        case 1: return "GOING_CEBO1";
        case 2: return "GOING_CEBO2";
        case 3: return "RETURNING";
        case 4: return "ARRIVED";
        default: return "IDLE";
    }
}

static double distRemaining(const TelemetriaBarco& t) {
    if (!t.fix) return 0;
    auto hav = [](float lat1, float lng1, float lat2, float lng2) -> double {
        const double R = 6371000.0;
        double dLat = (lat2 - lat1) * DEG_TO_RAD;
        double dLng = (lng2 - lng1) * DEG_TO_RAD;
        double a = sin(dLat/2)*sin(dLat/2) +
                   cos(lat1*DEG_TO_RAD)*cos(lat2*DEG_TO_RAD)*sin(dLng/2)*sin(dLng/2);
        return R * 2 * atan2(sqrt(a), sqrt(1-a));
    };
    if (t.navState == 1) return hav(t.lat, t.lng, t.c1Lat, t.c1Lng);
    if (t.navState == 2) return hav(t.lat, t.lng, t.c2Lat, t.c2Lng);
    if (t.navState == 3) return hav(t.lat, t.lng, t.homeLat, t.homeLng);
    return 0;
}

// ============================================================
//  HANDLERS
// ============================================================
void handleRoot()    { server.send_P(200, "text/html", INDEX_HTML); }
void handleCaptive() { server.sendHeader("Location", "http://192.168.4.1", true); server.send(302, "text/plain", ""); }

void handleData() {
    const TelemetriaBarco& t = telemetria;
    bool linkOk = telemetriaRecibida && (millis() - ultimaTelemetriaMs < 3000);
    String ns = navStateStr(t.navState);
    double distRem = distRemaining(t);
    double eta = (t.speed > 0.5) ? distRem / (t.speed / 3.6) : 0;
    char timeStr[9] = "--:--:--";
    if (t.timeH || t.timeM || t.timeS)
        sprintf(timeStr, "%02d:%02d:%02d", t.timeH, t.timeM, t.timeS);
    String j = "{";
    j += "\"linkOk\":"    + String(linkOk ? "true" : "false") + ",";
    j += "\"fix\":"       + String(t.fix ? "true" : "false") + ",";
    j += "\"lat\":"       + String(t.lat, 6) + ",";
    j += "\"lng\":"       + String(t.lng, 6) + ",";
    j += "\"speed\":"     + String(t.speed, 1) + ",";
    j += "\"alt\":"       + String(t.alt, 1) + ",";
    j += "\"sats\":"      + String(t.sats) + ",";
    j += "\"hdop\":"      + String(t.hdop, 1) + ",";
    j += "\"course\":"    + String(t.course, 1) + ",";
    j += "\"navState\":\"" + ns + "\",";
    j += "\"distRemaining\":" + String(distRem, 1) + ",";
    j += "\"eta\":"       + String(eta, 0) + ",";
    j += "\"totalDist\":" + String(t.totalDist, 1) + ",";
    j += "\"maxSpeed\":"  + String(t.maxSpeed, 1) + ",";
    j += "\"avgSpeed\":"  + String(t.avgSpeed, 1) + ",";
    j += "\"tripSecs\":"  + String(t.tripSecs) + ",";
    j += "\"throttleMax\":" + String(t.throttleMax) + ",";
    j += "\"throttleMin\":" + String(t.throttleMin) + ",";
    j += "\"motorPct\":"  + String(t.motorRunning ? t.motorPctActual : 0) + ",";
    j += "\"timonAngle\":" + String(t.timonAngle) + ",";
    j += "\"timonReferenciada\":" + String(t.timonReferenciada ? "true" : "false") + ",";
    j += "\"trimTimon\":"  + String(t.trimTimon) + ",";
    j += "\"cebo1Open\":" + String(t.cebo1Abierto ? "true" : "false") + ",";
    j += "\"cebo2Open\":" + String(t.cebo2Abierto ? "true" : "false") + ",";
    j += "\"homeValid\":" + String(t.homeValid ? "true" : "false") + ",";
    j += "\"homeLat\":"   + String(t.homeLat, 6) + ",";
    j += "\"homeLng\":"   + String(t.homeLng, 6) + ",";
    j += "\"c1Valid\":"   + String(t.c1Valid ? "true" : "false") + ",";
    j += "\"c1Lat\":"     + String(t.c1Lat, 6) + ",";
    j += "\"c1Lng\":"     + String(t.c1Lng, 6) + ",";
    j += "\"c2Valid\":"   + String(t.c2Valid ? "true" : "false") + ",";
    j += "\"c2Lat\":"     + String(t.c2Lat, 6) + ",";
    j += "\"c2Lng\":"     + String(t.c2Lng, 6) + ",";
    j += "\"timeStr\":\"" + String(timeStr) + "\",";
    j += "\"invertirTimon\":" + String(t.invertirTimon ? "true" : "false") + ",";
    j += "\"distProximidad\":" + String(t.distProximidad) + ",";
    j += "\"pausaMotor\":" + String(t.pausaMotor) + ",";
    j += "\"calibOK\":" + String(t.calibOK ? "true" : "false") + ",";
    j += "\"calibProgress\":" + String(t.calibProgress) + ",";
    j += "\"calibOffsetX\":" + String(t.calibOffsetX, 2) + ",";
    j += "\"calibOffsetY\":" + String(t.calibOffsetY, 2) + ",";
    j += "\"calibScaleX\":" + String(t.calibScaleX, 3) + ",";
    j += "\"calibScaleY\":" + String(t.calibScaleY, 3) + ",";
    j += "\"targetBearing\":" + String(t.targetBearing, 1) + ",";
    j += "\"history\":[]";
    j += "}";
    server.send(200, "application/json", j);
}

void handleSave() {
    if (!server.hasArg("p")) { server.send(400); return; }
    bool linkOk = telemetriaRecibida && (millis() - ultimaTelemetriaMs < 3000);
    if (!linkOk && !telemetria.fix) { server.send(200, "application/json", "{\"ok\":false}"); return; }
    String p = server.arg("p");
    ComandoPlaya cmd = {};
    cmd.tipo = CMD_GUARDAR_PUNTO;
    if (p == "home")  cmd.punto = 0;
    if (p == "cebo1") cmd.punto = 1;
    if (p == "cebo2") cmd.punto = 2;
    EnviarComando(cmd);
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleStartTrip() {
    if (!telemetria.homeValid || !telemetria.c1Valid || !telemetria.c2Valid) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Define home, cebo1 y cebo2 primero\"}");
        return;
    }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_START_TRIP;
    EnviarComando(cmd);
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleStopTrip() {
    ComandoPlaya cmd = {}; cmd.tipo = CMD_STOP_TRIP;
    EnviarComando(cmd);
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleThrottle() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_THROTTLE;
    cmd.nuevoThrottle = constrain(server.arg("v").toInt(), 0, 100);
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleThrottleMin() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_THROTTLE_MIN;
    cmd.nuevoThrottleMin = constrain(server.arg("v").toInt(), 0, 50);
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleCebo() {
    if (!server.hasArg("n")) { server.send(400); return; }
    int n = server.arg("n").toInt();
    ComandoPlaya cmd = {}; cmd.tipo = CMD_CEBO;
    cmd.numeroCebo = n;
    cmd.abrirCebo = (n == 1) ? !telemetria.cebo1Abierto : !telemetria.cebo2Abierto;
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleJoystick() {
    if (!server.hasArg("t") || !server.hasArg("s")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_JOYSTICK;
    cmd.throttle = constrain(server.arg("t").toInt(), 0, 100);
    cmd.rumbo    = constrain(server.arg("s").toInt(), -1, 1);  // solo -1, 0, +1
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleTrim() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_TRIM;
    cmd.trimTimon = constrain(server.arg("v").toInt(), -150, 150);
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleInvertTimon() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_INVERT_TIMON;
    cmd.invertirTimon = server.arg("v").toInt() != 0;
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handleCenterTimon() {
    ComandoPlaya cmd = {}; cmd.tipo = CMD_CENTER_TIMON;
    EnviarComando(cmd);
    server.send(200, "application/json", "{\"ok\":true}");
}

void handleProximidad() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_PROXIMIDAD;
    cmd.distProximidad = constrain(server.arg("v").toInt(), 1, 30);
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

void handlePausa() {
    if (!server.hasArg("v")) { server.send(400); return; }
    ComandoPlaya cmd = {}; cmd.tipo = CMD_PAUSA;
    cmd.pausaMotor = constrain(server.arg("v").toInt(), 0, 30);
    EnviarComando(cmd); server.send(200, "application/json", "{\"ok\":true}");
}

// ============================================================
//  PAGINA CALIBRACION
// ============================================================
const char CALIB_HTML[] PROGMEM = R"calibhtml(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Calibrar Br&#250;jula</title>
<style>
:root{--bg:#03080d;--panel:#071118;--border:#0d2d45;--accent:#00d4ff;--accent2:#00ff88;--accent3:#ff6b35;--red:#ff3b3b;--green:#00ff88;--dim:#3a6a84;--text:#c8e6f0;}
body.light{--bg:#f0f4f8;--panel:#fff;--border:#c8d8e8;--accent:#005f8a;--accent2:#006638;--accent3:#cc5500;--red:#aa1111;--green:#006638;--dim:#4a6a80;--text:#0d1a26;}
*{box-sizing:border-box;margin:0;padding:0;}
body{background:var(--bg);color:var(--text);font-family:'Courier New',monospace;min-height:100vh;display:flex;flex-direction:column;align-items:center;justify-content:center;padding:20px;}
.card{background:var(--panel);border:1px solid var(--border);border-radius:8px;padding:24px;max-width:420px;width:100%;text-align:center;}
h2{color:var(--accent);font-size:1.1rem;letter-spacing:.1em;margin-bottom:6px;}
.sub{color:var(--dim);font-size:.78rem;margin-bottom:20px;}
.countdown{font-size:3.5rem;font-weight:700;color:var(--accent3);line-height:1;margin:10px 0;}
.progress-wrap{background:var(--bg);border:1px solid var(--border);border-radius:20px;height:28px;overflow:hidden;margin:16px 0;}
.progress-bar{height:100%;border-radius:20px;background:var(--accent2);transition:width .3s;width:0%;}
.pct{font-size:1.4rem;font-weight:700;color:var(--accent2);margin:8px 0;}
.status{font-size:.82rem;color:var(--dim);min-height:20px;margin:6px 0;}
.calib-vals{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin:14px 0;text-align:left;}
.cv{background:var(--bg);border:1px solid var(--border);border-radius:4px;padding:6px 10px;font-size:.75rem;}
.cv-lbl{color:var(--dim);font-size:.65rem;}
.cv-val{color:var(--accent);font-weight:700;}
.btn-back{display:inline-block;margin-top:16px;padding:10px 24px;background:transparent;border:1px solid var(--accent);border-radius:4px;color:var(--accent);font-family:inherit;font-size:.82rem;cursor:pointer;text-decoration:none;letter-spacing:.08em;}
.btn-back:hover{background:rgba(0,212,255,.1);}
.icon{font-size:2.5rem;margin-bottom:8px;}
</style>
</head>
<body>
<div class="card">
  <div class="icon" id="icon">&#129517;</div>
  <h2>CALIBRAR BR&#218;JULA</h2>
  <div class="sub" id="sub">Preparando calibraci&#243;n...</div>
  <div class="countdown" id="countdown" style="display:none;"></div>
  <div class="progress-wrap" id="prog-wrap" style="display:none;">
    <div class="progress-bar" id="prog-bar"></div>
  </div>
  <div class="pct" id="pct" style="display:none;">0%</div>
  <div class="status" id="status"></div>
  <div class="calib-vals" id="calib-vals" style="display:none;">
    <div class="cv"><div class="cv-lbl">OFFSET X</div><div class="cv-val" id="cv-ox">--</div></div>
    <div class="cv"><div class="cv-lbl">OFFSET Y</div><div class="cv-val" id="cv-oy">--</div></div>
    <div class="cv"><div class="cv-lbl">ESCALA X</div><div class="cv-val" id="cv-sx">--</div></div>
    <div class="cv"><div class="cv-lbl">ESCALA Y</div><div class="cv-val" id="cv-sy">--</div></div>
  </div>
  <a href="/" class="btn-back" id="btn-back" style="display:none;">&#8592; VOLVER</a>
</div>
<script>
(function(){if(localStorage.getItem('tema')==='light')document.body.classList.add('light');})();
const sub=document.getElementById('sub'),countdown=document.getElementById('countdown'),progWrap=document.getElementById('prog-wrap'),progBar=document.getElementById('prog-bar'),pctEl=document.getElementById('pct'),status=document.getElementById('status'),calibVals=document.getElementById('calib-vals'),btnBack=document.getElementById('btn-back'),icon=document.getElementById('icon');
let phase='countdown',secs=10,pollTimer=null;
function startCountdown(){sub.textContent='Prepara el sensor. Gira el barco 360\u00b0 cuando empiece.';countdown.style.display='block';countdown.textContent=secs;const cd=setInterval(()=>{secs--;countdown.textContent=secs;if(secs<=0){clearInterval(cd);countdown.style.display='none';startCalib();}},1000);}
function startCalib(){phase='calibrating';sub.textContent='\u{1F504} Gira el barco lentamente 360 grados completos...';progWrap.style.display='block';pctEl.style.display='block';status.textContent='Enviando comando al barco...';fetch('/startCalib').then(r=>r.json()).then(d=>{if(d.ok){status.textContent='Calibrando - sigue girando...';pollTimer=setInterval(pollCalib,500);}else status.textContent='Error: sin conexion';}).catch(()=>{status.textContent='Error de red';});}
function pollCalib(){fetch('/data').then(r=>r.json()).then(d=>{const pct=d.calibProgress;if(pct===255||pct===undefined){if(phase==='calibrating')finishCalib(d);return;}progBar.style.width=pct+'%';pctEl.textContent=pct+'%';status.textContent='Muestras: '+Math.round(pct*5)+' / 500';}).catch(()=>{});}
function finishCalib(d){clearInterval(pollTimer);phase='done';icon.textContent='\u2705';sub.textContent='Calibraci\u00f3n completada y guardada.';progBar.style.width='100%';pctEl.textContent='100%';status.textContent='';if(d.calibOffsetX!==undefined){calibVals.style.display='grid';document.getElementById('cv-ox').textContent=d.calibOffsetX.toFixed(2);document.getElementById('cv-oy').textContent=d.calibOffsetY.toFixed(2);document.getElementById('cv-sx').textContent=d.calibScaleX.toFixed(3);document.getElementById('cv-sy').textContent=d.calibScaleY.toFixed(3);}btnBack.style.display='inline-block';let t=5;const ri=setInterval(()=>{btnBack.textContent='\u2190 VOLVER ('+t+'s)';t--;if(t<0){clearInterval(ri);window.location='/';}},1000);}
startCountdown();
</script>
</body>
</html>
)calibhtml";

void handleCalibPage()  { server.send_P(200, "text/html", CALIB_HTML); }
void handleStartCalib() { ComandoPlaya cmd={}; cmd.tipo=CMD_CALIB_GY273; EnviarComando(cmd); server.send(200,"application/json","{\"ok\":true}"); }
void handleTrip()       { server.send(200,"application/json","null"); }

void SetupServidorWeb() {
    dns.start(53, "*", WiFi.softAPIP());

    server.on("/",            handleRoot);
    server.on("/data",        handleData);
    server.on("/save",        handleSave);
    server.on("/startTrip",   handleStartTrip);
    server.on("/stopTrip",    handleStopTrip);
    server.on("/throttle",    handleThrottle);
    server.on("/throttleMin", handleThrottleMin);
    server.on("/cebo",        handleCebo);
    server.on("/trip",        handleTrip);
    server.on("/joystick",    handleJoystick);
    server.on("/trim",        handleTrim);
    server.on("/invertTimon", handleInvertTimon);
    server.on("/centerTimon", handleCenterTimon);
    server.on("/calib",       handleCalibPage);
    server.on("/proximidad",  handleProximidad);
    server.on("/pausa",       handlePausa);
    server.on("/startCalib",  handleStartCalib);

    server.on("/generate_204",              handleCaptive);
    server.on("/gen_204",                   handleCaptive);
    server.on("/hotspot-detect.html",       handleCaptive);
    server.on("/library/test/success.html", handleCaptive);
    server.on("/ncsi.txt",                  handleCaptive);
    server.on("/connecttest.txt",           handleCaptive);
    server.on("/redirect",                  handleCaptive);
    server.onNotFound(handleCaptive);

    server.begin();
    Serial.println("Server OK");
}

void LoopServidorWeb() {
    dns.processNextRequest();
    server.handleClient();
}
