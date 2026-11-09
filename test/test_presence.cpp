// Test della logica di presenza della trottola.
#include "../firmware/spinlite/presence.h"

#include <cstdio>

using namespace spinlite;

static int failures = 0;
#define CHECK(cond)                                                                                \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      std::printf("  FALLITO %s:%d: %s\n", __FILE__, __LINE__, #cond);                             \
      ++failures;                                                                                  \
    }                                                                                              \
  } while (0)

// Porta il contatto a uno stato e aspetta che il debounce lo confermi.
static uint32_t settle(PresenceGate &g, bool pressed, uint32_t t, bool measuring = false) {
  g.update(pressed, t, measuring);
  g.update(pressed, t + PresenceGate::DEBOUNCE_MS, measuring);
  return t + PresenceGate::DEBOUNCE_MS + 1;
}

static PresenceGate emptyGate() {
  PresenceGate g;
  g.reset(false, 0);
  return g;
}

static void dryPullIsRejected() {
  PresenceGate g = emptyGate();
  CHECK(!g.consumeLaunch());
}

static void normalLaunch() {
  PresenceGate g = emptyGate();
  uint32_t t = settle(g, true, 100);
  CHECK(g.present() && g.armed());
  t = settle(g, false, t, true); // la trottola parte durante la misura
  CHECK(!g.present() && g.armed());
  CHECK(g.consumeLaunch());
  CHECK(!g.armed());
  CHECK(!g.consumeLaunch()); // strappo a vuoto subito dopo
}

static void resultBeforeRelease() {
  PresenceGate g = emptyGate();
  uint32_t t = settle(g, true, 100);
  CHECK(g.consumeLaunch());
  // contatto ancora premuto: niente nuovo armamento finche' non si libera
  t = settle(g, true, t);
  CHECK(!g.armed());
  CHECK(!g.consumeLaunch());
  t = settle(g, false, t);
  settle(g, true, t);
  CHECK(g.armed());
}

static void removedByHandDisarms() {
  PresenceGate g = emptyGate();
  uint32_t t = settle(g, true, 100);
  settle(g, false, t, false);
  CHECK(!g.armed());
  CHECK(!g.consumeLaunch());
}

static void bounceIsIgnored() {
  PresenceGate g = emptyGate();
  CHECK(!g.update(true, 100, false));
  CHECK(!g.update(false, 110, false));
  CHECK(!g.update(true, 120, false));
  CHECK(!g.update(false, 130, false));
  CHECK(!g.update(false, 500, false));
  CHECK(!g.present() && !g.armed());
  // il tempo riparte dall'ultimo rimbalzo
  CHECK(!g.update(true, 600, false));
  CHECK(!g.update(true, 600 + PresenceGate::DEBOUNCE_MS - 1, false));
  CHECK(g.update(true, 600 + PresenceGate::DEBOUNCE_MS, false));
  CHECK(g.present() && g.armed());
}

static void insertedAtBoot() {
  PresenceGate g;
  g.reset(true, 0);
  CHECK(g.present() && g.armed());
  CHECK(g.consumeLaunch());
}

static void autotestArms() {
  PresenceGate g = emptyGate();
  g.arm();
  uint32_t t = settle(g, false, 100);
  (void)t;
  CHECK(g.armed()); // nessun cambio di stato: l'armamento resta
  CHECK(g.consumeLaunch());
}

static void millisWrap() {
  PresenceGate g;
  g.reset(false, 0xFFFFFFF0u);
  CHECK(!g.update(true, 0xFFFFFFF8u, false));
  CHECK(g.update(true, 0xFFFFFFF8u + PresenceGate::DEBOUNCE_MS, false));
  CHECK(g.armed());
}

int main(int argc, char **) {
  dryPullIsRejected();
  normalLaunch();
  resultBeforeRelease();
  removedByHandDisarms();
  bounceIsIgnored();
  insertedAtBoot();
  autotestArms();
  millisWrap();
  if (failures) {
    std::printf("%d controlli falliti\n", failures);
    return 1;
  }
  if (argc < 2) std::printf("presenza: tutti i controlli superati\n");
  return 0;
}
