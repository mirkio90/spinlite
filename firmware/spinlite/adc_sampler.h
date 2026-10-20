// SpinLite - acquisizione continua dall'ADC1 (DMA) e rilevatore in un task.
//
// Un task FreeRTOS ad alta priorita' legge i frame di adc_continuous, somma 4
// letture del sensore per campione (10 kHz utili), chiama RpmDetector::feed()
// e pubblica:
//   - i lanci finiti su una coda (popResult);
//   - uno snapshot dello stato ogni ~50 ms (status);
//   - i fronti, se la traccia e' attiva (popEdge), per il comando "fronti";
//   - la tensione della batteria, media dei campioni B su ~1 s.
//
// All'avvio misura per ~2 s la frequenza vera dei campioni contro
// esp_timer_get_time() (che segue il quarzo) e riconfigura il rilevatore con
// il tickNs misurato: l'errore sugli RPM e' quello sulla frequenza ADC.
//
// Solo per ESP32 (Arduino-ESP32 3.x, ESP-IDF 5.x).
#pragma once
#include <stdint.h>

#include "rpm_detector.h"

namespace spinlite {

struct LiveStatus {
  Phase phase = Phase::Settling;
  uint16_t level = 0;      // livello del segnale, conteggi ADC
  uint16_t noise = 0;
  uint16_t swing = 0;      // ampiezza tipica dei segni
  uint16_t hysteresis = 0;
  uint16_t liveRpm = 0;
  bool fault = false;      // sensore scollegato o segnale assente
  uint32_t edges = 0, launches = 0, anomalies = 0;
  uint32_t overflows = 0;  // buffer DMA traboccato (dati persi)
};

namespace sampler {

// Avvia ADC e task. cfg.tickNs/scale/smooth vengono ignorati: li decide il
// campionatore. Ritorna false se l'ADC non parte.
bool begin(const DetectorConfig &cfg);

// Nuove soglie dalla console. Thread-safe: il task le applica al prossimo
// frame, mantenendo tickNs, scale e smooth correnti. Il rilevatore riparte.
void reconfigure(const DetectorConfig &cfg);

// Configurazione attiva nel rilevatore (copia).
DetectorConfig config();

// Lancio finito, se c'e'. timeoutMs = 0: non aspetta.
bool popResult(LaunchResult &out, uint32_t timeoutMs = 0);

LiveStatus status();

// true dopo la misura della frequenza (circa 2 s dopo begin).
bool calibrated();
// Frequenza misurata dei campioni passati al rilevatore, in mHz
// (10.000.000 = 10 kHz esatti). 0 prima della misura.
uint32_t rateMilliHz();
uint32_t tickNs();

// Batteria in mV (gia' moltiplicata per il partitore). -1 se PIN_BATTERY < 0
// o se la prima media non e' ancora pronta.
int batteryMv();

// Traccia dei fronti per la console. Coda piccola: se nessuno legge si perdono.
void setEdgeTrace(bool on);
bool popEdge(EdgeInfo &out);

} // namespace sampler
} // namespace spinlite
