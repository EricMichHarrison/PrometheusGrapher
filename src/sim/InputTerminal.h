#pragma once
// src/sim/InputTerminal.h
//
// Non-blocking keyboard input straight from the terminal, so `--live`
// can be typed into interactively. Two implementations behind one
// interface, chosen at compile time:
//   - POSIX (Linux/macOS): raw termios mode, verified in this sandbox.
//   - Windows: conio.h's _kbhit()/_getch(). NOT compiled/verified here
//     — no Windows toolchain available in this sandbox — but it's a
//     standard, well-documented pattern; flag it if it misbehaves.

#include "IKeyboard.h"

namespace simhal {

class TerminalKeyboard : public hal::IKeyboard {
 public:
  TerminalKeyboard();
  ~TerminalKeyboard() override;

  hal::KeyPress poll() override;

#ifndef _WIN32
 private:
  bool rawModeEnabled_ = false;
#endif
};

}  // namespace simhal
