// Test del rilevatore con segnali simulati: g++ -std=c++17, nessuna dipendenza.
#include "../firmware/spinlite/rpm_detector.h"
#include "signal_sim.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace spinlite;

static int failures = 0;
#define CHECK(cond, ...)                                                                           \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      std::printf("  FALLITO %s:%d: %s -> ", __FILE__, __LINE__, #cond);                           \
      std::printf(__VA_ARGS__);                                                                    \
      std::printf("\n");                                                                           \
      ++failures;                                                                                  \
    }                                                                                              \
  } while (0)

struct Run {
  std::vector<LaunchResult> results;
  sim::TruePeak truth;
  uint32_t anomalies = 0;
};

static DetectorConfig baseConfig() {
  DetectorConfig c;
  c.tickNs = 100000;
  c.scale = 4;
  c.smooth = 4;
  return c;
}

static Run run(sim::Signal &s, double seconds, DetectorConfig cfg = baseConfig(),
               std::function<void(RpmDetector &, double)> hook = nullptr) {
  RpmDetector d;
  d.configure(cfg);
  Run r;
  const int n = int(seconds * 10000);
  for (int i = 0; i < n; ++i) {
    if (hook) hook(d, s.time());
    if (d.feed(s.next()) & RpmDetector::EV_RESULT) r.results.push_back(d.result());
  }
  r.anomalies = d.anomalies();
  r.truth = sim::truePeak(s.markTimes(), 0, seconds);
  return r;
}

static double err(double measured, double truth) { return truth ? (measured - truth) / truth * 100 : 999; }

static void report(const char *name, const Run &r) {
  std::printf("%-34s lanci=%zu", name, r.results.size());
  if (!r.results.empty()) {
    const auto &x = r.results.front();
    std::printf("  rpm=%u (vero %.0f, %+.2f%%)  giro=%u (vero %.0f, %+.2f%%)  giri=%u  fine=%s", x.rpm,
                r.truth.sustained, err(x.sustainedRpm, r.truth.sustained), x.singleRpm, r.truth.single,
                err(x.singleRpm, r.truth.single), x.revolutions, endName(x.end));
  }
  std::printf("\n");
}

static void expectOne(const char *name, const Run &r, double tolSustained = 0.5, double tolSingle = 1.0) {
  report(name, r);
  CHECK(r.results.size() == 1, "attesi 1 lancio, trovati %zu", r.results.size());
  if (r.results.size() != 1) return;
  const auto &x = r.results.front();
  CHECK(std::abs(err(x.sustainedRpm, r.truth.sustained)) <= tolSustained, "3 giri: %u vs %.0f", x.sustainedRpm,
        r.truth.sustained);
  CHECK(std::abs(err(x.singleRpm, r.truth.single)) <= tolSingle, "giro singolo: %u vs %.0f", x.singleRpm,
        r.truth.single);
  CHECK(x.curveCount >= 3, "curva vuota");
}

static void cleanLaunch() {
  sim::Signal s({}, sim::pullProfile(0.5, 12000));
  expectOne("lancio pulito 12000", run(s, 3));
}

static void severalSpeeds() {
  for (double rpm : {1500.0, 4000.0, 8000.0, 15000.0, 20000.0, 25000.0, 30000.0}) {
    sim::Signal s({}, sim::pullProfile(0.4, rpm), uint32_t(rpm));
    char name[64];
    std::snprintf(name, sizeof name, "velocita' %.0f", rpm);
    expectOne(name, run(s, 3), rpm > 20000 ? 0.8 : 0.5, rpm > 20000 ? 2.0 : 1.0);
  }
}

static void weakMarks() {
  sim::Optics o;
  o.depth = 220; // segno visto attraverso l'apertura del launcher: poco contrasto
  o.noise = 6;
  sim::Signal s(o, sim::pullProfile(0.5, 11000), 7);
  expectOne("segni deboli (220 conteggi)", run(s, 3), 0.8, 2.0);
}

