/*
 * Cardputer Graphing Calculator
 * ------------------------------
 * A simple y = f(x) graphing calculator for the M5Stack Cardputer / Cardputer ADV.
 *
 * Type a math expression in terms of x using the keyboard, press Enter,
 * and the graph updates live. Use Fn + keys to pan/zoom/reset the view.
 *
 * Controls:
 *   letters/digits/operators   -> edit the expression
 *   Enter                      -> re-plot (also happens automatically as you type)
 *   Del/Backspace               -> delete last character
 *   Fn + Del                    -> clear the whole expression
 *   Fn + ,                      -> pan left
 *   Fn + /                      -> pan right
 *   Fn + ;                      -> pan up
 *   Fn + .                      -> pan down
 *   Fn + =                      -> zoom in
 *   Fn + -                      -> zoom out
 *   Fn + r                      -> reset view
 *
 * Supported syntax:
 *   + - * / ^ ( )  , x  pi  e
 *   sin cos tan asin acos atan sqrt abs log ln exp floor ceil pow(a,b)
 */

#include <M5Cardputer.h>
#include <cmath>
#include <string>

// ---------------------------------------------------------------------------
// Expression parser / evaluator (recursive descent)
// ---------------------------------------------------------------------------
struct ExprEval {
    const char* p;
    double xval;
    bool error;

    void skipSpaces() {
        while (*p == ' ') p++;
    }

    double parseExpression() {
        double v = parseTerm();
        skipSpaces();
        while (!error && (*p == '+' || *p == '-')) {
            char op = *p++;
            double rhs = parseTerm();
            v = (op == '+') ? v + rhs : v - rhs;
            skipSpaces();
        }
        return v;
    }

    double parseTerm() {
        double v = parseUnary();
        skipSpaces();
        while (!error && (*p == '*' || *p == '/')) {
            char op = *p++;
            double rhs = parseUnary();
            if (op == '*') {
                v = v * rhs;
            } else {
                if (rhs == 0.0) {
                    error = true;
                    v = 0.0;
                } else {
                    v = v / rhs;
                }
            }
            skipSpaces();
        }
        return v;
    }

    double parseUnary() {
        skipSpaces();
        if (*p == '-') {
            p++;
            return -parseUnary();
        }
        if (*p == '+') {
            p++;
            return parseUnary();
        }
        return parsePower();
    }

    double parsePower() {
        double base = parsePrimary();
        skipSpaces();
        if (!error && *p == '^') {
            p++;
            double exponent = parseUnary();
            return pow(base, exponent);
        }
        return base;
    }

    double parsePrimary() {
        skipSpaces();
        if (error || *p == '\0') {
            error = true;
            return 0.0;
        }

        if (*p == '(') {
            p++;
            double v = parseExpression();
            skipSpaces();
            if (*p == ')') {
                p++;
            } else {
                error = true;
            }
            return v;
        }

        if (isdigit((unsigned char)*p) || *p == '.') {
            char* end = nullptr;
            double v = strtod(p, &end);
            if (end == p) {
                error = true;
                return 0.0;
            }
            p = end;
            return v;
        }

        if (isalpha((unsigned char)*p)) {
            std::string ident;
            while (isalpha((unsigned char)*p)) {
                ident += *p;
                p++;
            }
            skipSpaces();

            if (ident == "x") return xval;
            if (ident == "pi") return M_PI;
            if (ident == "e") return M_E;

            if (*p == '(') {
                p++;
                double a = parseExpression();
                double b = 0.0;
                bool twoArgs = false;
                skipSpaces();
                if (*p == ',') {
                    p++;
                    b = parseExpression();
                    twoArgs = true;
                    skipSpaces();
                }
                if (*p == ')') {
                    p++;
                } else {
                    error = true;
                }

                if (ident == "sin") return sin(a);
                if (ident == "cos") return cos(a);
                if (ident == "tan") return tan(a);
                if (ident == "asin") return asin(a);
                if (ident == "acos") return acos(a);
                if (ident == "atan") return atan(a);
                if (ident == "sqrt") return a < 0 ? (error = true, 0.0) : sqrt(a);
                if (ident == "abs") return fabs(a);
                if (ident == "log") return a <= 0 ? (error = true, 0.0) : log10(a);
                if (ident == "ln") return a <= 0 ? (error = true, 0.0) : log(a);
                if (ident == "exp") return exp(a);
                if (ident == "floor") return floor(a);
                if (ident == "ceil") return ceil(a);
                if (ident == "pow" && twoArgs) return pow(a, b);

                error = true;
                return 0.0;
            }

            error = true;
            return 0.0;
        }

        error = true;
        return 0.0;
    }
};

