#include "ExpressionParser.h"
#include <math.h>

namespace {

// Grammar (lowest to highest precedence):
//   expr    := term (('+' | '-') term)*
//   term    := unary (('*' | '/') unary | implicitUnary)*
//   unary   := ('-' | '+')? power
//   power   := primary ('^' unary)?      // right-associative, rhs allows unary
//   primary := number | 'x' | constant | funcName '(' expr ')' | '(' expr ')'
//
// Unary minus is deliberately placed ABOVE '^' in the call chain (unary
// calls power) so that "-x^2" parses as -(x^2), matching standard math
// notation rather than (-x)^2.

class Parser {
public:
    explicit Parser(const String& src) : s(src), pos(0), len(src.length()) {}

    ExprNode* parse() {
        ExprNode* result = parseExpr();
        skipSpaces();
        if (ok && pos != len) {
            fail("unexpected trailing characters");
        }
        return result;
    }

    bool isOk() const { return ok; }
    String error() const { return errMsg; }

private:
    String s;
    int pos;
    int len;
    bool ok = true;
    String errMsg = "";

    void fail(const String& msg) {
        if (ok) { // keep the first error, it's usually the most useful
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
        return c == '(' || isAlpha(c);
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
            ExprNode* rhs = parseUnary(); // allows things like 2^-1
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
        if (isDigit(c) || c == '.') {
            return parseNumber();
        }
        if (isAlpha(c)) {
            return parseIdentifier();
        }
        fail(String("unexpected character '") + c + "'");
        pos++; // consume it so we don't spin forever
        return makeNum(0);
    }

    ExprNode* parseNumber() {
        int start = pos;
        bool seenDot = false;
        while (pos < len && (isDigit(s[pos]) || (s[pos] == '.' && !seenDot))) {
            if (s[pos] == '.') seenDot = true;
            pos++;
        }
        String numStr = s.substring(start, pos);
        return makeNum(numStr.toDouble());
    }

    ExprNode* parseIdentifier() {
        int start = pos;
        while (pos < len && isAlpha(s[pos])) pos++;
        String ident = s.substring(start, pos);
        ident.toLowerCase();

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
            return pow(evalNode(node->left, x), evalNode(node->right, x));
        case NodeType::NEG:
            return -evalNode(node->left, x);
        case NodeType::FUNC: {
            double a = evalNode(node->left, x);
            switch (node->func) {
                case FuncType::SIN:  return sin(a);
                case FuncType::COS:  return cos(a);
                case FuncType::TAN:  return tan(a);
                case FuncType::SQRT: return (a < 0.0) ? NAN : sqrt(a);
                case FuncType::ABS:  return fabs(a);
                case FuncType::LOG:  return (a <= 0.0) ? NAN : log10(a);
                case FuncType::LN:   return (a <= 0.0) ? NAN : log(a);
                case FuncType::EXP:  return exp(a);
                default: return NAN;
            }
        }
    }
    return NAN;
}

} // namespace

Expression parseExpression(const String& input) {
    Expression result;

    String cleaned = input;
    cleaned.trim();
    if (cleaned.length() == 0) {
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
    return;
}
