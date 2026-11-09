// SpinLite - impostazioni e storico in memoria flash (NVS, namespace "spinlite").
//
// La codifica dei parametri e' portabile (testata sul PC); il salvataggio usa
// Preferences ed esiste solo su Arduino.
#pragma once
#include <stddef.h>
#include <stdint.h>

#include "bbp_protocol.h"
#include "rpm_detector.h"

namespace spinlite {
namespace settings {

// Parametri del rilevatore modificabili da console ai valori di config.h.
void defaults(DetectorConfig &cfg);

// Blob "cfg": versione + campi persistenti + CRC32. Solo i campi
// marksPerRev, minMark, minLaunchRpm, maxRpm, endPercent, endGapMs, quietMs,
// singleRevPeak; gli altri campi di cfg non vengono toccati da decodeConfig.
constexpr size_t CONFIG_BYTES = 4 + 1 + 1 + 2 + 2 + 2 + 1 + 2 + 2 + 1 + 4;
size_t encodeConfig(const DetectorConfig &cfg, uint8_t *buf, size_t len);
// false (cfg intatto) se versione, lunghezza o CRC non tornano, o se un
// valore e' fuori dai limiti accettati da setParam.
bool decodeConfig(const uint8_t *buf, size_t len, DetectorConfig &cfg);

// Imposta un parametro per nome (nomi della console: marks, minmark,
// minrpm, maxrpm, endpct, endgap, quiet, single). false se il nome non esiste
// o il valore e' fuori limite; in quel caso cfg resta intatto.
bool setParam(DetectorConfig &cfg, const char *name, long value);
// Scrive "nome=valore" per ogni parametro, separati da spazio.
size_t describe(const DetectorConfig &cfg, char *out, size_t len);

#ifdef ARDUINO
void begin();
bool loadConfig(DetectorConfig &cfg);
bool saveConfig(const DetectorConfig &cfg);
void eraseConfig();
bool loadHistory(bbp::History &h);
bool saveHistory(const bbp::History &h);
#endif

} // namespace settings
} // namespace spinlite
