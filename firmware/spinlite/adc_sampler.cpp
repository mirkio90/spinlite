#include "adc_sampler.h"

#include <Arduino.h>
#include <string.h>

#include "config.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace spinlite {
namespace sampler {
namespace {

constexpr bool HAS_BATTERY = PIN_BATTERY >= 0;

// Conversioni per ogni campione del rilevatore: 4 del sensore (+1 batteria).
constexpr unsigned PATTERN_LEN = ADC_SENSOR_PER_OUTPUT + (HAS_BATTERY ? 1 : 0);
constexpr unsigned FRAME_OUTPUTS = 100; // campioni del rilevatore per frame
constexpr uint32_t FRAME_BYTES = PATTERN_LEN * SOC_ADC_DIGI_RESULT_BYTES * FRAME_OUTPUTS; // 2000 con batteria
constexpr uint32_t STORE_BYTES = 32768;
// Il sensore va letto DETECTOR_RATE_HZ * ADC_SENSOR_PER_OUTPUT volte al secondo.
constexpr uint32_t ADC_FREQ_HZ = DETECTOR_RATE_HZ * PATTERN_LEN;

constexpr uint32_t NOMINAL_TICK_NS = 1000000000UL / DETECTOR_RATE_HZ;
constexpr int64_t CAL_US = 2000000;          // durata della misura di frequenza
constexpr unsigned STATUS_EVERY = DETECTOR_RATE_HZ / 20; // ~50 ms in campioni
constexpr uint32_t BATTERY_SAMPLES = DETECTOR_RATE_HZ;   // ~1 s di letture B
constexpr unsigned EDGE_QUEUE_LEN = 32;
constexpr unsigned RESULT_QUEUE_LEN = 4;

// Il partitore e' un rapporto costante: lo applico in millesimi, tutto intero.
constexpr int32_t DIVIDER_MILLI = (int32_t)(BATTERY_DIVIDER_RATIO * 1000.0f + 0.5f);

adc_continuous_handle_t s_adc = nullptr;
adc_cali_handle_t s_cali = nullptr;
TaskHandle_t s_task = nullptr;
QueueHandle_t s_resultQ = nullptr;
QueueHandle_t s_edgeQ = nullptr;
adc_channel_t s_chSensor = ADC_CHANNEL_0;
adc_channel_t s_chBattery = ADC_CHANNEL_0;

portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;

// Condivisi con le callback in ISR (scrive l'ISR, legge il task).
volatile uint32_t s_ovfCount = 0;
volatile uint32_t s_doneCount = 0;
volatile int64_t s_doneUs = 0;

// Condivisi con gli altri task, sempre sotto s_mux.
LiveStatus s_status;
DetectorConfig s_activeCfg;
DetectorConfig s_pendingCfg;
bool s_hasPending = false;
bool s_calibrated = false;
uint32_t s_rateMilliHz = 0;
uint32_t s_tickNs = NOMINAL_TICK_NS;
int s_batteryMv = -1;
volatile bool s_trace = false;

RpmDetector s_detector; // usato solo dal task

// ISR: un frame completo e' pronto. Segno anche l'istante: la misura di
// frequenza usa questi, non il momento in cui il task si sveglia.
bool IRAM_ATTR onConvDone(adc_continuous_handle_t, const adc_continuous_evt_data_t *, void *) {
  s_doneUs = esp_timer_get_time();
  s_doneCount = s_doneCount + 1;
  BaseType_t woken = pdFALSE;
  if (s_task) vTaskNotifyGiveFromISR(s_task, &woken);
  return woken == pdTRUE;
}

// ISR: il buffer e' pieno, il driver ha scartato dati.
bool IRAM_ATTR onPoolOvf(adc_continuous_handle_t, const adc_continuous_evt_data_t *, void *) {
  s_ovfCount = s_ovfCount + 1;
  return false;
}

void publishStatus() {
  LiveStatus s;
  s.phase = s_detector.phase();
  s.level = s_detector.levelCounts();
  s.noise = s_detector.noiseCounts();
  s.swing = s_detector.swingCounts();
  s.hysteresis = s_detector.hysteresisCounts();
  s.liveRpm = s_detector.liveRpm();
  s.fault = s_detector.signalFault();
  s.edges = s_detector.edges();
  s.launches = s_detector.launches();
  s.anomalies = s_detector.anomalies();
  s.overflows = s_ovfCount;
  taskENTER_CRITICAL(&s_mux);
  s_status = s;
  taskEXIT_CRITICAL(&s_mux);
}

void applyConfig(const DetectorConfig &cfg) {
  s_detector.configure(cfg);
  taskENTER_CRITICAL(&s_mux);
  s_activeCfg = cfg;
  taskEXIT_CRITICAL(&s_mux);
}

// Media delle letture B -> mV al pin -> mV della batteria.
void publishBattery(uint32_t sum, uint32_t count) {
  int raw = (int)(sum / count);
  int mv = 0;
  if (!s_cali || adc_cali_raw_to_voltage(s_cali, raw, &mv) != ESP_OK) {
    mv = raw * 2500 / 4095; // senza calibrazione: fondo scala di circa 2,5 V a 12 dB
  }
  int batt = (int)((int64_t)mv * DIVIDER_MILLI / 1000);
  taskENTER_CRITICAL(&s_mux);
  s_batteryMv = batt;
  taskEXIT_CRITICAL(&s_mux);
}

void handleEvents(uint8_t ev) {
  if (ev & RpmDetector::EV_RESULT) {
    LaunchResult r = s_detector.result();
    xQueueSend(s_resultQ, &r, 0);
  }
  if ((ev & RpmDetector::EV_EDGE) && s_trace && s_edgeQ) {
    EdgeInfo e = s_detector.lastEdge();
    xQueueSend(s_edgeQ, &e, 0);
  }
  if (ev & (RpmDetector::EV_PHASE | RpmDetector::EV_RESULT)) publishStatus();
}

void sensorTask(void *) {
  static uint8_t buf[FRAME_BYTES];

  uint32_t seenOvf = s_ovfCount;
  int32_t sum = 0;
  unsigned sumN = 0;
  unsigned sinceStatus = 0;
  uint32_t battSum = 0, battN = 0;

  // misura della frequenza
  bool calibrating = true;
  bool calStarted = false;
  uint32_t calCount0 = 0;
  int64_t calUs0 = 0;

  for (;;) {
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));

