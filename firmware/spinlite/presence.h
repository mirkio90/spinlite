// SpinLite - presenza della trottola (microswitch sul pistoncino del launcher).
//
// Regola: un lancio vale solo se la trottola e' stata inserita prima.
//   - l'inserimento (fronte libero -> premuto) arma;
//   - un risultato consuma l'armamento: il prossimo lancio vuole un nuovo
//     inserimento, anche se il contatto risulta ancora premuto;
//   - il rilascio DURANTE la misura non disarma, perche' e' il lancio stesso;
//     il rilascio fuori misura e' la trottola tolta a mano e disarma.
// Solo logica, testata sul PC: il pin lo legge spinlite.ino.
#pragma once
#include <stdint.h>

namespace spinlite {

class PresenceGate {
public:
  static constexpr uint32_t DEBOUNCE_MS = 30;

  // Stato iniziale: una trottola gia' inserita all'avvio conta come inserita.
  void reset(bool pressed, uint32_t nowMs);

  // Da chiamare spesso. true quando lo stato stabile e' appena cambiato.
  bool update(bool pressed, uint32_t nowMs, bool measuring);

  // Da chiamare a ogni risultato: true se il lancio e' valido.
  bool consumeLaunch();

  // Arma senza contatto (autotest).
  void arm() { armed_ = true; }

  // Da spenta consumeLaunch accetta tutto: serve quando il microswitch non c'e'.
  void setEnabled(bool on) { enabled_ = on; }
  bool enabled() const { return enabled_; }

  bool present() const { return stable_; }
  bool armed() const { return armed_; }

private:
  bool enabled_ = false;
  bool stable_ = false;
  bool raw_ = false;
  bool armed_ = false;
  uint32_t rawSinceMs_ = 0;
};

} // namespace spinlite
