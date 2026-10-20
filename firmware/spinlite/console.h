// SpinLite - console seriale (USB CDC, 115200), non bloccante.
//
// Comandi: aiuto, stato, raw [s], fronti, storico, cancella,
// set <parametro> <valore>, salva, default. Solo per Arduino.
#pragma once
#include <stdint.h>

#include "bbp_protocol.h"
#include "rpm_detector.h"

namespace spinlite {
namespace console {

// Cio' che la console puo' vedere e modificare; lo passa spinlite.ino.
struct Hooks {
  const bbp::History *history = nullptr; // storico (sola lettura)
  DetectorConfig *config = nullptr;      // soglie correnti: "set" le modifica
  void (*clearHistory)() = nullptr;      // comando "cancella"
};

void begin(const Hooks &hooks);

// Da chiamare a ogni giro di loop(): legge i caratteri disponibili, esegue le
// righe complete e alimenta "raw" e "fronti". Non aspetta mai.
void loop();

} // namespace console
} // namespace spinlite