    // nuove soglie dalla console: tickNs/scale/smooth restano quelli attuali
    DetectorConfig pending;
    bool apply = false;
    taskENTER_CRITICAL(&s_mux);
    if (s_hasPending) {
      pending = s_pendingCfg;
      s_hasPending = false;
      apply = true;
    }
    taskEXIT_CRITICAL(&s_mux);
    if (apply) {
      const DetectorConfig &cur = s_detector.config();
      pending.tickNs = cur.tickNs;
      pending.scale = cur.scale;
      pending.smooth = cur.smooth;
      applyConfig(pending);
      sum = 0;
      sumN = 0;
    }

    for (;;) {
      uint32_t len = 0;
      esp_err_t err = adc_continuous_read(s_adc, buf, sizeof(buf), &len, 0);
      if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) break; // ESP_ERR_TIMEOUT: vuoto
      if (len == 0) break;

      // buco nei dati: un lancio in corso e' perso, la misura di frequenza si rifa'
      uint32_t ovf = s_ovfCount;
      if (ovf != seenOvf || err == ESP_ERR_INVALID_STATE) {
        seenOvf = ovf;
        s_detector.dataLoss();
        sum = 0;
        sumN = 0;
        if (calibrating) calStarted = false;
      }

      for (uint32_t i = 0; i + SOC_ADC_DIGI_RESULT_BYTES <= len; i += SOC_ADC_DIGI_RESULT_BYTES) {
        const adc_digi_output_data_t *d = (const adc_digi_output_data_t *)&buf[i];
        if (d->type2.unit != 0) continue; // ADC1
        uint32_t ch = d->type2.channel;
        uint32_t val = d->type2.data;
        if (ch == (uint32_t)s_chSensor) {
          sum += (int32_t)val;
          if (++sumN >= ADC_SENSOR_PER_OUTPUT) {
            uint8_t ev = s_detector.feed(sum);
            sum = 0;
            sumN = 0;
            if (ev) handleEvents(ev);
            if (++sinceStatus >= STATUS_EVERY) {
              sinceStatus = 0;
              publishStatus();
            }
          }
        } else if (HAS_BATTERY && ch == (uint32_t)s_chBattery) {
          battSum += val;
          if (++battN >= BATTERY_SAMPLES) {
            publishBattery(battSum, battN);
            battSum = 0;
            battN = 0;
          }
        }
      }