// Evaluate exprStr at a given x. Returns true on success via ok.
static double evaluate(const std::string& exprStr, double xval, bool& ok) {
    ExprEval ev;
    ev.p = exprStr.c_str();
    ev.xval = xval;
    ev.error = false;
    double v = ev.parseExpression();
    ev.skipSpaces();
    if (*ev.p != '\0') ev.error = true;
    ok = !ev.error && std::isfinite(v);
    return v;
}

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------
static std::string expr = "sin(x)";

struct View {
    double xmin = -10.0, xmax = 10.0;
    double ymin = -5.0, ymax = 5.0;
} view;

static const View kDefaultView;

// Layout: graph occupies the top portion of the 240x135 panel, the bottom
// strip is reserved for the expression input / status line.
static const int SCREEN_W = 240;
static const int SCREEN_H = 135;
static const int GRAPH_W = 240;
static const int GRAPH_H = 116;
static const int INPUT_Y = GRAPH_H;
static const int INPUT_H = SCREEN_H - GRAPH_H;

static M5Canvas graphSprite(&M5Cardputer.Display);

// ---------------------------------------------------------------------------
// View manipulation helpers
// ---------------------------------------------------------------------------
static void panX(double dir) {
    double range = view.xmax - view.xmin;
    double step = range * 0.15 * dir;
    view.xmin += step;
    view.xmax += step;
}

static void panY(double dir) {
    double range = view.ymax - view.ymin;
    double step = range * 0.15 * dir;
    view.ymin += step;
    view.ymax += step;
}

static void zoom(double factor) {
    double cx = (view.xmin + view.xmax) * 0.5;
    double cy = (view.ymin + view.ymax) * 0.5;
    double hw = (view.xmax - view.xmin) * 0.5 * factor;
    double hh = (view.ymax - view.ymin) * 0.5 * factor;
    view.xmin = cx - hw;
    view.xmax = cx + hw;
    view.ymin = cy - hh;
    view.ymax = cy + hh;
}

