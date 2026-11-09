# Presenza della trottola (opzionale)

Il launcher ha un **pistoncino a molla** accanto alla finestrella del sensore: resta sotto il filo quando è vuoto, sale di circa 1 mm con la trottola agganciata e ricade al lancio. Un microswitch appoggiato sopra dice al firmware se la trottola c'è.

Serve a due cose:

- **scartare gli strappi a vuoto**, che altrimenti finiscono nello storico come lanci;
- **dare alle app il bit vero** "trottola inserita" (0x04 nel pacchetto A0), che senza microswitch è solo simulato.

Stato: **firmware pronto e testato su PC, mai provato su hardware. Supporto meccanico ancora da progettare** (vedi sotto).

## Come lo fanno gli altri

| Progetto | Rilevazione | Cosa abbiamo preso |
| --- | --- | --- |
| [OpenRip](https://github.com/moon-lite/OpenRip) (MIT, hardware CERN-OHL-P) | microswitch a leva a bassa forza sul pistoncino; debounce 5 ms | l'idea del pistoncino, le quote, la slitta regolabile per il supporto |
| [Beyblade-Dragoon-RPM](https://github.com/Luciandraw/Beyblade-Dragoon-RPM) (firmware MIT) | tasto verso GND | **armamento "memorizzato"**: il rilascio durante il lancio non annulla la misura |
| [BRD](https://github.com/printfChins/Beyblade-RPM-Detector-Device) (GPL-3.0, solo consultato) | sensore IR "LOAD" su GPIO, debounce 50 ms | due comportamenti: serve un vero fronte libero → premuto per riarmare, e il bit 0x04 del pacchetto A0 segue il contatto |
| [bx-tacho](https://github.com/dazchi/bx-tacho) (GPL-3.0, solo consultato) | secondo fotoriflettore ITR8307 | scartato, vedi sotto |

Dai progetti GPL viene solo il comportamento, nessuna riga di codice.

**Perché il microswitch e non un secondo sensore ottico:** il secondo fotoriflettore non ha parti che toccano il launcher, ma occupa un altro canale ADC nel pattern da cui dipendono i 40.000 campioni al secondo. In bx-tacho la sua luce disturba il sensore dei giri, tanto che il firmware deve accenderne il LED solo in momenti precisi. Il microswitch occupa solo un GPIO digitale.

## Come funziona

1. Inserisci la trottola: il contatto si chiude e il dispositivo si **arma**.
2. Tiri: il rilevatore misura come sempre, il percorso a 10 kHz non è stato toccato.
3. La trottola parte e il contatto si apre: durante la misura **non disarma**.
4. Arriva il risultato: viene accettato e l'armamento si consuma.
5. Se tiri senza aver inserito la trottola, in console compare `SCARTATO,<rpm>,trottola non inserita` e il lancio non va nello storico.

Se togli la trottola a mano senza lanciare, il dispositivo si disarma.

Con la presenza attiva il LED fa il lampeggio breve di "pronto" solo da armato, e l'OLED mostra `ATTESA` finché la trottola non è inserita.

## Collegamento e attivazione

| Microswitch | SuperMini |
| --- | --- |
| COM | GPIO5 |
| NO (normalmente aperto) | GND |

Serve solo un filo per contatto: il pull-up è quello interno. Poi, da console:

```
presenza on      attiva e salva
presenza         mostra lo stato: contatto premuto/libero, armato si/no
presenza off     torna al funzionamento senza microswitch (default)
```

Per provarlo senza launcher: `presenza on`, poi tocca GPIO5 con un filo collegato a GND e controlla con `presenza` che il contatto risulti premuto. Il comando `test` si arma da solo, quindi funziona anche con la presenza attiva.

## Cosa manca: il supporto meccanico

Non si può disegnare alla cieca. Con il launcher in mano va controllato:

1. **Dov'è il pistoncino** rispetto alla finestrella (distanza e direzione), e quanto sale. OpenRip misura circa 1 mm, sul launcher a manovella.
2. **Se c'è anche sul launcher a corda**: OpenRip l'ha verificato solo sul modello a manovella.
3. **Se la base LaunchLab lo copre.** Non ho verificato la posizione del pistoncino rispetto alla base. Se la copre serve un foro: la base modificata resterebbe un derivato CC BY-SA, da tenere separata dalla scatola MIT.

Indicazioni di OpenRip per il supporto:

- microswitch a leva **a bassa forza** (sotto 0,5 N, classe Omron SS-01GL): la leva non deve impedire alla trottola di agganciarsi;
- montaggio su **slitta regolabile** (asola + vite M2), perché 1 mm di corsa è dentro le tolleranze di stampa. Si regola una volta sola: con la trottola inserita il contatto deve scattare, senza deve rilasciarsi.
