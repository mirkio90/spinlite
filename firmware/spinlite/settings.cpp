#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "config.h"

#ifdef ARDUINO
#include <Preferences.h>
#endif

namespace spinlite {
namespace settings {
namespace {

// L'ultimo byte dipende dal sensore: passando da un firmware TCRT5000 a uno
// QRE1113 (o viceversa) le soglie salvate per l'altro sensore vengono
// ignorate e si riparte dai default giusti.
#if SPINLITE_SENSOR_QRE
const uint8_t MAGIC[4] = {'S', 'P', 'L', 'Q'};
#else
const uint8_t MAGIC[4] = {'S', 'P', 'L', 'C'};
#endif
constexpr uint8_t CONFIG_VERSION = 1;

struct Param {
  const char *name;
  long lo, hi;
};
const Param PARAMS[] = {
    {"marks", 1, 8},       {"minmark", 20, 2000}, {"minrpm", 300, 10000}, {"maxrpm", 5000, 60000},
    {"endpct", 5, 90},     {"endgap", 50, 2000},  {"quiet", 200, 5000},   {"single", 0, 1},
};
constexpr int NPARAMS = int(sizeof PARAMS / sizeof PARAMS[0]);

long getValue(const DetectorConfig &c, int i) {
  switch (i) {
  case 0: return c.marksPerRev;
  case 1: return c.minMark;
  case 2: return c.minLaunchRpm;
  case 3: return c.maxRpm;
  case 4: return c.endPercent;
  case 5: return c.endGapMs;
  case 6: return c.quietMs;
  default: return c.singleRevPeak ? 1 : 0;
  }
}

void setValue(DetectorConfig &c, int i, long v) {
  switch (i) {
  case 0: c.marksPerRev = uint8_t(v); break;
  case 1: c.minMark = uint16_t(v); break;
  case 2: c.minLaunchRpm = uint16_t(v); break;
  case 3: c.maxRpm = uint16_t(v); break;
  case 4: c.endPercent = uint8_t(v); break;
  case 5: c.endGapMs = uint16_t(v); break;
  case 6: c.quietMs = uint16_t(v); break;
  default: c.singleRevPeak = v != 0; break;
  }
}

bool inRange(int i, long v) { return v >= PARAMS[i].lo && v <= PARAMS[i].hi; }

void put16(uint8_t *p, uint16_t v) {
  p[0] = uint8_t(v & 0xFF);
  p[1] = uint8_t(v >> 8);
}
uint16_t get16(const uint8_t *p) { return uint16_t(p[0] | (p[1] << 8)); }

} // namespace

void defaults(DetectorConfig &cfg) {
  cfg.marksPerRev = DEFAULT_MARKS_PER_REV;
  cfg.minMark = DEFAULT_MIN_MARK;
  cfg.minLaunchRpm = DEFAULT_MIN_LAUNCH_RPM;
  cfg.maxRpm = DEFAULT_MAX_RPM;
  cfg.endPercent = DEFAULT_END_PERCENT;
  cfg.endGapMs = DEFAULT_END_GAP_MS;
  cfg.quietMs = DEFAULT_QUIET_MS;
  cfg.singleRevPeak = DEFAULT_SINGLE_REV_PEAK != 0;
}

size_t encodeConfig(const DetectorConfig &cfg, uint8_t *buf, size_t len) {
  if (len < CONFIG_BYTES) return 0;
  memset(buf, 0, CONFIG_BYTES);
  uint8_t *p = buf;
  memcpy(p, MAGIC, 4);
  p += 4;
  *p++ = CONFIG_VERSION;
  *p++ = cfg.marksPerRev;
  put16(p, cfg.minMark);
  p += 2;
  put16(p, cfg.minLaunchRpm);
  p += 2;
  put16(p, cfg.maxRpm);
  p += 2;
  *p++ = cfg.endPercent;
  put16(p, cfg.endGapMs);
  p += 2;
  put16(p, cfg.quietMs);
  p += 2;
  *p++ = cfg.singleRevPeak ? 1 : 0;
  const uint32_t crc = bbp::crc32(buf, CONFIG_BYTES - 4);
  for (int i = 0; i < 4; ++i) p[i] = uint8_t(crc >> (8 * i));
  return CONFIG_BYTES;
}

bool decodeConfig(const uint8_t *buf, size_t len, DetectorConfig &cfg) {
  if (len != CONFIG_BYTES || memcmp(buf, MAGIC, 4) != 0 || buf[4] != CONFIG_VERSION) return false;
  const uint8_t *e = buf + CONFIG_BYTES - 4;
  const uint32_t crc = uint32_t(e[0]) | (uint32_t(e[1]) << 8) | (uint32_t(e[2]) << 16) | (uint32_t(e[3]) << 24);
  if (crc != bbp::crc32(buf, CONFIG_BYTES - 4)) return false;
  const uint8_t *p = buf + 5;
  long v[NPARAMS];
  v[0] = *p++;
  v[1] = get16(p);
  p += 2;
  v[2] = get16(p);
  p += 2;
  v[3] = get16(p);
  p += 2;
  v[4] = *p++;
  v[5] = get16(p);
  p += 2;
  v[6] = get16(p);
  p += 2;
  v[7] = *p++;
  for (int i = 0; i < NPARAMS; ++i)
    if (!inRange(i, v[i])) return false;
  DetectorConfig c = cfg;
  for (int i = 0; i < NPARAMS; ++i) setValue(c, i, v[i]);
  cfg = c;
  return true;
}

bool setParam(DetectorConfig &cfg, const char *name, long value) {
  if (!name) return false;
  for (int i = 0; i < NPARAMS; ++i) {
    if (strcmp(name, PARAMS[i].name) != 0) continue;
    if (!inRange(i, value)) return false;
    setValue(cfg, i, value);
    return true;
  }
  return false;
}

size_t describe(const DetectorConfig &cfg, char *out, size_t len) {
  if (!out || len == 0) return 0;
  size_t n = 0;
  out[0] = 0;
  for (int i = 0; i < NPARAMS; ++i) {
    const int w = snprintf(out + n, len - n, "%s%s=%ld", i ? " " : "", PARAMS[i].name, getValue(cfg, i));
    if (w < 0 || size_t(w) >= len - n) {
      out[len - 1] = 0;
      return strlen(out);
    }
    n += size_t(w);
  }
  return n;
}

#ifdef ARDUINO
namespace {
const char *NS = "spinlite";
}

void begin() {}

bool loadConfig(DetectorConfig &cfg) {
  Preferences pr;
  if (!pr.begin(NS, true)) return false;
  uint8_t buf[CONFIG_BYTES];
  const size_t n = pr.getBytes("cfg", buf, sizeof buf);
  pr.end();
  return n == CONFIG_BYTES && decodeConfig(buf, n, cfg);
}

bool saveConfig(const DetectorConfig &cfg) {
  uint8_t buf[CONFIG_BYTES];
  const size_t n = encodeConfig(cfg, buf, sizeof buf);
  if (!n) return false;
  Preferences pr;
  if (!pr.begin(NS, false)) return false;
  const bool ok = pr.putBytes("cfg", buf, n) == n;
  pr.end();
  return ok;
}

void eraseConfig() {
  Preferences pr;
  if (!pr.begin(NS, false)) return;
  pr.remove("cfg");
  pr.end();
}

bool loadHistory(bbp::History &h) {
  Preferences pr;
  if (!pr.begin(NS, true)) return false;
  uint8_t buf[bbp::STORE_BYTES];
  const size_t n = pr.getBytes("hist", buf, sizeof buf);
  pr.end();
  if (n != bbp::STORE_BYTES) return false;
  bbp::History tmp;
  if (!bbp::decode(buf, n, tmp)) return false;
  h = tmp;
  return true;
}

bool saveHistory(const bbp::History &h) {
  uint8_t buf[bbp::STORE_BYTES];
  const size_t n = bbp::encode(h, buf, sizeof buf);
  if (!n) return false;
  Preferences pr;
  if (!pr.begin(NS, false)) return false;
  const bool ok = pr.putBytes("hist", buf, n) == n;
  pr.end();
  return ok;
}
#endif

} // namespace settings
} // namespace spinlite
