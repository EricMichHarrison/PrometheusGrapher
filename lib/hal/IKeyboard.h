#pragma once
// lib/hal/IKeyboard.h
//
// Keeps the app loop ignorant of whether keys come from the Cardputer's
// TCA8418 matrix, an SDL keydown event, or a scripted test sequence.

namespace hal {

enum class KeyEvent {
  kNone,
  kChar,       // a printable character; see `ch`
  kBackspace,
  kEnter,
  kLeft,
  kRight,
  kUp,
  kDown,
  kQuit,       // e.g. Esc in a terminal/sim front end; unused on device
};

struct KeyPress {
  KeyEvent event = KeyEvent::kNone;
  char ch = 0;  // valid only when event == kChar
};

class IKeyboard {
 public:
  virtual ~IKeyboard() = default;

  // Non-blocking: returns a KeyPress with event == kNone if nothing is
  // pending.
  virtual KeyPress poll() = 0;
};

}  // namespace hal
