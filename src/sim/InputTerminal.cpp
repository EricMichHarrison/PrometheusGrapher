#include "InputTerminal.h"

// ===========================================================================
// Windows implementation — NOT compiled or run in this sandbox (no Windows
// toolchain available here). Standard conio.h non-blocking pattern; check
// this first if arrow keys or ANSI colors misbehave on your machine.
// ===========================================================================
#ifdef _WIN32

#include <conio.h>
#include <windows.h>

namespace simhal {

TerminalKeyboard::TerminalKeyboard() {
  // Classic conhost (plain cmd.exe/older PowerShell) needs this to
  // honor the ANSI truecolor codes TerminalView.cpp writes; Windows
  // Terminal already has it on, so this is a no-op there.
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)) {
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
}

TerminalKeyboard::~TerminalKeyboard() {}

hal::KeyPress TerminalKeyboard::poll() {
  hal::KeyPress kp;
  if (!_kbhit()) return kp;

  int c = _getch();
  if (c == 0 || c == 0xE0) {  // extended-key prefix (arrows, F-keys, ...)
    int c2 = _getch();
    switch (c2) {
      case 72: kp.event = hal::KeyEvent::kUp; return kp;
      case 80: kp.event = hal::KeyEvent::kDown; return kp;
      case 75: kp.event = hal::KeyEvent::kLeft; return kp;
      case 77: kp.event = hal::KeyEvent::kRight; return kp;
      default: return kp;  // unmapped extended key: ignore
    }
  }
  if (c == 27) { kp.event = hal::KeyEvent::kQuit; return kp; }
  if (c == 8)  { kp.event = hal::KeyEvent::kBackspace; return kp; }
  if (c == 13) { kp.event = hal::KeyEvent::kEnter; return kp; }
  if (c >= 32 && c < 127) {
    kp.event = hal::KeyEvent::kChar;
    kp.ch = char(c);
  }
  return kp;
}

}  // namespace simhal

// ===========================================================================
// POSIX implementation — compiled and exercised in this sandbox.
// ===========================================================================
#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

namespace simhal {

namespace {
termios g_origTermios{};

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &g_origTermios);
  termios raw = g_origTermios;
  raw.c_lflag &= ~(unsigned(ECHO) | unsigned(ICANON));
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSANOW, &g_origTermios);
}

int readByteNonBlocking() {
  unsigned char c;
  ssize_t n = read(STDIN_FILENO, &c, 1);
  return n == 1 ? int(c) : -1;
}
}  // namespace

TerminalKeyboard::TerminalKeyboard() {
  enableRawMode();
  rawModeEnabled_ = true;
}

TerminalKeyboard::~TerminalKeyboard() {
  if (rawModeEnabled_) disableRawMode();
}

hal::KeyPress TerminalKeyboard::poll() {
  hal::KeyPress kp;
  int c = readByteNonBlocking();
  if (c < 0) return kp;

  if (c == 27) {  // Esc, or the start of an arrow-key escape sequence
    int c2 = readByteNonBlocking();
    if (c2 == '[') {
      int c3 = readByteNonBlocking();
      switch (c3) {
        case 'A': kp.event = hal::KeyEvent::kUp; return kp;
        case 'B': kp.event = hal::KeyEvent::kDown; return kp;
        case 'C': kp.event = hal::KeyEvent::kRight; return kp;
        case 'D': kp.event = hal::KeyEvent::kLeft; return kp;
        default: return kp;  // unmapped escape sequence: ignore
      }
    }
    kp.event = hal::KeyEvent::kQuit;  // bare Esc
    return kp;
  }
  if (c == 127 || c == 8) { kp.event = hal::KeyEvent::kBackspace; return kp; }
  if (c == 13 || c == 10) { kp.event = hal::KeyEvent::kEnter; return kp; }
  if (c >= 32 && c < 127) {
    kp.event = hal::KeyEvent::kChar;
    kp.ch = char(c);
  }
  return kp;
}

}  // namespace simhal

#endif
