#pragma once
// lib/core/GrapherState.h
//
// The app's persistent state, extracted from what were loose globals in
// the original src/main.cpp (lineEntries[], selEquation, posx/posy).
// Zero hardware includes — natively testable, same as ExpressionParser.
//
// ParsedExpression wraps Expression with RAII cleanup. The original
// code called freeExpression() by hand at each reassignment site; this
// makes that automatic and safe under move/reassignment instead of
// relying on every call site remembering to free the old one first.

#include "ExpressionParser.h"
#include <string>
#include <array>

namespace core {

class ParsedExpression {
 public:
  ParsedExpression() = default;
  explicit ParsedExpression(Expression e) : expr_(e) {}
  ParsedExpression(const ParsedExpression&) = delete;
  ParsedExpression& operator=(const ParsedExpression&) = delete;

  ParsedExpression(ParsedExpression&& other) noexcept { *this = std::move(other); }
  ParsedExpression& operator=(ParsedExpression&& other) noexcept {
    if (this != &other) {
      freeExpression(expr_);
      expr_ = other.expr_;
      other.expr_ = Expression{};
    }
    return *this;
  }
  ~ParsedExpression() { freeExpression(expr_); }

  bool valid() const { return expr_.valid; }
  const std::string& errorMessage() const { return expr_.errorMessage; }
  double evaluate(double x) const { return evaluateExpression(expr_, x); }

 private:
  Expression expr_;
};

constexpr int kEquationSlots = 4;

class GrapherState {
 public:
  GrapherState();

  const std::string& equationSlot(int index) const;
  void setEquationSlot(int index, std::string text);

  // -1 = nothing currently graphed, matching the original selEquation
  // convention (and its 1-based slot numbering: slot 1..4).
  int selectedEquation() const { return selected_; }

  // Parses equationSlot(slotIndex) and, on success, makes it the active
  // graph. Returns false (and leaves the previous selection/parse
  // untouched) if the text doesn't parse.
  bool graphEquation(int slotIndex);
  void clearSelection();

  bool hasValidExpression() const { return selected_ != -1 && parsed_.valid(); }
  double evaluateAt(double x) const { return parsed_.evaluate(x); }
  const std::string& lastParseError() const { return parsed_.errorMessage(); }

  int panX() const { return panX_; }
  int panY() const { return panY_; }
  void panBy(int dx, int dy);
  void resetPan();

 private:
  std::array<std::string, kEquationSlots> equations_;
  int selected_ = -1;
  ParsedExpression parsed_;
  int panX_ = 0;
  int panY_ = 0;
};

}  // namespace core
