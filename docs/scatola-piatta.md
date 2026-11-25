# Scatola piatta (corpo unico sul launcher)

La versione attuale del dispositivo: un corpo basso che si aggancia direttamente al launcher, senza la base LaunchLab. Corpo, vassoio del sensore, coperchio, 2 clip e cappuccio del tasto si stampano a parte. File: `hardware/enclosure/spinlite_flat.scad`.

**Schermo:** l'OLED 128x32 sta sotto il coperchio, sopra il vassoio, senza alzare il corpo. Firmware `spinlite_qre_oled`. Chi non lo vuole imposta `oled = false` e il coperchio resta chiuso.

**Tasto di presenza:** obbligatorio. Sta nel vassoio e appoggia direttamente sul pistoncino del launcher: niente perni e niente fori da fare. Senza tasto il firmware scarta tutti i lanci. Dettagli in [presenza della trottola](presenza-trottola.md).

![Vista d'assieme senza coperchio](../hardware/img/piatta_assieme.png)

**Stato: verificata solo a CAD. Nulla è stato stampato né provato sul launcher.** Le scatole precedenti restano in [hardware/README.md](../hardware/README.md) come archivio: non hanno il tasto di presenza, che lì richiederebbe di forare la base. Cosa comprare: [distinta base](../hardware/BOM.md).

## Cosa cambia

| | Scatola su base LaunchLab | Scatola piatta |
| --- | --- | --- |
| Pezzi | base LaunchLab + barretta + scatola + coperchio | corpo + vassoio del sensore + coperchio + 2 clip + cappuccio del tasto |
| Tasto di presenza | non previsto: servirebbe forare la base | direttamente sul pistoncino |
| Altezza sul launcher | circa 28 mm | circa 10 mm |
| Lunghezza | circa 70 mm | circa 119 mm |
| Licenze | base CC BY-SA a parte | tutto MIT |

## Da dove vengono le quote

La disposizione segue il progetto [Beyblade-Dragoon-RPM](https://github.com/Luciandraw/Beyblade-Dragoon-RPM) di Luciandraw, un dispositivo costruito davvero. I suoi modelli sono CC BY-NC-SA e non sono nel repo: la geometria è disegnata da zero, e da lì vengono solo idee e posizioni funzionali. Grazie a Luciandraw.

Quote riprese:

- altezza della faccia inferiore e della fascia centrale che entra nell'incavo del launcher;
- posizione del QRE1113 (Y 5,44) e del pistoncino (Y 13,05), e quota del cappuccio del tasto (Z 5,40);
- posizione del perno delle clip e dei loro denti;
- larghezza: il corpo non è più largo della sagoma di Dragoon. In lunghezza invece è spostato in avanti di circa 10 mm, così dietro resta a filo del launcher come il Battle Pass originale.

Nel suo modello le due clip non sono simmetriche di circa 0,7 mm: ho usato la destra, specchiata. Perno e dente coincidono con quelli delle clip di LaunchLab, un secondo progetto indipendente.

## Disposizione

Da dietro in avanti:

1. **Vano posteriore**, vuoto: il retro è a filo del launcher e lì non c'è spazio per le schede.
2. **QRE1113 e tasto** nel fondo, con le **clip** ai lati.
3. **SuperMini** di traverso, con la USB-C nel fianco -X; **interruttore SS12F15** accanto, sul fianco +X.
4. **TP4056** di traverso, con la USB-C sul fianco +X.
5. **LiPo 502030**.

Sopra il vano posteriore e il vassoio, sotto il coperchio, c'è l'**OLED** per il lungo, con i pin verso il retro. Accanto c'è il **tasto esterno**.

## Interfaccia

- **OLED:** mostra ultimo lancio, massimo e informazioni. Sopra il vassoio ci sono circa 5 mm liberi e il modulo ne occupa meno di 3, per questo non serve alzare il corpo.
- **Tasto esterno:** va in parallelo al BOOT (GPIO9), quindi il firmware non cambia. Una pressione cambia pagina dell'OLED. Tenuto premuto mentre accendi, mette la scheda in modalità di programmazione: serve se un firmware sbagliato blocca la USB.
- **LED:** con la scatola chiusa si vedono attraverso due zone sottili del coperchio, spesse 0,6 mm, ma solo se il coperchio è stampato in un filamento chiaro. Le zone sono larghe perché la posizione dei LED cambia da scheda a scheda.

## Perno chiuso

Nel primo spezzone le clip si sfilavano: il perno appoggiava sul blocco della cerniera, l'aletta del coperchio gli stava sopra con 1,5 mm d'aria, e verso l'esterno non lo fermava niente. Staccando SpinLite dal launcher, la clip scivolava via.

L'aletta del coperchio è più lunga del blocco della cerniera, e ora sotto di essa ci sono:

- un **appoggio** che scende fin sopra il perno;
- un **tacco** che scende oltre il perno, fin sotto il suo centro.

A coperchio avvitato il perno è chiuso su quattro lati: blocco sotto, appoggio sopra, tacco fuori, parete del corpo dentro, con 0,2 mm d'aria. La clip ruota normalmente, ma non si sfila e non balla. L'aria si regola con `cap_gap`; `hinge_cap = false` toglie tacco e appoggio.

## Verso di montaggio

Sul launcher la finestrella del disco è più vicina alla corda, il foro del pistoncino più lontano. Il corpo si monta quindi con la **parte corta e la USB-C della SuperMini verso la corda** e la **parte lunga con la LiPo verso il fondo** del launcher.

| Dal pistoncino verso | Launcher | Corpo | Sporgenza |
| --- | --- | --- | --- |
| Corda | 30 mm (misurato) | 30 mm | a filo |
| Fondo | circa 62 mm (stima dalle foto) | 89 mm | circa 27 mm |

Il lato corda è misurato col metro sul launcher vero ed è in `rear_len`: il retro del corpo segue da solo. Il lato fondo è stimato da una foto, e dato che la stima del lato corda era sbagliata di 6 mm, anche questa può esserlo: la sporgenza vera va vista sul pezzo.

![Launcher visto dal lato della corda, con finestrella e pistoncino nella fascia nera](../hardware/img/render_launcher.png)

Nei render il launcher è ridisegnato dalle foto di uno vero, con la corda dal lato giusto. Forma e proporzioni sono quelle delle foto, ma le misure hanno un errore di qualche mm: non usarlo come riferimento per modificare il corpo.

## Prima prova: lo spezzone

Non stampare subito tutto il corpo. `part = "prova"` mette sul piatto solo la zona centrale di corpo e coperchio, con vassoio, clip e cappuccio: circa 7 g di filamento invece di 20. Lo spezzone si chiude con 4 viti. Ti dice le cose che il CAD non può verificare:

1. le clip si agganciano al launcher e tengono senza gioco;
2. il QRE1113 è sopra la finestrella del disco;
3. il tasto si chiude con la trottola inserita, senza impedire l'aggancio.

Se qualcosa non va, si corregge e si ristampa lo spezzone.

## Stampa

`part = "plate"` mette tutti i pezzi sul piatto, oppure singolarmente `"body"`, `"lid"`, `"tray"`, `"clips"` e `"button"`. In tutto servono circa 20 g di filamento.

- PETG, layer 0,2 mm.
- **Vassoio in filamento nero:** il sensore lavora a infrarossi e una plastica chiara e sottile può far passare luce ambiente.
- **Coperchio in filamento chiaro** se vuoi vedere i LED attraverso le zone sottili.
- Corpo, vassoio e coperchio senza supporti: ognuno appoggia su una faccia piana. Il vassoio è un pezzo a parte proprio per questo: la sua fascia sporge 1 mm sotto il fondo, e se fosse unita al corpo il resto del fondo resterebbe sospeso.
- Le clip si stampano coricate su un braccio, così gli strati seguono il verso in cui lavorano.
- Se la tua stampante è piccola, stampa i pezzi uno alla volta: il corpo è lungo 119 mm.

## Montaggio

Nelle immagini i pezzi già montati sono al loro posto, quello nuovo è staccato e la freccia magenta indica dove entra. I componenti sono blocchi semplificati, con le misure dei parametri: i tuoi possono essere un po' diversi. Le immagini si rigenerano con `python hardware/render/render_montaggio.py`.

### 0. Sedi in ottone nel corpo

Prima di montare i componenti, inserisci le 9 sedi M2 nelle colonnine del corpo:

1. appoggia la sede sul foro, con il lato più stretto in basso: l'invito la tiene centrata;
2. scaldala con il saldatore, a circa 220 °C per il PLA e 240 °C per il PETG, e spingi piano senza forzare;
3. fermati quando è a filo della cima della colonnina, togli il saldatore e premi un attimo con una superficie piatta e fredda, così resta dritta.

Sotto ogni sede c'è un foro più stretto, che accoglie la plastica spostata e la punta della vite.

### 1. Sensore e tasto di presenza nel vassoio

![Passo 1](../hardware/img/montaggio/passo_1.png)

- **QRE1113:** salda i tre fili direttamente al breakout, senza pettine. Sensore verso il basso nella finestra, PCB sugli appoggi tra le sponde, una goccia di colla a caldo sui bordi.
- **Tasto SMD 6×6:** cappuccio nel foro verso il basso, tra le sponde. Salda due fili su due piedini che si collegano solo a tasto premuto (trovali col tester). Una goccia di colla.

### 2. Vassoio nel corpo, da sotto

![Passo 2](../hardware/img/montaggio/passo_2.png)

Il vassoio entra dal fondo del corpo: la fascia resta fuori e appoggia al fondo. Fissalo con qualche goccia di colla lungo il bordo; la spinta del pistoncino lo tiene comunque premuto contro il corpo.

### 3. SuperMini

![Passo 3](../hardware/img/montaggio/passo_3.png)

Di traverso, con la USB-C nel foro del fianco -X, scheda sui binari tra le battute.

### 4. Interruttore

![Passo 4](../hardware/img/montaggio/passo_4.png)

Sul fianco +X accanto alla SuperMini, con la leva nella feritoia. Si fissa con 2 viti M2×6 da fuori e i dadi dentro.

### 5. TP4056 e LiPo

![Passo 5](../hardware/img/montaggio/passo_5.png)

Il TP4056 va tra le guide, con la USB-C nel foro del fianco +X. La LiPo nel vano davanti, su un velo di gommapiuma.

### 6. Tasto esterno

![Passo 6](../hardware/img/montaggio/passo_6.png)

Sulla torretta accanto al vassoio, cappuccio verso l'alto e piedini lungo il corpo, con una goccia di colla sotto. Due fili su due piedini che si collegano solo a tasto premuto.

### 7. Collegamenti

![Schema di cablaggio](img/cablaggio.svg)

Salda i collegamenti come nello schema: tasto di presenza tra GPIO5 e GND, tasto esterno tra GPIO9 e GND. Lascia i fili abbastanza lunghi da poter aprire il coperchio, perché l'OLED è attaccato a lui.

### 8. OLED e cappuccio nel coperchio

![Passo 7](../hardware/img/montaggio/passo_7.png)

Nell'immagine il coperchio è capovolto.

- **OLED:** salda quattro fili direttamente alle piazzole, senza pettine, con i pin verso il retro. Fissalo nella cornice con il vetro contro la finestra e una goccia di colla sui bordi. SDA su GPIO6, SCL su GPIO7, alimentazione a 3V3.
- **Cappuccio del tasto esterno:** nel suo foro, con la flangia verso l'interno.

### 9. Clip

![Passo 8](../hardware/img/montaggio/passo_8.png)

Appoggia ognuna sul blocco della cerniera, con il dente verso il launcher.

### 10. Coperchio

![Passo 9](../hardware/img/montaggio/passo_9.png)

Chiudi con **9 viti M2×6**, che si avvitano nelle sedi in ottone. Tacco e appoggio sotto le alette chiudono il perno delle clip nella sede.

## Da controllare al primo pezzo stampato

1. **Clip sul launcher.** Nel primo spezzone si sfilavano dalla cerniera. Con il coperchio avvitato non devono uscire né ballare, e devono comunque agganciarsi e sganciarsi dal launcher. Dalle foto le sedi laterali sono all'altezza del pistoncino, come le clip del modello, ma sono cornici sporgenti con una feritoia rettangolare: nei render la loro forma è solo dedotta.
2. **Tasto e pistoncino.** Il cappuccio è alla stessa quota di Dragoon. Con la trottola inserita il contatto deve risultare premuto (`presenza` in console): se non si chiude, tutti i lanci vengono scartati. Se la trottola entra a fatica, serve un tasto a bassa forza (circa 100 gf).
3. **Altezza del cappuccio del tuo tasto:** `tact_h` e `tact_body`. La punta resta alla quota giusta da sola, entro i limiti controllati dagli `assert`.
4. **Breakout QRE:** `qre_pcb` e `qre_h`. Largo 9 mm sta nello spazio con 0,1 mm di margine verso il tasto: se è più largo, un `assert` lo segnala.
5. **Presa:** il corpo è lungo e piatto come quello di Dragoon, ma la comodità va provata in mano.
6. **Interruttore:** passo dei fori e corsa della leva sono stimati, come nell'altra scatola (`sf_pitch`, `sf_travel`).
7. **Retro a filo:** il corpo deve finire al bordo del launcher lato corda, e la maniglia a T riavvolta non deve toccarlo. Se sporge o rientra, correggi `rear_len`.
8. **OLED:** misura il modulo (`oled_pcb`, `oled_glass`) e dov'è l'area visibile (`oled_win_dy`), che spesso non è al centro del PCB. Se il testo risulta capovolto, si cambia una riga del firmware (`0xA1, 0xC8` in `oled_ui.cpp`).
9. **LED:** verifica che si vedano attraverso le zone sottili. Se non si vedono, con la posizione vera del LED si può aprire un foro.
10. **Tasto esterno:** la corsa è di 0,25 mm. Il cappuccio deve appoggiare sul tasto senza premerlo a riposo.

## Verifiche fatte a CAD

- Corpo, vassoio, coperchio e clip sono ognuno un solido unico.
- Nessuna collisione tra QRE, tasti, cappuccio, OLED, SuperMini, TP4056, LiPo, interruttore, corpo, vassoio e coperchio, con `oled = true` (dopo l'ultima disposizione non è stato ricontrollato `oled = false`).
- Perno chiuso: con il coperchio la clip ruota libera fino a 60°; spostandola di 0,4 mm verso fuori, in su o in giù tocca tacco, appoggio o blocco, quindi non può sfilarsi.
- Lo spezzone stampato (senza perno chiuso) ha confermato aggancio, finestrella e posizione del pistoncino.
