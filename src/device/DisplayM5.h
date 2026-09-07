#pragma once
// src/device/DisplayM5.h
//
// NOTE: this file is written against the M5Unified / M5GFX APIs but has
// NOT been compiled or flashed — this sandbox has no network access to
// pull those libraries or an ESP32 toolchain. Treat it as a documented
// starting point; expect to fix small API-signature mismatches against
// whatever M5Unified version PlatformIO resolves for you.

#include "IDisplay.h"
#include <M5Unified.h>

namespace device {

class DisplayM5 : public hal::IDisplay {
 public:
  explicit DisplayM5(int width, int height) : sprite_(&M5.Display) {
    sprite_.setColorDepth(16);
    sprite_.createSprite(width, height);
  }

  void push(const core::Framebuffer& fb) override {
    // The reference project notes the 16-bit sprite buffer stores
    // RGB565 byte-swapped; pushImage below expects the buffer in the
    // panel's native order, so we swap bytes per pixel on the way in
    // rather than assuming Framebuffer's little-endian layout matches.
    for (int y = 0; y < fb.height(); ++y) {
      for (int x = 0; x < fb.width(); ++x) {
        core::Color565 c = fb.getPixel(x, y);
        uint16_t swapped = uint16_t((c << 8) | (c >> 8));
        sprite_.drawPixel(x, y, swapped);
      }
    }
    sprite_.pushSprite(0, 0);
  }

 private:
  LGFX_Sprite sprite_;
};

}  // namespace device
