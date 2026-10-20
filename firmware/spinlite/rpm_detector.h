// SpinLite - rilevatore di giri da segnale ottico analogico.
//
// Codice portabile: niente Arduino, niente float, niente allocazioni. Gira
// sull'ESP32-C3 (che non ha FPU) e sul PC per i test con segnali simulati.
//
// Idee riprese (non il codice) da LaunchLab, che ha documentato bene cosa va
// storto con un sensore a riflessione puntato su un launcher:
//   - soglie adattive invece di un comparatore a soglia fissa;
//   - un lancio parte solo dopo 3 giri coerenti sopra 1000 RPM;
//   - dopo il lancio si aspetta 1 s di quiete: il riavvolgimento della corda
//     fa girare l'albero come un nuovo lancio e un solo sensore non vede il verso;
//   - il tempo si misura contando i campioni ADC, non con l'orologio del loop.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace spinlite {

struct DetectorConfig {
  uint32_t tickNs = 100000;  // tempo tra due campioni passati a feed()
  uint8_t scale = 4;         // ogni campione e' la somma di `scale` letture ADC
  uint8_t smooth = 4;        // media mobile sui campioni (1..8)
  uint8_t marksPerRev = 1;   // segni ottici per giro (1..8)
  uint16_t minMark = 150;    // ampiezza minima di un segno, conteggi ADC
  uint16_t minLaunchRpm = 1000;
  uint16_t maxRpm = 40000;
  uint8_t endPercent = 30;   // fine lancio sotto questa % del picco
  uint16_t endGapMs = 250;   // fine lancio se mancano segni per questo tempo
  uint16_t quietMs = 1000;   // quiete richiesta prima di riarmare
  uint8_t swingPercent = 30; // isteresi = questa % dell'ampiezza tipica dei segni
  uint8_t noiseFactor = 16;  // isteresi minima = fattore x mediana delle variazioni
  bool singleRevPeak = false;
};

enum class Phase : uint8_t { Settling, Ready, Measuring, Rearm };
enum class EndReason : uint8_t { None, Slowdown, Gap, Signal, DataLoss };

struct LaunchResult {
  static constexpr unsigned CURVE_MAX = 32;
  uint16_t rpm = 0;          // valore riportato (3 giri o giro singolo)
  uint16_t sustainedRpm = 0; // miglior media su 3 giri consecutivi
  uint16_t singleRpm = 0;    // giro singolo piu' veloce
  uint16_t revolutions = 0;  // giri validi contati
  uint32_t durationMs = 0;
  uint16_t weakestMark = 0;  // segno piu' debole, conteggi ADC
  EndReason end = EndReason::None;
  uint8_t curveCount = 0;
  uint32_t curveUs[CURVE_MAX] = {}; // periodo dei primi 32 giri, in microsecondi
};

struct EdgeInfo {
  uint64_t timeNs = 0;
  uint32_t revPeriodNs = 0;
  uint16_t amplitude = 0; // conteggi ADC
  bool valid = false;
};

class RpmDetector {
public:
  enum Event : uint8_t { EV_NONE = 0, EV_EDGE = 1, EV_PHASE = 2, EV_RESULT = 4 };

  RpmDetector();
  void configure(const DetectorConfig &cfg); // applica e riparte da zero
  const DetectorConfig &config() const { return cfg_; }
  void reset();
  // Buco nei campioni (buffer ADC traboccato): un lancio in corso e' perso.
  void dataLoss();
  // Un campione alla volta, a intervalli regolari di tickNs. Ritorna Event.
  uint8_t feed(int32_t sample);

