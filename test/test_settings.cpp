// Test di impostazioni (codifica, limiti) e stima della batteria.
#include "../firmware/spinlite/battery.h"
#include "../firmware/spinlite/config.h"
#include "../firmware/spinlite/settings.h"

#include <cstdio>
#include <cstring>

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

static void fixCrc(uint8_t *buf) {
  const uint32_t crc = bbp::crc32(buf, settings::CONFIG_BYTES - 4);
  for (int i = 0; i < 4; ++i) buf[settings::CONFIG_BYTES - 4 + i] = uint8_t(crc >> (8 * i));
}

static void defaultsTest() {
  DetectorConfig c;
  c.marksPerRev = 5;
  c.minMark = 999;
  settings::defaults(c);
  CHECK(c.marksPerRev == DEFAULT_MARKS_PER_REV, "marks %u", c.marksPerRev);
  CHECK(c.minMark == DEFAULT_MIN_MARK, "minmark %u", c.minMark);
  CHECK(c.minLaunchRpm == DEFAULT_MIN_LAUNCH_RPM && c.maxRpm == DEFAULT_MAX_RPM, "rpm");
  CHECK(c.endPercent == DEFAULT_END_PERCENT && c.endGapMs == DEFAULT_END_GAP_MS, "fine lancio");
  CHECK(c.quietMs == DEFAULT_QUIET_MS && c.singleRevPeak == (DEFAULT_SINGLE_REV_PEAK != 0), "quiete");
}

static void roundTrip() {
  DetectorConfig c;
  settings::defaults(c);
  c.marksPerRev = 2;
  c.minMark = 220;
  c.minLaunchRpm = 1500;
  c.maxRpm = 30000;
  c.endPercent = 40;
  c.endGapMs = 300;
  c.quietMs = 1500;
  c.singleRevPeak = true;
  uint8_t buf[settings::CONFIG_BYTES];
  CHECK(settings::encodeConfig(c, buf, sizeof buf) == settings::CONFIG_BYTES, "encode");
  CHECK(settings::encodeConfig(c, buf, settings::CONFIG_BYTES - 1) == 0, "buffer corto");

  DetectorConfig d;
  d.tickNs = 123456; // campo non persistente: deve restare com'e'
  d.smooth = 3;
  settings::defaults(d);
  CHECK(settings::decodeConfig(buf, sizeof buf, d), "decode");
  CHECK(d.marksPerRev == 2 && d.minMark == 220 && d.minLaunchRpm == 1500 && d.maxRpm == 30000, "campi 1");
  CHECK(d.endPercent == 40 && d.endGapMs == 300 && d.quietMs == 1500 && d.singleRevPeak, "campi 2");
  CHECK(d.tickNs == 123456 && d.smooth == 3, "campi non persistenti toccati");
}

static void rejects() {
  DetectorConfig c;
  settings::defaults(c);
  uint8_t good[settings::CONFIG_BYTES];
  settings::encodeConfig(c, good, sizeof good);
  DetectorConfig d;
  settings::defaults(d);
  d.minMark = 777;

  uint8_t bad[settings::CONFIG_BYTES];
  memcpy(bad, good, sizeof bad);
  bad[8] ^= 0x01; // un byte di dati: il CRC non torna
  CHECK(!settings::decodeConfig(bad, sizeof bad, d), "CRC corrotto accettato");
  CHECK(d.minMark == 777, "cfg modificato da un blob rifiutato");

  memcpy(bad, good, sizeof bad);
  bad[4] = 99; // versione sbagliata, CRC ricalcolato
  fixCrc(bad);
  CHECK(!settings::decodeConfig(bad, sizeof bad, d), "versione sbagliata accettata");

  memcpy(bad, good, sizeof bad);
  bad[0] = 'X'; // magic sbagliato
  fixCrc(bad);
  CHECK(!settings::decodeConfig(bad, sizeof bad, d), "magic sbagliato accettato");

  CHECK(!settings::decodeConfig(good, sizeof good - 1, d), "lunghezza sbagliata accettata");

  memcpy(bad, good, sizeof bad);
  bad[5] = 0; // marks = 0: fuori limite, CRC valido
  fixCrc(bad);
  CHECK(!settings::decodeConfig(bad, sizeof bad, d), "valore fuori limite accettato");
  CHECK(d.minMark == 777, "cfg modificato da un blob fuori limite");
}

