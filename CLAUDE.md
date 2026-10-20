# SpinLite: passaggio di consegne per Claude Code

Questo file è il punto di ripartenza per lavorare in locale. Claude Code lo legge da solo quando apri la cartella del repo.
Aggiornalo quando cambia lo stato (sezione "Stato" e "Prossimi passi").

## Preferenze di lavoro dell'utente (Mirko)

- Parla **italiano**, risposte dirette e senza preamboli.
- Vuole **imparare**: per ogni soluzione spiega anche *come* hai affrontato il problema e perché, non solo il risultato.
- Commit piccoli e frequenti, push a ogni passo: vuole vedere il repo crescere.
- Messaggi di commit in italiano.

## Obiettivo

Il conta giri **più economico possibile** per launcher Beyblade X:

- **ESP32-C3 SuperMini** (circa 3-4 € da AliExpress) + modulo **TCRT5000/LM393** (uscita analogica AO) + LiPo piccola + modulo di ricarica.
- Dati al telefono via **BLE con lo stesso profilo del Bey Battle Pass BX-09**, così funzionano le app esistenti: BeyGear, Spin Meter, Beyblader Helper, 321GoShoot e la dashboard web di MarkChan0225. Nessuna app da scrivere.
- **OLED 128x32 opzionale** (flag di compilazione `SPINLITE_OLED`).
- Algoritmo di misura sul segnale **analogico** con soglie adattive in software: è il punto di forza di LaunchLab, che invece costa uno StickS3 da circa 29 $.

## Da dove nasce (contesto della ricerca)

- `mirkio90/launchlab` è un fork di `shadohead/launchlab`, firmware M5StickS3 con circa 8.400 righe. Lo usiamo **solo come fonte di idee**. Il suo `analog_tachometer.h` documenta molti problemi reali, ed è da lì che vengono le soglie usate qui: niente lanci sotto 1000 RPM, ampiezza minima del segno, riarmo di 1 s per il riavvolgimento della corda, tempo misurato in campioni ADC.
- Progetti simili trovati:
  - **BRD** di printfChins: ESP32-C3 + OLED + emulazione Battle Pass, ma con fronti digitali; licenza GPL-3.0.
  - **Beyblade-Dragoon-RPM** di Luciandraw: ESP32-S3-Zero + e-paper.
  - **PWS3D Battle Bypass**: Waveshare ESP32-S3-LCD-1.47B, firmware chiuso.
  - **make-muda "Blay Rouzer"**: Seeeduino XIAO + display a 7 segmenti.
  - **CraftLong**: venduto su Etsy.
- Verifica indipendente di SpiningDays (note.com): lo "shoot power" del Battle Pass con i launcher a corda **sovrastima** gli RPM reali della trottola, per via del rimbalzo del riavvolgimento. Noi misuriamo l'**albero del launcher**, non la trottola: va detto chiaramente nella guida.

## Licenze: attenzione

- SpinLite è **MIT**.
- **BRD è GPL-3.0**: si può consultare per confrontare i valori del protocollo, **mai copiarne codice**. Copiandolo il progetto diventerebbe GPL.
- I supporti 3D di LaunchLab sono **CC BY-SA**, perché derivano dal BP Gear Port Connector di Migbello. Vanno **citati e linkati**, non copiati nel repo. Quelli compatibili con il TCRT5000 sono:
  - `hardware/minimal_TCRT_sensor_only_v042`
  - `hardware/minimal_TCRT_split_keepers_v043`

  Il v019 invece usa il dock dell'M5 come fermo del PCB, quindi non va bene da solo.
- Il nome BLE di default è `BEYBLADE_TOOL01`, quello del dispositivo originale, perché alcune app lo cercano per nome. Il progetto non è affiliato a Takara Tomy o Hasbro.

## Struttura del repo

