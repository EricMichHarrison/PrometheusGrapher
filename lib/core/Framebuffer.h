#pragma once
// lib/core/Framebuffer.h
//
// A plain RGB565 pixel buffer with zero hardware / Arduino / lgfx includes.
// This is the "visual buffer" the whole testing architecture is built
// around: because it's pure C++17, it can be filled, inspected, hashed
// and asserted on from native Unity tests (test/test_native) with no
// device and no simulator window required. The same buffer is what the
// device HAL pushes to the real screen, and what the simulator dumps to
// an image file — one buffer, three destinations (test / sim / device).

#include <cstdint>
#include <cstddef>
#include <vector>

namespace core {

// RGB565, byte order matches what most SPI TFT panels (incl. the
// Cardputer's ST7789) expect once byte-swapped by the HAL layer just
// before pushing — see docs/NOTES.md. Keep colours as named constants,
// never raw literals, to avoid RGB888-vs-RGB565 mixups.
using Color565 = uint16_t;

namespace colors {
constexpr Color565 kBlack = 0x0000;
constexpr Color565 kWhite = 0xFFFF;
constexpr Color565 kGrey = 0x7BEF;
constexpr Color565 kGreen = 0x07E0;
constexpr Color565 kAmber = 0xFD20;
constexpr Color565 kCursor = 0x07FF;
// Matches the original app's plotColours[] (RED/GREEN/BLUE/MAGENTA) so
// device code can reference the same constants either via core:: or
// via M5GFX's own macros interchangeably.
constexpr Color565 kRed = 0xF800;
constexpr Color565 kBlue = 0x001F;
constexpr Color565 kMagenta = 0xF81F;
}  // namespace colors

class Framebuffer {
 public:
  Framebuffer(int width, int height);

  int width() const { return width_; }
  int height() const { return height_; }

  void fill(Color565 color);
  void setPixel(int x, int y, Color565 color);
  Color565 getPixel(int x, int y) const;

  // Draws a single glyph (see Font5x7) at the given top-left pixel.
  // Out-of-range glyphs draw nothing; out-of-bounds pixels are clipped.
  void drawGlyph(int x, int y, const uint8_t glyphRows[7], Color565 fg,
                  Color565 bg, bool drawBg);

  // Fast, order-sensitive hash of the whole buffer. Two renders of the
  // same logical state at the same `now` must produce the same
  // checksum — that determinism is what makes "lock this exact frame"
  // tests possible without storing reference images.
  uint32_t checksum() const;

  const uint8_t* raw() const { return pixels_.data(); }
  size_t rawSizeBytes() const { return pixels_.size(); }

 private:
  int width_;
  int height_;
  // Stored as raw bytes (2 per pixel, little-endian) rather than a
  // uint16_t vector so raw()/rawSizeBytes() map directly onto what a
  // real panel DMA transfer or a PPM writer wants.
  std::vector<uint8_t> pixels_;

  size_t indexOf(int x, int y) const;
};

}  // namespace core
