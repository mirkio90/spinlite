// SpinLite - configurazione di compilazione.
// Tutti i valori modificabili a runtime (soglie del rilevatore) hanno qui il
// loro default; quelli salvati in memoria da console seriale hanno la precedenza.
#pragma once

#define SPINLITE_VERSION "0.1.0"

// ---------------------------------------------------------------------------
// Sensore ottico
// ---------------------------------------------------------------------------
// 0 = modulo TCRT5000/LM393 (uscita AO), 1 = breakout QRE1113 analogico
// (tipo SparkFun ROB-09453) montato nella base LaunchLab che guarda un anello
// bianco/nero interno al launcher. Si sceglie con l'ambiente PlatformIO
// (spinlite_qre, spinlite_qre_oled) oppure con -DSPINLITE_SENSOR_QRE=1.
#ifndef SPINLITE_SENSOR_QRE
#define SPINLITE_SENSOR_QRE 0
#endif
#if SPINLITE_SENSOR_QRE
#define SPINLITE_SENSOR_NAME "QRE1113"
#else
#define SPINLITE_SENSOR_NAME "TCRT5000"
#endif

// ---------------------------------------------------------------------------
// Pin (ESP32-C3 SuperMini)
// ---------------------------------------------------------------------------
// Uscita analogica del sensore (AO del TCRT5000, OUT del QRE1113). Deve essere un pin ADC1 (GPIO0-4):
// sul C3 l'ADC2 non funziona in modalita' continua (DMA).
#define PIN_SENSOR 3
// Partitore 100k/100k dalla batteria (dopo l'interruttore). -1 se non c'e'.
#define PIN_BATTERY 4
// LED blu sulla scheda (GPIO8, acceso a livello basso). -1 per disattivarlo.
#define PIN_LED 8
#define LED_ACTIVE_LOW 1
// Pulsante BOOT della scheda, usabile come tasto dopo l'avvio.
#define PIN_BUTTON 9

// ---------------------------------------------------------------------------
// Display OLED opzionale (SSD1306 128x32, I2C)
// ---------------------------------------------------------------------------
#ifndef SPINLITE_OLED
#define SPINLITE_OLED 0
#endif
#define PIN_OLED_SDA 6
#define PIN_OLED_SCL 7
#define OLED_I2C_ADDRESS 0x3C

// ---------------------------------------------------------------------------
// Bluetooth (profilo compatibile con il Bey Battle Pass BX-09)
// ---------------------------------------------------------------------------
// Il Battle Pass originale si annuncia come "BEYBLADE_TOOL01". Diverse app lo
// cercano per nome, quindi il default e' lo stesso. Cambialo se hai anche un
// Battle Pass vero e vuoi distinguerli.
#ifndef SPINLITE_BLE_NAME
#define SPINLITE_BLE_NAME "BEYBLADE_TOOL01"
#endif
#define BLE_TX_POWER_DBM 3
// Pausa tra due notifiche consecutive (pacchetti da 17 byte).
#define BLE_NOTIFY_SPACING_MS 12

// ---------------------------------------------------------------------------
// Acquisizione
// ---------------------------------------------------------------------------
// Ogni campione passato al rilevatore e' la somma di 4 letture ADC.
// Con la batteria nel pattern (S,S,S,S,B) servono 50 kHz per avere 10 kHz utili.
#define ADC_SENSOR_PER_OUTPUT 4
#define DETECTOR_RATE_HZ 10000
#define CPU_MHZ 80

// ---------------------------------------------------------------------------
// Rilevatore: default (modificabili da seriale, salvati in NVS)
// ---------------------------------------------------------------------------
#define DEFAULT_MARKS_PER_REV 1     // segni ottici per giro dell'albero
#if SPINLITE_SENSOR_QRE
// QRE1113: stessa soglia del profilo "QRE / Standard" di LaunchLab (80
// conteggi, analog_tachometer.h e sensor_profile.h). Da loro, nella base
// montata, i segni veri misurano 900-1100 conteggi, i disturbi dopo un lancio
// 100-270 e i fantasmi a riposo 10-21: 80 e' solo un pavimento sopra il
// rumore. L'unita' e' la stessa (conteggi di una singola lettura ADC a 12 bit).
// Valore da tarare sul campo con `raw`, vedi la guida.
#define DEFAULT_MIN_MARK 80         // ampiezza minima di un segno, conteggi ADC
#else
#define DEFAULT_MIN_MARK 150        // ampiezza minima di un segno, conteggi ADC
#endif
#define DEFAULT_MIN_LAUNCH_RPM 1000 // sotto questa velocita' non parte un lancio
#define DEFAULT_MAX_RPM 40000       // periodi piu' brevi sono scartati come disturbo
#define DEFAULT_END_PERCENT 30      // fine lancio quando scende al 30% del picco
#define DEFAULT_END_GAP_MS 250      // fine lancio se non arrivano segni
#define DEFAULT_QUIET_MS 1000       // riarmo: 1 s senza rotazioni (riavvolgimento)
#define DEFAULT_SINGLE_REV_PEAK 0   // 0 = picco su 3 giri, 1 = giro singolo

// ---------------------------------------------------------------------------
// Batteria LiPo
// ---------------------------------------------------------------------------
#define BATTERY_DIVIDER_RATIO 2.0f // 100k + 100k
#define BATTERY_USB_THRESHOLD_MV 2500 // sotto: interruttore spento, alimentato da USB
