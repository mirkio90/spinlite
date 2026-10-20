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

File: **`enclosure/spinlite_top.scad`** (OpenSCAD, MIT). Si avvita sopra la base ai 4 fori pilota M2 (29 × 24 mm).

> Stato: verificata a CAD, **non ancora stampata**. Scatola e coperchio sono pezzi unici (anche con `oled = true`). Un controllo automatico (punti campionati sulle superfici) non trova sovrapposizioni con la base, le clip chiuse, le spine del sensore, la barretta e le viti di LaunchLab, né tra i componenti (rappresentati come blocchi) e scatola/coperchio. Lo stesso controllo, contro il dock M5 di LaunchLab, trova migliaia di collisioni: quindi funziona. Le misure dei componenti sono tipiche, non misurate: controllale col calibro.

### Ingombro e interfaccia

- 34,6 × 69,5 mm in pianta, alta circa 30 mm dal piano della base (con il coperchio).
- Resta nell'ingombro del dock M5 di LaunchLab (X ±17,3 mm), che loro hanno verificato a CAD contro il movimento delle clip. Esce solo di 4,5 mm verso il lato del connettore, perché i fili del sensore devono curvare verso l'alto. In quella zona le clip non arrivano.
- Il fondo sta 0,3 mm sopra i blocchi delle cerniere (Z 13,28). Quattro colonnine scendono sui rilievi dei fori (Z 11,2).

### Disposizione interna

```
  lato connettore sensore (-Y)                                  (+Y) pareti USB-C
  ┌──────────┬────────────────────────────┬───────────────────────────────┐
  │ fili     │ LiPo 502030                │ TP4056 (sotto)                │ ← USB-C ricarica
  │ sensore  │ (guide laterali)           │ SuperMini (sopra, su traversa │ ← USB-C SuperMini
  │ ↑ dal    │            interruttore →  │  e mensole)                   │
  │ fondo    │            (parete +X)     │                               │
  └──────────┴────────────────────────────┴───────────────────────────────┘
```

### Montaggio

1. Stampa scatola (`part = "box"`, aperta in alto) e coperchio (`part = "lid"`, già capovolto). PETG o PLA, layer 0,2 mm. Niente supporti: la traversa è un ponte e le mensole sono a 45°.
2. Avvita la **scatola vuota** alla base con **4 viti M2×6**. Le teste stanno nelle sedi del fondo e il cacciavite scende dritto lungo i canali.
3. **TP4056**: appoggialo sul fondo davanti alla traversa e fallo scorrere verso le pareti USB-C, sotto la traversa, finché la USB-C entra nel suo foro.
4. **LiPo** nel vano tra le guide, su un velo di gommapiuma o biadesivo.
5. Fai salire i fili del sensore dall'apertura nel fondo. Metti l'interruttore nella culla, con la leva nella feritoia.
6. Salda i collegamenti (vedi la guida). Lascia i fili abbastanza lunghi da poter alzare la SuperMini.
7. **SuperMini**: inserisci prima la USB-C nel foro superiore, poi abbassa la coda sulla traversa, dentro la battuta.
8. Chiudi il coperchio con **4 viti M2×6**. Una nervatura preme sulla USB-C e un limitatore resta sopra la coda.

Il pulsante BOOT, che cambia pagina sull'OLED, si preme dal foro nel coperchio con uno stuzzicadenti. Il LED della SuperMini resta dentro: con un filamento chiaro probabilmente si vede in trasparenza (non verificato).

### Parametri da controllare

| Parametro | Cosa |
| --- | --- |
| `sm_*`, `tp_*` | misure di SuperMini e TP4056, sporgenza della USB-C oltre il PCB (`*_usb_out`) |
| `lipo_l/w/h` | misure della LiPo (default 502030). Se è troppo lunga, un `assert` lo segnala |
| `sw_*` | interruttore SS12D00: corpo, feritoia, posizione |
| `sm_boot` | posizione del pulsante BOOT, per il foro nel coperchio |
| `oled` | `true` aggiunge 4 mm di altezza, la finestra nel coperchio e la cornice per il modulo |
| `fit`, `clearance` | giochi di schede e coperchio |
| `base_stl` | percorso dell'STL di anteprima di LaunchLab, solo per vederla in `part = "assembly"` |

Le quote d'interfaccia (`pilot_*`, `seat_z`, `hinge_*`, `plug_*`) sono misurate sulla base v102: non cambiarle senza rimisurare.

Con `part = "assembly"` vedi scatola, coperchio e componenti al loro posto. Con `base_stl` puoi aggiungere la base. Usa il file `preview/01_QRE_open_holder_body.stl` del pacchetto [top-dock-v098](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/top-dock-v098.zip), che è già nelle coordinate di montaggio.

## Scatola laterale (alternativa)

`enclosure/spinlite_box.scad` è la prima versione: una scatola da fissare a fianco con una fascetta. Resta nel repo per chi usa il TCRT5000 con il supporto laterale di LaunchLab, oppure monta SpinLite in un altro modo. Non è pensata per la base QRE.