static void resetView() {
    view = kDefaultView;
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
static int xToPx(double x) {
    return (int)round((x - view.xmin) / (view.xmax - view.xmin) * (GRAPH_W - 1));
}

static int yToPy(double y) {
    return (int)round((GRAPH_H - 1) - (y - view.ymin) / (view.ymax - view.ymin) * (GRAPH_H - 1));
}

static void drawGrid() {
    graphSprite.fillSprite(TFT_BLACK);

    // Axes (x = 0 and y = 0), only drawn if within the current view.
    if (view.xmin <= 0.0 && view.xmax >= 0.0) {
        int px = xToPx(0.0);
        graphSprite.drawFastVLine(px, 0, GRAPH_H, TFT_DARKGREY);
    }
    if (view.ymin <= 0.0 && view.ymax >= 0.0) {
        int py = yToPy(0.0);
        graphSprite.drawFastHLine(0, py, GRAPH_W, TFT_DARKGREY);
    }
}

static bool exprHasError = false;

static void drawCurve() {
    bool prevValid = false;
    int prevPx = 0, prevPy = 0;

    for (int px = 0; px < GRAPH_W; px++) {
        double x = view.xmin + (double)px / (GRAPH_W - 1) * (view.xmax - view.xmin);
        bool ok = false;
        double y = evaluate(expr, x, ok);

        if (px == 0) exprHasError = !ok && expr.length() > 0;

        if (!ok) {
            prevValid = false;
            continue;
        }

        int py = yToPy(y);

        // Clamp for drawing purposes, but detect huge jumps (asymptotes)
        // and avoid connecting across them.
        bool onScreen = py > -GRAPH_H && py < 2 * GRAPH_H;
        int pyClamped = py;
        if (pyClamped < -GRAPH_H) pyClamped = -GRAPH_H;
        if (pyClamped > 2 * GRAPH_H) pyClamped = 2 * GRAPH_H;

        if (onScreen) {
            if (prevValid && abs(pyClamped - prevPy) < GRAPH_H * 2) {
                graphSprite.drawLine(prevPx, prevPy, px, pyClamped, TFT_GREEN);
            } else {
                graphSprite.drawPixel(px, pyClamped, TFT_GREEN);
            }
            prevPx = px;
            prevPy = pyClamped;
            prevValid = true;
        } else {
            prevValid = false;
        }
    }
}

static void drawAxisLabels() {
    graphSprite.setTextColor(TFT_DARKGREY);
    graphSprite.setTextSize(1);
    graphSprite.setCursor(2, 1);
    graphSprite.printf("%.2g", view.ymax);
    graphSprite.setCursor(2, GRAPH_H - 9);
    graphSprite.printf("%.2g", view.ymin);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.2g", view.xmin);
    graphSprite.setCursor(1, GRAPH_H / 2 + 2);
    graphSprite.print(buf);

    snprintf(buf, sizeof(buf), "%.2g", view.xmax);
    int w = graphSprite.textWidth(buf);
    graphSprite.setCursor(GRAPH_W - w - 1, GRAPH_H / 2 + 2);
    graphSprite.print(buf);
}

static void drawInputBar() {
    auto& d = M5Cardputer.Display;
    d.fillRect(0, INPUT_Y, SCREEN_W, INPUT_H, TFT_NAVY);
    d.setTextSize(1);
    d.setTextColor(exprHasError ? TFT_RED : TFT_WHITE, TFT_NAVY);
    d.setCursor(4, INPUT_Y + 4);
    d.print("y=");
    d.print(expr.c_str());
    if (!exprHasError) {
        d.print("_");  // cursor
    } else {
        d.setTextColor(TFT_RED, TFT_NAVY);
        d.print("  ERR");
    }
}

static void render() {
    drawGrid();
    drawCurve();
    drawAxisLabels();
    graphSprite.pushSprite(0, 0);
    drawInputBar();
}

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------
void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(80);
    M5Cardputer.Display.fillScreen(TFT_BLACK);

    graphSprite.createSprite(GRAPH_W, GRAPH_H);
    graphSprite.setTextFont(1);

    render();
}

void loop() {
    M5Cardputer.update();

    bool dirty = false;

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto status = M5Cardputer.Keyboard.keysState();

        if (status.fn) {
            // Modifier-driven view controls.
            for (char c : status.word) {
                switch (c) {
                    case ',': panX(-1.0); dirty = true; break;
                    case '/': panX(1.0); dirty = true; break;
                    case ';': panY(1.0); dirty = true; break;
                    case '.': panY(-1.0); dirty = true; break;
                    case '=': zoom(0.8); dirty = true; break;
                    case '-': zoom(1.25); dirty = true; break;
                    case 'r': resetView(); dirty = true; break;
                    default: break;
                }
            }
            if (status.del) {
                expr.clear();
                dirty = true;
            }
        } else {
            for (char c : status.word) {
                if (expr.length() < 64) {
                    expr += c;
                    dirty = true;
                }
            }
            if (status.del && !expr.empty()) {
                expr.pop_back();
                dirty = true;
            }
            if (status.enter) {
                dirty = true;  // explicit re-plot request
            }
        }
    }

    if (dirty) {
        render();
    }

    delay(10);
}
