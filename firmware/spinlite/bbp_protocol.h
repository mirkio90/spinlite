// SpinLite - pacchetti Bluetooth nel formato del Bey Battle Pass (BX-09).
//
// Il protocollo non e' pubblico. Il formato qui sotto e' ricostruito da fonti
// indipendenti: la documentazione di beyblade-tachometer-web (MarkChan0225),
// il firmware BRD (printfChins, GPL-3.0, solo consultato per confrontare i
// valori: nessun codice copiato) e uno script di prova sul dispositivo vero
// (danielbyomujuni). Dove le fonti non concordano e' indicato nei commenti.
//
// Tutti i pacchetti sono lunghi 17 byte; i numeri a 16 bit sono little-endian.
//   A0          stato: batteria, massimo, totale lanci, identificativo
//   B0..B6      storico degli ultimi 50 lanci (8 valori per pacchetto)
//   B7          checksum dello storico
//   70..73      curva dell'ultimo lancio: 32 periodi di giro in unita' da 8 us
//
// Codice portabile, testato sul PC.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace spinlite {
namespace bbp {

constexpr size_t PACKET = 17;
constexpr size_t HISTORY = 50;
constexpr size_t CURVE = 32;
constexpr size_t DUMP_PAGES = 12; // B0..B7 + 70..73

constexpr uint8_t HDR_STATUS = 0xA0;
constexpr uint8_t HDR_HISTORY = 0xB0;
constexpr uint8_t HDR_CHECKSUM = 0xB7;
constexpr uint8_t HDR_CURVE = 0x70;

// Comandi che l'app scrive sulla caratteristica.
constexpr uint8_t CMD_STATUS = 0x51; // risposta: A0
constexpr uint8_t CMD_DUMP = 0x74;   // risposta: B0..B7 e 70..73
constexpr uint8_t CMD_CLEAR = 0x75;  // azzera lo storico (secondo BRD; MarkChan lo usa per un test sensore)

// Bit del byte di stato in A0.
constexpr uint8_t FLAG_BATTERY_CRITICAL = 0x01;
constexpr uint8_t FLAG_READY = 0x04; // nel Battle Pass vero: trottola inserita
constexpr uint8_t FLAG_BATTERY_LOW = 0x10;

struct History {
  uint32_t total = 0;  // lanci totali da quando e' stato azzerato
  uint16_t best = 0;   // massimo di sempre
  uint8_t count = 0;   // lanci in memoria, al massimo 50
  uint8_t next = 0;    // prossima posizione nel buffer circolare
  uint16_t records[HISTORY] = {};
  uint8_t curveCount = 0;
  uint16_t curve[CURVE] = {}; // ultimo lancio, unita' da 8 us
};

void clear(History &h);
void add(History &h, uint16_t rpm, const uint32_t *curveUs, uint8_t curveCount);
uint16_t latest(const History &h);
// i-esimo lancio in ordine cronologico (0 = il piu' vecchio in memoria)
uint16_t chronological(const History &h, uint8_t i);

// Un periodo di giro in microsecondi -> unita' da 8 us (RPM = 7.500.000 / valore).
uint16_t encodePeriod(uint32_t periodUs);
uint32_t rpmFromInterval(uint16_t interval);
uint8_t batteryByte(uint8_t percent); // 0..100 % -> 0..250

void buildStatus(const History &h, uint8_t flags, uint8_t battery, const uint8_t uid[6], uint8_t out[PACKET]);
void buildDump(const History &h, uint8_t out[DUMP_PAGES][PACKET]);

// Salvataggio in memoria flash, con versione e CRC.
constexpr size_t STORE_BYTES = 4 + 1 + 4 + 2 + 1 + 1 + 1 + HISTORY * 2 + CURVE * 2 + 4;
size_t encode(const History &h, uint8_t *buf, size_t len);
bool decode(const uint8_t *buf, size_t len, History &h);
uint32_t crc32(const uint8_t *data, size_t len);

} // namespace bbp
} // namespace spinlite
