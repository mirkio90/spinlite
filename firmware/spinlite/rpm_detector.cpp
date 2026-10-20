#include "rpm_detector.h"

#include <algorithm>

namespace spinlite {
namespace {

constexpr uint64_t NS_PER_MIN = 60000000000ULL;
constexpr uint64_t MS = 1000000ULL;
constexpr uint64_t SETTLE_NS = 200 * MS;       // assestamento iniziale
constexpr uint64_t RESYNC_NS = 50 * MS;        // dopo un buco nei dati
constexpr uint64_t NOISE_QUIET_NS = 200 * MS;  // rumore stimato solo da fermo
constexpr uint64_t SWING_FORGET_NS = 2000 * MS;
constexpr uint64_t FAULT_NS = 1000 * MS;
constexpr int32_t FAULT_LOW = 30, FAULT_CLEAR = 100; // conteggi ADC
constexpr uint8_t MAX_BAD_RUN = 4;

uint16_t clamp16(uint64_t v) { return v > 65535 ? 65535 : static_cast<uint16_t>(v); }

uint64_t periodForRpm(uint32_t rpm) { return rpm ? NS_PER_MIN / rpm : UINT64_MAX; }

uint16_t rpmFor(uint64_t periodNs, uint32_t revolutions) {
  if (!periodNs) return 0;
  return clamp16((NS_PER_MIN * revolutions + periodNs / 2) / periodNs);
}

} // namespace

RpmDetector::RpmDetector() { configure(DetectorConfig()); }

void RpmDetector::configure(const DetectorConfig &cfg) {
  cfg_ = cfg;
  cfg_.smooth = std::min<uint8_t>(8, std::max<uint8_t>(1, cfg_.smooth));
  cfg_.marksPerRev = std::min<uint8_t>(8, std::max<uint8_t>(1, cfg_.marksPerRev));
  if (!cfg_.scale) cfg_.scale = 1;
  if (!cfg_.tickNs) cfg_.tickNs = 100000;
  if (!cfg_.endPercent || cfg_.endPercent > 95) cfg_.endPercent = 30;
  if (cfg_.minLaunchRpm < 100) cfg_.minLaunchRpm = 100;
  if (cfg_.maxRpm <= cfg_.minLaunchRpm) cfg_.maxRpm = 40000;
  if (!cfg_.noiseFactor) cfg_.noiseFactor = 16;
  unit_ = int32_t(cfg_.scale) * cfg_.smooth;
  reset();
}

void RpmDetector::reset() {
  ev_ = 0;
  smoothPos_ = smoothCount_ = 0;
  smoothSum_ = 0;
  index_ = 0;
  level_ = 0;
  diffPos_ = 0;
  diffCount_ = 0;
  noise_ = 10 * unit_;
  started_ = false;
  high_ = true;
  haveTrough_ = false;
  crossPending_ = false;
  swingPos_ = swingCount_ = 0;
  ringPos_ = 0;
  ringCount_ = 0;
  validRun_ = 0;
  lastValidRevNs_ = 0;
  lastEdgeNs_ = lastStrongNs_ = 0;
  anyStrong_ = false;
  lastEdge_ = EdgeInfo();
  phase_ = Phase::Settling;
  settleUntilNs_ = SETTLE_NS;
  quietSinceNs_ = launchStartNs_ = 0;
  bestRevNs_ = bestTripleNs_ = 0;
  edgesSinceCurve_ = 0;
  badRun_ = 0;
  cur_ = LaunchResult();
  result_ = LaunchResult();
  fault_ = low_ = false;
  lowSinceNs_ = 0;
  edgesTotal_ = launchesTotal_ = anomaliesTotal_ = 0;
}

void RpmDetector::dataLoss() {
  // Il tempo misurato in campioni non e' piu' continuo: si butta tutto cio'
  // che dipende dal tempo e si riparte con un breve assestamento.
  if (phase_ == Phase::Measuring) anomaliesTotal_++;
  smoothPos_ = smoothCount_ = 0;
  smoothSum_ = 0;
  started_ = false;
  haveTrough_ = false;
  crossPending_ = false;
  ringCount_ = 0;
  validRun_ = 0;
  badRun_ = 0;
  setPhase(Phase::Settling);
  settleUntilNs_ = nowNs() + RESYNC_NS;
}

void RpmDetector::setPhase(Phase p) {
  if (p == phase_) return;
  phase_ = p;
  ev_ |= EV_PHASE;
}

uint8_t RpmDetector::feed(int32_t sample) {
  ++index_;
  const uint64_t now = timeOf(index_);

  // Media mobile: toglie il rumore ad alta frequenza senza spostare i periodi.
  if (smoothCount_ < cfg_.smooth) {
    smoothCount_++;
  } else {
    smoothSum_ -= smoothBuf_[smoothPos_];
  }
  smoothBuf_[smoothPos_] = sample;
  smoothSum_ += sample;
  smoothPos_ = uint8_t((smoothPos_ + 1) % cfg_.smooth);
  if (smoothCount_ < cfg_.smooth) {
    const uint8_t out = ev_;
    ev_ = 0;
    return out;
  }
  const int32_t v = smoothSum_;
  hist_[index_ % HIST] = v;

  // Variazioni campione-campione: la loro mediana misura il rumore di fondo.
  if (started_) {
    int32_t d = v - level_;
    if (d < 0) d = -d;
    diffs_[diffPos_] = uint16_t(std::min<int32_t>(d, 65535));
    diffPos_ = uint8_t((diffPos_ + 1) % DIFFS);
    if (diffCount_ < DIFFS) diffCount_++;
    if (diffPos_ == 0) updateNoise();
  }
  level_ = v;

  // Segnale incollato a zero per 1 s: sensore staccato o senza alimentazione.
  const int32_t raw = v / unit_;
  if (raw < FAULT_LOW) {
    if (!low_) {
      low_ = true;
      lowSinceNs_ = now;
    } else if (!fault_ && now - lowSinceNs_ >= FAULT_NS) {
      fault_ = true;
    }
  } else {
    low_ = false;
    if (raw > FAULT_CLEAR) fault_ = false;
  }

  if (swingCount_ && now - lastEdgeNs_ > SWING_FORGET_NS) swingCount_ = 0;

  track(v);

  switch (phase_) {
  case Phase::Settling:
    if (now >= settleUntilNs_) {
      ringCount_ = 0;
      validRun_ = 0;
      setPhase(Phase::Ready);
    }
    break;
  case Phase::Measuring: {
    uint64_t gap = uint64_t(cfg_.endGapMs) * MS;
    const uint64_t slow = 4ULL * lastValidRevNs_;
    if (slow > gap) gap = slow;
    if (now - lastEdgeNs_ > gap) finish(EndReason::Gap);
    break;
  }
  case Phase::Rearm:
    if (now - quietSinceNs_ >= uint64_t(cfg_.quietMs) * MS) {
      ringCount_ = 0;
      validRun_ = 0;
      setPhase(Phase::Ready);
    }
    break;
  default:
    break;
  }

  const uint8_t out = ev_;
  ev_ = 0;
  return out;
}

int32_t RpmDetector::swingMedian() const {
  if (!swingCount_) return 0;
  int32_t tmp[SWINGS];
  std::copy(swings_, swings_ + swingCount_, tmp);
  std::sort(tmp, tmp + swingCount_);
  return tmp[swingCount_ / 2];
}

int32_t RpmDetector::hysteresis() const {
  int32_t h = noise_;
  const int32_t s = swingMedian() * cfg_.swingPercent / 100;
  return s > h ? s : h;
}

void RpmDetector::updateNoise() {
  // Durante una rotazione ogni campione cambia molto e la mediana direbbe
  // "rumore altissimo": si aggiorna solo quando non ci sono segni veri recenti.
  const bool quiet = !anyStrong_ || nowNs() - lastStrongNs_ >= NOISE_QUIET_NS;
  if (phase_ == Phase::Measuring || !quiet || diffCount_ < DIFFS) return;
  uint16_t tmp[DIFFS];
  std::copy(diffs_, diffs_ + DIFFS, tmp);
  std::nth_element(tmp, tmp + DIFFS / 2, tmp + DIFFS);
  const int32_t median = tmp[DIFFS / 2];
  noise_ = std::max<int32_t>(3 * unit_, int32_t(cfg_.noiseFactor) * median + unit_);
}

void RpmDetector::track(int32_t v) {
  if (!started_) {
    started_ = true;
    high_ = true;
    peak_ = trough_ = v;
    return;
  }
  const int32_t h = hysteresis();
  if (high_) {
    if (v > peak_) {
      peak_ = v;
    } else if (v <= peak_ - h) {
      confirmFall(v);
    }
    return;
  }

  if (v < trough_) trough_ = v;
  if (crossPending_ && v <= crossLevel_) {
    // Il segnale ha appena attraversato meta' ampiezza: interpolazione lineare
    // tra i due campioni per avere il tempo con risoluzione sotto i 100 us.
    const int32_t prev = hist_[(index_ - 1) % HIST];
    uint64_t t = timeOf(index_);
    if (prev > crossLevel_ && prev != v) {
      t = timeOf(index_ - 1) + uint64_t(int64_t(cfg_.tickNs) * (prev - crossLevel_) / (prev - v));
    }
    crossPending_ = false;
    emitEdge(t, crossAmp_);
  }
  if (v >= trough_ + h) {
    if (crossPending_) {
      // Segno troppo debole per scendere fino a meta': vale il tempo di conferma.
      crossPending_ = false;
      emitEdge(confirmNs_, crossAmp_);
    }
    const int32_t swing = peak_ - trough_;
    if (swing > 0) {
      swings_[swingPos_] = swing;
      swingPos_ = uint8_t((swingPos_ + 1) % SWINGS);
      if (swingCount_ < SWINGS) swingCount_++;
    }
    lastTrough_ = trough_;
    haveTrough_ = true;
    high_ = true;
    peak_ = v;
  }
}

void RpmDetector::confirmFall(int32_t v) {
  // Discesa confermata (v e' sceso di un'isteresi sotto il massimo). Il
  // fronte vero lo si prende dove il segnale passa a meta' tra massimo e
  // minimo: e' il punto meno sensibile alla forma e all'ampiezza del segno.
  high_ = false;
  trough_ = v;
  const int32_t amp = haveTrough_ ? peak_ - lastTrough_ : 0;
  const int32_t level = haveTrough_ ? (peak_ + lastTrough_) / 2 : v;
  confirmNs_ = timeOf(index_);
  if (v > level) {
    crossPending_ = true;
    crossLevel_ = level;
    crossAmp_ = amp;
    return;
  }
  uint64_t t = confirmNs_;
  for (uint64_t j = index_; j > 1 && index_ - j + 1 < HIST; --j) {
    const int32_t cur = hist_[j % HIST], prev = hist_[(j - 1) % HIST];
    if (prev > level && cur <= level) {
      t = timeOf(j - 1) + uint64_t(int64_t(cfg_.tickNs) * (prev - level) / (prev - cur));
      break;
    }
  }
  emitEdge(t, amp);
}

void RpmDetector::emitEdge(uint64_t tNs, int32_t amplitude) {
  const int32_t counts = amplitude > 0 ? amplitude / unit_ : 0;
  onEdge(tNs, uint16_t(std::min<int32_t>(counts, 65535)));
}

void RpmDetector::onEdge(uint64_t t, uint16_t amp) {
  edgesTotal_++;
  ev_ |= EV_EDGE;
  const unsigned m = cfg_.marksPerRev;
  const uint64_t minRevNs = periodForRpm(cfg_.maxRpm);
  const uint64_t startMaxNs = periodForRpm(cfg_.minLaunchRpm);
  const uint64_t maxTrackNs = 4 * startMaxNs;
  const bool strong = amp >= cfg_.minMark;

  // Periodo di un giro: distanza dallo stesso segno un giro fa. edgeAt(0) e'
  // il fronte precedente, quindi con m segni per giro e' edgeAt(m - 1).
  uint64_t rev = ringCount_ >= m ? t - edgeAt(m - 1) : 0;
  lastEdge_ = EdgeInfo();
  lastEdge_.timeNs = t;
  lastEdge_.amplitude = amp;
  lastEdge_.revPeriodNs = uint32_t(std::min<uint64_t>(rev, UINT32_MAX));

  // Fronte spurio (un disturbo in mezzo a un giro): accorcerebbe il periodo.
  // Lo si scarta senza memorizzarlo, cosi' il giro dopo torna giusto.
  const bool spurious =
      rev && (rev < minRevNs ||
              (phase_ == Phase::Measuring && lastValidRevNs_ && rev * 100 < uint64_t(lastValidRevNs_) * 55));
  if (spurious) {
    anomaliesTotal_++;
    if (phase_ == Phase::Measuring && ++badRun_ >= MAX_BAD_RUN) finish(EndReason::Signal);
    return;
  }

  bool valid = rev && rev <= maxTrackNs;
  // Un giro lungo piu' di 1,67 volte il precedente: o un segno perso o una
  // frenata brusca. In entrambi i casi non e' un periodo affidabile.
  if (valid && validRun_ && lastValidRevNs_ && rev * 100 > uint64_t(lastValidRevNs_) * 167) valid = false;
  lastEdge_.valid = valid;

  const uint64_t triple = (valid && ringCount_ >= 3 * m) ? t - edgeAt(3 * m - 1) : 0;

  switch (phase_) {
  case Phase::Ready:
    if (valid && validRun_ + 1 >= 3 * m && ringCount_ >= 3 * m) {
      const uint64_t p1 = rev;
      const uint64_t p2 = edgeAt(m - 1) - edgeAt(2 * m - 1);
      const uint64_t p3 = edgeAt(2 * m - 1) - edgeAt(3 * m - 1);
      const uint64_t lo = std::min(p1, std::min(p2, p3));
      const uint64_t hi = std::max(p1, std::max(p2, p3));
      bool marks = strong;
      uint16_t weakest = amp;
      for (unsigned i = 0; i < 3 * m; ++i) {
        marks = marks && ampAt(i) >= cfg_.minMark;
        weakest = std::min(weakest, ampAt(i));
      }
      if (marks && hi <= startMaxNs && hi * 10 <= lo * 16) {
        // Tre giri coerenti sopra la soglia: e' un lancio.
        setPhase(Phase::Measuring);
        cur_ = LaunchResult();
        cur_.weakestMark = weakest;
        launchStartNs_ = edgeAt(3 * m - 1);
        bestRevNs_ = uint32_t(lo);
        bestTripleNs_ = uint32_t(triple);
        edgesSinceCurve_ = 0;
        badRun_ = 0;
        const uint64_t revs[3] = {p3, p2, p1};
        for (uint64_t r : revs) {
          if (cur_.curveCount < LaunchResult::CURVE_MAX) cur_.curveUs[cur_.curveCount++] = uint32_t((r + 500) / 1000);
          cur_.revolutions++;
        }
      }
    }
    break;

  case Phase::Measuring:
    if (valid) {
      badRun_ = 0;
      cur_.weakestMark = std::min(cur_.weakestMark, amp);
      if (++edgesSinceCurve_ >= m) {
        edgesSinceCurve_ = 0;
        cur_.revolutions++;
        if (cur_.curveCount < LaunchResult::CURVE_MAX) cur_.curveUs[cur_.curveCount++] = uint32_t((rev + 500) / 1000);
      }
      if (rev < bestRevNs_) bestRevNs_ = uint32_t(rev);
      if (validRun_ + 1 >= 3 * m && triple && (!bestTripleNs_ || triple < bestTripleNs_)) bestTripleNs_ = uint32_t(triple);
    } else {
      anomaliesTotal_++;
      ++badRun_;
    }
    break;

  case Phase::Rearm:
    // Ogni segno vero durante il riarmo (rallentamento, riavvolgimento della
    // corda) fa ripartire l'attesa.
    if (strong) quietSinceNs_ = t;
    break;

  default:
    break;
  }

  // Memorizza il fronte.
  ringT_[ringPos_] = t;
  ringA_[ringPos_] = amp;
  ringPos_ = uint8_t((ringPos_ + 1) % RING);
  ringCount_++;
  lastEdgeNs_ = t;
  if (strong) {
    lastStrongNs_ = t;
    anyStrong_ = true;
  }
  if (valid) {
    validRun_++;
    lastValidRevNs_ = uint32_t(rev);
  } else {
    validRun_ = 0;
  }

  if (phase_ == Phase::Measuring) {
    const uint64_t ref = bestTripleNs_ ? bestTripleNs_ / 3 : bestRevNs_;
    if (rev && ref && rev * cfg_.endPercent > ref * 100) {
      finish(EndReason::Slowdown);
    } else if (badRun_ >= MAX_BAD_RUN) {
      finish(EndReason::Signal);
    }
  }
}

void RpmDetector::finish(EndReason why) {
  cur_.end = why;
  cur_.sustainedRpm = bestTripleNs_ ? rpmFor(bestTripleNs_, 3) : 0;
  cur_.singleRpm = bestRevNs_ ? rpmFor(bestRevNs_, 1) : 0;
  cur_.rpm = (cfg_.singleRevPeak || !cur_.sustainedRpm) ? cur_.singleRpm : cur_.sustainedRpm;
  cur_.durationMs = uint32_t((lastEdgeNs_ - launchStartNs_) / MS);
  const bool ok = why != EndReason::DataLoss && cur_.revolutions >= 3 && cur_.rpm >= cfg_.minLaunchRpm;
  if (ok) {
    result_ = cur_;
    launchesTotal_++;
    ev_ |= EV_RESULT;
  }
  quietSinceNs_ = nowNs();
  setPhase(Phase::Rearm);
}

uint16_t RpmDetector::noiseCounts() const { return uint16_t(noise_ / unit_); }
uint16_t RpmDetector::swingCounts() const { return uint16_t(swingMedian() / unit_); }
uint16_t RpmDetector::hysteresisCounts() const { return uint16_t(hysteresis() / unit_); }
uint16_t RpmDetector::levelCounts() const { return uint16_t(std::max<int32_t>(0, level_ / unit_)); }

uint16_t RpmDetector::liveRpm() const {
  return (phase_ == Phase::Measuring && lastValidRevNs_) ? rpmFor(lastValidRevNs_, 1) : 0;
}

const char *phaseName(Phase p) {
  switch (p) {
  case Phase::Settling: return "AVVIO";
  case Phase::Ready: return "PRONTO";
  case Phase::Measuring: return "MISURA";
  case Phase::Rearm: return "RIARMO";
  }
  return "?";
}

const char *endName(EndReason e) {
  switch (e) {
  case EndReason::None: return "-";
  case EndReason::Slowdown: return "rallentamento";
  case EndReason::Gap: return "pausa";
  case EndReason::Signal: return "segnale";
  case EndReason::DataLoss: return "dati persi";
  }
  return "?";
}

} // namespace spinlite
