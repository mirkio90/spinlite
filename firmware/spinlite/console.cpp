#include "console.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "adc_sampler.h"
#include "autotest.h"
#include "battery.h"
#include "ble_bbp.h"
#include "config.h"
#include "settings.h"

namespace spinlite {
namespace console {
namespace {

Hooks hooks;
char line[64];
size_t lineLen = 0;
bool lineOverflow = false;

bool rawOn = false;
uint32_t rawEndMs = 0, rawNextMs = 0;
bool edgeOn = false;

void help() {
  Serial.println("Comandi:");
  Serial.println("  aiuto                      questo elenco");
  Serial.println("  stato                      fase, segnale, frequenza, batteria, BLE");
  Serial.println("  raw [s]                    livello/rumore/ampiezza a 20 Hz (default 10 s)");
  Serial.println("  fronti                     attiva/disattiva la stampa di ogni fronte");
  Serial.println("  storico                    lanci in memoria");
  Serial.println("  cancella                   azzera lo storico");
  Serial.println("  set <param> <valore>       marks minmark minrpm maxrpm endpct endgap quiet single");
  Serial.println("  salva                      salva le soglie in memoria");
  Serial.println("  default                    ripristina le soglie di fabbrica");
  Serial.println("  test [rpm] [s]             lancio finto su GPIO10 (filo GPIO10 -> GPIO3, sensore staccato)");
  Serial.println("  test stop                  ferma il test e libera GPIO10");
}

void printConfig() {
  char buf[160];
  settings::describe(*hooks.config, buf, sizeof buf);
  Serial.println(buf);
}

void status() {
  const LiveStatus s = sampler::status();
  Serial.printf("Sensore: %s\n", SPINLITE_SENSOR_NAME);
  Serial.printf("Fase: %s%s\n", phaseName(s.phase), s.fault ? " (GUASTO sensore)" : "");
  Serial.printf("Segnale: livello=%u rumore=%u ampiezza=%u isteresi=%u rpm=%u\n", s.level, s.noise, s.swing,
                s.hysteresis, s.liveRpm);
  if (sampler::calibrated()) {
    const uint32_t f = sampler::rateMilliHz();
    Serial.printf("Frequenza misurata: %lu.%03lu Hz (tick %lu ns)\n", (unsigned long)(f / 1000),
                  (unsigned long)(f % 1000), (unsigned long)sampler::tickNs());
  } else {
    Serial.println("Frequenza: calibrazione in corso");
  }
  const int mv = sampler::batteryMv();
  if (mv < 0) {
    Serial.println("Batteria: non misurata");
  } else {
    const int pc = batteryPercent(mv);
    if (pc < 0)
      Serial.printf("Batteria: %d mV (alimentato da USB)\n", mv);
    else
      Serial.printf("Batteria: %d mV, circa %d%% (stima)\n", mv, pc);
  }
  Serial.printf("BLE: %s, notifiche %s, comandi ricevuti %lu\n", ble::connected() ? "connesso" : "non connesso",
                ble::subscribed() ? "attive" : "no", (unsigned long)ble::commandsReceived());
  Serial.printf("Fronti %lu, lanci %lu, anomalie %lu, overflow DMA %lu\n", (unsigned long)s.edges,
                (unsigned long)s.launches, (unsigned long)s.anomalies, (unsigned long)s.overflows);
  if (hooks.history)
    Serial.printf("Storico: totale %lu, massimo %u RPM, in memoria %u\n", (unsigned long)hooks.history->total,
                  hooks.history->best, hooks.history->count);
  printConfig();
}

void history() {
  const bbp::History *h = hooks.history;
  if (!h) return;
  Serial.printf("Totale %lu, massimo %u, in memoria %u (dal piu' vecchio)\n", (unsigned long)h->total, h->best,
                h->count);
  for (uint8_t i = 0; i < h->count; ++i) Serial.printf("%u: %u\n", i + 1, bbp::chronological(*h, i));
}

void startRaw(const char *arg) {
  long sec = 10;
  if (arg) {
    char *end = nullptr;
    sec = strtol(arg, &end, 10);
    if (end == arg || *end || sec < 1 || sec > 600) {
      Serial.println("Uso: raw [secondi 1..600]");
      return;
    }
  }
  Serial.println("livello,rumore,ampiezza,isteresi,fase,rpm  (un tasto per fermare)");
  rawOn = true;
  rawEndMs = millis() + uint32_t(sec) * 1000u;
  rawNextMs = millis();
}

void rawTick() {
  const uint32_t now = millis();
  if (int32_t(now - rawEndMs) >= 0) {
    rawOn = false;
    Serial.println("raw terminato");
    return;
  }
  if (int32_t(now - rawNextMs) < 0) return;
  rawNextMs += 50;
  if (int32_t(now - rawNextMs) > 0) rawNextMs = now + 50;
  const LiveStatus s = sampler::status();
  Serial.printf("%u,%u,%u,%u,%s,%u\n", s.level, s.noise, s.swing, s.hysteresis, phaseName(s.phase), s.liveRpm);
}

void edgeTick() {
  EdgeInfo e;
  for (int i = 0; i < 8 && sampler::popEdge(e); ++i) {
    const uint32_t rpm = e.revPeriodNs ? uint32_t(60000000000ull / e.revPeriodNs) : 0;
    Serial.printf("FRONTE,%lu,%lu,%lu,%u,%s\n", (unsigned long)(e.timeNs / 1000000ull),
                  (unsigned long)(e.revPeriodNs / 1000u), (unsigned long)rpm, e.amplitude,
                  e.valid ? "valido" : "scartato");
  }
}

// "test [rpm] [secondi]" e "test stop": lancio finto con velocita' nota.
void startTest(int n, char **tok) {
  if (n > 1 && !strcmp(tok[1], "stop")) {
    autotest::stop();
    Serial.println("TEST fermato, GPIO10 libero");
    return;
  }
  long rpm = 3000, sec = 1;
  char *end = nullptr;
  if (n > 1) rpm = strtol(tok[1], &end, 10);
  if (n > 1 && (end == tok[1] || *end)) rpm = -1;
  if (n > 2) sec = strtol(tok[2], &end, 10);
  if (n > 2 && (end == tok[2] || *end)) sec = -1;
  uint32_t half = 0;
  const uint8_t marks = hooks.config ? hooks.config->marksPerRev : 1;
  if (rpm < 300 || rpm > 40000 || sec < 1 || sec > 10 ||
      !autotest::start(uint32_t(rpm), uint32_t(sec) * 1000u, marks, &half)) {
    Serial.println("Uso: test [rpm 300..40000] [secondi 1..10]   oppure: test stop");
    return;
  }
  // velocita' effettiva con il mezzo periodo intero, in decimi di RPM
  const uint32_t rpm10 = uint32_t(300000000ull / (uint64_t(half) * marks));
  Serial.printf("TEST: GPIO10 -> GPIO3, %ld RPM per %ld s (mezzo periodo %lu us, attesi %lu.%lu RPM), poi rallenta\n",
                rpm, sec, (unsigned long)half, (unsigned long)(rpm10 / 10), (unsigned long)(rpm10 % 10));
  Serial.println("TEST: 1,5 s di riposo, poi parte. Il risultato arriva come riga LANCIO.");
}

void execute(char *cmd) {
  char *tok[4];
  int n = 0;
  for (char *p = strtok(cmd, " \t"); p && n < 4; p = strtok(nullptr, " \t")) tok[n++] = p;
  if (n == 0) return;
  for (char *ch = tok[0]; *ch; ++ch)
    if (*ch >= 'A' && *ch <= 'Z') *ch = char(*ch + 32);
  const char *c = tok[0];

  if (!strcmp(c, "aiuto") || !strcmp(c, "help") || !strcmp(c, "?")) {
    help();
  } else if (!strcmp(c, "stato")) {
    status();
  } else if (!strcmp(c, "raw")) {
    startRaw(n > 1 ? tok[1] : nullptr);
  } else if (!strcmp(c, "fronti")) {
    edgeOn = !edgeOn;
    sampler::setEdgeTrace(edgeOn);
    Serial.println(edgeOn ? "FRONTE,tempo_ms,periodo_us,rpm,ampiezza,esito" : "fronti disattivati");
  } else if (!strcmp(c, "storico")) {
    history();
  } else if (!strcmp(c, "cancella")) {
    if (hooks.clearHistory) hooks.clearHistory();
    Serial.println("Storico azzerato");
  } else if (!strcmp(c, "set")) {
    if (n != 3) {
      Serial.println("Uso: set <param> <valore>   (marks minmark minrpm maxrpm endpct endgap quiet single)");
      return;
    }
    char *end = nullptr;
    const long v = strtol(tok[2], &end, 10);
    if (end == tok[2] || *end || !settings::setParam(*hooks.config, tok[1], v)) {
      Serial.println("Parametro sconosciuto o valore fuori limite");
      return;
    }
    sampler::reconfigure(*hooks.config);
    printConfig();
    Serial.println("Applicato (non salvato: usa 'salva')");
  } else if (!strcmp(c, "salva")) {
    Serial.println(settings::saveConfig(*hooks.config) ? "Soglie salvate" : "Salvataggio fallito");
  } else if (!strcmp(c, "test")) {
    startTest(n, tok);
  } else if (!strcmp(c, "default")) {
    settings::defaults(*hooks.config);
    settings::eraseConfig();
    sampler::reconfigure(*hooks.config);
    printConfig();
    Serial.println("Soglie di fabbrica ripristinate");
  } else {
    Serial.println("Comando sconosciuto, prova 'aiuto'");
  }
}

} // namespace

void begin(const Hooks &h) {
  hooks = h;
  lineLen = 0;
}

void loop() {
  int budget = 128; // caratteri per chiamata
  while (budget-- > 0 && Serial.available() > 0) {
    const int ch = Serial.read();
    if (ch < 0) break;
    if (rawOn) { // qualsiasi tasto interrompe
      rawOn = false;
      Serial.println("raw interrotto");
      continue;
    }
    if (ch == '\r' || ch == '\n') {
      if (lineOverflow) {
        Serial.println("Riga troppo lunga");
      } else if (lineLen > 0) {
        line[lineLen] = 0;
        execute(line);
      }
      lineLen = 0;
      lineOverflow = false;
    } else if (lineLen < sizeof line - 1) {
      line[lineLen++] = char(ch);
    } else {
      lineOverflow = true;
    }
  }
  if (rawOn) rawTick();
  if (edgeOn) edgeTick();
  if (autotest::loop()) Serial.println("TEST: segnale fermo, GPIO10 resta alto (riposo). 'test stop' per liberarlo");
}

} // namespace console
} // namespace spinlite
