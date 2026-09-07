#pragma once
// lib/hal/IDisplay.h
//
// The boundary between the pure-C++ core::Framebuffer and an actual
// screen. Device code implements this over M5GFX/LovyanGFX; sim code
// implements it as "write to a PPM file" and/or an SDL window. Nothing
// above this interface knows or cares which one it's talking to.

#include "Framebuffer.h"

namespace hal {

class IDisplay {
 public:
  virtual ~IDisplay() = default;

  // Pushes the given buffer to whatever this display actually is.
  virtual void push(const core::Framebuffer& fb) = 0;
};

}  // namespace hal
