#include "GrapherState.h"

namespace core {

GrapherState::GrapherState() {
  // Matches the original demo defaults (lineEntries[] in main.cpp).
  equations_[0] = "x^2";
  equations_[1] = "test";
  equations_[2] = "test";
  equations_[3] = "test";
}

const std::string& GrapherState::equationSlot(int index) const {
  static const std::string kEmpty;
  if (index < 0 || index >= kEquationSlots) return kEmpty;
  return equations_[size_t(index)];
}

void GrapherState::setEquationSlot(int index, std::string text) {
  if (index < 0 || index >= kEquationSlots) return;
  equations_[size_t(index)] = std::move(text);
}

bool GrapherState::graphEquation(int slotIndex) {
  if (slotIndex < 0 || slotIndex >= kEquationSlots) return false;
  Expression e = parseExpression(equations_[size_t(slotIndex)]);
  if (!e.valid) {
    freeExpression(e);
    return false;
  }
  parsed_ = ParsedExpression(e);
  selected_ = slotIndex + 1;  // preserve original's 1-based selEquation convention
  return true;
}

void GrapherState::clearSelection() {
  parsed_ = ParsedExpression();
  selected_ = -1;
}

void GrapherState::panBy(int dx, int dy) {
  panX_ += dx;
  panY_ += dy;
}

void GrapherState::resetPan() {
  panX_ = 0;
  panY_ = 0;
}

}  // namespace core