```
firmware/spinlite/
  config.h          pin, default, flag di compilazione       (fatto)
  rpm_detector.*    rilevatore portabile, solo interi        (fatto, testato)
  bbp_protocol.*    pacchetti Battle Pass + salvataggio NVS  (fatto, testato)
  oled_ui.*         schermate 128x32 + driver SSD1306 I2C    (fatto; disegno testato, driver I2C non provato su hardware)
  spinlite.ino      DA FARE: setup/loop, acquisizione, BLE, console, batteria
test/
  Makefile          make -C test   (g++ C++17 con ASan/UBSan, -Werror)
  signal_sim.h      simulatore del segnale AO (giri, segni, rumore, deriva, riavvolgimento)
  test_detector.cpp 21 casi
  test_protocol.cpp layout pacchetti, checksum, buffer circolare, CRC; --dump file per il confronto incrociato
  test_oled.cpp     disegna le schermate in ASCII (--pbm cartella per le immagini)
```

## Stato (20/10/2026, sera)

Commit su `main`, nell'ordine:
1. avvio, rilevatore e simulatore;
2. picco calcolato solo su giri coerenti, test più severi;
3. pacchetti Battle Pass;
4. OLED e questo file.

`make -C test` è tutto verde. Risultati del rilevatore sui segnali simulati:
- da 1.500 a 30.000 RPM l'errore è sotto lo 0,1%; il giro singolo resta sotto lo 0,6% anche con 2 segni per giro;
- segni deboli (220 conteggi), rumore forte e riposo saturo a 4095 vengono gestiti;
- un disturbo a metà giro, provato in 12 posizioni, dà un errore massimo dello 0,06%;
- il riavvolgimento non viene contato come lancio;
- fermo per 30 s con disturbi: zero falsi lanci;
- dopo una perdita di dati non nasce un lancio fantasma;
- il sensore scollegato viene rilevato.

I pacchetti generati sono stati letti correttamente anche dal parser JavaScript indipendente di `MarkChan0225/beyblade-tachometer-web` (`protocol.mjs`): stato, 50 lanci, checksum valido, curva a 10.000 RPM.

**Nulla è stato ancora provato su hardware vero.** In questa sessione cloud non c'era la toolchain ESP32: il proxy blocca il registry PlatformIO e i pacchetti Espressif.

## Prossimi passi, in ordine