static void noisy() {
  sim::Optics o;
  o.noise = 40;
  o.depth = 900;
  sim::Signal s(o, sim::pullProfile(0.5, 13000), 9);
  expectOne("rumore forte (sigma 40)", run(s, 3), 0.8, 2.5);
}

static void idleNoLaunch() {
  sim::Optics o;
  o.noise = 12;
  o.wander = 35;
  sim::Signal s(o, [](double) { return 0.0; }, 11);
  // qualche disturbo isolato, come una mano che passa davanti
  s.distort = [](double t, double v) {
    const double phase = std::fmod(t, 0.9);
    return phase < 0.004 ? v - 120 : v;
  };
  auto r = run(s, 30);
  report("fermo 30 s con disturbi", r);
  CHECK(r.results.empty(), "nessun lancio atteso, trovati %zu", r.results.size());
}

static void slowRotationNoLaunch() {
  // rotazione lenta a 600 RPM (es. si gira il launcher a mano): sotto soglia
  sim::Signal s({}, [](double t) { return t > 0.5 ? 600.0 : 0.0; }, 13);
  auto r = run(s, 5);
  report("rotazione lenta 600 RPM", r);
  CHECK(r.results.empty(), "nessun lancio atteso, trovati %zu", r.results.size());
}

static void rewindIgnored() {
  sim::Signal s({}, sim::pullProfile(0.5, 14000, 0.18, 0.08, 3500, 0.15, 0.6), 15);
  expectOne("riavvolgimento ignorato", run(s, 4));
}

static void twoLaunches() {
  auto p = sim::sumProfiles(sim::pullProfile(0.5, 9000), sim::pullProfile(4.0, 13000));
  sim::Signal s({}, p, 17);
  auto r = run(s, 7);
  std::printf("%-34s lanci=%zu\n", "due lanci a 3,5 s", r.results.size());
  CHECK(r.results.size() == 2, "attesi 2 lanci, trovati %zu", r.results.size());
  if (r.results.size() == 2) {
    auto t1 = sim::truePeak(s.markTimes(), 0, 3.5), t2 = sim::truePeak(s.markTimes(), 3.5, 7);
    CHECK(std::abs(err(r.results[0].sustainedRpm, t1.sustained)) < 0.5, "primo %u vs %.0f",
          r.results[0].sustainedRpm, t1.sustained);
    CHECK(std::abs(err(r.results[1].sustainedRpm, t2.sustained)) < 0.5, "secondo %u vs %.0f",
          r.results[1].sustainedRpm, t2.sustained);
  }
}

static void spikeDuringLaunch() {
  sim::Signal s({}, sim::pullProfile(0.5, 12000), 19);
  // un disturbo stretto e profondo in mezzo a un giro (es. riflesso)
  s.distort = [](double t, double v) { return (t > 0.75 && t < 0.7504) ? v - 1400 : v; };
  expectOne("disturbo durante il lancio", run(s, 3), 0.5, 1.0);
}

static void spikeAnywhere() {
  // Lo stesso disturbo in 12 posizioni diverse dentro un giro (~5 ms a 12000 RPM):
  // un fronte in piu' non deve mai gonfiare il picco.
  int worstK = -1;
  double worst = 0;
  for (int k = 0; k < 12; ++k) {
    const double at = 0.76 + k * 0.00042;
    sim::Signal s({}, sim::pullProfile(0.5, 12000), uint32_t(100 + k));
    s.distort = [at](double t, double v) { return (t > at && t < at + 0.0004) ? v - 1400 : v; };
    auto r = run(s, 3);
    CHECK(r.results.size() == 1, "posizione %d: %zu lanci", k, r.results.size());
    if (r.results.size() != 1) continue;
    const double e1 = std::abs(err(r.results[0].sustainedRpm, r.truth.sustained));
    const double e2 = std::abs(err(r.results[0].singleRpm, r.truth.single));
    if (std::max(e1, e2) > worst) {
      worst = std::max(e1, e2);
      worstK = k;
    }
  }
  std::printf("%-34s errore peggiore %.2f%% (posizione %d)\n", "disturbo in 12 posizioni", worst, worstK);
  CHECK(worst <= 1.0, "errore %.2f%%", worst);
}

