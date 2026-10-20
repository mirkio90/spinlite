#include "bbp_protocol.h"

#include <string.h>

namespace spinlite {
namespace bbp {
namespace {

void put16(uint8_t *p, uint16_t v) {
  p[0] = uint8_t(v & 0xFF);
  p[1] = uint8_t(v >> 8);
}
void put32(uint8_t *p, uint32_t v) {
  for (int i = 0; i < 4; ++i) p[i] = uint8_t(v >> (8 * i));
}
uint16_t get16(const uint8_t *p) { return uint16_t(p[0] | (p[1] << 8)); }
uint32_t get32(const uint8_t *p) {
  return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
const uint8_t MAGIC[4] = {'S', 'P', 'L', 'H'};
constexpr uint8_t STORE_VERSION = 1;

} // namespace

void clear(History &h) { h = History(); }

void add(History &h, uint16_t rpm, const uint32_t *curveUs, uint8_t curveCount) {
  h.records[h.next] = rpm;
  h.next = uint8_t((h.next + 1) % HISTORY);
  if (h.count < HISTORY) h.count++;
  h.total++;
  if (rpm > h.best) h.best = rpm;
  h.curveCount = curveCount > CURVE ? CURVE : curveCount;
  for (size_t i = 0; i < CURVE; ++i) h.curve[i] = i < h.curveCount ? encodePeriod(curveUs[i]) : 0;
}

uint16_t latest(const History &h) {
  if (!h.count) return 0;
  return h.records[(h.next + HISTORY - 1) % HISTORY];
}

uint16_t chronological(const History &h, uint8_t i) {
  if (i >= h.count) return 0;
  const uint8_t oldest = h.count == HISTORY ? h.next : 0;
  return h.records[(oldest + i) % HISTORY];
}

uint16_t encodePeriod(uint32_t periodUs) {
  uint32_t v = (periodUs + 4) / 8;
  if (v < 1) v = 1;
  if (v > 65535) v = 65535;
  return uint16_t(v);
}

uint32_t rpmFromInterval(uint16_t interval) { return interval ? (7500000u + interval / 2) / interval : 0; }

uint8_t batteryByte(uint8_t percent) {
  if (percent > 100) percent = 100;
  return uint8_t((uint16_t(percent) * 250 + 50) / 100);
}

void buildStatus(const History &h, uint8_t flags, uint8_t battery, const uint8_t uid[6], uint8_t out[PACKET]) {
  memset(out, 0, PACKET);
  out[0] = HDR_STATUS;
  out[1] = 0x3A; // valore fisso osservato da BRD; significato sconosciuto
  out[3] = flags;
  out[4] = battery > 250 ? 250 : battery;
  put16(out + 7, h.best);
  put16(out + 9, uint16_t(h.total));
  // Byte 11-16: identificativo del dispositivo (BRD). Nel firmware documentato
  // da MarkChan sono riempitivo 0x67; le app note non sembrano usarli.
  memcpy(out + 11, uid, 6);
}

void buildDump(const History &h, uint8_t out[DUMP_PAGES][PACKET]) {
  memset(out, 0, DUMP_PAGES * PACKET);
  for (size_t p = 0; p < 8; ++p) out[p][0] = uint8_t(HDR_HISTORY + p);
  for (size_t p = 0; p < 4; ++p) out[8 + p][0] = uint8_t(HDR_CURVE + p);

  // Storico dal piu' vecchio al piu' recente, 8 valori per pacchetto: il
  // settimo pacchetto (B6) contiene solo gli ultimi 2 valori.
  for (uint8_t i = 0; i < h.count; ++i) put16(out[i / 8] + 1 + (i % 8) * 2, chronological(h, i));

  // Coda di B6: massimo, totale e numero di lanci in memoria. MarkChan legge
  // il byte 11 come "indice dell'ultimo": con lo storico ordinato da 0 i due
  // significati portano allo stesso risultato.
  put16(out[6] + 7, h.best);
  put16(out[6] + 9, uint16_t(h.total));
  out[6][11] = h.count;

  uint8_t sum = 0;
  for (size_t p = 0; p < 7; ++p)
    for (size_t b = 1; b < PACKET; ++b) sum = uint8_t(sum + out[p][b]);
  out[7][16] = sum;

  for (size_t i = 0; i < CURVE; ++i) put16(out[8 + i / 8] + 1 + (i % 8) * 2, h.curve[i]);
}

uint32_t crc32(const uint8_t *data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int k = 0; k < 8; ++k) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
  }
  return ~crc;
}

size_t encode(const History &h, uint8_t *buf, size_t len) {
  if (len < STORE_BYTES) return 0;
  memset(buf, 0, STORE_BYTES);
  uint8_t *p = buf;
  memcpy(p, MAGIC, 4);
  p += 4;
  *p++ = STORE_VERSION;
  put32(p, h.total);
  p += 4;
  put16(p, h.best);
  p += 2;
  *p++ = h.count;
  *p++ = h.next;
  *p++ = h.curveCount;
  for (size_t i = 0; i < HISTORY; ++i, p += 2) put16(p, h.records[i]);
  for (size_t i = 0; i < CURVE; ++i, p += 2) put16(p, h.curve[i]);
  put32(p, crc32(buf, size_t(p - buf)));
  return STORE_BYTES;
}

bool decode(const uint8_t *buf, size_t len, History &h) {
  if (len < STORE_BYTES || memcmp(buf, MAGIC, 4) != 0 || buf[4] != STORE_VERSION) return false;
  if (get32(buf + STORE_BYTES - 4) != crc32(buf, STORE_BYTES - 4)) return false;
  History out;
  const uint8_t *p = buf + 5;
  out.total = get32(p);
  p += 4;
  out.best = get16(p);
  p += 2;
  out.count = *p++;
  out.next = *p++;
  out.curveCount = *p++;
  if (out.count > HISTORY || out.next >= HISTORY || out.curveCount > CURVE) return false;
  for (size_t i = 0; i < HISTORY; ++i, p += 2) out.records[i] = get16(p);
  for (size_t i = 0; i < CURVE; ++i, p += 2) out.curve[i] = get16(p);
  h = out;
  return true;
}

} // namespace bbp
} // namespace spinlite
