#include "ExpressionParser.h"

#include <cmath>
#include <cctype>
#include <algorithm>
#include <cstdlib>

namespace core {

namespace {

// Grammar (lowest to highest precedence) — unchanged from the original:
//   expr    := term (('+' | '-') term)*
//   term    := unary (('*' | '/') unary | implicitUnary)*
//   unary   := ('-' | '+')? power
//   power   := primary ('^' unary)?      // right-associative, rhs allows unary
//   primary := number | 'x' | constant | funcName '(' expr ')' | '(' expr ')'
//
// Unary minus is deliberately placed ABOVE '^' in the call chain (unary
// calls power) so that "-x^2" parses as -(x^2), matching standard math
// notation rather than (-x)^2.

bool isAsciiAlpha(char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0; }
bool isAsciiDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                  [](unsigned char c) { return char(std::tolower(c)); });
  return s;
}

std::string trimmed(const std::string& s) {
  size_t start = s.find_first_not_of(' ');
  if (start == std::string::npos) return "";
  size_t end = s.find_last_not_of(' ');
  return s.substr(start, end - start + 1);
}

class Parser {
 public:
  explicit Parser(const std::string& src) : s(src), pos(0), len(int(src.size())) {}

  ExprNode* parse() {
    ExprNode* result = parseExpr();
    skipSpaces();
    if (ok && pos != len) {
      fail("unexpected trailing characters");
    }
    return result;
  }

  bool isOk() const { return ok; }
  std::string error() const { return errMsg; }

 private:
  std::string s;
  int pos;
  int len;
  bool ok = true;
  std::string errMsg;

  void fail(const std::string& msg) {
    if (ok) {  // keep the first error, it's usually the most useful
      ok = false;
      errMsg = msg;
    }
  }

  void skipSpaces() {
    while (pos < len && s[pos] == ' ') pos++;
  }

  char peek() {
    skipSpaces();
    if (pos >= len) return '\0';
    return s[pos];
  }

  bool match(char c) {
    if (peek() == c) { pos++; return true; }
    return false;
  }

  // Whether the current position could start an implicit-multiplication
  // operand, e.g. the "x" in "2x" or the "(" in "2(x+1)" or the "s" in "2sin(x)".
  bool atImplicitOperand() {
    char c = peek();
    return c == '(' || isAsciiAlpha(c);
  }

  ExprNode* makeNum(double v) {
    ExprNode* n = new ExprNode(NodeType::NUM);
    n->value = v;
    return n;
  }

  ExprNode* parseExpr() {
    ExprNode* node = parseTerm();
    while (ok) {
      char c = peek();
      if (c == '+') {
        pos++;
        ExprNode* rhs = parseTerm();
        ExprNode* n = new ExprNode(NodeType::ADD);
        n->left = node; n->right = rhs;
        node = n;
      } else if (c == '-') {
        pos++;
        ExprNode* rhs = parseTerm();
        ExprNode* n = new ExprNode(NodeType::SUB);
        n->left = node; n->right = rhs;
        node = n;
      } else {
        break;
      }
    }
    return node;
  }

  ExprNode* parseTerm() {
    ExprNode* node = parseUnary();
    while (ok) {
      char c = peek();
      if (c == '*') {
        pos++;
        ExprNode* rhs = parseUnary();
        ExprNode* n = new ExprNode(NodeType::MUL);
        n->left = node; n->right = rhs;
        node = n;
      } else if (c == '/') {
        pos++;
        ExprNode* rhs = parseUnary();
        ExprNode* n = new ExprNode(NodeType::DIV);
        n->left = node; n->right = rhs;
        node = n;
      } else if (atImplicitOperand()) {
        ExprNode* rhs = parseUnary();
        ExprNode* n = new ExprNode(NodeType::MUL);
        n->left = node; n->right = rhs;
        node = n;
      } else {
        break;
      }
    }
    return node;
  }

  ExprNode* parseUnary() {
    char c = peek();
    if (c == '-') {
      pos++;
      ExprNode* n = new ExprNode(NodeType::NEG);
      n->left = parseUnary();
      return n;
    }
    if (c == '+') {
      pos++;
      return parseUnary();
    }
    return parsePower();
  }

  ExprNode* parsePower() {
    ExprNode* node = parsePrimary();
    if (ok && peek() == '^') {
      pos++;
      ExprNode* rhs = parseUnary();  // allows things like 2^-1
      ExprNode* n = new ExprNode(NodeType::POW);
      n->left = node; n->right = rhs;
      node = n;
    }
    return node;
  }