      if (calibrating) {
        // coppia (frame completati, istante) coerente, scritta dall'ISR
        uint32_t cnt;
        int64_t us;
        taskENTER_CRITICAL(&s_mux);
        cnt = s_doneCount;
        us = s_doneUs;
        taskEXIT_CRITICAL(&s_mux);
        if (!calStarted) {
          calStarted = true;
          calCount0 = cnt;
          calUs0 = us;
        } else if (us - calUs0 >= CAL_US) {
          uint64_t elapsedUs = (uint64_t)(us - calUs0);
          uint64_t count = (uint64_t)(cnt - calCount0) * FRAME_OUTPUTS;
          calibrating = false;
          if (count > 0) {
            uint64_t tick = elapsedUs * 1000ULL / count;
            uint32_t rate = (uint32_t)(count * 1000000000ULL / elapsedUs);
            // un errore oltre il 10% indica un problema, non un quarzo storto
            bool plausible = tick > NOMINAL_TICK_NS * 9ULL / 10 && tick < NOMINAL_TICK_NS * 11ULL / 10;
            if (plausible) {
              DetectorConfig cfg = s_detector.config();
              cfg.tickNs = (uint32_t)tick;
              applyConfig(cfg);
              sum = 0;
              sumN = 0;
              taskENTER_CRITICAL(&s_mux);
              s_tickNs = (uint32_t)tick;
              s_rateMilliHz = rate;
              taskEXIT_CRITICAL(&s_mux);
            }
            Serial.printf("ADC: frequenza misurata %u.%03u Hz, tickNs %u%s\n", (unsigned)(rate / 1000),
                          (unsigned)(rate % 1000), (unsigned)tick,
                          plausible ? "" : " (fuori tolleranza, uso il nominale)");
          }
          taskENTER_CRITICAL(&s_mux);
          s_calibrated = true;
          taskEXIT_CRITICAL(&s_mux);
        }
      }
    }
  }
}

} // namespace

