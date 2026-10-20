// SpinLite - conta giri economico per launcher Beyblade X.
// ESP32-C3 SuperMini + TCRT5000 (uscita analogica), dati via BLE con il profilo
// del Bey Battle Pass. Vedi CLAUDE.md per il progetto.
//
// Questo file contiene solo il collegamento tra i moduli:
//   adc_sampler  acquisizione e rilevatore (task ad alta priorita')
//   ble_bbp      servizio Bluetooth LE
//   settings     soglie e storico in flash
//   console      comandi da seriale
//   oled_ui      display opzionale
// Il loop non blocca mai: tutto e' guidato da millis().
#include <Arduino.h>

#include "adc_sampler.h"
#include "battery.h"
#include "bbp_protocol.h"
#include "ble_bbp.h"
#include "config.h"
#include "console.h"
#include "oled_ui.h"
#include "rpm_detector.h"
#include "settings.h"

using namespace spinlite;

namespace {

constexpr uint32_t SAVE_MIN_INTERVAL_MS = 2000;
constexpr uint32_t DEBOUNCE_MS = 30;

bbp::History history;
DetectorConfig config;
uint8_t uid[6];

bool historyDirty = false;
bool saved = false; // almeno un salvataggio fatto (per il primo senza attesa)
uint32_t lastSaveMs = 0;

bool lastReady = false;

// LED
uint32_t resultBlinkStartMs = 0;
bool resultBlinkActive = false;

// pulsante
bool buttonStable = true; // INPUT_PULLUP: true = rilasciato
bool buttonRaw = true;
uint32_t buttonChangeMs = 0;

#if SPINLITE_OLED
constexpr uint32_t OLED_REFRESH_MS = 250;
Oled oled;
Frame frame;
bool oledOk = false;
uint32_t lastOledMs = 0;
bool oledDirty = true;
Phase lastOledPhase = Phase::Settling;
#endif
UiState ui;

// ---------------------------------------------------------------------------
// Batteria
// ---------------------------------------------------------------------------
// Percentuale, -1 se alimentato da USB (o batteria non misurata).
int batteryNow() { return batteryPercent(sampler::batteryMv()); }

// ---------------------------------------------------------------------------
// Provider BLE
// ---------------------------------------------------------------------------
void provideStatus(uint8_t out[bbp::PACKET]) {
  uint8_t flags = 0;
  if (sampler::status().phase == Phase::Ready) flags |= bbp::FLAG_READY;
  const int pc = batteryNow();
  uint8_t battery = 250; // USB: pieno, nessuna segnalazione
  if (pc >= 0) {
    battery = bbp::batteryByte(uint8_t(pc));
    if (pc <= 20) flags |= bbp::FLAG_BATTERY_LOW;
    if (pc <= 5) flags |= bbp::FLAG_BATTERY_CRITICAL;
  }
  bbp::buildStatus(history, flags, battery, uid, out);
}

void provideDump(uint8_t out[bbp::DUMP_PAGES][bbp::PACKET]) { bbp::buildDump(history, out); }

void clearHistory() {
  bbp::clear(history);
  historyDirty = true;
#if SPINLITE_OLED
  oledDirty = true;
#endif
}

// ---------------------------------------------------------------------------
// LED (GPIO8, acceso a livello basso)
// ---------------------------------------------------------------------------
void ledWrite(bool on) {
#if PIN_LED >= 0
  digitalWrite(PIN_LED, (on == (LED_ACTIVE_LOW == 0)) ? HIGH : LOW);
#else
  (void)on;
#endif
}

void ledUpdate(uint32_t now, const LiveStatus &s) {
  bool on = false;
  if (s.phase == Phase::Measuring) {
    on = true;
  } else if (resultBlinkActive) {
    const uint32_t t = now - resultBlinkStartMs; // doppio lampeggio: 0-80 acceso, 160-240 acceso
    if (t >= 400) resultBlinkActive = false;
    on = t < 80 || (t >= 160 && t < 240);
  } else if (s.fault) {
    on = (now % 200) < 100;
  } else if (s.phase == Phase::Ready) {
    on = (now % 2000) < 60;
  }
  ledWrite(on);
}

// ---------------------------------------------------------------------------
// Pulsante (GPIO9): cambia pagina dell'OLED
// ---------------------------------------------------------------------------
void buttonUpdate(uint32_t now) {
  const bool raw = digitalRead(PIN_BUTTON) != LOW;
  if (raw != buttonRaw) {
    buttonRaw = raw;
    buttonChangeMs = now;
  }
  if (raw != buttonStable && now - buttonChangeMs >= DEBOUNCE_MS) {
    buttonStable = raw;
    if (!buttonStable) { // pressione
      switch (ui.page) {
      case UiState::Page::Last: ui.page = UiState::Page::Best; break;
      case UiState::Page::Best: ui.page = UiState::Page::Info; break;
      default: ui.page = UiState::Page::Last; break;
      }
#if SPINLITE_OLED
      oledDirty = true;
#endif
    }
  }
}

// ---------------------------------------------------------------------------
// OLED
// ---------------------------------------------------------------------------
#if SPINLITE_OLED
const char *statusText(const LiveStatus &s) {
  if (s.fault) return "GUASTO";
  switch (s.phase) {
  case Phase::Ready: return "PRONTO";
  case Phase::Measuring: return "MISURA";
  case Phase::Rearm: return "RIARMO";
  default: return "ATTESA";
  }
}

void oledUpdate(uint32_t now, const LiveStatus &s) {
  if (!oledOk) return;
  if (s.phase == Phase::Measuring) return; // niente I2C durante la misura
  if (s.phase != lastOledPhase) {
    lastOledPhase = s.phase;
    oledDirty = true;
  }
  if (!oledDirty && now - lastOledMs < OLED_REFRESH_MS) return;
  if (now - lastOledMs < 100) return;
  oledDirty = false;
  lastOledMs = now;
  ui.status = statusText(s);
  ui.lastRpm = bbp::latest(history);
  ui.bestRpm = history.best;
  ui.launches = history.total;
  const int mv = sampler::batteryMv();
  ui.batteryMv = uint16_t(mv > 0 ? mv : 0);
  ui.batteryPercent = batteryPercent(mv);
  ui.bleConnected = ble::connected();
  render(frame, ui);
  oled.show(frame);
}
#endif

// ---------------------------------------------------------------------------
// Salvataggio differito: mai durante la misura
// ---------------------------------------------------------------------------
void saveUpdate(uint32_t now, Phase phase) {
  if (!historyDirty || phase == Phase::Measuring) return;
  if (saved && now - lastSaveMs < SAVE_MIN_INTERVAL_MS) return;
  settings::saveHistory(history);
  historyDirty = false;
  saved = true;
  lastSaveMs = now;
}

void handleResult(const LaunchResult &r, uint32_t now) {
  bbp::add(history, r.rpm, r.curveUs, r.curveCount);
  historyDirty = true;
  Serial.printf("LANCIO,%u,%u,%u,%u,%lu,%u,%s\n", r.rpm, r.sustainedRpm, r.singleRpm, r.revolutions,
                (unsigned long)r.durationMs, r.weakestMark, endName(r.end));
  ble::notifyLaunch();
  resultBlinkActive = true;
  resultBlinkStartMs = now;
#if SPINLITE_OLED
  oledDirty = true;
#endif
}

} // namespace

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

