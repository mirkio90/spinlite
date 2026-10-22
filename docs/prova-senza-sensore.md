# Prova con la sola SuperMini (senza sensore)

Con questa prova verifichi su hardware vero tutta la catena di misura, prima che arrivi il sensore:

1. acquisizione ADC;
2. calibrazione della frequenza;
3. rilevamento del lancio;
4. riga CSV;
5. Bluetooth e app.

Il comando `test` genera un **lancio finto a velocità nota** su GPIO10. Un filo lo porta a GPIO3, l'ingresso del sensore. Il dispositivo deve misurare la velocità che ha generato.

## Cosa serve

- ESP32-C3 SuperMini.
- Cavo **USB-C dati**. Molti cavi servono solo a caricare: se il PC non vede la scheda, prova un altro cavo.
- Un **filo** tra due pin della scheda. Va bene uno qualsiasi di questi:
  - un ponticello Dupont maschio-maschio, con le punte infilate nei fori di GPIO10 e GPIO3 e tenute ferme con un pezzo di nastro;
  - un filo sottile saldato provvisoriamente alle due piazzole (poi si dissalda);
  - la SuperMini su una breadboard con i suoi pettini infilati senza saldarli, e un ponticello. Il contatto è precario, ma per una prova basta.

> Non saldare i pettini in modo definitivo: nella scatola la SuperMini va montata senza, con i fili saldati direttamente sulle piazzole.

## Collegamento

```
   SuperMini (USB-C in alto)
   ┌─────────────────────┐
   │        [USB-C]      │
   │  5V            GPIO5│ ...
   │  GND           GPIO6│
   │  3V3           GPIO7│
   │  GPIO4         GPIO8│
   │  GPIO3 ◄──┐    GPIO9│
   │  ...      │   GPIO10│ ──┐
   └───────────┼─────────┘   │
               └─────────────┘   un solo filo: GPIO10 → GPIO3
```

La posizione dei pin nel disegno è indicativa: **vale la serigrafia sulla tua scheda**, dove sono stampati i numeri (3, 10, ...).

- Collega **solo GPIO10 con GPIO3**, nient'altro.
- **Mai** GPIO10 verso 5V, 3V3 o GND.
- Se un giorno ripeti la prova con il sensore montato, **stacca il sensore**: due uscite collegate insieme possono danneggiarsi.

## 1. Caricare il firmware

Da PowerShell, nella cartella del repo:

```
& "$env:USERPROFILE\.spinlite-pio\Scripts\pio.exe" run -e spinlite_qre -t upload
& "$env:USERPROFILE\.spinlite-pio\Scripts\pio.exe" device monitor
```

In alternativa c'è il [web flasher](https://mirkio90.github.io/spinlite/flasher/), versione **QRE1113**: dopo il caricamento usa un terminale seriale a 115200.

Se la porta non compare, **tieni premuto BOOT** mentre colleghi il cavo, poi rilascialo.

## 2. Controlli all'avvio (ancora senza filo)

Nel monitor seriale devono comparire:

```
SpinLite 0.1.0, sensore QRE1113
Scrivi 'aiuto' per i comandi
ADC: frequenza misurata 99xx.xxx Hz, tickNs 100xxx
```

| Cosa guardare | Valore buono | Se è diverso |
| --- | --- | --- |
| Frequenza misurata | tra 9.000 e 11.000 Hz, tipicamente vicino a 10.000 | se compare "fuori tolleranza", copia la riga e mandamela |
| `stato` → "Frequenza misurata" | lo stesso valore | — |
| `stato` → "Fase" | qualsiasi: con GPIO3 scollegato il segnale è rumore, anche GUASTO è normale | — |

## 3. Il lancio finto

Collega il filo GPIO10 → GPIO3 e scrivi:

```
test 3000 1
```

Cioè 3.000 RPM per 1 secondo, poi rallenta. Dovresti vedere:

```
TEST: GPIO10 -> GPIO3, 3000 RPM per 1 s (mezzo periodo 10000 us, attesi 3000.0 RPM), poi rallenta
TEST: 1,5 s di riposo, poi parte. Il risultato arriva come riga LANCIO.
TEST: segnale fermo, GPIO10 resta alto (riposo). 'test stop' per liberarlo
LANCIO,3000,3000,3001,....,rallentamento
```

Il LED della scheda resta acceso durante la misura e fa un doppio lampeggio quando arriva il risultato.

Il **primo numero** della riga `LANCIO` va confrontato con gli "attesi". L'errore dovrebbe stare **sotto lo 0,2%**, cioè 3.000 ± 6.

Prova anche altre velocità, aspettando 2-3 secondi tra una prova e l'altra (il dispositivo si riarma dopo 1 s di quiete):

```
test 1500 2
test 10000 1
test 25000 1
```

| Risultato | Cosa significa |
| --- | --- |
| RPM giusti (sotto lo 0,2%), fine "rallentamento" | acquisizione, calibrazione e rilevatore funzionano |
| Errore costante, per esempio sempre +1,5% | la frequenza ADC misurata all'avvio è sbagliata: mandami la riga "ADC: frequenza misurata" |
| Nessuna riga LANCIO | controlla il filo; poi `raw 5` durante un `test`: il livello deve alternare valori alti e bassi |
| Fine "segnale" o "dati persi" | mandami l'uscita completa |

## 4. Bluetooth e app

1. Installa **nRF Connect** sul telefono e cerca `BEYBLADE_TOOL01`, poi collegati.
2. Nel servizio `55c40000-…` controlla gli **handle**, di solito visibili nei dettagli o nel log: f001 = **15**, f002 = **17**, CCCD = **18**. Mandami uno screenshot comunque: è il punto 2 dei prossimi passi del progetto.
3. Attiva le notifiche su f002 (icona con le frecce). Poi scrivi `51` su f001: deve arrivare un pacchetto che inizia con `A0`.
4. Fai un `test 3000 1` dalla console: devono arrivare da soli `A0`, poi `B0`…`B7` e `70`…`73`.
5. Scollega nRF Connect e prova la [dashboard](https://mirkio90.github.io/spinlite/) da Chrome (computer o Android), oppure BeyGear. Il lancio finto deve comparire nello storico.

## 5. Alla fine

```
test stop      libera GPIO10
cancella       toglie i lanci finti dallo storico
```

Poi stacca il filo. Incollami tutta l'uscita del monitor seriale e gli screenshot di nRF Connect: li confronto con i valori attesi.