bool begin(const DetectorConfig &cfgIn) {
  if (s_adc) return true;

  adc_unit_t unit;
  if (adc_continuous_io_to_channel(PIN_SENSOR, &unit, &s_chSensor) != ESP_OK || unit != ADC_UNIT_1) return false;
  if (HAS_BATTERY) {
    if (adc_continuous_io_to_channel(PIN_BATTERY, &unit, &s_chBattery) != ESP_OK || unit != ADC_UNIT_1) return false;
  }

  DetectorConfig cfg = cfgIn;
  cfg.tickNs = NOMINAL_TICK_NS;
  cfg.scale = ADC_SENSOR_PER_OUTPUT;
  cfg.smooth = 4;
  s_detector.configure(cfg);
  s_activeCfg = cfg;

  s_resultQ = xQueueCreate(RESULT_QUEUE_LEN, sizeof(LaunchResult));
  s_edgeQ = xQueueCreate(EDGE_QUEUE_LEN, sizeof(EdgeInfo));
  if (!s_resultQ || !s_edgeQ) return false;

  adc_continuous_handle_cfg_t hc = {};
  hc.max_store_buf_size = STORE_BYTES;
  hc.conv_frame_size = FRAME_BYTES;
  if (adc_continuous_new_handle(&hc, &s_adc) != ESP_OK) return false;

  // Pattern: S,S,S,S[,B]. Il canale si legge comunque da ogni conversione.
  adc_digi_pattern_config_t pat[PATTERN_LEN] = {};
  for (unsigned i = 0; i < PATTERN_LEN; i++) {
    bool isBatt = HAS_BATTERY && i == ADC_SENSOR_PER_OUTPUT;
    pat[i].atten = ADC_ATTEN_DB_12;
    pat[i].channel = (uint8_t)(isBatt ? s_chBattery : s_chSensor);
    pat[i].unit = ADC_UNIT_1;
    pat[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
  }
  adc_continuous_config_t dc = {};
  dc.pattern_num = PATTERN_LEN;
  dc.adc_pattern = pat;
  dc.sample_freq_hz = ADC_FREQ_HZ;
  dc.conv_mode = ADC_CONV_SINGLE_UNIT_1;
  dc.format = ADC_DIGI_OUTPUT_FORMAT_TYPE2;
  if (adc_continuous_config(s_adc, &dc) != ESP_OK) {
    adc_continuous_deinit(s_adc);
    s_adc = nullptr;
    return false;
  }

  adc_continuous_evt_cbs_t cbs = {};
  cbs.on_conv_done = onConvDone;
  cbs.on_pool_ovf = onPoolOvf;
  if (adc_continuous_register_event_callbacks(s_adc, &cbs, nullptr) != ESP_OK) {
    adc_continuous_deinit(s_adc);
    s_adc = nullptr;
    return false;
  }

  if (HAS_BATTERY) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cc = {};
    cc.unit_id = ADC_UNIT_1;
    cc.chan = s_chBattery;
    cc.atten = ADC_ATTEN_DB_12;
    cc.bitwidth = ADC_BITWIDTH_DEFAULT;
    if (adc_cali_create_scheme_curve_fitting(&cc, &s_cali) != ESP_OK) s_cali = nullptr;
#endif
  }

  // Il task parte prima dell'ADC, cosi' la prima notifica non si perde.
  if (xTaskCreate(sensorTask, "adc", 4096, nullptr, configMAX_PRIORITIES - 2, &s_task) != pdPASS) {
    adc_continuous_deinit(s_adc);
    s_adc = nullptr;
    return false;
  }
  if (adc_continuous_start(s_adc) != ESP_OK) {
    vTaskDelete(s_task);
    s_task = nullptr;
    adc_continuous_deinit(s_adc);
    s_adc = nullptr;
    return false;
  }
  return true;
}

void reconfigure(const DetectorConfig &cfg) {
  taskENTER_CRITICAL(&s_mux);
  s_pendingCfg = cfg;
  s_hasPending = true;
  taskEXIT_CRITICAL(&s_mux);
  if (s_task) xTaskNotifyGive(s_task);
}

DetectorConfig config() {
  taskENTER_CRITICAL(&s_mux);
  DetectorConfig c = s_activeCfg;
  taskEXIT_CRITICAL(&s_mux);
  return c;
}

bool popResult(LaunchResult &out, uint32_t timeoutMs) {
  if (!s_resultQ) return false;
  return xQueueReceive(s_resultQ, &out, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

LiveStatus status() {
  taskENTER_CRITICAL(&s_mux);
  LiveStatus s = s_status;
  taskEXIT_CRITICAL(&s_mux);
  return s;
}

bool calibrated() {
  taskENTER_CRITICAL(&s_mux);
  bool v = s_calibrated;
  taskEXIT_CRITICAL(&s_mux);
  return v;
}

uint32_t rateMilliHz() {
  taskENTER_CRITICAL(&s_mux);
  uint32_t v = s_rateMilliHz;
  taskEXIT_CRITICAL(&s_mux);
  return v;
}

uint32_t tickNs() {
  taskENTER_CRITICAL(&s_mux);
  uint32_t v = s_tickNs;
  taskEXIT_CRITICAL(&s_mux);
  return v;
}

int batteryMv() {
  if (!HAS_BATTERY) return -1;
  taskENTER_CRITICAL(&s_mux);
  int v = s_batteryMv;
  taskEXIT_CRITICAL(&s_mux);
  return v;
}

void setEdgeTrace(bool on) {
  if (on && s_edgeQ) xQueueReset(s_edgeQ);
  s_trace = on;
}

bool popEdge(EdgeInfo &out) {
  if (!s_edgeQ) return false;
  return xQueueReceive(s_edgeQ, &out, 0) == pdTRUE;
}

} // namespace sampler
} // namespace spinlite