#if PIN_LED >= 0
  pinMode(PIN_LED, OUTPUT);
  ledWrite(false);
#endif
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  settings::begin();
  settings::defaults(config);
  if (!settings::loadConfig(config)) {
    settings::defaults(config); // blob assente o non valido: restano i default
  }
  settings::loadHistory(history);

  const uint64_t mac = ESP.getEfuseMac();
  for (int i = 0; i < 6; ++i) uid[i] = uint8_t(mac >> (8 * i));

#if SPINLITE_OLED
  oledOk = oled.begin(PIN_OLED_SDA, PIN_OLED_SCL, OLED_I2C_ADDRESS);
  Serial.println(oledOk ? "OLED ok" : "OLED non risponde");
#endif

  Serial.printf("SpinLite %s, sensore %s\n", SPINLITE_VERSION, SPINLITE_SENSOR_NAME);
  if (!sampler::begin(config)) Serial.println("ERRORE: ADC non avviato");

  ble::Provider provider;
  provider.status = provideStatus;
  provider.dump = provideDump;
  provider.clear = clearHistory;
  if (!ble::begin(SPINLITE_BLE_NAME, BLE_TX_POWER_DBM, provider)) Serial.println("ERRORE: BLE non avviato");

  console::Hooks hooks;
  hooks.history = &history;
  hooks.config = &config;
  hooks.clearHistory = clearHistory;
  console::begin(hooks);
  Serial.println("Scrivi 'aiuto' per i comandi");
}

void loop() {
  const uint32_t now = millis();
  const LiveStatus s = sampler::status();

  LaunchResult r;
  while (sampler::popResult(r, 0)) handleResult(r, now);

  ble::setQuiet(s.phase == Phase::Measuring);
  const bool ready = s.phase == Phase::Ready;
  if (ready != lastReady && s.phase != Phase::Measuring) {
    lastReady = ready;
    ble::notifyStatus();
  }
  ble::loop();

  console::loop();
  buttonUpdate(now);
  ledUpdate(now, s);
#if SPINLITE_OLED
  oledUpdate(now, s);
#endif
  saveUpdate(now, s.phase);

  delay(1);
}