static void repeatability() {
  // 25 lanci con rumore diverso: quanto varia l'errore.
  double worst = 0, sum = 0;
  for (int i = 0; i < 25; ++i) {
    sim::Optics o;
    o.noise = 10;
    o.depth = 600;
    sim::Signal s(o, sim::pullProfile(0.5, 9000 + 300 * i, 0.15 + 0.004 * i), uint32_t(500 + i));
    auto r = run(s, 3);
    CHECK(r.results.size() == 1, "giro %d: %zu lanci", i, r.results.size());
    if (r.results.size() != 1) continue;
    const double e = std::abs(err(r.results[0].sustainedRpm, r.truth.sustained));
    worst = std::max(worst, e);
    sum += e;
  }
  std::printf("%-34s errore medio %.3f%%, peggiore %.3f%%\n", "ripetibilita' (25 lanci)", sum / 25, worst);
  CHECK(worst <= 0.3, "errore peggiore %.3f%%", worst);
}

static void missedMark() {
  sim::Signal s({}, sim::pullProfile(0.5, 12000), 21);
  // per un giro il segno quasi non si vede
  s.distort = [](double t, double v) { return (t > 0.80 && t < 0.806) ? 3000 : v; };
  auto r = run(s, 3);
  expectOne("un segno perso", r, 0.6, 1.0);
}

static void twoMarks() {
  sim::Optics o;
  o.marks = 2;
  o.markOffset = 0.04; // segni non perfettamente opposti
  sim::Signal s(o, sim::pullProfile(0.5, 10000), 23);
  auto cfg = baseConfig();
  cfg.marksPerRev = 2;
  expectOne("2 segni per giro", run(s, 3, cfg), 0.5, 1.0);
}

static void saturatedBaseline() {
  sim::Optics o;
  o.baseline = 4300; // a riposo il sensore satura a 4095
  o.depth = 2200;
  sim::Signal s(o, sim::pullProfile(0.5, 12500), 25);
  expectOne("riposo saturo a 4095", run(s, 3));
}

static void dataLossDuringLaunch() {
  sim::Signal s({}, sim::pullProfile(0.5, 12000), 27);
  bool done = false;
  auto r = run(s, 3, baseConfig(), [&](RpmDetector &d, double t) {
    if (!done && t > 0.72) {
      d.dataLoss();
      done = true;
    }
  });
  report("dati persi durante il lancio", r);
  // il lancio interrotto e' perso e la sua coda non deve diventare un lancio nuovo
  CHECK(r.results.empty(), "nessun risultato atteso, trovati %zu", r.results.size());
}

static void sensorFault() {
  sim::Optics o;
  o.baseline = 5;
  o.depth = 0;
  o.wander = 0;
  o.noise = 1;
  sim::Signal s(o, [](double) { return 0.0; }, 29);
  RpmDetector d;
  d.configure(baseConfig());
  for (int i = 0; i < 20000; ++i) d.feed(s.next());
  std::printf("%-34s guasto=%d\n", "sensore scollegato", d.signalFault());
  CHECK(d.signalFault(), "guasto non rilevato");
}

int main() {
  std::printf("Rilevatore - segnali simulati\n");
  cleanLaunch();
  severalSpeeds();
  weakMarks();
  noisy();
  idleNoLaunch();
  slowRotationNoLaunch();
  rewindIgnored();
  twoLaunches();
  spikeDuringLaunch();
  spikeAnywhere();
  repeatability();
  missedMark();
  twoMarks();
  saturatedBaseline();
  dataLossDuringLaunch();
  sensorFault();
  std::printf(failures ? "\n%d controlli FALLITI\n" : "\nTutti i controlli superati\n", failures);
  return failures ? 1 : 0;
}
