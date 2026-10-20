import { parseStatus, DumpAssembler, hex, CMD_STATUS, CMD_DUMP, CMD_CLEAR } from "./protocol.js";

const SERVICE = "55c40000-f8eb-11ec-b939-0242ac120002";
const CH_W = "55c4f001-f8eb-11ec-b939-0242ac120002";
const CH_N = "55c4f002-f8eb-11ec-b939-0242ac120002";
const $ = (id) => document.getElementById(id);

let device, writeChar, assembler = new DumpAssembler(), lastHistory = null, lastCurve = null;

if (!navigator.bluetooth) {
  $("nosupport").hidden = false;
  $("connect").disabled = true;
}

const css = (n) => getComputedStyle(document.documentElement).getPropertyValue(n).trim();
const setState = (t) => ($("state").textContent = t);

function log(dir, bytes) {
  const el = $("log");
  const lines = (el.textContent + `${dir} ${hex(bytes)}\n`).split("\n");
  el.textContent = lines.slice(-300).join("\n");
  el.scrollTop = el.scrollHeight;
}

async function send(cmd) {
  if (!writeChar) return;
  const data = Uint8Array.of(cmd);
  log("TX", data);
  try {
    if (writeChar.properties.writeWithoutResponse) await writeChar.writeValueWithoutResponse(data);
    else await writeChar.writeValue(data);
  } catch (e) { setState("Errore scrittura: " + e.message); }
}

function onNotify(ev) {
  const v = ev.target.value;
  const p = new Uint8Array(v.buffer.slice(v.byteOffset, v.byteOffset + v.byteLength));
  log("RX", p);
  if (p[0] === 0xa0) {
    showStatus(parseStatus(p));
  } else if (assembler.push(p)) {
    const r = assembler.result();
    assembler = new DumpAssembler();
    showDump(r);
  }
}

function showStatus(s) {
  if (!s) return;
  $("best").textContent = s.best;
  $("total").textContent = s.total;
  $("batt").textContent = s.batteryPercent;
  const tags = [];
  if (s.ready) tags.push("PRONTO");
  if (s.batteryCritical) tags.push("Batteria critica");
  else if (s.batteryLow) tags.push("Batteria bassa");
  $("flags").innerHTML = tags.map((t) => `<span class="tag">${t}</span>`).join("") || '<span class="note">nessun flag</span>';
}

function showDump({ history, curve }) {
  lastHistory = history; lastCurve = curve;
  if (history) {
    const c = $("chk");
    c.textContent = history.checksumOk ? "OK" : "KO";
    c.className = "val " + (history.checksumOk ? "ok" : "ko");
    $("best").textContent = history.best;
    $("total").textContent = history.total;
    $("last").textContent = history.count ? history.launches[history.count - 1] : "-";
  }
  draw();
}

function setup(cv) {
  const r = devicePixelRatio || 1, w = cv.clientWidth, h = cv.clientHeight;
  cv.width = w * r; cv.height = h * r;
  const g = cv.getContext("2d");
  g.scale(r, r); g.clearRect(0, 0, w, h);
  g.font = "11px system-ui"; g.fillStyle = css("--mut"); g.strokeStyle = css("--bd");
  return { g, w, h };
}

function drawHistory() {
  const { g, w, h } = setup($("hist"));
  const v = lastHistory ? lastHistory.launches : [];
  if (!v.length) { g.fillText("Nessun dato", 8, 20); return; }
  const max = Math.max(...v, 1), L = 40, B = 16, bw = (w - L) / 50;
  for (let i = 0; i <= 4; i++) {
    const y = (h - B) * (1 - i / 4);
    g.beginPath(); g.moveTo(L, y); g.lineTo(w, y); g.stroke();
    g.fillText(Math.round((max * i) / 4), 2, y - 2);
  }
  g.fillStyle = css("--bar");
  v.forEach((x, i) => { const bh = ((h - B) * x) / max; g.fillRect(L + i * bw + 1, h - B - bh, Math.max(bw - 2, 1), bh); });
}

function drawCurve() {
  const { g, w, h } = setup($("curve"));
  const pts = lastCurve ? lastCurve.rpm.map((r, i) => [i, r]).filter((p) => p[1] > 0) : [];
  if (pts.length < 2) { g.fillText("Nessuna curva", 8, 20); return; }
  const max = Math.max(...pts.map((p) => p[1])), L = 40, B = 16, n = pts[pts.length - 1][0] || 1;
  for (let i = 0; i <= 4; i++) {
    const y = (h - B - 6) * (1 - i / 4) + 6;
    g.beginPath(); g.moveTo(L, y); g.lineTo(w, y); g.stroke();
    g.fillText(Math.round((max * i) / 4), 2, y - 2);
  }
  g.fillText("giro", w - 28, h - 3);
  g.strokeStyle = css("--acc"); g.lineWidth = 2; g.beginPath();
  pts.forEach(([i, r], k) => {
    const x = L + ((w - L - 8) * i) / n, y = (h - B - 6) * (1 - r / max) + 6;
    if (k) g.lineTo(x, y); else g.moveTo(x, y);
  });
  g.stroke();
}

function draw() { drawHistory(); drawCurve(); }
addEventListener("resize", draw);
matchMedia("(prefers-color-scheme: dark)").addEventListener("change", draw);
draw();

async function refresh() {
  await send(CMD_STATUS);
  await new Promise((r) => setTimeout(r, 100));
  await send(CMD_DUMP);
}

function setConnected(on) {
  $("refresh").disabled = !on;
  $("clear").disabled = !on;
  $("connect").textContent = on ? "Scollega" : "Collega";
}

async function connect() {
  if (device && device.gatt.connected) { device.gatt.disconnect(); return; }
  try {
    device = await navigator.bluetooth.requestDevice({ filters: [{ services: [SERVICE] }], optionalServices: [SERVICE] });
    device.addEventListener("gattserverdisconnected", () => { setConnected(false); setState("Scollegato"); writeChar = null; });
    setState("Connessione...");
    const server = await device.gatt.connect();
    const svc = await server.getPrimaryService(SERVICE);
    const notif = await svc.getCharacteristic(CH_N);
    try { writeChar = await svc.getCharacteristic(CH_W); } catch { writeChar = notif; }
    notif.addEventListener("characteristicvaluechanged", onNotify);
    await notif.startNotifications();
    setConnected(true);
    setState("Collegato a " + (device.name || "dispositivo"));
    assembler = new DumpAssembler();
    setTimeout(refresh, 300);
  } catch (e) {
    setState(e.name === "NotFoundError" ? "Nessun dispositivo scelto" : "Errore: " + e.message);
  }
}

$("connect").onclick = connect;
$("refresh").onclick = () => { assembler = new DumpAssembler(); refresh(); };
$("clear").onclick = async () => {
  if (!confirm("Azzerare lo storico dei lanci sul dispositivo?")) return;
  await send(CMD_CLEAR);
  assembler = new DumpAssembler();
  setTimeout(refresh, 400);
};
