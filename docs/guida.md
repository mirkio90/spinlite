# SpinLite: guida completa

SpinLite è un conta giri economico per launcher Beyblade X. Legge il segno ottico sull'albero del launcher con un TCRT5000, calcola gli RPM con un ESP32-C3 e li manda al telefono via Bluetooth, con lo stesso profilo del Bey Battle Pass BX-09. Le app esistenti funzionano senza modifiche.

Leggi prima la sezione [Limiti](#limiti): cosa misura SpinLite non è esattamente ciò che misura la trottola.

## 1. Distinta base

Prezzi indicativi da AliExpress, ottobre 2026. Variano col venditore e con le spedizioni.

| Componente | Note | Prezzo indicativo |
| --- | --- | --- |
| ESP32-C3 SuperMini | la scheda principale | 3-4 € |
| Modulo TCRT5000 con LM393 | serve l'uscita analogica **AO** | 0,5-1 € |
| LiPo 1S 300-500 mAh | piccola, con connettore o cavetti | 2-4 € |
| Modulo TP4056 con protezione | tipo USB-C, 6 pin, con DW01 | 0,5 € |
| Interruttore a slitta | piccolo, da saldare | 0,2 € |
| 2 resistenze da 100 kΩ | partitore batteria | 0,05 € |
| Condensatore 100 nF | sul partitore | 0,05 € |
| Cavetti, stagno | | 1 € |
| *Opzionale:* OLED SSD1306 128x32 I2C | display sul launcher | 1,5-2 € |

**Totale stimato: circa 8-11 € senza OLED, 10-13 € con OLED.** Un Bey Battle Pass originale costa molto di più.

## 2. Cablaggio

![Schema di cablaggio](img/cablaggio.svg)

| Da | A |
| --- | --- |
| TCRT5000 VCC | 3V3 della SuperMini |
| TCRT5000 GND | GND |
| TCRT5000 AO | GPIO3 (DO non collegato) |
| LiPo + (dopo l'interruttore) | partitore 100k/100k → GPIO4, **con 100 nF verso GND** sul punto centrale |
| Uscita del modulo di ricarica (TP4056 con protezione) | pin 5V della SuperMini, tramite l'interruttore |
| OLED SDA / SCL (opzionale) | GPIO6 / GPIO7, alimentazione 3V3 |

Note:

- Il condensatore da 100 nF è necessario. L'ADC campiona di continuo e, senza, la sorgente da 50 kΩ falsa la lettura della batteria.
- Collega l'uscita (OUT+ / OUT-) del TP4056, non i pin B+/B- della batteria, al resto del circuito. La LiPo va su B+/B-.
- Il sensore deve stare su GPIO3 e la batteria su GPIO4: sul C3 l'ADC in modalità continua funziona solo su ADC1 (GPIO0-4).
- Senza il partitore il firmware funziona lo stesso, ma non conosce la batteria.

## 3. Montaggio del sensore

### Supporto stampato

Il progetto LaunchLab ha supporti 3D per il TCRT5000. Non sono copiati in questo repo: scaricali da lì.

- [`minimal_TCRT_sensor_only_v042`](https://github.com/shadohead/launchlab/tree/main/hardware/minimal_TCRT_sensor_only_v042)
- [`minimal_TCRT_split_keepers_v043`](https://github.com/shadohead/launchlab/tree/main/hardware/minimal_TCRT_split_keepers_v043)

Licenza **CC BY-SA**. Derivano dal *BP Gear Port Connector* di Migbello: se li ridistribuisci, cita l'autore e mantieni la stessa licenza.

**Attenzione al v019**: usa il dock dell'M5 come fermo del PCB, quindi da solo non va bene con questo sensore.

La scatolina per l'elettronica di SpinLite è in [`hardware/`](../hardware/README.md).

### Segno ottico sull'albero

Il sensore vede il contrasto tra una zona chiara e una scura.

- Usa un pezzetto di **nastro nero opaco** su un fondo chiaro, oppure nastro bianco su fondo scuro.
- Di default il firmware si aspetta **1 segno per giro** (`marks` = 1).
- Il segno deve passare davanti al sensore a pochi millimetri di distanza, sempre alla stessa distanza.
- Evita nastro lucido: i riflessi creano falsi fronti.

## 4. Installazione del firmware

### Web flasher (il più semplice)

Apri il [web flasher](../flasher/) con Chrome o Edge, collega la SuperMini via USB e segui le istruzioni. Adesso: <https://mirkio90.github.io/spinlite/flasher/>.

### PlatformIO

```
pio run -t upload                  # versione base, solo Bluetooth
pio run -e spinlite_oled -t upload # con display OLED
pio device monitor                 # console seriale, 115200
```

### Arduino IDE

- Scheda: **ESP32C3 Dev Module**, core `esp32` **3.x**.
- Attiva **USB CDC On Boot**.
- Installa la libreria **NimBLE-Arduino 2.x**.
- Apri `firmware/spinlite/spinlite.ino`. Per l'OLED definisci `SPINLITE_OLED=1`.

Se la scheda non viene riconosciuta, tieni premuto BOOT mentre colleghi il cavo USB.

## 5. Taratura con la console seriale

Collegati a 115200 baud (`pio device monitor`, o un qualsiasi terminale seriale).

| Comando | Cosa fa |
| --- | --- |
| `aiuto` | elenco dei comandi |
| `stato` | fase, livello, rumore, ampiezza, isteresi, frequenza misurata, batteria, BLE |
| `raw [s]` | stampa livello, rumore, ampiezza, isteresi, fase e RPM a 20 Hz per `s` secondi (default 10); un tasto qualsiasi lo ferma. Serve per tarare |
| `fronti` | accende/spegne la stampa di ogni fronte: `FRONTE,tempo_ms,periodo_us,rpm,ampiezza,valido/scartato` |
| `set <parametro> <valore>` | cambia un parametro |
| `salva` | salva i parametri in memoria (NVS) |
| `default` | torna ai valori di fabbrica e cancella quelli salvati |
| `storico` | stampa lo storico dei lanci |
| `cancella` | azzera lo storico |

### Parametri di `set`

| Parametro | Default | A cosa serve |
| --- | --- | --- |
| `marks` | 1 | segni ottici per giro dell'albero |
| `minmark` | 150 | ampiezza minima di un segno, in conteggi ADC. Più bassa = più sensibile, ma più falsi segni |
| `minrpm` | 1000 | sotto questa velocità non parte un lancio |
| `maxrpm` | 40000 | periodi più brevi di questo limite sono scartati come disturbo |
| `endpct` | 30 | il lancio finisce quando la velocità scende a questa percentuale del picco |
| `endgap` | 250 | il lancio finisce se per questi millisecondi non arrivano segni |
| `quiet` | 1000 | ms senza rotazioni prima del riarmo (copre il riavvolgimento della corda) |
| `single` | 0 | 0 = il picco è calcolato su 3 giri, 1 = su un giro singolo |

Esempio: `set minmark 200` poi `salva`.

### Procedura

1. Scrivi `raw`. Vedi livello, rumore e ampiezza.
2. Gira l'albero **a mano**, piano. Il segno deve far salire l'ampiezza.
3. Regola la **vite del TCRT** (il trimmer del LM393 non conta: noi leggiamo AO) e la **distanza** dall'albero. L'obiettivo è un'ampiezza dei segni **ben sopra `minmark`** e un rumore basso a riposo.
4. Scrivi `fronti` e gira ancora l'albero. Devi vedere un fronte valido per ogni segno, nessun fronte valido a riposo. Scrivi di nuovo `fronti` per spegnere la stampa.
5. Fai un lancio vero. Se va, `salva`.

Se `stato` segnala il sensore scollegato, controlla cavi e alimentazione del modulo.

### Riga per ogni lancio

Dopo ogni lancio la console stampa una riga CSV:

```
LANCIO,rpm,rpm3giri,giro_singolo,giri,durata_ms,segno_min,fine
```

- `rpm`: il valore di picco usato (secondo `single`).
- `rpm3giri`: picco su 3 giri.
- `giro_singolo`: picco su un giro.
- `giri`: giri contati.
- `durata_ms`: durata del lancio.
- `segno_min`: ampiezza minima di segno incontrata (indica se sei vicino alla soglia).
- `fine`: motivo della fine del lancio.

## 6. LED e pulsante

LED blu della scheda (GPIO8):

- lampeggio breve ogni 2 s: pronto;
- acceso fisso: misura in corso;
- doppio lampeggio: è arrivato un risultato;
- lampeggio rapido: guasto del sensore.

Pulsante **BOOT** (GPIO9), dopo l'avvio: ogni pressione cambia pagina dell'OLED (ultimo → massimo → info).

Durante la misura OLED, Bluetooth e memoria restano fermi, per non disturbare il segnale analogico. Tutto avviene dopo il risultato.

## 7. App compatibili

SpinLite si annuncia come il Battle Pass originale e funziona con:

- [BeyGear](https://beygear.app/bey-battle-pass/)
- Spin Meter
- Beyblader Helper
- 321GoShoot
- la [dashboard web di SpinLite](../) (<https://mirkio90.github.io/spinlite/>)
- la [dashboard di MarkChan0225](https://github.com/MarkChan0225/beyblade-tachometer-web)

Il nome Bluetooth è `BEYBLADE_TOOL01`, quello del dispositivo originale, perché alcune app lo cercano per nome. Se hai anche un Battle Pass vero e vuoi distinguerli, cambia `SPINLITE_BLE_NAME`, per esempio con `-DSPINLITE_BLE_NAME=\"MIO_SPINLITE\"` in `build_flags` di `platformio.ini`. Poi alcune app potrebbero non trovarlo.

Alla connessione SpinLite manda subito lo stato, poi stato e storico completo dopo ogni lancio.

## Limiti

- **Misuriamo l'albero del launcher, non la trottola.** Gli RPM sono quelli del launcher, non quelli che la trottola avrà dopo lo stacco.
- Anche il Battle Pass originale, con i launcher a corda, **sovrastima** gli RPM reali: la verifica indipendente di SpiningDays lo mostra, per via del rimbalzo del riavvolgimento. Vedi <https://note.com/spiningdays/n/nb40b4beb37d2>. Non confrontare i numeri di SpinLite con quelli di altri strumenti come se fossero la stessa grandezza.
- La **percentuale della batteria è una stima**: viene da una tabella approssimata tensione-carica per una LiPo, interpolata.
- **Nulla di questo è ancora provato su hardware vero** (stato del progetto: vedi README).

Cose del C3 che ti possono confondere:

- A riposo il segnale AO può stare **saturo a circa 4095**. È normale: l'ADC a 12 dB arriva a circa 2,5 V e il TCRT a riposo può stare sopra. È il segno scuro che abbassa la tensione.
- Alimentando dal pin 5V con la LiPo (3,7-4,2 V) si passa dall'LDO della scheda. Funziona, ma sotto circa **3,5 V** la tensione a 3V3 cala e le misure possono peggiorare. Da verificare sulla tua scheda.
- Non tenere a massa GPIO2, GPIO8 o GPIO9 all'accensione (strapping).
- Sotto 2,5 V sul pin batteria il firmware assume "interruttore spento, alimentato da USB" e non mostra la percentuale.

## Crediti e licenze

- SpinLite è **MIT**.
- Ispirato a [LaunchLab](https://github.com/shadohead/launchlab) (shadohead), che usa soglie adattive sul segnale analogico. Usato solo come fonte di idee.
- [BRD](https://github.com/printfChins/Beyblade-RPM-Detector-Device) di printfChins è **GPL-3.0**: consultato per confrontare i valori del protocollo, nessun codice copiato.
- Il protocollo Battle Pass è stato ricostruito dalla documentazione di [MarkChan0225](https://github.com/MarkChan0225/beyblade-tachometer-web) e dallo [script Python di danielbyomujuni](https://github.com/danielbyomujuni/beyblade-x-beybattlepass-python).
- I supporti sensore di LaunchLab sono CC BY-SA (derivati dal lavoro di Migbello), linkati e non copiati.
- Progetto non affiliato a Takara Tomy né a Hasbro.