static void setParamTest() {
  DetectorConfig c;
  settings::defaults(c);
  CHECK(settings::setParam(c, "marks", 2) && c.marksPerRev == 2, "marks");
  CHECK(settings::setParam(c, "minmark", 300) && c.minMark == 300, "minmark");
  CHECK(settings::setParam(c, "minrpm", 2000) && c.minLaunchRpm == 2000, "minrpm");
  CHECK(settings::setParam(c, "maxrpm", 50000) && c.maxRpm == 50000, "maxrpm");
  CHECK(settings::setParam(c, "endpct", 25) && c.endPercent == 25, "endpct");
  CHECK(settings::setParam(c, "endgap", 400) && c.endGapMs == 400, "endgap");
  CHECK(settings::setParam(c, "quiet", 2000) && c.quietMs == 2000, "quiet");
  CHECK(settings::setParam(c, "single", 1) && c.singleRevPeak, "single");

  // limiti inclusi
  CHECK(settings::setParam(c, "marks", 1) && settings::setParam(c, "marks", 8), "marks 1..8");
  CHECK(settings::setParam(c, "minmark", 20) && settings::setParam(c, "minmark", 2000), "minmark 20..2000");
  CHECK(settings::setParam(c, "minrpm", 300) && settings::setParam(c, "minrpm", 10000), "minrpm");
  CHECK(settings::setParam(c, "maxrpm", 5000) && settings::setParam(c, "maxrpm", 60000), "maxrpm");
  CHECK(settings::setParam(c, "endpct", 5) && settings::setParam(c, "endpct", 90), "endpct");
  CHECK(settings::setParam(c, "endgap", 50) && settings::setParam(c, "endgap", 2000), "endgap");
  CHECK(settings::setParam(c, "quiet", 200) && settings::setParam(c, "quiet", 5000), "quiet");
  CHECK(settings::setParam(c, "single", 0) && !c.singleRevPeak, "single 0");

  // fuori limite o nome errato: cfg intatto
  settings::defaults(c);
  const DetectorConfig ref = c;
  CHECK(!settings::setParam(c, "marks", 0) && !settings::setParam(c, "marks", 9), "marks fuori");
  CHECK(!settings::setParam(c, "minmark", 19) && !settings::setParam(c, "minmark", 2001), "minmark fuori");
  CHECK(!settings::setParam(c, "minrpm", 299) && !settings::setParam(c, "minrpm", 10001), "minrpm fuori");
  CHECK(!settings::setParam(c, "maxrpm", 4999) && !settings::setParam(c, "maxrpm", 60001), "maxrpm fuori");
  CHECK(!settings::setParam(c, "endpct", 4) && !settings::setParam(c, "endpct", 91), "endpct fuori");
  CHECK(!settings::setParam(c, "endgap", 49) && !settings::setParam(c, "endgap", 2001), "endgap fuori");
  CHECK(!settings::setParam(c, "quiet", 199) && !settings::setParam(c, "quiet", 5001), "quiet fuori");
  CHECK(!settings::setParam(c, "single", 2) && !settings::setParam(c, "single", -1), "single fuori");
  CHECK(!settings::setParam(c, "nonesiste", 1) && !settings::setParam(c, "", 1) && !settings::setParam(c, nullptr, 1),
        "nome sconosciuto accettato");
  CHECK(!settings::setParam(c, "marks", 65537L), "valore enorme");
  CHECK(c.marksPerRev == ref.marksPerRev && c.minMark == ref.minMark && c.minLaunchRpm == ref.minLaunchRpm &&
            c.maxRpm == ref.maxRpm && c.endPercent == ref.endPercent && c.endGapMs == ref.endGapMs &&
            c.quietMs == ref.quietMs && c.singleRevPeak == ref.singleRevPeak,
        "cfg modificato da un set rifiutato");
}

static void describeTest() {
  DetectorConfig c;
  settings::defaults(c);
  char out[200];
  const size_t n = settings::describe(c, out, sizeof out);
  CHECK(n == strlen(out), "lunghezza %zu vs %zu", n, strlen(out));
  CHECK(strstr(out, "marks=1") != nullptr, "%s", out);
  CHECK(strstr(out, "minmark=150") != nullptr, "%s", out);
  CHECK(strstr(out, "minrpm=1000") != nullptr, "%s", out);
  CHECK(strstr(out, "maxrpm=40000") != nullptr, "%s", out);
  CHECK(strstr(out, "endpct=30") != nullptr && strstr(out, "endgap=250") != nullptr, "%s", out);
  CHECK(strstr(out, "quiet=1000") != nullptr && strstr(out, "single=0") != nullptr, "%s", out);
  char small[12];
  settings::describe(c, small, sizeof small);
  CHECK(strlen(small) < sizeof small, "buffer piccolo: terminatore");
  CHECK(settings::describe(c, small, 0) == 0, "buffer zero");
}

static void batteryTest() {
  const int pts[][2] = {{4200, 100}, {4100, 90}, {4000, 80}, {3900, 65}, {3800, 50}, {3750, 40},
                        {3700, 30},  {3650, 20}, {3600, 12}, {3500, 5},  {3300, 0}};
  for (const auto &p : pts) CHECK(batteryPercent(p[0]) == p[1], "%d mV -> %d (atteso %d)", p[0], batteryPercent(p[0]), p[1]);
  CHECK(batteryPercent(4350) == 100 && batteryPercent(4201) == 100, "sopra 4200");
  CHECK(batteryPercent(3000) == 0 && batteryPercent(2500) == 0, "sotto 3300 ma con batteria");
  CHECK(batteryPercent(4150) == 95, "interpolazione %d", batteryPercent(4150));
  CHECK(batteryPercent(3950) == 72, "interpolazione %d", batteryPercent(3950));
  CHECK(batteryPercent(3400) == 2, "interpolazione %d", batteryPercent(3400));
  CHECK(batteryPercent(2499) == -1 && batteryPercent(0) == -1 && batteryPercent(-5) == -1, "USB");
  int prev = -1;
  for (int mv = 2500; mv <= 4300; ++mv) {
    const int p = batteryPercent(mv);
    CHECK(p >= prev && p >= 0 && p <= 100, "monotonia a %d mV: %d dopo %d", mv, p, prev);
    prev = p;
  }
}

int main() {
  std::printf("Impostazioni e batteria\n");
  defaultsTest();
  roundTrip();
  rejects();
  setParamTest();
  describeTest();
  batteryTest();
  std::printf(failures ? "%d controlli FALLITI\n" : "Tutti i controlli superati\n", failures);
  return failures ? 1 : 0;
}
