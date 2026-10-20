// Simulatore del segnale AO di un sensore a riflessione puntato sull'albero del
// launcher. Produce gli stessi campioni che il firmware passa al rilevatore
// (somma di 4 letture ADC ogni 100 us) e tiene traccia dei tempi "veri" a cui
// il segno passa davanti al sensore, per confrontare il risultato misurato.
#pragma once
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

namespace sim {

struct Optics {
  double baseline = 3000;  // AO senza segno davanti (conteggi ADC)
  double depth = 1500;     // quanto il segno abbassa AO
  double duty = 0.35;      // frazione di giro occupata dal segno
  double noise = 6;        // rumore gaussiano (sigma, conteggi)
  double wander = 20;      // deriva lenta (ampiezza, conteggi)
  int marks = 1;           // segni per giro, equidistanti salvo `markOffset`
  double markOffset = 0;   // sposta il 2o segno (frazione di giro) per segni irregolari
};

// Velocita' dell'albero in RPM in funzione del tempo (s). Negativa = verso opposto.
using Profile = std::function<double(double)>;

class Signal {
public:
  static constexpr double CONV_HZ = 50000; // conversioni ADC al secondo
  static constexpr int SENSOR_PER_OUT = 4; // pattern S,S,S,S,B

  Signal(Optics o, Profile p, uint32_t seed = 1) : o_(o), p_(std::move(p)), rng_(seed), n_(0, 1) {}

  // Prossimo campione per il rilevatore (somma di 4 letture del sensore).
  int32_t next() {
    int32_t sum = 0;
    for (int i = 0; i < SENSOR_PER_OUT + 1; ++i) {
      step();
      if (i < SENSOR_PER_OUT) sum += read();
    }
    return sum;
  }

  double time() const { return t_; }
  // Tempi (s) a cui il primo segno e' passato davanti al sensore, in avanti.
  const std::vector<double> &markTimes() const { return marks_; }
  // Iniezioni di difetti.
  std::function<double(double t, double value)> distort;

private:
  void step() {
    const double dt = 1.0 / CONV_HZ;
    const double rpm = p_(t_);
    const double before = theta_;
    theta_ += rpm / 60.0 * dt; // in giri
    // registra il passaggio del primo segno (giri interi) in avanti
    if (rpm > 0 && std::floor(theta_) > std::floor(before)) {
      const double frac = (std::floor(theta_) - before) / (theta_ - before);
      marks_.push_back(t_ + frac * dt);
    }
    t_ += dt;
  }

  double shape(double phase) const {
    // segno a coseno rialzato centrato a phase=0, largo `duty` di giro
    phase -= std::floor(phase);
    if (phase > 0.5) phase -= 1.0;
    const double half = o_.duty / o_.marks / 2;
    if (std::fabs(phase) >= half) return 0;
    return 0.5 * (1 + std::cos(M_PI * phase / half));
  }

  int32_t read() {
    double s = 0;
    for (int k = 0; k < o_.marks; ++k) {
      double offset = double(k) / o_.marks + (k == 1 ? o_.markOffset : 0);
      // il fronte di discesa misurato e' a meta' discesa: spostiamo il segno
      // in modo che il "passaggio" registrato coincida con il suo centro
      s = std::max(s, shape(theta_ - offset));
    }
    double v = o_.baseline - o_.depth * s + o_.wander * std::sin(2 * M_PI * 0.37 * t_) + o_.noise * n_(rng_);
    if (distort) v = distort(t_, v);
    if (v < 0) v = 0;
    if (v > 4095) v = 4095;
    return int32_t(std::lround(v));
  }

  Optics o_;
  Profile p_;
  std::mt19937 rng_;
  std::normal_distribution<double> n_;
  double t_ = 0, theta_ = 0.25; // parte con il segno lontano dal sensore
  std::vector<double> marks_;
};

// Picchi "veri" calcolati dai passaggi del segno: giro singolo e media su 3 giri.
struct TruePeak {
  double single = 0, sustained = 0;
};

inline TruePeak truePeak(const std::vector<double> &t, double from, double to) {
  TruePeak p;
  for (size_t i = 1; i < t.size(); ++i) {
    if (t[i - 1] < from || t[i] > to) continue;
    p.single = std::max(p.single, 60.0 / (t[i] - t[i - 1]));
    if (i >= 3 && t[i - 3] >= from) p.sustained = std::max(p.sustained, 180.0 / (t[i] - t[i - 3]));
  }
  return p;
}

// Profilo di un lancio con la corda: accelera fino al picco in `pull` secondi,
// poi la ruota libera rallenta; con `rewind` > 0 la corda riavvolgendosi fa
// girare l'albero all'indietro.
inline Profile pullProfile(double start, double peakRpm, double pull = 0.18, double decay = 0.25,
                           double rewindRpm = 0, double rewindAfter = 0.35, double rewindLen = 0.5) {
  return [=](double t) {
    const double x = t - start;
    if (x < 0) return 0.0;
    if (x < pull) return peakRpm * std::pow(x / pull, 0.8);
    const double y = x - pull;
    if (rewindRpm > 0 && y >= rewindAfter && y < rewindAfter + rewindLen) {
      const double r = (y - rewindAfter) / rewindLen;
      return -rewindRpm * std::sin(M_PI * r);
    }
    const double v = peakRpm * std::exp(-y / decay);
    return v < 30 ? 0.0 : v;
  };
}

inline Profile sumProfiles(Profile a, Profile b) {
  return [=](double t) { return a(t) + b(t); };
}

} // namespace sim
