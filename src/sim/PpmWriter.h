#pragma once
// src/sim/PpmWriter.h
//
// Writes a core::Framebuffer to a binary PPM (P6) file. PPM is chosen
// deliberately: it's a trivial, well-documented format any image tool
// (ImageMagick, GIMP, Python/Pillow, ffmpeg) can read, and writing it
// needs zero third-party dependencies — so `pio run -e sim` produces
// deterministic capture files with nothing beyond the native toolchain.
// Swap this for a PNG writer later if you want smaller files.

#include "Framebuffer.h"
#include <string>

namespace sim {

// Returns true on success.
bool writePpm(const core::Framebuffer& fb, const std::string& path);

}  // namespace sim
