# SpinLite: distinta base (BOM)

Lista ufficiale dei componenti, quella da cui partire per ordinare. La guida la linka, non la ripete.

- Le tabelle descrivono la **versione attuale**: [scatola piatta](../docs/scatola-piatta.md) con sensore QRE1113 analogico, verificata solo a CAD e non ancora stampata. I montaggi precedenti sono [in fondo](#archivio-montaggi-precedenti).
- Prezzi indicativi di **ottobre 2026**, IVA inclusa, spedizione esclusa. Variano col venditore: controllali prima di ordinare.
- La colonna "Link" riporta un'offerta trovata e controllata. "da scegliere" significa che va bene qualsiasi venditore, purché rispetti la nota.

## Elettronica

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 1 | ESP32-C3 SuperMini | 1 | 22,5 × 18 mm, USB-C, pulsanti BOOT e RST, LED su GPIO8. Alcuni lotti hanno l'antenna debole: conta poco, perché il BLE lavora a 3 dBm su pochi centimetri. | [AliExpress](https://it.aliexpress.com/item/1005007479144456.html) | 3,01 € (2,71 € da 5 pezzi) |
| 2 | Sensore QRE1113 **analogico** su breakout | 1 | 3 pin **VCC / OUT / GND**, 2 resistenze, **nessun condensatore**. La versione "digitale" ha un condensatore di scarica e **non va bene**. Largo al massimo 9 mm sul lato corto: vedi i [controlli all'arrivo](#quando-arrivano-i-componenti-controlli). | [AliExpress](https://it.aliexpress.com/item/1005012222539797.html) | 7,49 € (7,09 € da 2 pezzi) |
| 3 | Batteria LiPo 1S | 1 | Formato **502030** (5 × 20 × 30 mm, circa 250 mAh), con cavetti. Altre misure si impostano con il parametro `lipo`. | da scegliere | 2-4 € |
| 4 | Modulo di ricarica TP4056 **con protezione**, formato **28 × 17 mm** | 1 | Modello classico USB-C: TP4056 + DW01 + doppio MOSFET, piazzole B+/B−/OUT+/OUT− sul lato opposto alla USB-C. Va bene anche il [modulo compatto](https://it.aliexpress.com/item/1005012104722935.html) (2,35 € per 3), ma è più corto: misuralo e reimposta `tp_l` e `tp_w`. Caricare **solo a 5 V**. | da scegliere | 0,5-1 € |
| 5 | Interruttore a slitta **SS12F15** (leva da 4 mm) | 1 | Flangia da 19,5 mm con due fori, corpo 10,5 × 6,1 × 5,3 mm. Si avvita da fuori al fianco con 2 viti M2 e dadi. Accorcia i piedini a circa 1,5 mm. | da scegliere | ~0,2 € |
| 6 | Resistenza 100 kΩ | 2 | Partitore per leggere la tensione della batteria. | da scegliere | 0,05 € |
| 7 | Condensatore ceramico 100 nF ("104") | 1 | Sul punto centrale del partitore, verso GND. A disco, reofori lunghi: si salda in linea. | [AliExpress](https://it.aliexpress.com/item/1005008483990919.html) (variante 100nF 104) | 1,79 € per 50 |
| 8 | OLED SSD1306 128x32 I2C da 0,91" | 1 | Consigliato: finestra e cornice sono già nel coperchio. 4 pin (GND, VCC, SCL, SDA), senza pettine: 4 fili saldati alle piazzole. Firmware `spinlite_qre_oled`. Chi non lo vuole imposta `oled = false`. | da scegliere | 1,5-2 € |
| 9 | Tasto **SMD 6×6, altezza 3,4 mm** | 2 | Uno per la presenza della trottola, premuto dal pistoncino ([dettagli](../docs/presenza-trottola.md)), uno come tasto esterno in parallelo al BOOT. **Obbligatori:** senza il primo il firmware scarta tutti i lanci. | da scegliere | ~0,5-1 € |
| 10 | Filo sottile (circa 28-30 AWG), stagno, guaina termorestringente | – | Niente pettini né connettori: tutti i collegamenti sono saldati. | da scegliere | 1 € |

## Stampa 3D e viteria

| # | Componente | Q.tà | Requisiti / note | Link | Prezzo |
| --- | --- | --- | --- | --- | --- |
| 11 | Filamento **PETG nero** | ~1 g | Vassoio del sensore: una plastica chiara e sottile fa passare la luce ambiente, e il sensore lavora a infrarossi. | da scegliere | – |
| 12 | Filamento **PETG chiaro** | ~20 g | Corpo, coperchio, clip e cappuccio. Chiaro per vedere i LED attraverso le zone sottili del coperchio. Il PLA va bene per lo spezzone di prova (circa 7 g). | da scegliere | ~0,5 € |
| 13 | Vite M2×6 a testa cilindrica | 12 | 10 chiudono il coperchio, 2 tengono l'interruttore. | da scegliere | – |
| 14 | Dado M2 | 2 | Dentro il corpo, sulle viti dell'interruttore. | da scegliere | – |

Un kit assortito di viti M2 (circa 3-5 €) basta per molti dispositivi. Il costo per pezzo è trascurabile. I pezzi da stampare sono in `enclosure/spinlite_flat.scad`, e non serve nient'altro di stampato.

## Materiali di consumo

| # | Componente | Note | Prezzo |
| --- | --- | --- | --- |
| 15 | Colla a caldo | Fissa QRE1113, tasti, vassoio e OLED. | 1-2 € se non l'hai |
| 16 | Gommapiuma da 0,5 mm o biadesivo in schiuma | Tra LiPo e fondo. | < 1 € |

## Attrezzi

- Saldatore a punta fine.
- Tester, per trovare i piedini dei tasti che si collegano solo a tasto premuto.
- Cacciavite adatto alle viti M2.
- Tronchesino, per accorciare i piedini dell'interruttore.
- Calibro, per i controlli all'arrivo.
- Cavo **USB-C dati** per caricare il firmware. Molti cavi servono solo a ricaricare.
- Stampante 3D.

## Totale indicativo

| Configurazione | Totale |
| --- | --- |
| Scatola piatta completa: OLED e 2 tasti | **circa 17-21 €** |
| Scatola piatta senza OLED | circa 15-19 € |

Esclusi attrezzi, kit di viti e rotoli interi di filamento.

## Quando arrivano i componenti: controlli

Cloni diversi possono avere misure diverse di qualche decimo di millimetro. Con il calibro, correggendo i parametri in `enclosure/spinlite_flat.scad`:

1. **QRE1113:** scheda (`qre_pcb`) e altezza del sensore (`qre_h`). Se è più larga di 9 mm, un `assert` lo segnala. Verifica che abbia 2 resistenze, nessun condensatore e i pin VCC, OUT, GND.
2. **Tasti:** altezza totale (`tact_h`) e corpo (`tact_body`).
3. **OLED:** scheda (`oled_pcb`), vetro (`oled_glass`) e posizione dell'area visibile (`oled_win_dy`), che spesso non è al centro.
4. **SuperMini e TP4056:** `sm_*` e `tp_*`, compresa la sporgenza della USB-C.
5. **Interruttore:** passo dei fori (`sf_pitch`) e corsa della leva (`sf_travel`), che sono stimati.

## Archivio: montaggi precedenti

Descritti in [hardware/README.md](README.md). Queste scatole **non hanno il tasto di presenza**, che sulla base LaunchLab richiederebbe di forare: il firmware attuale con loro non registra lanci. Rispetto alle tabelle sopra:

**Scatola avvitata sulla base LaunchLab** (`enclosure/spinlite_top.scad`). Questi pezzi sostituiscono le righe 11-14:

| Componente | Q.tà | Note | Link |
| --- | --- | --- | --- |
| Base sensore QRE1113 **LaunchLab v102** (stampata) | 1 | Clip e sede del sensore. Licenza **CC BY-SA**: non è nel repo. Istruzioni in [README.md](README.md#base-sensore-launchlab). ~10 g. | [3MF](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.3mf) · [ZIP con STL/STEP](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.zip) |
| Scatola SpinLite + coperchio (stampati) | 1 | [README.md](README.md#scatola-spinlite). ~12 g. | questo repo |
| Filamento PETG | ~25 g | | da scegliere |
| Vite M2×4 a testa cilindrica | 2 | Barretta che preme il QRE nella base. | da scegliere |
| Vite M2×6 a testa cilindrica | 10 | 4 fissano la scatola alla base, 4 chiudono il coperchio, 2 tengono l'interruttore. | da scegliere |
| Dado M2 | 2 | Sulle viti dell'interruttore. | da scegliere |
| Header maschio a 90° da 3 pin + cavetto Dupont femmina a 3 fili | 1 | Collegamento al sensore. | da scegliere |

Non servono i tasti SMD. Le misure della LiPo si impostano con i parametri `lipo_*`. In alternativa all'SS12F15 c'è l'[SS12D00](https://it.aliexpress.com/item/1005005633418066.html) (1,79 € per 20), con `switch_type = "SS12D00"`. Totale: circa 15-18 € senza OLED, 17-20 € con OLED.

**Controllo del QRE1113 con la base LaunchLab:** misura scheda e posizione di foro e sensore rispetto ai bordi, poi confrontale con la sede della base (le quote nominali sono nello STEP di LaunchLab).

**Modulo TCRT5000 + LM393 laterale.** Più economico (0,5-1 € al posto del QRE1113), ma alto circa 19 mm: non sta sotto la scatola e va usato con il supporto laterale [TCRT5000 v102 di LaunchLab](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/standard-sensor-v102.zip) e la scatola `enclosure/spinlite_box.scad`. Usa l'uscita **AO**. Totale senza OLED: circa 9-12 €.
