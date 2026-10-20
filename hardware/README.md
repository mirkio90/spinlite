# Hardware: scatolina

`enclosure/spinlite_box.scad` è una scatolina parametrica in OpenSCAD (millimetri) per l'elettronica di SpinLite. Licenza **MIT**, come il resto del repo.

Contiene SuperMini, TP4056, LiPo e interruttore a slitta. Ha le aperture USB-C di SuperMini e TP4056, il foro per i cavi del sensore, una finestrella OLED opzionale e due linguette con foro/asola per fissarla al supporto del sensore (vite M2 o fascetta).

> Stato: il file non è ancora stato compilato né stampato. Controlla l'anteprima in OpenSCAD e fai una prova di stampa prima di chiudere l'elettronica.

## Come stampare

1. Apri il file in OpenSCAD. Scegli `part = "box"`, `"lid"` o `"both"` (in alto nel file).
2. `F6` per il render, poi esporta in STL.
3. Stampa **senza supporti**:
   - la scatola con l'apertura verso l'alto;
   - il coperchio con la piastra sul piatto (il bordo di incastro è rivolto in alto).
4. PLA o PETG, altezza layer 0,2 mm, 2-3 perimetri.

Se il coperchio è troppo stretto o lento, cambia `clearance` (parti da 0,2 mm e muoviti di 0,05).

## Parametri principali

Tutti sono in cima al file, con i commenti.

| Parametro | Cosa fa |
| --- | --- |
| `part` | `"box"`, `"lid"` o `"both"` |
| `wall`, `floor_t`, `lid_t`, `lip_t`, `lip_h` | spessori e bordo di incastro |
| `clearance` | gioco del coperchio |
| `sm_*`, `tp_*` | dimensioni e aperture USB-C di SuperMini e TP4056 |
| `lipo_l`, `lipo_w`, `lipo_h` | dimensioni LiPo (default 30x20x6, tipica da 300 mAh) |
| `sw_w`, `sw_h`, `sw_x` | foro dell'interruttore |
| `cable_w`, `cable_h`, `cable_x` | foro per i cavi del sensore |
| `oled`, `oled_win_w/h`, `oled_x/y` | finestrella OLED (`oled = true` la apre e alza la scatola) |
| `ear_*`, `screw_d`, `ear_slot` | linguette di fissaggio |

Misura i tuoi componenti con un calibro: LiPo e moduli TP4056 cambiano da un venditore all'altro.

La LiPo sta sul fondo, SuperMini e TP4056 stanno sopra, affiancate, con le USB-C sui lati corti opposti. Fissa i componenti con nastro biadesivo. L'OLED va incollato sotto la finestrella.

## Supporti sensore (non inclusi)

I supporti per il TCRT5000 sono del progetto LaunchLab. Non sono copiati qui: scaricali da lì.

- <https://github.com/shadohead/launchlab/tree/main/hardware/minimal_TCRT_sensor_only_v042>
- <https://github.com/shadohead/launchlab/tree/main/hardware/minimal_TCRT_split_keepers_v043>

Licenza **CC BY-SA**. Derivano dal *BP Gear Port Connector* di **Migbello**: se li modifichi o li ridistribuisci, cita l'autore e mantieni la stessa licenza.

Non usare il `v019`: usa il dock dell'M5 come fermo del PCB e da solo non va bene con il TCRT5000.

Questa scatolina è indipendente da quei file e resta MIT. Le linguette sono pensate per essere fissate al supporto con fascetta o viti M2: adatta posizione e misure al supporto che scegli.
