// test/test_native/test_main.cpp
//
// Runs entirely on the host — no device, no simulator window.
//   pio test -e test-native
//
// Covers the three things actually migrated into lib/core for this
// project: the ported ExpressionParser (verified against the original
// Arduino-String version's behavior before this file was written — see
// migration notes in README.md), GrapherState (new — extracted from
// main.cpp's globals), and GraphRenderer's curve plotting (new — the
// original app never implemented this part).

#include <unity.h>

#include "ExpressionParser.h"
#include "GrapherState.h"
#include "Framebuffer.h"
#include "GraphRenderer.h"

#include <cmath>

void setUp(void) {}
void tearDown(void) {}

// ---------------------------------------------------------------------
// ExpressionParser — ported from the original; these cases were cross-
// checked against the original algorithm's expected behavior before
// being written down (see the conversation / commit history).
// ---------------------------------------------------------------------

void test_parser_basic_power(void) {
  core::Expression e = core::parseExpression("x^2");
  TEST_ASSERT_TRUE(e.valid);
  TEST_ASSERT_EQUAL_DOUBLE(9.0, core::evaluateExpression(e, 3));
  TEST_ASSERT_EQUAL_DOUBLE(9.0, core::evaluateExpression(e, -3));
  core::freeExpression(e);
}

void test_parser_implicit_multiplication(void) {
  core::Expression e = core::parseExpression("2x + 1");
  TEST_ASSERT_TRUE(e.valid);
  TEST_ASSERT_EQUAL_DOUBLE(11.0, core::evaluateExpression(e, 5));
  core::freeExpression(e);
}

void test_parser_unary_minus_binds_looser_than_power(void) {
  // "-x^2" must parse as -(x^2), not (-x)^2 — this is the specific
  // precedence rule called out in the parser's own grammar comment.
  core::Expression e = core::parseExpression("-x^2");
  TEST_ASSERT_TRUE(e.valid);
  TEST_ASSERT_EQUAL_DOUBLE(-9.0, core::evaluateExpression(e, 3));
  core::freeExpression(e);
}

void test_parser_power_rhs_allows_unary(void) {
  core::Expression e = core::parseExpression("2^-1");
  TEST_ASSERT_TRUE(e.valid);
  TEST_ASSERT_EQUAL_DOUBLE(0.5, core::evaluateExpression(e, 0));
  core::freeExpression(e);
}

void test_parser_domain_errors_return_nan(void) {
  core::Expression div0 = core::parseExpression("1/0");
  TEST_ASSERT_TRUE(div0.valid);
  TEST_ASSERT_TRUE(std::isnan(core::evaluateExpression(div0, 0)));
  core::freeExpression(div0);

  core::Expression negSqrt = core::parseExpression("sqrt(-1)");
  TEST_ASSERT_TRUE(negSqrt.valid);
  TEST_ASSERT_TRUE(std::isnan(core::evaluateExpression(negSqrt, 0)));
  core::freeExpression(negSqrt);
}

void test_parser_rejects_unknown_identifier(void) {
  core::Expression e = core::parseExpression("foo(x)");
  TEST_ASSERT_FALSE(e.valid);
  core::freeExpression(e);
}

void test_parser_rejects_unbalanced_parens(void) {
  core::Expression e = core::parseExpression("(2+3");
  TEST_ASSERT_FALSE(e.valid);
  core::freeExpression(e);
}

void test_parser_rejects_empty_input(void) {
  core::Expression e = core::parseExpression("");
  TEST_ASSERT_FALSE(e.valid);
  core::freeExpression(e);
}

// ---------------------------------------------------------------------
// GrapherState — new, extracted from what were loose globals
// (lineEntries[], selEquation, posx/posy) in the original main.cpp.
// ---------------------------------------------------------------------

void test_grapher_state_default_slots(void) {
  core::GrapherState state;
  TEST_ASSERT_EQUAL_STRING("x^2", state.equationSlot(0).c_str());
  TEST_ASSERT_EQUAL(-1, state.selectedEquation());
}

void test_grapher_state_graph_valid_equation(void) {
  core::GrapherState state;
  state.setEquationSlot(1, "sin(x)");
  bool ok = state.graphEquation(1);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL(2, state.selectedEquation());  // 1-based, matches original convention
  TEST_ASSERT_TRUE(state.hasValidExpression());
  TEST_ASSERT_EQUAL_DOUBLE(0.0, state.evaluateAt(0.0));
}

void test_grapher_state_graph_invalid_equation_fails_cleanly(void) {
  core::GrapherState state;
  state.setEquationSlot(0, "not valid math (((");
  bool ok = state.graphEquation(0);
  TEST_ASSERT_FALSE(ok);
  TEST_ASSERT_FALSE(state.hasValidExpression());
  TEST_ASSERT_EQUAL(-1, state.selectedEquation());
}

void test_grapher_state_pan(void) {
  core::GrapherState state;
  TEST_ASSERT_EQUAL(0, state.panX());
  state.panBy(5, -3);
  state.panBy(2, 1);
  TEST_ASSERT_EQUAL(7, state.panX());
  TEST_ASSERT_EQUAL(-2, state.panY());
  state.resetPan();
  TEST_ASSERT_EQUAL(0, state.panX());
  TEST_ASSERT_EQUAL(0, state.panY());
}

// ---------------------------------------------------------------------
// GraphRenderer — the curve-plotting logic the original app never
// implemented (it only drew the grid; see GraphRenderer.cpp's header
// comment). These lock exact rendered frames the same way the
// reference architecture's typewriter demo does.
// ---------------------------------------------------------------------

void test_render_no_selection_draws_grid_only(void) {
  core::GrapherState state;
  core::Framebuffer fb(40, 30);
  ui::renderGraph(state, fb);
  TEST_ASSERT_EQUAL_UINT32(0x4A89E929, fb.checksum());
}

void test_render_linear_equation_locked_frame(void) {
  core::GrapherState state;
  state.setEquationSlot(0, "x");
  state.graphEquation(0);
  core::Framebuffer fb(40, 30);
  ui::renderGraph(state, fb);
  TEST_ASSERT_EQUAL_UINT32(0x0AD2CE76, fb.checksum());
}

void test_render_panning_changes_the_frame(void) {
  core::GrapherState state;
  state.setEquationSlot(0, "x");
  state.graphEquation(0);
  state.panBy(5, -3);
  core::Framebuffer fb(40, 30);
  ui::renderGraph(state, fb);
  TEST_ASSERT_EQUAL_UINT32(0x3C105C46, fb.checksum());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_parser_basic_power);
  RUN_TEST(test_parser_implicit_multiplication);
  RUN_TEST(test_parser_unary_minus_binds_looser_than_power);
  RUN_TEST(test_parser_power_rhs_allows_unary);
  RUN_TEST(test_parser_domain_errors_return_nan);
  RUN_TEST(test_parser_rejects_unknown_identifier);
  RUN_TEST(test_parser_rejects_unbalanced_parens);
  RUN_TEST(test_parser_rejects_empty_input);
  RUN_TEST(test_grapher_state_default_slots);
  RUN_TEST(test_grapher_state_graph_valid_equation);
  RUN_TEST(test_grapher_state_graph_invalid_equation_fails_cleanly);
  RUN_TEST(test_grapher_state_pan);
  RUN_TEST(test_render_no_selection_draws_grid_only);
  RUN_TEST(test_render_linear_equation_locked_frame);
  RUN_TEST(test_render_panning_changes_the_frame);
  return UNITY_END();
}
