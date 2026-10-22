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
- I supporti 3D di LaunchLab sono **CC BY-SA**, perché derivano dal BP Gear Port Connector di Migbello (Thingiverse 6856080). Vanno **citati e linkati**, non copiati nel repo né fusi con i nostri modelli.
- **Decisione (20/10/2026):** sensore di riferimento **QRE1113 analogico** (breakout tipo SparkFun ROB-09453) nella **base QRE v102 di LaunchLab** (compact-base, linkata al commit `87bb60d`), stampata senza modifiche. La nostra scatola (MIT) si **avvita sopra** ai 4 fori pilota M2 su 29 x 24 mm, come il loro "Top Mount" dell'M5. Dalla base prendiamo solo quote d'interfaccia (fori, ingombri), non la geometria. Il TCRT5000 resta un'alternativa con il loro supporto laterale standard-sensor v102.
- Motivi: la porta del Battle Pass ha una finestrella a trapezio da 5-6 mm sopra un anello bianco/nero interno (1-2 mm sotto); il TCRT (lenti 7x6 mm, alto ~19 mm) è troppo grosso, il QRE lavora a ~1 mm. Fondere gli STL rovinerebbe le cerniere print-in-place (servono i "hinge support blocker" del loro 3MF).
- Il nome BLE di default è `BEYBLADE_TOOL01`, quello del dispositivo originale, perché alcune app lo cercano per nome. Il progetto non è affiliato a Takara Tomy o Hasbro.

## Struttura del repo

```
platformio.ini      pioarduino 55.03.312-1 (core 3.3.12), NimBLE-Arduino 2.5.1; env spinlite e spinlite_oled
firmware/spinlite/
  config.h          pin, default, flag di compilazione
  rpm_detector.*    rilevatore portabile, solo interi        (testato su PC)
  bbp_protocol.*    pacchetti Battle Pass + formato storico  (testato su PC)
  oled_ui.*         schermate 128x32 + driver SSD1306 I2C    (disegno testato; I2C non provato)
  adc_sampler.*     ADC1 continuo, task del rilevatore, calibrazione frequenza, batteria
  ble_bbp.*         servizio BLE Battle Pass (NimBLE), coda comandi, notifiche distanziate
  settings.*        soglie (blob con CRC) e storico in NVS   (parte portabile testata)
  battery.*         percentuale LiPo stimata                 (testato)
  console.*         comandi seriali
  spinlite.ino      collegamento dei moduli, LED, pulsante, OLED, salvataggi
test/               make -C test (g++ C++17, ASan/UBSan, -Werror): detector, protocol, oled, settings
docs/               GitHub Pages: dashboard Web Bluetooth (protocol.js + node docs/protocol.test.mjs),
                    flasher/ (esp-web-tools), guida.md, img/cablaggio.svg
hardware/           BOM.md (distinta base ufficiale), README.md (montaggio, base LaunchLab linkata),
                    enclosure/spinlite_top.scad (scatola avvitata sopra la base QRE: quella di riferimento),
                    enclosure/spinlite_box.scad (scatola laterale, alternativa per TCRT o altri montaggi)
                    enclosure/spinlite_views.scad (viste con tutti i componenti e i fili), img/ (render + README con licenze)
                    third_party/fetch_launchlab.py -> third_party/launchlab/ (file LaunchLab CC BY-SA, NEL .gitignore)
                    render/render.py + scene.py (render Blender; launcher BX-28 stilizzato dalle foto, non quotato)
.github/workflows/  build.yml (test + firmware + release su tag v*), pages.yml, _firmware.yml (riutilizzabile)
```

## Stato (20/10/2026, notte)

Fatto e verificato:
- firmware completo, compila senza warning in locale e in CI (RAM ~9%, flash ~49%);
- CI verde: 4 test C++ con sanitizer + test Node del parser web;
- sito online: https://mirkio90.github.io/spinlite/ (dashboard) e /flasher/ con i `.bin` (firmware.factory.bin di pioarduino, offset 0) pubblicati dalla CI;
- rilevatore sui segnali simulati: errore sotto lo 0,1% da 1.500 a 30.000 RPM, niente falsi lanci, riavvolgimento ignorato (dettagli nella storia git);
- pacchetti letti correttamente anche dal parser indipendente di MarkChan0225.

**Nulla è stato ancora provato su hardware vero.** Toolchain locale: PlatformIO in `%USERPROFILE%\.spinlite-pio\Scripts\pio.exe` (non nel PATH); g++, make e OpenSCAD assenti, i test C++ girano solo in CI.

## Prossimi passi, in ordine

