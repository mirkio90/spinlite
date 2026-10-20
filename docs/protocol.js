// Parser dei pacchetti SpinLite / Bey Battle Pass. Funzioni pure, nessuna dipendenza dal browser.
// Tutti i pacchetti sono lunghi 17 byte, numeri a 16 bit little-endian.
export const PACKET = 17;
export const HISTORY = 50;
export const CURVE = 32;
export const HDR_STATUS = 0xa0;
export const HDR_HISTORY = 0xb0; // B0..B6, B7 = checksum
export const HDR_CURVE = 0x70; // 70..73
export const CMD_STATUS = 0x51;
export const CMD_DUMP = 0x74;
export const CMD_CLEAR = 0x75;
export const FLAG_BATTERY_CRITICAL = 0x01;
export const FLAG_READY = 0x04;
export const FLAG_BATTERY_LOW = 0x10;

const u16 = (p, o) => p[o] | (p[o + 1] << 8);
const hex = (p) => Array.from(p, (b) => b.toString(16).padStart(2, "0")).join(" ");
export { hex };

export function rpmFromInterval(v) {
  return v ? Math.round(7500000 / v) : 0;
}

export function parseStatus(p) {
  if (!p || p.length < PACKET || p[0] !== HDR_STATUS) return null;
  const flags = p[3];
  return {
    flags,
    ready: !!(flags & FLAG_READY),
    batteryLow: !!(flags & FLAG_BATTERY_LOW),
    batteryCritical: !!(flags & FLAG_BATTERY_CRITICAL),
    batteryRaw: p[4],
    batteryPercent: Math.min(100, Math.round((p[4] * 100) / 250)),
    best: u16(p, 7),
    total: u16(p, 9),
    uid: Array.from(p.slice(11, 17), (b) => b.toString(16).padStart(2, "0")).join(":"),
  };
}

// pages: array di 8 pacchetti (indice 0..7 = B0..B7). Mancanti = undefined.
export function parseHistoryPages(pages) {
  for (let i = 0; i < 8; i++) if (!pages[i] || pages[i].length < PACKET) return null;
  let sum = 0;
  for (let i = 0; i < 7; i++) for (let b = 1; b < PACKET; b++) sum = (sum + pages[i][b]) & 0xff;
  const checksumOk = sum === pages[7][16];
  const b6 = pages[6];
  const count = Math.min(b6[11], HISTORY);
  const values = [];
  for (let i = 0; i < HISTORY; i++) values.push(u16(pages[Math.floor(i / 8)], 1 + (i % 8) * 2));
  return {
    values, // 50 valori, dal piu' vecchio al piu' recente (oltre count sono 0)
    launches: values.slice(0, count),
    best: u16(b6, 7),
    total: u16(b6, 9),
    count,
    checksumOk,
    checksumExpected: pages[7][16],
    checksumComputed: sum,
  };
}

// pages: array di 4 pacchetti (70..73). Ritorna 32 valori grezzi e RPM (0 = vuoto).
export function parseCurve(pages) {
  for (let i = 0; i < 4; i++) if (!pages[i] || pages[i].length < PACKET) return null;
  const raw = [];
  for (let i = 0; i < CURVE; i++) raw.push(u16(pages[Math.floor(i / 8)], 1 + (i % 8) * 2));
  return { raw, rpm: raw.map(rpmFromInterval) };
}

// Raccoglie i pacchetti del dump man mano che arrivano.
export class DumpAssembler {
  constructor() { this.reset(); }
  reset() { this.hist = new Array(8); this.curve = new Array(4); }
  // Ritorna true quando il dump e' completo (B0..B7 e 70..73).
  push(p) {
    const h = p[0];
    // B0 apre sempre un dump nuovo: scarta i resti di uno interrotto.
    if (h === HDR_HISTORY) this.reset();
    if (h >= HDR_HISTORY && h <= HDR_HISTORY + 7) this.hist[h - HDR_HISTORY] = Uint8Array.from(p);
    else if (h >= HDR_CURVE && h <= HDR_CURVE + 3) this.curve[h - HDR_CURVE] = Uint8Array.from(p);
    else return false;
    return this.complete();
  }
  complete() {
    for (let i = 0; i < 8; i++) if (!this.hist[i]) return false;
    for (let i = 0; i < 4; i++) if (!this.curve[i]) return false;
    return true;
  }
  result() {
    return { history: parseHistoryPages(this.hist), curve: parseCurve(this.curve) };
  }
}
