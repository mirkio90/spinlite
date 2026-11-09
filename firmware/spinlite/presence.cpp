#include "presence.h"

namespace spinlite {

void PresenceGate::reset(bool pressed, uint32_t nowMs) {
  stable_ = raw_ = armed_ = pressed;
  rawSinceMs_ = nowMs;
}

bool PresenceGate::update(bool pressed, uint32_t nowMs, bool measuring) {
  if (pressed != raw_) {
    raw_ = pressed;
    rawSinceMs_ = nowMs;
  }
  if (raw_ == stable_ || nowMs - rawSinceMs_ < DEBOUNCE_MS) return false;
  stable_ = raw_;
  if (stable_)
    armed_ = true;
  else if (!measuring)
    armed_ = false;
  return true;
}

bool PresenceGate::consumeLaunch() {
  const bool valid = !enabled_ || armed_;
  armed_ = false;
  return valid;
}

} // namespace spinlite
