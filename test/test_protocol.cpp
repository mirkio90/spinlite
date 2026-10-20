// Test dei pacchetti Battle Pass e del salvataggio in flash.
// Con "--dump file" scrive i pacchetti in esadecimale, uno per riga, per
// confrontarli con un parser indipendente.
#include "../firmware/spinlite/bbp_protocol.h"

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

static uint16_t u16(const uint8_t *p) { return uint16_t(p[0] | (p[1] << 8)); }

static void fill(bbp::History &h, int launches) {
  uint32_t curve[40];
  for (int i = 0; i < 40; ++i) curve[i] = 6000 - i * 20; // ~10000 RPM
  for (int i = 0; i < launches; ++i) bbp::add(h, uint16_t(8000 + i), curve, uint8_t(i % 2 ? 40 : 12));
}

static void statusPacket() {
  bbp::History h;
  fill(h, 3);
  const uint8_t uid[6] = {1, 2, 3, 4, 5, 6};
  uint8_t p[bbp::PACKET];
  bbp::buildStatus(h, bbp::FLAG_READY, bbp::batteryByte(80), uid, p);
  CHECK(p[0] == 0xA0, "intestazione %02X", p[0]);
  CHECK(p[3] == 0x04, "flag %02X", p[3]);
  CHECK(p[4] == 200, "batteria %u", p[4]);
  CHECK(u16(p + 7) == 8002, "massimo %u", u16(p + 7));
  CHECK(u16(p + 9) == 3, "totale %u", u16(p + 9));
  CHECK(memcmp(p + 11, uid, 6) == 0, "uid");
  CHECK(bbp::batteryByte(100) == 250 && bbp::batteryByte(0) == 0, "scala batteria");
}

static void dumpLayout() {
  bbp::History h;
  fill(h, 20);
  uint8_t d[bbp::DUMP_PAGES][bbp::PACKET];
  bbp::buildDump(h, d);
  for (int i = 0; i < 8; ++i) CHECK(d[i][0] == 0xB0 + i, "pagina %d: %02X", i, d[i][0]);
  for (int i = 0; i < 4; ++i) CHECK(d[8 + i][0] == 0x70 + i, "curva %d: %02X", i, d[8 + i][0]);
  CHECK(u16(d[0] + 1) == 8000, "primo lancio %u", u16(d[0] + 1));
  CHECK(u16(d[2] + 7) == 8019, "ultimo lancio (indice 19) %u", u16(d[2] + 7));
  CHECK(u16(d[2] + 9) == 0, "slot vuoto");
  CHECK(d[6][11] == 20, "numero lanci %u", d[6][11]);
  CHECK(u16(d[6] + 9) == 20, "totale %u", u16(d[6] + 9));
  uint8_t sum = 0;
  for (int p = 0; p < 7; ++p)
    for (int b = 1; b < 17; ++b) sum = uint8_t(sum + d[p][b]);
  CHECK(d[7][16] == sum, "checksum %02X vs %02X", d[7][16], sum);
  // ultimo lancio (indice 19, dispari) ha 40 periodi -> curva piena
  CHECK(u16(d[8] + 1) == 750, "primo periodo %u (6000 us / 8)", u16(d[8] + 1));
  CHECK(bbp::rpmFromInterval(750) == 10000, "rpm da intervallo %u", bbp::rpmFromInterval(750));
  CHECK(u16(d[11] + 15) == (6000 - 31 * 20 + 4) / 8, "ultimo periodo %u", u16(d[11] + 15));
}

static void wrapAround() {
  bbp::History h;
  fill(h, 57); // 7 oltre la capienza
  CHECK(h.count == 50, "count %u", h.count);
  CHECK(h.total == 57, "totale %u", h.total);
  CHECK(bbp::chronological(h, 0) == 8007, "piu' vecchio %u", bbp::chronological(h, 0));
  CHECK(bbp::chronological(h, 49) == 8056, "piu' recente %u", bbp::chronological(h, 49));
  CHECK(bbp::latest(h) == 8056, "latest %u", bbp::latest(h));
  uint8_t d[bbp::DUMP_PAGES][bbp::PACKET];
  bbp::buildDump(h, d);
  CHECK(u16(d[0] + 1) == 8007, "B0 inizia dal piu' vecchio %u", u16(d[0] + 1));
  CHECK(u16(d[6] + 3) == 8056, "B6 finisce col piu' recente %u", u16(d[6] + 3));
}

static void shortCurve() {
  bbp::History h;
  uint32_t curve[3] = {5000, 4800, 4700};
  bbp::add(h, 12000, curve, 3);
  uint8_t d[bbp::DUMP_PAGES][bbp::PACKET];
  bbp::buildDump(h, d);
  CHECK(u16(d[8] + 5) == 588, "terzo periodo %u", u16(d[8] + 5));
  CHECK(u16(d[8] + 7) == 0, "oltre la curva deve essere 0");
}

static void storage() {
  bbp::History h;
  fill(h, 61);
  uint8_t buf[bbp::STORE_BYTES];
  CHECK(bbp::encode(h, buf, sizeof buf) == bbp::STORE_BYTES, "encode");
  bbp::History back;
  CHECK(bbp::decode(buf, sizeof buf, back), "decode");
  CHECK(back.total == h.total && back.best == h.best && back.count == h.count && back.next == h.next,
        "campi diversi");
  CHECK(memcmp(back.records, h.records, sizeof h.records) == 0, "storico diverso");
  CHECK(memcmp(back.curve, h.curve, sizeof h.curve) == 0, "curva diversa");
  buf[40] ^= 0x10;
  CHECK(!bbp::decode(buf, sizeof buf, back), "CRC non ha rilevato la corruzione");
  CHECK(bbp::crc32(reinterpret_cast<const uint8_t *>("123456789"), 9) == 0xCBF43926u, "CRC32 di riferimento");
}

static void dumpForCrossCheck(const char *path) {
  FILE *f = std::fopen(path, "w");
  if (!f) return;
  bbp::History h;
  fill(h, 57);
  const uint8_t uid[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  uint8_t s[bbp::PACKET];
  bbp::buildStatus(h, bbp::FLAG_READY, 200, uid, s);
  uint8_t d[bbp::DUMP_PAGES][bbp::PACKET];
  bbp::buildDump(h, d);
  auto line = [&](const uint8_t *p) {
    for (int i = 0; i < 17; ++i) std::fprintf(f, "%02x", p[i]);
    std::fprintf(f, "\n");
  };
  line(s);
  for (auto &p : d) line(p);
  std::fclose(f);
}

int main(int argc, char **argv) {
  std::printf("Protocollo Battle Pass\n");
  statusPacket();
  dumpLayout();
  wrapAround();
  shortCurve();
  storage();
  if (argc == 3 && std::strcmp(argv[1], "--dump") == 0) dumpForCrossCheck(argv[2]);
  std::printf(failures ? "%d controlli FALLITI\n" : "Tutti i controlli superati\n", failures);
  return failures ? 1 : 0;
}
