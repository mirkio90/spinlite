// SpinLite - servizio Bluetooth LE con il profilo del Bey Battle Pass.
//
// Service 55c40000-f8eb-11ec-b939-0242ac120002:
//   55c4f001-...  WRITE, WRITE_NR                 (creata per prima: handle 15)
//   55c4f002-...  NOTIFY, WRITE, WRITE_NR, READ   (dopo: valore 17, CCCD 18)
// I comandi sono accettati su entrambe. Le callback NimBLE mettono i comandi
// in coda; loop() li esegue e invia i pacchetti uno alla volta, distanziati di
// BLE_NOTIFY_SPACING_MS. I pacchetti li costruisce il chiamante (Provider):
// questo modulo non conosce lo storico.
//
// Solo per ESP32 (NimBLE-Arduino 2.x).
#pragma once
#include <stdint.h>

#include "bbp_protocol.h"

namespace spinlite {
namespace ble {

struct Provider {
  void (*status)(uint8_t out[bbp::PACKET]) = nullptr;            // A0
  void (*dump)(uint8_t out[bbp::DUMP_PAGES][bbp::PACKET]) = nullptr; // B0..B7, 70..73
  void (*clear)() = nullptr;                                     // comando 0x75
};

bool begin(const char *name, int8_t txPowerDbm, const Provider &p);

// Da chiamare spesso dal loop principale: esegue i comandi in coda e invia
// al massimo un pacchetto per chiamata, rispettando la spaziatura.
void loop();

// Durante MISURA: nessun invio (i comandi restano in coda). Al ritorno a
// false si riprende da dove si era.
void setQuiet(bool quiet);

// Dopo un lancio: A0 + dump completo, se un client e' iscritto alle notifiche.
void notifyLaunch();
// Solo A0 (es. cambio di stato PRONTO o batteria), se iscritto.
void notifyStatus();

bool connected();
bool subscribed();
uint32_t commandsReceived();

} // namespace ble
} // namespace spinlite
