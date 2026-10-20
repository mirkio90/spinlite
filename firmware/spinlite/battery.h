// SpinLite - stima della carica di una LiPo da 1 cella. Portabile, solo interi.
//
// Tabella approssimata (a riposo, senza carico), interpolazione lineare:
// 4200=100 4100=90 4000=80 3900=65 3800=50 3750=40 3700=30 3650=20
// 3600=12 3500=5 3300=0. E' una stima, non un fuel gauge.
#pragma once
#include <stdint.h>

namespace spinlite {

// mV della batteria -> 0..100. -1 se mv < BATTERY_USB_THRESHOLD_MV (2500):
// interruttore spento, scheda alimentata da USB. Anche mv < 0 -> -1.
int batteryPercent(int mv);

} // namespace spinlite
