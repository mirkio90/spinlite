# SpinLite: distinta base (BOM)

Lista ufficiale dei componenti, quella da cui partire per ordinare. La guida la linka, non la ripete.

- Prezzi indicativi di **ottobre 2026**, IVA inclusa, spedizione esclusa. Variano col venditore: controllali prima di ordinare.
- La colonna "Link" riporta un'offerta trovata e controllata. "da scegliere" significa che va bene qualsiasi venditore, purché rispetti la nota.
- Configurazione di riferimento: **sensore QRE1113 analogico montato sopra il launcher**. Il sensore si aggancia con la base stampata di LaunchLab e la scatola SpinLite si avvita sopra.

## Elettronica

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 1 | ESP32-C3 SuperMini | 1 | 22,5 × 18 mm, USB-C, pulsanti BOOT e RST, LED su GPIO8; pettini inclusi. Alcuni lotti hanno l'antenna debole: conta poco, perché il BLE lavora a 3 dBm su pochi centimetri. | [AliExpress](https://it.aliexpress.com/item/1005007479144456.html) | 3,01 € (2,71 € da 5 pezzi) |
| 2 | Sensore QRE1113 **analogico** su breakout | 1 | 3 pin **VCC / OUT / GND**, 2 resistenze, **nessun condensatore**. La versione "digitale" ha un condensatore di scarica e **non va bene**. Formato della SparkFun ROB-09453: la sede della base LaunchLab è fatta su quella, quindi misura il clone quando arriva (vedi sotto). | [AliExpress](https://it.aliexpress.com/item/1005012222539797.html) | 7,49 € (7,09 € da 2 pezzi) |
| 3 | Batteria LiPo 1S | 1 | Formato **502030** (5 × 20 × 30 mm, circa 250 mAh), con cavetti. La scatola è dimensionata su questo; altre misure si impostano con i parametri `lipo_*` in `enclosure/spinlite_top.scad`. | da scegliere | 2-4 € |
| 4 | Modulo di ricarica TP4056 **con protezione**, formato **28 × 17 mm** | 1 | Modello classico USB-C: TP4056 + DW01 + doppio MOSFET, piazzole B+/B−/OUT+/OUT− sul lato opposto alla USB-C. **La scatola è dimensionata su questo** (`tp_l = 28`, `tp_w = 17.2`). Va bene anche il [modulo compatto](https://it.aliexpress.com/item/1005012104722935.html) (2,35 € per 3), ma è più corto: misuralo e reimposta `tp_l`/`tp_w`. Caricare **solo a 5 V** (il TP4056 regge al massimo ~8 V). | da scegliere | 0,5-1 € |
| 5 | Interruttore a slitta **SS12F15** (leva da 4 mm) | 1 | Flangia da 19,5 mm con due fori, corpo 10,5 × 6,1 × 5,3 mm: si avvita da fuori alla parete con 2 viti M2 e dadi, sopra la LiPo. Accorcia i piedini a ~1,5 mm. In alternativa l'SS12D00, più piccolo ([link](https://it.aliexpress.com/item/1005005633418066.html), 1,79 € per 20), con `switch_type = "SS12D00"`: va in una culla sul fondo. | da scegliere | ~0,2 € |
| 6 | Resistenza 100 kΩ | 2 | Partitore per leggere la tensione della batteria. | da scegliere | 0,05 € |
| 7 | Condensatore ceramico 100 nF ("104") | 1 | Sul punto centrale del partitore, verso GND. A disco, reofori lunghi: si salda in linea. | [AliExpress](https://it.aliexpress.com/item/1005008483990919.html) (variante 100nF 104) | 1,79 € per 50 |
| 8 | Header maschio a 90° da 3 pin + cavetto Dupont femmina a 3 fili | 1 | Collegamento al sensore. LaunchLab usa connettori Dupont a 90° sul QRE. | da scegliere | 0,5 € |
| 9 | Filo sottile, stagno, guaina termorestringente | – | | da scegliere | 1 € |

## Stampa 3D e viteria

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 10 | Base sensore QRE1113 **LaunchLab v102** (stampata) | 1 | Clip che si agganciano alla porta del Battle Pass sul launcher, più la sede del sensore. Licenza **CC BY-SA**: non è nel repo, si scarica da LaunchLab. Istruzioni in [README.md](README.md#base-sensore-launchlab). | [3MF](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.3mf) · [ZIP con STL/STEP](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.zip) | ~10 g di filamento |
| 11 | Scatola SpinLite + coperchio (stampati) | 1 | `enclosure/spinlite_top.scad`, si avvita sopra la base e contiene l'elettronica. Vedi [README.md](README.md#scatola-spinlite). | questo repo | ~12 g di filamento |
| 12 | Vite M2×4 a testa cilindrica (pan head) | 2 | Barretta che preme il QRE nella base. | da scegliere | – |
| 13 | Vite M2×6 a testa cilindrica | 10 | 4 fissano la scatola ai fori della base (rettangolo 29 × 24 mm), 4 chiudono il coperchio, 2 tengono l'interruttore SS12F15. | da scegliere | – |
| 13b | Dado M2 | 2 | Dentro la scatola, sulle viti dell'interruttore SS12F15. | da scegliere | – |
| 14 | Filamento **PETG** | ~25 g | Consigliato da LaunchLab per clip e parti avvitate. Il PLA va bene per le prove. | da scegliere | ~0,5 € |

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
