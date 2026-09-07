#pragma once
// lib/core/Cursor.h
//
// Blink timing is a pure function of a `now` you pass in — never a live
// clock read from inside core/. That's what makes a specific frame
// ("cursor visible at t=900ms") reproducible in a native test and in a
// simulator capture, with no sleeping, no flakiness, no hardware timer.

#include <cstdint>

namespace core {

// Returns true when the cursor should be drawn "on" at time `nowMs`,
// given a blink half-period of `periodMs` (default 500ms => 1Hz blink).
bool cursorVisible(uint32_t nowMs, uint32_t periodMs = 500);

}  // namespace core
