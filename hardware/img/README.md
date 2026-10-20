# Immagini del montaggio

Render di `../enclosure/spinlite_views.scad` (componenti e fili) e di `../enclosure/spinlite_top.scad` (pezzi da stampare).

I componenti sono **blocchi semplificati** con le misure tipiche usate nei parametri. Anche i percorsi dei fili sono **indicativi**: mostrano dove passano, non la lunghezza esatta. La piedinatura della SuperMini è disegnata come promemoria; controlla sempre la serigrafia della tua scheda.

| File | Cosa mostra |
| --- | --- |
| `montato.png` | Dispositivo chiuso sulla base LaunchLab, appoggiato sul launcher (piano grigio indicativo): viti del coperchio, foro BOOT, feritoia dell'interruttore, spina del sensore sotto la scatola, clip libere |
| `interno.png`, `interno_lato.png` | Scatola senza coperchio con tutti i componenti della [BOM](../BOM.md): TP4056 e LiPo sotto, SuperMini sopra, interruttore, partitore sotto guaina, fili |
| `interno_oled.png` | Come sopra, con l'OLED 128x32 sotto il coperchio (versione `oled = true`) |
| `sezione.png`, `sezione_3d.png` | Sezione a metà: il sensore QRE1113 nella base guarda in basso nella finestrella; la spina Dupont sta sotto la scatola; i fili salgono dall'apertura nel fondo, passano sopra la LiPo e oltre la traversa fino alla SuperMini |
| `esploso.png` | Ordine di montaggio dal basso: base con sensore, 4 viti M2×6, scatola, TP4056 (spostato indietro perché entra scorrendo sotto la traversa), LiPo con partitore, interruttore, SuperMini, coperchio e viti |
| `stampa.png` | Scatola e coperchio come vanno sul piatto di stampa |

Colori dei fili: rosso +, nero GND, giallo uscita del sensore (GPIO3), azzurro batteria (GPIO4), verde e viola SDA/SCL dell'OLED.

## Rigenerarle

Serve la cartella `preview/` del pacchetto [top-dock-v098 di LaunchLab](https://github.com/shadohead/launchlab/raw/87bb60d619c2127c9efd679d7505a144e52a30d8/site/public/hardware/downloads/top-dock-v098.zip): contiene la base già in coordinate di montaggio. Senza quella cartella, al posto della base compare un segnaposto grigio.

```
openscad -o montato.png  -D 'view="montato"'  -D 'launchlab_preview="<cartella preview>"' --camera=0,2,10,62,0,35,150 --imgsize=1400,900 --colorscheme=Tomorrow spinlite_views.scad
openscad -o interno.png  -D 'view="interno"'  ... --camera=0,2,18,50,0,215,135
openscad -o sezione.png  -D 'view="sezione"'  ... --camera=0,2,12,90,0,90,118 --projection=o
openscad -o esploso.png  -D 'view="esploso"'  ... --camera=0,0,68,58,0,35,400 --imgsize=1100,1400
```

## Licenze

- Scatola, coperchio, componenti e fili disegnati da SpinLite: **MIT**.
- La **base sensore** (grigio chiaro, con le clip) è di **LaunchLab**, derivata dal *BP Gear Port Connector* di **Migbello** ([Thingiverse 6856080](https://www.thingiverse.com/thing:6856080)), licenza **CC BY-SA**. Le immagini in cui compare ne sono una rappresentazione: se le riusi, mantieni questa attribuzione e la stessa licenza per quella parte.
