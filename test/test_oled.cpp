// Disegna le schermate dell'OLED sul terminale (e in PBM con --pbm cartella)
// per controllarle senza hardware.
#include "../firmware/spinlite/oled_ui.h"

#include <cstdio>
#include <cstring>
#include <string>

using namespace spinlite;

static int failures = 0;

static void ascii(const Frame &f) {
  for (int y = 0; y < Frame::H; ++y) {
    std::string row;
    for (int x = 0; x < Frame::W; ++x) row += f.get(x, y) ? '#' : '.';
    std::printf("%s\n", row.c_str());
  }
}

static void pbm(const Frame &f, const std::string &path) {
  FILE *out = std::fopen(path.c_str(), "w");
  if (!out) return;
  std::fprintf(out, "P1\n%d %d\n", Frame::W, Frame::H);
  for (int y = 0; y < Frame::H; ++y) {
    for (int x = 0; x < Frame::W; ++x) std::fprintf(out, "%d ", f.get(x, y) ? 1 : 0);
    std::fprintf(out, "\n");
  }
  std::fclose(out);
}

static bool inBounds(const Frame &f) {
  // nessun pixel acceso fuori dal display: controlla che il testo stia dentro
  int lit = 0;
  for (int y = 0; y < Frame::H; ++y)
    for (int x = 0; x < Frame::W; ++x) lit += f.get(x, y);
  return lit > 0;
}

int main(int argc, char **argv) {
  const bool quiet = argc > 1 && std::strcmp(argv[1], "-q") == 0;
  const std::string dir = (argc == 3 && std::strcmp(argv[1], "--pbm") == 0) ? argv[2] : "";
  struct Case {
    const char *name;
    UiState s;
  } cases[5];
  cases[0].name = "ultimo";
  cases[0].s.status = "PRONTO";
  cases[0].s.lastRpm = 12345;
  cases[0].s.batteryPercent = 80;
  cases[0].s.bleConnected = true;
  cases[1].name = "massimo";
  cases[1].s.page = UiState::Page::Best;
  cases[1].s.bestRpm = 29876;
  cases[1].s.batteryPercent = -1;
  cases[2].name = "info";
  cases[2].s.page = UiState::Page::Info;
  cases[2].s.launches = 57;
  cases[2].s.batteryPercent = 64;
  cases[2].s.batteryMv = 3874;
  cases[3].name = "messaggio";
  cases[3].s.message = "SPINLITE";
  cases[4].name = "nessun lancio";
  cases[4].s.status = "RIARMO";
  cases[4].s.batteryPercent = 5;

  Frame f;
  for (auto &c : cases) {
    render(f, c.s);
    if (!inBounds(f)) {
      std::printf("FALLITO: schermata '%s' vuota\n", c.name);
      ++failures;
    }
    if (!quiet) {
      std::printf("\n[%s]\n", c.name);
      ascii(f);
    }
    if (!dir.empty()) pbm(f, dir + "/oled_" + c.name + ".pbm");
  }
  // larghezza del testo: il numero piu' lungo deve stare con l'unita'
  if (Frame::textWidth("65.535", 3) + Frame::textWidth("RPM") + 2 > Frame::W &&
      Frame::textWidth("65.535", 2) + Frame::textWidth("RPM") + 2 > Frame::W) {
    std::printf("FALLITO: 65.535 RPM non entra\n");
    ++failures;
  }
  std::printf(failures ? "OLED: %d controlli FALLITI\n" : "OLED: tutti i controlli superati\n", failures);
  return failures ? 1 : 0;
}
