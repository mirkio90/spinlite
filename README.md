# SpinLite

Conta giri da pochi euro per launcher **Beyblade X**: ESP32-C3 SuperMini + sensore TCRT5000 + piccola LiPo.
I risultati vanno al telefono via Bluetooth con lo stesso profilo del Bey Battle Pass, oppure su un display OLED opzionale.

> **Lavori in corso.** Il rilevatore dei giri e i test con segnali simulati sono già nel repo; firmware completo, guida di montaggio e pagina web arrivano nei prossimi commit.

## Stato

| Parte | Stato |
| --- | --- |
| Rilevatore RPM (`firmware/spinlite/rpm_detector.*`) | scritto, in collaudo con segnali simulati |
| Test su PC (`test/`) | `g++ -std=c++17 test/test_detector.cpp firmware/spinlite/rpm_detector.cpp` |
| Protocollo BLE compatibile Battle Pass | da fare |
| Firmware ESP32-C3 (ADC, BLE, OLED) | da fare |
| Guida, cablaggio, dashboard web | da fare |

Licenza MIT. Progetto non affiliato con Takara Tomy o Hasbro.
