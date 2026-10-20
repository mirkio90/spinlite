// Esegui con: node docs/protocol.test.mjs
import assert from "node:assert/strict";
import { parseStatus, parseHistoryPages, parseCurve, DumpAssembler, rpmFromInterval } from "./protocol.js";

const put16 = (p, o, v) => { p[o] = v & 255; p[o + 1] = v >> 8; };

// Stessa logica di buildStatus / buildDump in bbp_protocol.cpp
function buildStatus(best, total, flags, battery, uid) {
  const o = new Uint8Array(17);
  o[0] = 0xa0; o[1] = 0x3a; o[3] = flags; o[4] = battery;
  put16(o, 7, best); put16(o, 9, total); o.set(uid, 11);
  return o;
}
function buildDump(rpms, best, total, curve) {
  const out = Array.from({ length: 12 }, () => new Uint8Array(17));
  for (let p = 0; p < 8; p++) out[p][0] = 0xb0 + p;
  for (let p = 0; p < 4; p++) out[8 + p][0] = 0x70 + p;
  rpms.forEach((v, i) => put16(out[i >> 3], 1 + (i % 8) * 2, v));
  put16(out[6], 7, best); put16(out[6], 9, total); out[6][11] = rpms.length;
  let sum = 0;
  for (let p = 0; p < 7; p++) for (let b = 1; b < 17; b++) sum = (sum + out[p][b]) & 255;
  out[7][16] = sum;
  curve.forEach((v, i) => put16(out[8 + (i >> 3)], 1 + (i % 8) * 2, v));
  return out;
}

// Stato
const uid = [1, 2, 3, 4, 5, 0xab];
const st = parseStatus(buildStatus(12345, 777, 0x14, 125, uid));
assert.equal(st.ready, true);
assert.equal(st.batteryLow, true);
assert.equal(st.batteryCritical, false);
assert.equal(st.batteryPercent, 50);
assert.equal(st.best, 12345);
assert.equal(st.total, 777);
assert.equal(st.uid, "01:02:03:04:05:ab");
assert.equal(parseStatus(new Uint8Array(17)), null);

// Storico pieno (50) con curva a 10000 RPM (750 unita')
const rpms = Array.from({ length: 50 }, (_, i) => 5000 + i * 100);
const curve = Array.from({ length: 32 }, (_, i) => (i < 20 ? 750 : 0));
const dump = buildDump(rpms, 9900, 321, curve);
const h = parseHistoryPages(dump.slice(0, 8));
assert.equal(h.checksumOk, true);
assert.equal(h.count, 50);
assert.deepEqual(h.values, rpms);
assert.equal(h.values[48], 9800); // B6 contiene 48 e 49
assert.equal(h.best, 9900);
assert.equal(h.total, 321);

// Checksum errato
const bad = dump.slice(0, 8).map((p) => Uint8Array.from(p));
bad[3][5] ^= 0x10;
assert.equal(parseHistoryPages(bad).checksumOk, false);

// Storico parziale
const part = buildDump([6000, 7000, 8000], 8000, 3, []);
const hp = parseHistoryPages(part.slice(0, 8));
assert.equal(hp.count, 3);
assert.deepEqual(hp.launches, [6000, 7000, 8000]);
assert.equal(hp.checksumOk, true);

// Curva
const c = parseCurve(dump.slice(8, 12));
assert.equal(c.rpm[0], 10000);
assert.equal(c.rpm[19], 10000);
assert.equal(c.rpm[20], 0);
assert.equal(rpmFromInterval(0), 0);

// Assemblatore: un dump interrotto a meta' non contamina il successivo
// (B0 riparte da zero); dopo B0 l'ordine puo' essere sparso.
const asm = new DumpAssembler();
const stale = Uint8Array.from(dump[3]); stale[1] ^= 0xff;
[dump[0], dump[1], dump[2], stale, dump[9]].forEach((p) => asm.push(p));
const order = [0, 8, 11, 1, 2, 3, 10, 4, 5, 6, 9, 7];
order.forEach((idx, k) => assert.equal(asm.push(dump[idx]), k === order.length - 1));
const r = asm.result();
assert.equal(r.history.checksumOk, true);
assert.equal(r.curve.rpm[0], 10000);
assert.equal(asm.push(buildStatus(0, 0, 0, 0, uid)), false);

console.log("protocol.test.mjs: tutti i test OK");
