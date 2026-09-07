#pragma once
// src/sim/TerminalView.h
//
// Renders a core::Framebuffer straight into the terminal using ANSI
// 24-bit ("truecolor") escape codes and the Unicode upper-half-block
// character (▀), which lets each terminal character cell show two
// real pixel rows at once (foreground = top pixel, background =
// bottom pixel). This is what makes `--live` a genuine pixel-buffer
// preview rather than a coarse ASCII approximation.
//
// Requires a truecolor-capable terminal. Windows Terminal, modern
// PowerShell/pwsh, and most Linux/macOS terminals qualify. Classic
// cmd.exe/conhost needs ENABLE_VIRTUAL_TERMINAL_PROCESSING enabled,
// which src/sim/InputTerminal.cpp turns on for you on Windows.

#include "Framebuffer.h"

namespace simhal {

// Downsamples horizontally by `colStride` (every Nth column) to fit
// typical terminal widths — 240px / 2 = 120 columns by default.
void printFramebufferAnsi(const core::Framebuffer& fb, int colStride = 2);

void ansiClearScreen();
void ansiHome();      // cursor to top-left, no clear (avoids flicker)
void ansiHideCursor();
void ansiShowCursor();

}  // namespace simhal
