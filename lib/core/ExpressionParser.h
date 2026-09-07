#pragma once
// lib/core/ExpressionParser.h
//
// Ported from the original src/ExpressionParser.h/.cpp: same grammar,
// same AST, same evaluation semantics — the only change is Arduino's
// String -> std::string and dropping <Arduino.h>, so this now has zero
// hardware includes and can be exercised by native Unity tests
// (test/test_native) with no device involved.
//
// Device code (src/device/main.cpp) converts at the boundary:
//   core::parseExpression(std::string(arduinoString.c_str()))

#include <string>

namespace core {

enum class NodeType { NUM, VAR, ADD, SUB, MUL, DIV, POW, NEG, FUNC };
enum class FuncType { NONE, SIN, COS, TAN, SQRT, ABS, LOG, LN, EXP };

struct ExprNode {
  NodeType type;
  double value = 0;                // used when type == NUM
  FuncType func = FuncType::NONE;  // used when type == FUNC
  ExprNode* left = nullptr;
  ExprNode* right = nullptr;

  explicit ExprNode(NodeType t) : type(t) {}
};

// Result of parsing a string. If valid == false, root is null and
// errorMessage explains why (unknown identifier, unbalanced parens, etc).
struct Expression {
  ExprNode* root = nullptr;
  bool valid = false;
  std::string errorMessage;
};

// Parses a math expression string (single variable: x) into an Expression.
// Examples of accepted input: "x^2", "2x + 1", "sin(x) * 3", "-x^2 + pi"
Expression parseExpression(const std::string& input);

// Evaluates a previously parsed Expression at a given x.
// Returns NAN if the expression is invalid or hits a math domain error
// (divide by zero, sqrt of a negative number, etc) so callers can skip
// that point when plotting instead of crashing.
double evaluateExpression(const Expression& expr, double x);

// Releases the AST allocated for an Expression. Call this when you're
// done with an Expression (e.g. before parsing a new one for the same
// slot) to avoid leaking heap memory. See ParsedExpression in
// GrapherState.h for an RAII wrapper that calls this automatically.
void freeExpression(Expression& expr);

}  // namespace core
