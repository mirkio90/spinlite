# SpinLite

Conta giri da pochi euro per launcher **Beyblade X**: ESP32-C3 SuperMini + sensore TCRT5000 + piccola LiPo.
I risultati vanno al telefono via Bluetooth con lo stesso profilo del Bey Battle Pass, oppure su un display OLED opzionale.

> **Lavori in corso.** Il rilevatore dei giri e i test con segnali simulati sono già nel repo; firmware completo, guida di montaggio e pagina web arrivano nei prossimi commit.

## Stato

| Parte | Stato |
| --- | --- |
| Rilevatore RPM (`firmware/spinlite/rpm_detector.*`) | fatto: 21 casi simulati, errore tipico sotto lo 0,1% |
| Test su PC (`test/`) | `make -C test` |
| Pacchetti Battle Pass (`bbp_protocol.*`) | fatto: verificati anche con un parser indipendente |
| Firmware ESP32-C3 (ADC, BLE, OLED) | da fare |
| Guida, cablaggio, dashboard web | da fare |

Licenza MIT. Progetto non affiliato con Takara Tomy o Hasbro.
