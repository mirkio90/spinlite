# Hardware: montaggio sul launcher

SpinLite si monta sulla **porta del Battle Pass** del launcher Beyblade X. Quella porta c'è sui launcher Takara Tomy, a corda e con ingranaggi interni. **Non** c'è sugli starter Hasbro europei.

Il montaggio è fatto di due pezzi stampati, uniti da 4 viti:

```
        ┌──────────────────────────┐
        │  SCATOLA SpinLite (MIT)  │  questo repo: SuperMini, TP4056, LiPo,
        │                          │  interruttore, USB-C, OLED opzionale
        └────┬────────────────┬────┘
   4 x M2x6 │                │  fori su un rettangolo 29 x 24 mm
        ┌────┴────────────────┴────┐
        │  BASE QRE1113 LaunchLab  │  da scaricare e stampare senza modifiche
        │  clip + sede del sensore │  (CC BY-SA, Migbello / LaunchLab)
        └──────────┬───────────────┘
           clip nella porta del Battle Pass; il QRE1113 guarda attraverso la
           finestrella del launcher l'anello bianco/nero interno (1-2 mm sotto)
```

Perché così:

- la base di LaunchLab ha già clip, finestrella ottica e sede del sensore progettate per quella porta, e i 4 fori sono previsti proprio per montarci sopra un dispositivo (loro ci mettono un M5StickS3);
- la scatola sta sopra il launcher, come il Battle Pass vero, quindi non ingombra la presa per il lancio;
- la base non viene copiata né modificata: SpinLite resta MIT, la base resta di LaunchLab con la sua licenza.

Componenti e link per ordinarli: **[BOM.md](BOM.md)**.

## Base sensore LaunchLab

**Versione: QRE1113 v102 ("compact base").** I link puntano a un commit preciso del repo LaunchLab, così i file non cambiano sotto i piedi:

- [progetto 3MF da aprire nello slicer](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.3mf) (consigliato);
- [ZIP con STL, STEP, README e licenza](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/compact-base-v102.zip);
- elenco delle versioni: [HARDWARE_VERSIONS.md](https://github.com/shadohead/launchlab/blob/87bb60d619c2127c9efd679d7505a144e52a30d8/docs/HARDWARE_VERSIONS.md); versioni più recenti su <https://launchlab.beywatch.gg/downloads>.

### Stampa

Le istruzioni sono riassunte dal README di LaunchLab: in caso di dubbio vale il loro.

1. Apri il **3MF come progetto**, non importare solo l'STL. Il 3MF contiene due oggetti "HINGE SUPPORT BLOCKER" che impediscono ai supporti di saldare le cerniere delle clip. Importando solo l'STL si perdono, e le clip escono incollate.
2. Scegli la tua stampante e il materiale, poi rifai lo slicing. Non usare il G-code già salvato (è per una Bambu P1S).
3. Materiale: **PETG** consigliato, perché le clip flettono e le parti ricevono viti. Il PLA va bene per una prima prova di misura.
4. Impostazioni di riferimento: ugello 0,4 mm, layer 0,2 mm, 4 perimetri, riempimento 25%, supporti ad albero solo dal piatto. Circa 10,5 g e 53 minuti.
5. Le clip si stampano aperte e già agganciate (print-in-place). Togli i supporti esterni a pezzo freddo, poi ruota le clip con delicatezza fino a chiuderle.

### Montaggio del sensore

1. Salda sul QRE1113 l'header a 90° (VCC, OUT, GND).
2. Metti la scheda nella sede della base, con il sensore verso la finestrella.
3. Fissala con la barretta (pressure bar) e **2 viti M2×4**.
4. Aggancia la base alla porta del Battle Pass e controlla che non abbia gioco.

### Adattamenti

- **Clone del QRE1113 con misure diverse.** La sede è fatta per la SparkFun ROB-09453. Se la tua scheda non entra, o il sensore non cade sulla finestrella, nello ZIP ci sono i file **STEP** da modificare in un CAD (FreeCAD, Fusion). Una base modificata resta **CC BY-SA**: va distribuita con la stessa licenza e citando Migbello e LaunchLab, e non va messa in questo repo MIT.
- **Gioco o ritenzione delle clip.** LaunchLab dichiara che incastro, ritenzione e fatica delle clip non sono provati fisicamente. Se la base balla, prova prima il PETG, poi segnalalo a LaunchLab.
- **Versioni più nuove della base.** Prima di usarle controlla nel loro README che ci siano ancora i **4 fori pilota M2 su 29 × 24 mm**: la scatola SpinLite si avvita lì.
- **TCRT5000 al posto del QRE1113.** Usa il [supporto TCRT5000 v102](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/standard-sensor-v102.zip). Il sensore è troppo alto per stare sotto la scatola: la scatola va montata a fianco, con un cavo. Configurazione non ancora studiata.

### Licenza della base

La base è **CC BY-SA** (versione non indicata dalla fonte), derivata dal *BP Gear Port Connector* di **Migbello** ([Thingiverse 6856080](https://www.thingiverse.com/thing:6856080)), con modifiche di LaunchLab. Per questo non è copiata qui: si linka. Se ridistribuisci la base o una sua modifica, cita Migbello e LaunchLab e mantieni la stessa licenza.

## Scatola SpinLite

> **In riprogettazione.** `enclosure/spinlite_box.scad` è la prima versione: una scatola da montare a fianco con le fascette, **superata**. La nuova versione si avvita sopra la base ai 4 fori M2 (29 × 24 mm), tiene ferme le schede con guide e sedi (non con il biadesivo) e lascia libere le clip.

Il file attuale compila in OpenSCAD e resta nel repo come riferimento finché la nuova versione non è pronta. Se vuoi guardarlo: apri il file, `F5` per l'anteprima, parametri in **Window → Customizer**.