1. **Prova senza sensore** (la SuperMini c'e' gia'): `docs/prova-senza-sensore.md`, comando `test`, poi handle BLE con nRF Connect. Poi **flashare e provare con il segnale vero** (`pio run -t upload`, poi `pio device monitor`). Controllare all'avvio la riga "ADC: frequenza misurata" (atteso ~10.000 Hz), poi `raw` girando l'albero a mano, regolare vite del TCRT e `minmark`, poi `fronti`.
2. **Verificare gli handle BLE con nRF Connect**: f001 valore = 15, f002 valore = 17, CCCD = 18. Dipende da quanti attributi NimBLE mette nei servizi GAP/GATT; se non tornano, valutare di aggiungere o togliere attributi prima del nostro service.
3. **Provare le app**: BeyGear, Spin Meter, Beyblader Helper, 321GoShoot, dashboard del progetto.
4. Provare l'OLED (driver I2C mai provato) e la lettura batteria col partitore.
5. **Stampa di prova** di `hardware/enclosure/spinlite_top.scad` sulla base QRE v102 di LaunchLab, con i componenti veri: misurare SuperMini, TP4056, LiPo e interruttore e correggere i parametri; verificare la presa delle viti M2x6 e che la presa sul launcher sia libera. Verificata solo a CAD (nessuna collisione con le parti LaunchLab di anteprima, metodo in "Note di verifica").
6. Prima release: tag `v0.1.0` → la CI pubblica i `.bin`.

Note di verifica della scatola (per rifarle dopo una modifica): gli STL di LaunchLab in coordinate di montaggio sono nella cartella `preview/` del pacchetto top-dock-v098 (base, clip chiuse, spine, barretta, viti). Si esportano `part="box"`, `"lid_mounted"` e `"check_tp|lipo|sm|sw"` (componenti ristretti di 0,1 mm) e si controlla con uno script Python che nessun punto campionato su una superficie cada dentro l'altro solido (raggio verticale, parita' degli attraversamenti). Controprova: contro `04_M5_dock_with_cable_shroud.stl` deve trovare migliaia di punti. OpenSCAD locale e' 2021.01 (niente backend manifold; CGAL rifiuta le mesh LaunchLab).

Note di review aperte (minori): `salva` da console scrive in flash anche durante una misura; `notifyLaunch` + un 0x74 dell'app possono mandare il dump due volte; le action GitHub sono fissate per versione maggiore, non per hash.

## Backlog (dopo la versione base: NON iniziare finché Mirko non lo chiede)

Decisione del 21/10/2026: prima si completa e si prova la versione basilare; queste idee restano qui.

1. **Inclinazione con IMU** (come le versioni su ESP32-S3; LaunchLab ha la funzione "level and tilt" ancora in sviluppo sull'M5StickS3).
   - Modulo I2C da ~1-3 € sullo stesso bus dell'OLED (GPIO6/7): meglio LSM6DS3 o BMI160 del MPU-6050 (fuori produzione, molti cloni).
   - Fusione accelerometro + giroscopio (Madgwick o filtro complementare) a 100-200 Hz in un task a bassa priorità: ~2-3% CPU anche in virgola mobile software. Il percorso a 10 kHz resta in interi.
   - Dati: inclinazione e rollio all'istante dello strappo, stabilità prima del lancio, "bolla" sull'OLED.
   - Rischi: il protocollo Battle Pass non ha un campo per l'inclinazione (le app non lo vedono: solo dashboard, console, OLED); il traffico I2C durante MISURA potrebbe sporcare il segnale analogico (provare con `raw`; in alternativa buffer continuo e valore preso all'inizio del lancio); l'IMU va fissata rigida e allineata (serve una sede nella scatola, ~16x21 mm per un GY-521).
   - Tappe: (a) IMU letta + prova di disturbo con `raw`; (b) inclinazione al lancio in CSV e OLED; (c) caratteristica BLE estesa + dashboard; (d) sede nella scatola.
2. **Caratteristica BLE estesa** per la nostra dashboard (curva completa, statistiche, inclinazione), senza toccare il profilo Battle Pass.
3. **WiFi** del C3: aggiornamento OTA e pagina servita dal dispositivo.
4. **Buzzer** su un GPIO (~0,3 €): bip a PRONTO e al risultato.
5. **Spegnimento automatico** per inattività.

Non adatti al C3 (servirebbe un S3): display a colori grandi e fluidi, elaborazioni pesanti.

## Progetto del firmware (implementato: riferimento delle scelte)

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
- `test [rpm] [s]`, `test stop` — lancio finto: onda quadra su GPIO10 (GPTimer a 1 MHz, interrupt che inverte il pin), da collegare con un filo a GPIO3 senza sensore. Guida: `docs/prova-senza-sensore.md`
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
| Sensore VCC | 3V3 della SuperMini |
| Sensore GND | GND |
| QRE1113 OUT (o TCRT5000 AO) | GPIO3 (sul TCRT il DO resta scollegato) |
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
