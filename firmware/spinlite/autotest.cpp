#include "autotest.h"

#include <Arduino.h>

#include "config.h"
#include "driver/gptimer.h"
#include "soc/gpio_reg.h"

namespace spinlite {
namespace autotest {
namespace {

constexpr uint32_t REST_MS = 1500;  // riposo iniziale: il rilevatore va in PRONTO
constexpr uint32_t STEP_MS = 25;    // passo del rallentamento
constexpr uint32_t STEP_PCT = 93;   // ogni passo la velocita' scende al 93%
constexpr uint32_t STOP_PCT = 25;   // sotto il 25% della velocita' iniziale si ferma

enum class State : uint8_t { Off, Rest, Hold, Decay, Idle };

gptimer_handle_t s_timer = nullptr;
volatile bool s_level = true;
State s_state = State::Off;
uint32_t s_t0 = 0, s_holdMs = 0, s_lastStep = 0;
uint32_t s_rpmNow = 0, s_rpmStop = 0;
uint8_t s_marks = 1;

// Interrupt del timer: inverte il pin scrivendo direttamente nei registri
// (gpio_set_level non e' garantito in IRAM).
bool IRAM_ATTR onAlarm(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *) {
  s_level = !s_level;
  if (s_level)
    REG_WRITE(GPIO_OUT_W1TS_REG, 1u << PIN_TEST_OUT);
  else
    REG_WRITE(GPIO_OUT_W1TC_REG, 1u << PIN_TEST_OUT);
  return false;
}

// Mezzo periodo in us: un giro sono marks segni, ogni segno un periodo intero.
uint32_t halfPeriodUs(uint32_t rpm) {
  return (30000000u + rpm * s_marks / 2) / (rpm * s_marks);
}

bool setHalf(uint32_t halfUs) {
  gptimer_alarm_config_t a = {};
  a.alarm_count = halfUs;
  a.reload_count = 0;
  a.flags.auto_reload_on_alarm = true;
  // Il mezzo periodo cresce soltanto (rallentamento): il contatore e' sempre
  // sotto il nuovo allarme, quindi il cambio non perde un fronte.
  return gptimer_set_alarm_action(s_timer, &a) == ESP_OK;
}

void timerOff() {
  if (!s_timer) return;
  gptimer_stop(s_timer);
  gptimer_disable(s_timer);
  gptimer_del_timer(s_timer);
  s_timer = nullptr;
}

void restHigh() {
  s_level = true;
  digitalWrite(PIN_TEST_OUT, HIGH);
}

} // namespace

bool start(uint32_t rpm, uint32_t holdMs, uint8_t marksPerRev, uint32_t *halfUs) {
#if PIN_TEST_OUT < 0
  (void)rpm; (void)holdMs; (void)marksPerRev; (void)halfUs;
  return false;
#else
  stop();
  if (rpm < 300 || rpm > 40000 || holdMs < 200 || holdMs > 10000) return false;
  s_marks = marksPerRev ? marksPerRev : 1;
  s_rpmNow = rpm;
  s_rpmStop = rpm * STOP_PCT / 100;
  s_holdMs = holdMs;

  pinMode(PIN_TEST_OUT, OUTPUT);
  restHigh();

  gptimer_config_t tc = {};
  tc.clk_src = GPTIMER_CLK_SRC_DEFAULT;
  tc.direction = GPTIMER_COUNT_UP;
  tc.resolution_hz = 1000000; // 1 tick = 1 us
  if (gptimer_new_timer(&tc, &s_timer) != ESP_OK) {
    s_timer = nullptr;
    return false;
  }
  gptimer_event_callbacks_t cb = {};
  cb.on_alarm = onAlarm;
  if (gptimer_register_event_callbacks(s_timer, &cb, nullptr) != ESP_OK || gptimer_enable(s_timer) != ESP_OK ||
      !setHalf(halfPeriodUs(rpm))) {
    timerOff();
    return false;
  }
  if (halfUs) *halfUs = halfPeriodUs(rpm);
  s_state = State::Rest;
  s_t0 = millis();
  return true;
#endif
}

void stop() {
  timerOff();
  if (s_state != State::Off) {
#if PIN_TEST_OUT >= 0
    pinMode(PIN_TEST_OUT, INPUT); // alta impedenza: non disturba un sensore vero
#endif
  }
  s_state = State::Off;
}

bool loop() {
  const uint32_t now = millis();
  switch (s_state) {
  case State::Rest:
    if (now - s_t0 >= REST_MS) {
      gptimer_set_raw_count(s_timer, 0);
      gptimer_start(s_timer);
      s_state = State::Hold;
      s_t0 = now;
    }
    return false;
  case State::Hold:
    if (now - s_t0 >= s_holdMs) {
      s_state = State::Decay;
      s_lastStep = now;
    }
    return false;
  case State::Decay:
    if (now - s_lastStep < STEP_MS) return false;
    s_lastStep = now;
    s_rpmNow = s_rpmNow * STEP_PCT / 100;
    if (s_rpmNow < s_rpmStop || s_rpmNow < 100) {
      timerOff();
      restHigh(); // torna a riposo: il rilevatore chiude il lancio
      s_state = State::Idle;
      return true;
    }
    setHalf(halfPeriodUs(s_rpmNow));
    return false;
  default:
    return false;
  }
}

bool active() { return s_state == State::Rest || s_state == State::Hold || s_state == State::Decay; }

} // namespace autotest
} // namespace spinlite