  ExprNode* parsePrimary() {
    char c = peek();
    if (c == '\0') {
      fail("unexpected end of expression");
      return makeNum(0);
    }
    if (c == '(') {
      pos++;
      ExprNode* inner = parseExpr();
      if (!match(')')) fail("missing closing ')'");
      return inner;
    }
    if (isAsciiDigit(c) || c == '.') {
      return parseNumber();
    }
    if (isAsciiAlpha(c)) {
      return parseIdentifier();
    }
    fail(std::string("unexpected character '") + c + "'");
    pos++;  // consume it so we don't spin forever
    return makeNum(0);
  }

  ExprNode* parseNumber() {
    int start = pos;
    bool seenDot = false;
    while (pos < len && (isAsciiDigit(s[pos]) || (s[pos] == '.' && !seenDot))) {
      if (s[pos] == '.') seenDot = true;
      pos++;
    }
    std::string numStr = s.substr(start, pos - start);
    return makeNum(std::atof(numStr.c_str()));
  }

  ExprNode* parseIdentifier() {
    int start = pos;
    while (pos < len && isAsciiAlpha(s[pos])) pos++;
    std::string ident = toLower(s.substr(start, pos - start));

    if (ident == "x") return new ExprNode(NodeType::VAR);
    if (ident == "pi") return makeNum(3.14159265358979323846);
    if (ident == "e")  return makeNum(2.71828182845904523536);

    FuncType f = FuncType::NONE;
    if (ident == "sin")       f = FuncType::SIN;
    else if (ident == "cos")  f = FuncType::COS;
    else if (ident == "tan")  f = FuncType::TAN;
    else if (ident == "sqrt") f = FuncType::SQRT;
    else if (ident == "abs")  f = FuncType::ABS;
    else if (ident == "log")  f = FuncType::LOG;
    else if (ident == "ln")   f = FuncType::LN;
    else if (ident == "exp")  f = FuncType::EXP;
    else {
      fail("unknown identifier '" + ident + "'");
      return makeNum(0);
    }

    if (!match('(')) {
      fail("expected '(' after '" + ident + "'");
      return makeNum(0);
    }
    ExprNode* arg = parseExpr();
    if (!match(')')) fail("missing closing ')'");

    ExprNode* n = new ExprNode(NodeType::FUNC);
    n->func = f;
    n->left = arg;
    return n;
  }
};

void freeNode(ExprNode* node) {
  if (!node) return;
  freeNode(node->left);
  freeNode(node->right);
  delete node;
}

double evalNode(const ExprNode* node, double x) {
  if (!node) return NAN;
  switch (node->type) {
    case NodeType::NUM: return node->value;
    case NodeType::VAR: return x;
    case NodeType::ADD: return evalNode(node->left, x) + evalNode(node->right, x);
    case NodeType::SUB: return evalNode(node->left, x) - evalNode(node->right, x);
    case NodeType::MUL: return evalNode(node->left, x) * evalNode(node->right, x);
    case NodeType::DIV: {
      double denom = evalNode(node->right, x);
      if (denom == 0.0) return NAN;
      return evalNode(node->left, x) / denom;
    }
    case NodeType::POW:
      return std::pow(evalNode(node->left, x), evalNode(node->right, x));
    case NodeType::NEG:
      return -evalNode(node->left, x);
    case NodeType::FUNC: {
      double a = evalNode(node->left, x);
      switch (node->func) {
        case FuncType::SIN:  return std::sin(a);
        case FuncType::COS:  return std::cos(a);
        case FuncType::TAN:  return std::tan(a);
        case FuncType::SQRT: return (a < 0.0) ? NAN : std::sqrt(a);
        case FuncType::ABS:  return std::fabs(a);
        case FuncType::LOG:  return (a <= 0.0) ? NAN : std::log10(a);
        case FuncType::LN:   return (a <= 0.0) ? NAN : std::log(a);
        case FuncType::EXP:  return std::exp(a);
        default: return NAN;
      }
    }
  }
  return NAN;
}

}  // namespace

Expression parseExpression(const std::string& input) {
  Expression result;

  std::string cleaned = trimmed(input);
  if (cleaned.empty()) {
    result.valid = false;
    result.errorMessage = "empty expression";
    return result;
  }

  Parser parser(cleaned);
  ExprNode* root = parser.parse();

  if (!parser.isOk()) {
    freeNode(root);
    result.root = nullptr;
    result.valid = false;
    result.errorMessage = parser.error();
    return result;
  }

  result.root = root;
  result.valid = true;
  return result;
}

double evaluateExpression(const Expression& expr, double x) {
  if (!expr.valid || expr.root == nullptr) return NAN;
  return evalNode(expr.root, x);
}

void freeExpression(Expression& expr) {
  freeNode(expr.root);
  expr.root = nullptr;
  expr.valid = false;
}

}  // namespace core
