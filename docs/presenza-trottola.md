# Presenza della trottola

Il launcher ha un **pistoncino a molla** accanto alla finestrella del sensore: resta sotto il filo quando è vuoto, sale di circa 1 mm con la trottola agganciata e ricade al lancio. Un tasto premuto dal pistoncino dice al firmware se la trottola c'è.

Il tasto è **obbligatorio**: senza, il firmware scarta tutti i lanci. Serve a due cose:

- **scartare gli strappi a vuoto**, che altrimenti finirebbero nello storico come lanci;
- **dare alle app il bit vero** "trottola inserita" (0x04 nel pacchetto A0).

Stato: **firmware testato su PC e supporto meccanico verificato a CAD. Nulla è stato ancora stampato o provato su hardware.**

## Come lo fanno gli altri

| Progetto | Rilevazione | Cosa abbiamo preso |
| --- | --- | --- |
| [OpenRip](https://github.com/moon-lite/OpenRip) (MIT, hardware CERN-OHL-P) | microswitch a leva a bassa forza sul pistoncino; debounce 5 ms | l'idea del pistoncino e le quote |
| [Beyblade-Dragoon-RPM](https://github.com/Luciandraw/Beyblade-Dragoon-RPM) (firmware MIT) | tasto verso GND, direttamente sul pistoncino | **armamento "memorizzato"**: il rilascio durante il lancio non annulla la misura |
| [BRD](https://github.com/printfChins/Beyblade-RPM-Detector-Device) (GPL-3.0, solo consultato) | sensore IR "LOAD" su GPIO, debounce 50 ms | due comportamenti: serve un vero fronte libero → premuto per riarmare, e il bit 0x04 del pacchetto A0 segue il contatto |
| [bx-tacho](https://github.com/dazchi/bx-tacho) (GPL-3.0, solo consultato) | secondo fotoriflettore ITR8307 | scartato, vedi sotto |

Dai progetti GPL viene solo il comportamento, nessuna riga di codice.

**Perché un contatto meccanico e non un secondo sensore ottico:** il secondo fotoriflettore non ha parti che toccano il launcher, ma occupa un altro canale ADC nel pattern da cui dipendono i 40.000 campioni al secondo. In bx-tacho la sua luce disturba il sensore dei giri, tanto che il firmware deve accenderne il LED solo in momenti precisi. Il contatto occupa solo un GPIO digitale.

## Come funziona

1. Inserisci la trottola: il contatto si chiude e il dispositivo si **arma**.
2. Tiri: il rilevatore misura come sempre, il percorso a 10 kHz non è stato toccato.
3. La trottola parte e il contatto si apre: durante la misura **non disarma**.
4. Arriva il risultato: viene accettato e l'armamento si consuma.
5. Se tiri senza aver inserito la trottola, in console compare `SCARTATO,<rpm>,trottola non inserita` e il lancio non va nello storico.

Se togli la trottola a mano senza lanciare, il dispositivo si disarma.

Il LED fa il lampeggio breve di "pronto" solo da armato, e l'OLED mostra `ATTESA` finché la trottola non è inserita.

## Collegamento

| Tasto | SuperMini |
| --- | --- |
| un contatto | GPIO5 |
| l'altro contatto | GND |

Il pull-up è quello interno, non servono resistenze. Salda i due fili su due piedini che si collegano **solo a tasto premuto**: trovali col tester, perché i piedini sono collegati a coppie.

In console, `presenza` mostra se il contatto è premuto e se il dispositivo è armato.

**Prova senza launcher:** tocca GPIO5 con un filo collegato a GND e controlla con `presenza` che il contatto risulti premuto. Il comando `test` si arma da solo, quindi funziona anche senza tasto.

## Il supporto meccanico

Nella [scatola piatta](scatola-piatta.md) il tasto SMD 6×6 sta nel vassoio del sensore, con il cappuccio verso il basso, e appoggia **direttamente sul pistoncino**. Non servono perni e non c'è niente da forare. La quota del cappuccio è la stessa di Dragoon, ma la geometria è nostra (MIT): i modelli di Dragoon sono CC BY-NC-SA e da lì vengono solo idea e quote.

### Dov'è il pistoncino

Coordinate della base LaunchLab (origine al centro delle cerniere, sensore verso -Y). Tre fonti indipendenti concordano:

| Fonte | Dato | Pistoncino risultante |
| --- | --- | --- |
| Modello 3D di Dragoon (`LauncherV2.glb`) | pulsante centrato tra le clip, a **7,6 mm** dal QRE; foro da Ø4,2 nel supporto | (0; ~-0,3) |
| Base LaunchLab, sede del QRE | centro del QRE a (0,14; -7,87) | coerente con i 7,6 mm |
| Base LaunchLab, faccia inferiore | **tasca circolare da circa Ø5, profonda 0,4-0,5 mm**, centrata su (0; 0) | è lo sfogo per il pistoncino |

La tasca è misurata su una griglia da 0,5-1 mm, quindi il centro vale ±0,5 mm.

Il pistoncino c'è, nella stessa posizione, sia sul launcher a manovella sia su quello a corda (controllato a mano).

### Le scatole precedenti

Sulla base LaunchLab il pistoncino è coperto dalla base stessa: per arrivarci servirebbe forarla. La soluzione è stata tolta dal progetto, quindi quelle scatole non hanno il tasto e con il firmware attuale non registrano lanci.

### Da controllare sul launcher vero

1. **Contatto:** con la trottola inserita `presenza` deve mostrare il contatto premuto. Se non si chiude, tutti i lanci vengono scartati.
2. **Forza:** Dragoon preme un tasto normale direttamente, ma OpenRip raccomanda una leva sotto 0,5 N per non contrastare l'aggancio. Se la trottola si aggancia a fatica, serve un tasto a bassa forza (circa 100 gf).
3. **Corsa:** il pistoncino sale di circa 1 mm a occhio, come dice OpenRip, ma non è chiaro se sia la corsa totale o solo la parte sopra il filo.
4. **Diametro del pistoncino:** deve entrare nel foro del fondo, da Ø4,3 (Dragoon usa Ø4,2).