1. **`spinlite.ino` + `platformio.ini`** (ambiente `esp32-c3-devkitm-1`, framework arduino; core Arduino-ESP32 3.x / pioarduino; NimBLE-Arduino 2.x). Il progetto è descritto sotto.
2. **Compilare e flashare in locale**, poi provare con il segnale vero. Usa il comando `raw` della console per vedere livello, rumore e ampiezza mentre giri l'albero a mano, e regola `minmark` e la vite del TCRT.
3. **GitHub Action** che compila con PlatformIO e pubblica il `.bin` come artifact o release.
4. **Dashboard Web Bluetooth** su GitHub Pages, scritta da zero (non copiare quella di MarkChan, che non ha licenza dichiarata), più un **web flasher** con esp-web-tools.
5. **README / guida in italiano**: distinta base con prezzi, cablaggio con foto o schema, taratura, limiti (misura sull'albero), app compatibili.
6. **Scatolina per l'elettronica**, parametrica (OpenSCAD o CadQuery), montata a fianco del sensore, con i supporti sensore di LaunchLab linkati.

## Progetto del firmware (da scrivere: `spinlite.ino` + moduli)

### Acquisizione (task FreeRTOS ad alta priorità)
- `adc_continuous` (ESP-IDF, `esp_adc/adc_continuous.h`), solo ADC1, formato di uscita `ADC_DIGI_OUTPUT_FORMAT_TYPE2`. Sul C3 si decodifica con `adc_digi_output_data_t.type2.{data,channel,unit}` (4 byte per conversione).
- Pattern `[S,S,S,S,B]` a 50 kHz (S = sensore su GPIO3/ADC1_CH3, B = batteria su GPIO4/ADC1_CH4). Senza batteria il pattern è `[S,S,S,S]` a 40 kHz. In entrambi i casi il sensore viene letto 40.000 volte al secondo.
- Si sommano 4 letture del sensore per ogni campione del rilevatore, quindi 10 kHz (tickNs = 100000) con `scale = 4` e `smooth = 4`.
- `max_store_buf_size` circa 32 KB (160 ms di margine per scritture NVS e altro), `conv_frame_size` = 5 × 4 × 100 = 2000 byte.
- Callback `on_pool_ovf`: imposta un flag; nel task, `detector.dataLoss()`.
- **Calibrare la frequenza vera**: all'avvio si contano i campioni per circa 2 s con `esp_timer_get_time()`, che segue il quarzo, e si chiama `detector.configure()` con il `tickNs` misurato. L'errore sugli RPM è proporzionale all'errore sulla frequenza ADC, e i divisori del C3 non danno esattamente 50 kHz. Il valore misurato va stampato in console.
- Il task pubblica:
  - i risultati su una coda FreeRTOS (`LaunchResult`, profondità 4);
  - uno snapshot di stato ogni circa 50 ms (fase, livello, rumore, ampiezza, isteresi, guasto, RPM live), protetto da un mutex o da una sezione critica.
- La batteria si ricava dalla media dei campioni B per circa 1 s, con `adc_cali_create_scheme_curve_fitting` per i mV e un fallback lineare. Sotto 2,5 V significa "interruttore spento, alimentato da USB" e la percentuale vale -1.

### BLE (NimBLE-Arduino 2.x)
- Service `55c40000-f8eb-11ec-b939-0242ac120002`.
- Caratteristica `55c4f001-...` con WRITE e WRITE_NR, **creata per prima**. Caratteristica `55c4f002-...` con NOTIFY, WRITE, WRITE_NR e READ, creata dopo.
  - Perché: lo script Python sul Battle Pass vero scrive sull'handle 15 e riceve notifiche sull'handle 17. Questo torna se f001 (dichiarazione 14, valore 15) precede f002 (dichiarazione 16, valore 17, CCCD 18).
  - BRD usa solo f002 per scrivere e notificare, MarkChan scrive su f001: accettare i comandi su entrambe copre tutti i casi.
- Advertising: flags 0x06 + UUID del service; il nome va nello scan response.
- Comandi: `0x51` → A0; `0x74` → B0..B7 + 70..73; `0x75` → azzera lo storico, come fa BRD (MarkChan lo chiama "sensor test", e sulla semantica le fonti non concordano); qualunque altro comando va ignorato.
- Le callback BLE mettono i comandi in una coda; il loop li esegue e invia le pagine una alla volta, a 12 ms l'una dall'altra.
- **Notifiche automatiche** (come BRD in "compat autonotify", che risulta compatibile con le app): A0 appena l'app si iscrive alle notifiche, poi A0 + dump completo dopo ogni lancio.
- Flag di A0:
  - 0x04 quando il rilevatore è PRONTO (nel Battle Pass vero significa "trottola inserita", e noi non abbiamo quel sensore);
  - 0x10 con batteria ≤ 20%;
  - 0x01 con batteria ≤ 5%.
- UID nei byte 11-16 di A0: MAC della eFuse (`ESP.getEfuseMac()`).
- Durante MISURA niente notifiche, niente OLED e niente scritture NVS, per tenere pulito il segnale analogico. Si fa tutto dopo il risultato, durante il riarmo di 1 s.

### Storico e impostazioni (Preferences / NVS, namespace `spinlite`)
- `hist`: blob `bbp::encode` (versione + CRC32), salvato al massimo ogni 2 s e solo fuori da MISURA.
- `cfg`: blob con versione dei parametri del rilevatore (`marksPerRev`, `minMark`, `minLaunchRpm`, `maxRpm`, `endPercent`, `endGapMs`, `quietMs`, `singleRevPeak`).

### Console seriale (USB CDC, 115200)
- `aiuto` — elenco dei comandi
- `stato` — fase, livello, rumore, ampiezza, isteresi, frequenza misurata, batteria, BLE
- `raw [s]` — stampa livello, rumore e ampiezza a 20 Hz: serve per la taratura
- `fronti` — stampa ogni fronte (tempo, periodo, ampiezza, valido o no)
- `storico`, `cancella`
- `set <parametro> <valore>`, `salva`, `default`
- Ogni lancio stampa una riga CSV: `LANCIO,rpm,rpm3giri,giro_singolo,giri,durata_ms,segno_min,fine`.

### Interfaccia
- LED su GPIO8, acceso a livello basso:
  - lampeggio breve ogni 2 s in PRONTO;
  - acceso durante MISURA;
  - doppio lampeggio quando arriva un risultato;
  - lampeggio rapido in caso di guasto del sensore.
- Pulsante BOOT (GPIO9), usabile dopo l'avvio: una pressione cambia pagina dell'OLED (ultimo → massimo → info).

### Batteria LiPo: tabella approssimata
4200 = 100%, 4100 = 90, 4000 = 80, 3900 = 65, 3800 = 50, 3750 = 40, 3700 = 30, 3650 = 20, 3600 = 12, 3500 = 5, 3300 = 0. Interpolazione lineare. È una stima: va detto.

## Hardware: cablaggio previsto (vedi `config.h`)

| Da | A |
| --- | --- |
| TCRT5000 VCC | 3V3 della SuperMini |
| TCRT5000 GND | GND |
| TCRT5000 AO | GPIO3 (DO non collegato) |
| LiPo + (dopo l'interruttore) | partitore 100k/100k → GPIO4, **con 100 nF verso GND** sul punto centrale |
| Uscita del modulo di ricarica (TP4056 con protezione) | pin 5V della SuperMini, tramite l'interruttore |
| OLED SDA / SCL (opzionale) | GPIO6 / GPIO7, alimentazione 3V3 |

Il condensatore sul partitore serve perché l'ADC campiona di continuo: senza, la sorgente da 50 kΩ falsa le letture.

## Trappole note dell'ESP32-C3

- In modalità continua (DMA) funziona **solo ADC1** (GPIO0-4).
- Con attenuazione da 12 dB la scala arriva a **circa 2,5 V**, non a 3,3 V. Sopra satura a 4095. È normale che AO del TCRT a riposo saturi: è il caso "riposo saturo" già testato. Il segno vero abbassa la tensione.
- GPIO2, 8 e 9 sono pin di strapping: non tenerli bassi all'avvio. GPIO9 è il pulsante BOOT, GPIO8 il LED.
- **Niente FPU**: il rilevatore usa solo interi. Non introdurre float nel percorso a 10 kHz.
- In Arduino IDE va attivato "USB CDC On Boot" (con PlatformIO: `-DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1`).
- Alimentare dal pin 5V con la LiPo (3,7–4,2 V) passa per l'LDO della scheda: funziona, ma sotto circa 3,5 V la tensione a 3V3 cala. Da verificare sulla propria scheda.

## Protocollo: formato dei pacchetti (17 byte, little-endian)

- **A0**:
  - byte 1 = 0x3A (fisso, secondo BRD);
  - byte 3 = flag;
  - byte 4 = batteria 0..250;
  - byte 7-8 = massimo;
  - byte 9-10 = totale lanci;
  - byte 11-16 = UID.
- **B0..B6**: storico, 8 valori a 16 bit per pacchetto, dal più vecchio al più recente.
  - B6 contiene i valori 48 e 49 nei byte 1-4, poi massimo (7-8), totale (9-10) e numero di lanci (11).
- **B7**: byte 16 = somma (mod 256) dei byte 1..16 di B0..B6.
- **70..73**: 32 periodi del primo tratto del lancio in unità da 8 µs (RPM = 7.500.000 / valore).

Fonti:
- documentazione di beyblade-tachometer-web (MarkChan0225);
- BRD (printfChins), confrontato senza copiarne codice;
- script Python sul dispositivo vero (danielbyomujuni).

## Link utili

- https://github.com/MarkChan0225/beyblade-tachometer-web
- https://github.com/printfChins/Beyblade-RPM-Detector-Device (GPL-3.0)
- https://github.com/danielbyomujuni/beyblade-x-beybattlepass-python
- https://github.com/Luciandraw/Beyblade-Dragoon-RPM
- https://github.com/shadohead/launchlab · fork: https://github.com/mirkio90/launchlab
- https://flash.pws3d.us/ (Battle Bypass)
- https://note.com/spiningdays/n/nb40b4beb37d2 (shoot power vs RPM reali)
- https://make-muda.net/2025/03/8646/ (Blay Rouzer)
- https://beygear.app/bey-battle-pass/
- https://randomnerdtutorials.com/getting-started-esp32-c3-super-mini (pinout)
