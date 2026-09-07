#include "PpmWriter.h"

#include <cstdio>
#include <filesystem>

namespace sim {

bool writePpm(const core::Framebuffer& fb, const std::string& path) {
  std::filesystem::path p(path);
  if (p.has_parent_path()) {
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
    // ec is intentionally ignored here: if the directory already
    // exists, or creation fails for a reason fopen() below will also
    // hit (e.g. permissions), we still get a clear failure signal from
    // the fopen() check that follows.
  }

  FILE* f = fopen(path.c_str(), "wb");
  if (!f) return false;

  fprintf(f, "P6\n%d %d\n255\n", fb.width(), fb.height());
  for (int y = 0; y < fb.height(); ++y) {
    for (int x = 0; x < fb.width(); ++x) {
      core::Color565 c = fb.getPixel(x, y);
      uint8_t r5 = (c >> 11) & 0x1F;
      uint8_t g6 = (c >> 5) & 0x3F;
      uint8_t b5 = c & 0x1F;
      uint8_t r8 = uint8_t((r5 * 255) / 31);
      uint8_t g8 = uint8_t((g6 * 255) / 63);
      uint8_t b8 = uint8_t((b5 * 255) / 31);
      fputc(r8, f);
      fputc(g8, f);
      fputc(b8, f);
    }
  }

  fclose(f);
  return true;
}

}  // namespace sim
