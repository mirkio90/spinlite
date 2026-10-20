// SpinLite - schermate per un OLED SSD1306 128x32 (opzionale).
//
// Il disegno avviene in un buffer di 512 byte nello stesso formato del
// controller (8 righe per byte, una "pagina" ogni 8 righe), cosi' l'invio via
// I2C e' una copia diretta. Il disegno e' portabile e testato sul PC; solo
// l'invio usa Arduino (oled_ui.cpp, sezione ARDUINO).
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace spinlite {

class Frame {
public:
  static constexpr int W = 128, H = 32;
  void clear();
  void pixel(int x, int y, bool on = true);
  bool get(int x, int y) const;
  // Testo con il font 5x7, ingrandito di `scale`. Ritorna la x finale.
  int text(int x, int y, const char *s, int scale = 1);
  static int textWidth(const char *s, int scale = 1);
  void fillRect(int x, int y, int w, int h, bool on = true);
  const uint8_t *data() const { return buf_; }

private:
  void glyph(int x, int y, char c, int scale);
  uint8_t buf_[W * H / 8] = {};
};

struct UiState {
  enum class Page : uint8_t { Last, Best, Info };
  Page page = Page::Last;
  const char *status = "PRONTO";
  uint16_t lastRpm = 0;
  uint16_t bestRpm = 0;
  uint32_t launches = 0;
  int batteryPercent = -1; // -1 = alimentato da USB
  uint16_t batteryMv = 0;
  bool bleConnected = false;
  const char *message = nullptr; // messaggio temporaneo a tutto schermo
};

void render(Frame &f, const UiState &s);

#ifdef ARDUINO
class Oled {
public:
  bool begin(int sda, int scl, uint8_t address);
  void show(const Frame &f);
  bool ok() const { return ok_; }

private:
  bool command(const uint8_t *cmds, size_t n);
  uint8_t addr_ = 0x3C;
  bool ok_ = false;
};
#endif

} // namespace spinlite
