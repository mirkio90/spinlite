# SpinLite: distinta base (BOM)

Lista ufficiale dei componenti, quella da cui partire per ordinare. La guida la linka, non la ripete.

- Prezzi indicativi di **ottobre 2026**, IVA inclusa, spedizione esclusa. Variano col venditore: controllali prima di ordinare.
- La colonna "Link" riporta un'offerta trovata e controllata. "da scegliere" significa che va bene qualsiasi venditore, purché rispetti la nota.
- Configurazione di riferimento: **sensore QRE1113 analogico montato sopra il launcher**. Il sensore si aggancia con la base stampata di LaunchLab e la scatola SpinLite si avvita sopra.

## Elettronica

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 1 | ESP32-C3 SuperMini | 1 | Con USB-C. | da scegliere | 3-4 € |
| 2 | Sensore QRE1113 **analogico** su breakout | 1 | 3 pin **VCC / OUT / GND**, 2 resistenze, **nessun condensatore**. La versione "digitale" ha un condensatore di scarica e **non va bene**. Formato della SparkFun ROB-09453: la sede della base LaunchLab è fatta su quella, quindi misura il clone quando arriva (vedi sotto). | [AliExpress](https://it.aliexpress.com/item/1005012222539797.html) | 7,49 € (7,09 € da 2 pezzi) |
| 3 | Batteria LiPo 1S | 1 | Da 300-500 mAh, con cavetti o connettore. Le misure devono entrare nella scatola: vedi i parametri `lipo_*` in `enclosure/spinlite_box.scad`. | da scegliere | 2-4 € |
| 4 | Modulo di ricarica TP4056 **con protezione** | 1 | USB-C, 6 pin, con chip DW01 (protezione da scarica eccessiva). | da scegliere | 0,5 € |
| 5 | Interruttore a slitta | 1 | Piccolo, da saldare (tipo SS12D00). | da scegliere | 0,2 € |
| 6 | Resistenza 100 kΩ | 2 | Partitore per leggere la tensione della batteria. | da scegliere | 0,05 € |
| 7 | Condensatore ceramico 100 nF | 1 | Sul punto centrale del partitore, verso GND. | da scegliere | 0,05 € |
| 8 | Header maschio a 90° da 3 pin + cavetto Dupont femmina a 3 fili | 1 | Collegamento al sensore. LaunchLab usa connettori Dupont a 90° sul QRE. | da scegliere | 0,5 € |
| 9 | Filo sottile, stagno, guaina termorestringente | – | | da scegliere | 1 € |

## Stampa 3D e viteria

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 10 | Base sensore QRE1113 **LaunchLab v102** (stampata) | 1 | Clip che si agganciano alla porta del Battle Pass sul launcher, più la sede del sensore. Licenza **CC BY-SA**: non è nel repo, si scarica da LaunchLab. Istruzioni in [README.md](README.md#base-sensore-launchlab). | [3MF](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.3mf) · [ZIP con STL/STEP](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.zip) | ~10 g di filamento |
| 11 | Scatola SpinLite (stampata) | 1 | Si avvita sopra la base, contiene l'elettronica. **In riprogettazione:** l'attuale `enclosure/spinlite_box.scad` è la versione laterale, superata. | questo repo | ~10 g di filamento |
| 12 | Vite M2×4 a testa cilindrica (pan head) | 2 | Barretta che preme il QRE nella base. | da scegliere | – |
| 13 | Vite M2×6 a testa cilindrica | 4 | Fissa la scatola SpinLite ai 4 fori della base (rettangolo 29 × 24 mm). | da scegliere | – |
| 14 | Filamento **PETG** | ~20 g | Consigliato da LaunchLab per clip e parti avvitate. Il PLA va bene per le prove. | da scegliere | ~0,5 € |

Viti: un kit assortito M2 (circa 3-5 €) basta per molti dispositivi. Il costo per pezzo è trascurabile.

## Opzionali

| # | Componente | Q.tà | Note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 15 | OLED SSD1306 128x32 I2C | 1 | Firmware `spinlite_oled`. 4 pin: GND, VCC, SCL, SDA. | da scegliere | 1,5-2 € |
| 16 | Modulo TCRT5000 + LM393 (alternativa al QRE1113) | 1 | Più economico (0,5-1 €), ma alto circa 19 mm: non sta sotto la scatola e va usato con il supporto laterale [TCRT5000 v102 di LaunchLab](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/standard-sensor-v102.zip). Usa l'uscita **AO**. | da scegliere | 0,5-1 € |

## Totale indicativo

| Configurazione | Totale |
| --- | --- |
| QRE1113, senza OLED | **circa 15-18 €** |
| QRE1113, con OLED | circa 17-20 € |
| TCRT5000 laterale, senza OLED | circa 9-12 € |

## Quando arriva il QRE1113: controlli

Cloni diversi possono avere misure diverse di qualche decimo di millimetro. Con il calibro:

1. scheda: lunghezza × larghezza × spessore (da confrontare con la sede della base: le misure nominali sono nello STEP di LaunchLab);
2. posizione del foro di fissaggio rispetto ai bordi;
3. posizione del QRE1113 rispetto ai bordi (deve cadere sulla finestrella ottica della base);
4. scheda: 2 resistenze, nessun condensatore; pin VCC, OUT, GND.

Riporta le misure in un'issue o nel file della scatola: servono per verificare l'allineamento con la base LaunchLab.