  Phase phase() const { return phase_; }
  const LaunchResult &result() const { return result_; }
  const EdgeInfo &lastEdge() const { return lastEdge_; }
  uint16_t noiseCounts() const;
  uint16_t swingCounts() const;
  uint16_t hysteresisCounts() const;
  uint16_t levelCounts() const;
  uint16_t liveRpm() const;
  bool signalFault() const { return fault_; }
  uint32_t edges() const { return edgesTotal_; }
  uint32_t launches() const { return launchesTotal_; }
  uint32_t anomalies() const { return anomaliesTotal_; }
  uint64_t nowNs() const { return index_ * cfg_.tickNs; }

private:
  static constexpr unsigned HIST = 64;   // campioni recenti, per l'interpolazione
  static constexpr unsigned DIFFS = 64;  // variazioni recenti, per stimare il rumore
  static constexpr unsigned SWINGS = 5;  // ampiezze recenti dei segni
  static constexpr unsigned RING = 32;   // fronti recenti (>= 3*8+1)

  void setPhase(Phase p);
  void track(int32_t v);
  void confirmFall(int32_t v);
  void emitEdge(uint64_t tNs, int32_t amplitude);
  void onEdge(uint64_t tNs, uint16_t amp);
  void finish(EndReason why);
  void updateNoise();
  int32_t hysteresis() const;
  int32_t swingMedian() const;
  uint64_t timeOf(uint64_t index) const { return index * cfg_.tickNs; }
  uint64_t edgeAt(unsigned back) const { return ringT_[(ringPos_ + RING - 1 - back) % RING]; }
  uint16_t ampAt(unsigned back) const { return ringA_[(ringPos_ + RING - 1 - back) % RING]; }
  uint32_t revAt(unsigned back) const { return ringR_[(ringPos_ + RING - 1 - back) % RING]; }

  DetectorConfig cfg_;
  int32_t unit_ = 16;
  uint8_t ev_ = 0;

  // media mobile
  int32_t smoothBuf_[8] = {};
  uint8_t smoothPos_ = 0, smoothCount_ = 0;
  int32_t smoothSum_ = 0;

  uint64_t index_ = 0; // indice del campione corrente
  int32_t hist_[HIST] = {};
  int32_t level_ = 0;

  // rumore
  uint16_t diffs_[DIFFS] = {};
  uint8_t diffPos_ = 0;
  uint16_t diffCount_ = 0;
  int32_t noise_ = 0;

  // inseguimento di massimi e minimi
  bool started_ = false, high_ = true, haveTrough_ = false;
  int32_t peak_ = 0, trough_ = 0, lastTrough_ = 0;
  bool crossPending_ = false;
  int32_t crossLevel_ = 0, crossAmp_ = 0;
  uint64_t confirmNs_ = 0;
  int32_t swings_[SWINGS] = {};
  uint8_t swingPos_ = 0, swingCount_ = 0;

  // fronti e giri
  uint64_t ringT_[RING] = {};
  uint16_t ringA_[RING] = {};
  uint32_t ringR_[RING] = {}; // periodo del giro chiuso da quel fronte, 0 se non valido
  uint8_t ringPos_ = 0;
  uint32_t ringCount_ = 0;
  uint32_t validRun_ = 0;
  uint32_t lastValidRevNs_ = 0;
  uint64_t lastEdgeNs_ = 0;   // ultimo fronte qualsiasi
  uint64_t lastStrongNs_ = 0; // ultimo fronte con ampiezza >= minMark
  bool anyStrong_ = false;
  EdgeInfo lastEdge_;

  // lancio
  Phase phase_ = Phase::Settling;
  uint64_t settleUntilNs_ = 0, quietSinceNs_ = 0, launchStartNs_ = 0;
  uint32_t bestRevNs_ = 0, bestTripleNs_ = 0, fastestNs_ = 0, anyTripleNs_ = 0;
  uint32_t edgesSinceCurve_ = 0;
  uint8_t badRun_ = 0;
  LaunchResult cur_, result_;

  bool fault_ = false;
  uint64_t lowSinceNs_ = 0;
  bool low_ = false;

  uint32_t edgesTotal_ = 0, launchesTotal_ = 0, anomaliesTotal_ = 0;
};

const char *phaseName(Phase p);
const char *endName(EndReason e);

} // namespace spinlite
