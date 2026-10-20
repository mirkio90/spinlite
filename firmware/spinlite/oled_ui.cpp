#include "oled_ui.h"

#include <stdio.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#endif

namespace spinlite {
namespace {

// Font 5x7 disegnato a mano: solo i caratteri che servono. Ogni byte e' una
// colonna, bit 0 in alto. Le minuscole sono mostrate come maiuscole.
struct Glyph {
  char c;
  uint8_t col[5];
};
const Glyph FONT[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00}}, {'!', {0x00, 0x00, 0x5F, 0x00, 0x00}},
    {'%', {0x23, 0x13, 0x08, 0x64, 0x62}}, {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
    {'.', {0x00, 0x60, 0x60, 0x00, 0x00}}, {'/', {0x20, 0x10, 0x08, 0x04, 0x02}},
    {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}}, {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
    {'2', {0x42, 0x61, 0x51, 0x49, 0x46}}, {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
    {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}}, {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
    {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}}, {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
    {'8', {0x36, 0x49, 0x49, 0x49, 0x36}}, {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
    {':', {0x00, 0x36, 0x36, 0x00, 0x00}}, {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
    {'B', {0x7F, 0x49, 0x49, 0x49, 0x36}}, {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
    {'D', {0x7F, 0x41, 0x41, 0x22, 0x1C}}, {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
    {'F', {0x7F, 0x09, 0x09, 0x09, 0x01}}, {'G', {0x3E, 0x41, 0x49, 0x49, 0x7A}},
    {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}}, {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
    {'J', {0x20, 0x40, 0x41, 0x3F, 0x01}}, {'K', {0x7F, 0x08, 0x14, 0x22, 0x41}},
    {'L', {0x7F, 0x40, 0x40, 0x40, 0x40}}, {'M', {0x7F, 0x02, 0x0C, 0x02, 0x7F}},
    {'N', {0x7F, 0x04, 0x08, 0x10, 0x7F}}, {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
    {'P', {0x7F, 0x09, 0x09, 0x09, 0x06}}, {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
    {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}}, {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
    {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}}, {'U', {0x3F, 0x40, 0x40, 0x40, 0x3F}},
    {'V', {0x1F, 0x20, 0x40, 0x20, 0x1F}}, {'W', {0x3F, 0x40, 0x38, 0x40, 0x3F}},
    {'X', {0x63, 0x14, 0x08, 0x14, 0x63}}, {'Y', {0x07, 0x08, 0x70, 0x08, 0x07}},
    {'Z', {0x61, 0x51, 0x49, 0x45, 0x43}},
};

const uint8_t *lookup(char c) {
  if (c >= 'a' && c <= 'z') c = char(c - 'a' + 'A');
  for (const Glyph &g : FONT)
    if (g.c == c) return g.col;
  return FONT[0].col;
}

// "12345" -> "12.345" per leggere meglio i numeri grandi
void thousands(char *out, size_t n, uint32_t v) {
  if (v >= 1000) {
    snprintf(out, n, "%lu.%03lu", (unsigned long)(v / 1000), (unsigned long)(v % 1000));
  } else {
    snprintf(out, n, "%lu", (unsigned long)v);
  }
}

void header(Frame &f, const char *left, const UiState &s) {
  f.text(0, 0, left);
  char right[12];
  if (s.batteryPercent < 0) {
    snprintf(right, sizeof right, "USB");
  } else {
    snprintf(right, sizeof right, "%d%%", s.batteryPercent);
  }
  int x = Frame::W - Frame::textWidth(right);
  f.text(x, 0, right);
  if (s.bleConnected) f.text(x - Frame::textWidth("BT "), 0, "BT");
}

void bigNumber(Frame &f, uint32_t v, const char *unit) {
  char num[12];
  if (v) {
    thousands(num, sizeof num, v);
  } else {
    snprintf(num, sizeof num, "-");
  }
  const int unitW = Frame::textWidth(unit);
  int scale = 3;
  if (Frame::textWidth(num, scale) + unitW + 2 > Frame::W) scale = 2;
  const int w = Frame::textWidth(num, scale);
  int x = (Frame::W - unitW - 2 - w) / 2;
  if (x < 0) x = 0;
  const int y = scale == 3 ? 10 : 14;
  f.text(x, y, num, scale);
  f.text(Frame::W - unitW, 25, unit);
}

} // namespace

void Frame::clear() { memset(buf_, 0, sizeof buf_); }

void Frame::pixel(int x, int y, bool on) {
  if (x < 0 || y < 0 || x >= W || y >= H) return;
  uint8_t &b = buf_[(y / 8) * W + x];
  const uint8_t bit = uint8_t(1u << (y % 8));
  b = on ? uint8_t(b | bit) : uint8_t(b & ~bit);
}

bool Frame::get(int x, int y) const {
  if (x < 0 || y < 0 || x >= W || y >= H) return false;
  return buf_[(y / 8) * W + x] & (1u << (y % 8));
}

void Frame::fillRect(int x, int y, int w, int h, bool on) {
  for (int j = y; j < y + h; ++j)
    for (int i = x; i < x + w; ++i) pixel(i, j, on);
}

void Frame::glyph(int x, int y, char c, int scale) {
  const uint8_t *col = lookup(c);
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 7; ++j)
      if (col[i] & (1u << j)) fillRect(x + i * scale, y + j * scale, scale, scale);
}

int Frame::text(int x, int y, const char *s, int scale) {
  for (; *s; ++s) {
    glyph(x, y, *s, scale);
    x += 6 * scale;
  }
  return x;
}

int Frame::textWidth(const char *s, int scale) {
  const int n = int(strlen(s));
  return n ? (n * 6 - 1) * scale : 0;
}

void render(Frame &f, const UiState &s) {
  f.clear();
  if (s.message) {
    const int scale = Frame::textWidth(s.message, 2) <= Frame::W ? 2 : 1;
    f.text((Frame::W - Frame::textWidth(s.message, scale)) / 2, scale == 2 ? 9 : 12, s.message, scale);
    return;
  }
  switch (s.page) {
  case UiState::Page::Last:
    header(f, s.status, s);
    bigNumber(f, s.lastRpm, "RPM");
    break;
  case UiState::Page::Best:
    header(f, "MAX", s);
    bigNumber(f, s.bestRpm, "RPM");
    break;
  case UiState::Page::Info: {
    char line[24];
    header(f, "INFO", s);
    snprintf(line, sizeof line, "LANCI %lu", (unsigned long)s.launches);
    f.text(0, 12, line);
    if (s.batteryPercent >= 0) {
      snprintf(line, sizeof line, "BATT %u.%02uV", s.batteryMv / 1000, (s.batteryMv % 1000) / 10);
    } else {
      snprintf(line, sizeof line, "BATT USB");
    }
    f.text(0, 22, line);
    break;
  }
  }
}

#ifdef ARDUINO
bool Oled::command(const uint8_t *cmds, size_t n) {
  Wire.beginTransmission(addr_);
  Wire.write(uint8_t(0x00)); // byte di controllo: seguono comandi
  Wire.write(cmds, n);
  return Wire.endTransmission() == 0;
}

bool Oled::begin(int sda, int scl, uint8_t address) {
  addr_ = address;
  Wire.begin(sda, scl, 400000);
  static const uint8_t init[] = {
      0xAE,       // spento durante la configurazione
      0xD5, 0x80, // clock
      0xA8, 0x1F, // 32 righe
      0xD3, 0x00, // nessuno scostamento verticale
      0x40,       // prima riga 0
      0x8D, 0x14, // pompa di carica interna
      0x20, 0x00, // indirizzamento orizzontale
      0xA1, 0xC8, // orientamento (ruotare: 0xA0, 0xC0)
      0xDA, 0x02, // configurazione pin per 128x32
      0x81, 0x8F, // contrasto
      0xD9, 0xF1, // precarica
      0xDB, 0x40, // VCOMH
      0xA4, 0xA6, // mostra la RAM, non invertito
      0x2E,       // niente scorrimento
      0xAF,       // acceso
  };
  ok_ = command(init, sizeof init);
  return ok_;
}

void Oled::show(const Frame &f) {
  if (!ok_) return;
  static const uint8_t window[] = {0x21, 0, 127, 0x22, 0, 3};
  if (!command(window, sizeof window)) return;
  const uint8_t *d = f.data();
  for (size_t i = 0; i < Frame::W * Frame::H / 8; i += 16) {
    Wire.beginTransmission(addr_);
    Wire.write(uint8_t(0x40)); // byte di controllo: seguono dati
    Wire.write(d + i, 16);
    Wire.endTransmission();
  }
}
#endif

} // namespace spinlite
