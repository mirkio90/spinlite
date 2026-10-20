#include "battery.h"

#include "config.h"

namespace spinlite {

namespace {
struct Point {
  int16_t mv;
  int8_t percent;
};
// Dal piu' carico al piu' scarico.
const Point TABLE[] = {{4200, 100}, {4100, 90}, {4000, 80}, {3900, 65}, {3800, 50}, {3750, 40},
                       {3700, 30},  {3650, 20}, {3600, 12}, {3500, 5},  {3300, 0}};
constexpr int N = int(sizeof TABLE / sizeof TABLE[0]);
} // namespace

int batteryPercent(int mv) {
  if (mv < BATTERY_USB_THRESHOLD_MV) return -1;
  if (mv >= TABLE[0].mv) return 100;
  if (mv <= TABLE[N - 1].mv) return 0;
  for (int i = 1; i < N; ++i) {
    if (mv >= TABLE[i].mv) {
      const int dmv = TABLE[i - 1].mv - TABLE[i].mv;
      const int dp = TABLE[i - 1].percent - TABLE[i].percent;
      return TABLE[i].percent + (mv - TABLE[i].mv) * dp / dmv;
    }
  }
  return 0;
}

} // namespace spinlite
