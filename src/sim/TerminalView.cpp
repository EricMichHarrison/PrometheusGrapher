#include "TerminalView.h"

#include <cstdio>
#include <cstdint>

namespace simhal {

void ansiClearScreen() { std::fputs("\x1b[2J\x1b[H", stdout); }
void ansiHome() { std::fputs("\x1b[H", stdout); }
void ansiHideCursor() { std::fputs("\x1b[?25l", stdout); }
void ansiShowCursor() { std::fputs("\x1b[?25h", stdout); }

namespace {
void unpack565(core::Color565 c, uint8_t& r, uint8_t& g, uint8_t& b) {
  uint8_t r5 = (c >> 11) & 0x1F;
  uint8_t g6 = (c >> 5) & 0x3F;
  uint8_t b5 = c & 0x1F;
  r = uint8_t((r5 * 255) / 31);
  g = uint8_t((g6 * 255) / 63);
  b = uint8_t((b5 * 255) / 31);
}
}  // namespace

void printFramebufferAnsi(const core::Framebuffer& fb, int colStride) {
  ansiHome();
  for (int y = 0; y < fb.height(); y += 2) {
    for (int x = 0; x < fb.width(); x += colStride) {
      core::Color565 top = fb.getPixel(x, y);
      core::Color565 bot = (y + 1 < fb.height()) ? fb.getPixel(x, y + 1) : top;
      uint8_t tr, tg, tb, br, bg, bb;
      unpack565(top, tr, tg, tb);
      unpack565(bot, br, bg, bb);
      // Foreground = top pixel, background = bottom pixel, glyph =
      // upper-half-block (U+2580, UTF-8: E2 96 80) -> two real pixel
      // rows per terminal character row.
      std::printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm\xe2\x96\x80",
                  tr, tg, tb, br, bg, bb);
    }
    std::fputs("\x1b[0m\x1b[K\n", stdout);  // reset + clear-to-eol
  }
  std::fflush(stdout);
}

}  // namespace simhal
