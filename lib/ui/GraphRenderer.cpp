#include "GraphRenderer.h"

#include "Font5x7.h"
#include <cmath>
#include <algorithm>

namespace ui {

namespace {

void drawVerticalRun(core::Framebuffer& fb, int x, int y0, int y1, core::Color565 c) {
  if (y0 > y1) std::swap(y0, y1);
  for (int y = y0; y <= y1; ++y) fb.setPixel(x, y, c);
}

void drawTextLine(core::Framebuffer& fb, int x, int y, const std::string& text,
                   core::Color565 fg) {
  for (size_t i = 0; i < text.size(); ++i) {
    fb.drawGlyph(x + int(i) * 6, y, core::glyphFor(text[i]), fg,
                 core::colors::kBlack, /*drawBg=*/false);
  }
}

}  // namespace

void renderGraph(const core::GrapherState& state, core::Framebuffer& fb,
                  const GraphColors& colors, const GraphMetrics& metrics) {
  const int w = fb.width();
  const int h = fb.height();
  const int cx = w / 2;
  const int cy = h / 2;
  const int panx = state.panX();
  const int pany = state.panY();

  fb.fill(colors.background);

  // Axes — ported 1:1 from the original's two drawLine calls.
  for (int y = 0; y < h; ++y) fb.setPixel(cx - panx, y, colors.axis);
  for (int x = 0; x < w; ++x) fb.setPixel(x, cy + pany, colors.axis);

  // Grid step lines — ported 1:1 from the original's four loops
  // (left/right/top/bottom), just re-expressed as setPixel columns/rows
  // instead of M5GFX drawLine calls.
  for (int i = metrics.lineStepCount; i < w - panx; i += metrics.lineStepCount)
    for (int y = 0; y < h; ++y) fb.setPixel(cx - panx - i, y, colors.grid);
  for (int i = metrics.lineStepCount; i < w + panx; i += metrics.lineStepCount)
    for (int y = 0; y < h; ++y) fb.setPixel(cx - panx + i, y, colors.grid);
  for (int i = metrics.lineStepCount; i < h + pany; i += metrics.lineStepCount)
    for (int x = 0; x < w; ++x) fb.setPixel(x, cy + pany - i, colors.grid);
  for (int i = metrics.lineStepCount; i < h - pany; i += metrics.lineStepCount)
    for (int x = 0; x < w; ++x) fb.setPixel(x, cy + pany + i, colors.grid);

  // The curve — NEW. Original code only drew the grid and printed the
  // slot number here (see file header comment). One sample per screen
  // column; consecutive samples are joined with a vertical run at the
  // new column so steep sections don't look dotted. A NAN sample (from
  // a domain error, e.g. sqrt of a negative number) breaks the run
  // rather than joining across the gap.
  if (state.hasValidExpression()) {
    bool havePrev = false;
    int prevY = 0;
    for (int sx = 0; sx < w; ++sx) {
      double gx = double(sx - cx + panx);
      double gy = state.evaluateAt(gx);
      if (!std::isfinite(gy)) { havePrev = false; continue; }

      int sy = int(std::lround(double(cy) + pany - gy));
      if (havePrev) {
        drawVerticalRun(fb, sx, prevY, sy, colors.curve);
      } else {
        fb.setPixel(sx, sy, colors.curve);
      }
      prevY = sy;
      havePrev = true;
    }

    int slot = state.selectedEquation() - 1;
    if (slot >= 0) drawTextLine(fb, 2, 2, "EQ" + std::to_string(state.selectedEquation()),
                                colors.curve);
  }
}

}  // namespace ui
