// SpinLite - autotest: un lancio finto a velocita' nota, senza sensore.
//
// Genera un'onda quadra su PIN_TEST_OUT (GPIO10): basta un filo da GPIO10 a
// GPIO3 (ingresso del sensore, scollegato) perche' il rilevatore veda un
// lancio vero e proprio. Profilo: 1,5 s fermo alto (sensore a riposo), poi
// velocita' costante per il tempo richiesto, poi rallenta del 7% ogni 25 ms
// fino al 25% e si ferma. Il pin resta alto (riposo) finche' non si da'
// "test stop".
//
// Il mezzo periodo e' contato da un timer hardware a 1 MHz (GPTimer) che
// inverte il pin dentro l'interrupt: errore sotto lo 0,05% anche a 30.000
// RPM. La periferica PWM (LEDC) accetta solo frequenze intere in Hz, che a
// 1000 RPM (16,67 Hz) darebbero il 2% di errore.
//
// Solo per ESP32.
#pragma once
#include <stdint.h>

namespace spinlite {
namespace autotest {

// Parte un lancio finto. rpm 300..40000, holdMs 200..10000. Ritorna false se
// il pin non e' configurato o il timer non parte. In *halfUs (se non nullo)
// il mezzo periodo usato, in microsecondi.
bool start(uint32_t rpm, uint32_t holdMs, uint8_t marksPerRev, uint32_t *halfUs = nullptr);

// Ferma tutto e rilascia il pin (alta impedenza).
void stop();

// Da chiamare a ogni giro di loop(): gestisce le fasi. Ritorna true una sola
// volta, quando il lancio finto e' finito.
bool loop();

bool active();

} // namespace autotest
} // namespace spinlite
