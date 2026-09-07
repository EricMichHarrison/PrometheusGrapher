#pragma once
// lib/ui/GraphRenderer.h
//
// Renders core::GrapherState into a core::Framebuffer: axes, grid step
// lines (mirrors the original DrawGraphPage's four loops exactly), and
// — new — the actual plotted curve of the graphed equation. The
// original app's DrawGraphPage never implemented this part; it just
// drew the grid and printed the equation's slot number where the curve
// should be (see the "PUT EQUATION GRAPHING DRAWER HERE" comment in
// the original src/main.cpp). This fills that in using the
// already-existing (but previously unused for this purpose)
// core::ExpressionParser evaluate function.
//
// Mapping: 1 pixel = 1 unit of x and y, origin at (width/2, height/2)
// offset by the pan, matching the original's un-scaled coordinate
// system (there's no zoom concept in the original app either). Screen
// y increases downward, so graph y is negated when converting.

#include "Framebuffer.h"
#include "GrapherState.h"

namespace ui {

struct GraphMetrics {
  int lineStepCount = 20;  // distance between grid marker lines, in pixels
};

struct GraphColors {
  core::Color565 axis = core::colors::kWhite;
  core::Color565 grid = core::colors::kGrey;
  core::Color565 curve = core::colors::kGreen;
  core::Color565 background = core::colors::kBlack;
};

void renderGraph(const core::GrapherState& state, core::Framebuffer& fb,
                  const GraphColors& colors = GraphColors{},
                  const GraphMetrics& metrics = GraphMetrics{});

}  // namespace ui
