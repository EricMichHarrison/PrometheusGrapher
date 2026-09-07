#include "Framebuffer.h"

namespace core {

Framebuffer::Framebuffer(int width, int height)
    : width_(width), height_(height), pixels_(size_t(width) * height * 2, 0) {}

size_t Framebuffer::indexOf(int x, int y) const {
  return (size_t(y) * width_ + x) * 2;
}

void Framebuffer::fill(Color565 color) {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      setPixel(x, y, color);
    }
  }
}

void Framebuffer::setPixel(int x, int y, Color565 color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  size_t i = indexOf(x, y);
  pixels_[i] = uint8_t(color & 0xFF);
  pixels_[i + 1] = uint8_t((color >> 8) & 0xFF);
}

Color565 Framebuffer::getPixel(int x, int y) const {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return colors::kBlack;
  size_t i = indexOf(x, y);
  return Color565(pixels_[i] | (uint16_t(pixels_[i + 1]) << 8));
}

void Framebuffer::drawGlyph(int x, int y, const uint8_t glyphRows[7],
                             Color565 fg, Color565 bg, bool drawBg) {
  for (int row = 0; row < 7; ++row) {
    uint8_t bits = glyphRows[row];
    for (int col = 0; col < 5; ++col) {
      bool on = (bits >> (4 - col)) & 0x1;
      if (on) {
        setPixel(x + col, y + row, fg);
      } else if (drawBg) {
        setPixel(x + col, y + row, bg);
      }
    }
  }
}

uint32_t Framebuffer::checksum() const {
  // FNV-1a over the raw byte buffer. Deterministic, order-sensitive,
  // cheap — good enough to catch "a pixel moved" without needing an
  // image diff library in native tests.
  uint32_t hash = 2166136261u;
  for (uint8_t b : pixels_) {
    hash ^= b;
    hash *= 16777619u;
  }
  return hash;
}

}  // namespace core
