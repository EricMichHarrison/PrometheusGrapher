// Inspired by voidos by avascik
// Author: SupremeEgg75
// https://github.com/personwithbeans/PrometheusGrapher
// Liscense: AGPL-3.0
//
// === MIGRATION NOTES (see README.md "Migrating an existing project") ===
// This file is the result of migrating the original single src/main.cpp
// onto the core/hal/ui/test/sim architecture:
//
//   - Equation storage, selection, and pan position — previously the
//     loose globals `lineEntries[]`, `selEquation`, `posx`/`posy` — now
//     live in core::GrapherState (lib/core/GrapherState.h), which has
//     zero hardware includes and is covered by native tests.
//   - ExpressionParser moved to lib/core/ExpressionParser (same
//     algorithm, Arduino String -> std::string) for the same reason.
//   - DrawGraphPage() now renders through core::Framebuffer +
//     ui::renderGraph() (lib/ui/GraphRenderer) instead of drawing
//     directly to M5Cardputer.Display. This is also where the curve
//     actually gets plotted now — the original left that as a TODO
//     ("PUT EQUATION GRAPHING DRAWER HERE") and only showed the grid.
//   - FunctionMenu(), SelectEquationSlot() (renamed from
//     SelectExpression(), see below), and helpScreen() intentionally
//     were NOT migrated to core/ui — they're menu/list screens with no
//     interesting logic to unit-test, so the risk/reward of rewriting
//     their working M5GFX drawing code wasn't worth it for this pass.
//     They now read/write equation text through GrapherState instead
//     of touching lineEntries[] directly, but otherwise work the same
//     as before.
//   - SelectExpression() used to return the equation *text* and let
//     DrawGraphPage() re-parse it. It now returns the chosen *slot
//     index* (or -1 for cancel) and DrawGraphPage() calls
//     state.graphEquation(slotIndex), since GrapherState already owns
//     parsing — that's a simplification, not a behavior change.
//   - NOT verified against real hardware in this sandbox (no network,
//     no ESP32 toolchain here) — same caveat as DisplayM5.h/KeyboardM5.h.
//     What *is* verified: lib/core + lib/ui compiled and ran correctly
//     under plain g++, including a real plotted sine wave, a parabola,
//     and a domain-error gap (sqrt(x) for x<0) all rendering exactly as
//     expected — see README.md.

#include "ExpressionParser.h"
#include "GrapherState.h"
#include "Framebuffer.h"
#include "GraphRenderer.h"
#include "DisplayM5.h"
#include "Icons.c"
#include <M5Cardputer.h>
#include <algorithm>
#include <string>

extern uint16_t SystemColour;
extern uint16_t BackgroundColour;
extern uint16_t StepColour;
extern bool instantBoot;
extern String versionNum;

// debounce delay
const unsigned long debounceDelay = 20;
unsigned long lastDebounceTime = 0;

// Screen Distance Constants
int cx = 0;
int cy = 0;
int maxx = 0;
int maxy = 0;
int posStep = 5;  // controls how fast the screen moves in graph view

const int lines = 4;

core::GrapherState grapherState;  // replaces lineEntries[]/selEquation/posx/posy
device::DisplayM5* g_display = nullptr;

// Forward declarations for functions defined later in this file
void DrawGraphPage();
void FunctionMenu();
int SelectEquationSlot();  // returns 0-based slot index, or -1 if cancelled
void helpScreen();

void setup() {
    M5Cardputer.begin();
    M5Cardputer.Display.setBrightness(10);  // default brightness for now, if I put in a settings menu or add aditional controls this will customizable.
    cx = M5Cardputer.Display.width() / 2;
    cy = M5Cardputer.Display.height() / 2;
    maxx = M5Cardputer.Display.width();
    maxy = M5Cardputer.Display.height();
    g_display = new device::DisplayM5(maxx, maxy);
    Serial.begin(115200);
    unsigned long serialWaitStart = millis();
    while (!Serial && (millis() - serialWaitStart < 2000)) {
        delay(10);
    }
    M5Cardputer.Display.setBrightness(100);
    Serial.print("=== STARTING ===\n");
    M5Cardputer.Display.setTextSize(2);
}

void loop() {  // main loop, the code literly flip flops between these funcitons
    DrawGraphPage();
    FunctionMenu();
}

void FunctionMenu() {  // === Either disable editing of currently drawn functions or redraw after they have been edited.
    M5Cardputer.Display.fillScreen(SystemColour);

    int lineSelection = 1;
    int lineStep = maxy / lines;  // distance between drawn lines in screen gride cordinates
    bool screenUpdate = true;
    bool selected = false;

    while (true) {
        M5Cardputer.update();
        if (screenUpdate) {
            M5Cardputer.Display.clear();
            for (int i = 1; i <= lines; i++) {
                M5Cardputer.Display.drawLine(0, lineStep * i, maxx, lineStep * i, SystemColour);
                M5Cardputer.Display.drawString(String(i) + ':', 18, (lineStep * i) - (lineStep / 2));
                M5Cardputer.Display.drawString(String(grapherState.equationSlot(i - 1).c_str()), 44, (lineStep * i) - (lineStep / 2));
            }
            if (selected) {  // filled triangle
                M5Cardputer.Display.fillTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            } else {  // empty triangle
                M5Cardputer.Display.drawTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            }
            if (grapherState.selectedEquation() >= 0) {  // gives green checkmark if equation is alredy drawn
                M5Cardputer.Display.fillTriangle(10, (lineStep * grapherState.selectedEquation()) - (lineStep / 2), 4, (lineStep * grapherState.selectedEquation()) - (lineStep / 2) + 4, 4, (lineStep * grapherState.selectedEquation()) - (lineStep / 2) - 4, GREEN);
            }
            screenUpdate = false;
        }

        if (M5Cardputer.Keyboard.isChange()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastDebounceTime >= debounceDelay) {  // prevent acidental double click

                if (selected) {  // typing mode
                    std::string text = grapherState.equationSlot(lineSelection - 1);
                    if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                        if (!text.empty()) text.pop_back();
                        grapherState.setEquationSlot(lineSelection - 1, text);
                    } else if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                        selected = false;
                    } else {
                        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                        for (auto i : status.word) {
                            text += i;
                        }
                        grapherState.setEquationSlot(lineSelection - 1, text);
                    }
                } else {  // selection mode
                    if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                        return;
                    } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                        selected = true;
                        // Select line and begin typing
                    } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                        if (lineSelection > 1) {
                            lineSelection--;
                        }
                    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                        if (lineSelection < 4) {
                            lineSelection++;
                        }
                    }
                }
                lastDebounceTime = currentMillis;
            }
            screenUpdate = true;
        }
    }

    return;
}

void DrawGraphPage() {
    M5Cardputer.Display.setBaseColor(BLACK);

    core::Framebuffer fb(maxx, maxy);
    ui::GraphColors colors;
    colors.axis = SystemColour;
    colors.grid = StepColour;
    colors.background = BackgroundColour;
    // colors.curve keeps its default (green) — see plotColours[] in
    // SettingsList.h for per-slot colours if you want to wire those in.

    // Track what we last rendered so we only redraw on an actual
    // change, same intent as the original's oldPosX/oldPosY check —
    // just without the goto-based "Clearing..." transitional redraw,
    // since ui::renderGraph always draws a complete frame from scratch
    // (there's no partial/incremental state to get out of sync).
    int lastPanX = grapherState.panX() + 1;  // force a redraw on entry
    int lastPanY = grapherState.panY();
    int lastSelected = grapherState.selectedEquation() - 1;  // force redraw

    while (true) {
        M5Cardputer.update();

        bool changed = (lastPanX != grapherState.panX() ||
                        lastPanY != grapherState.panY() ||
                        lastSelected != grapherState.selectedEquation());
        if (changed) {
            ui::renderGraph(grapherState, fb, colors);
            g_display->push(fb);
            lastPanX = grapherState.panX();
            lastPanY = grapherState.panY();
            lastSelected = grapherState.selectedEquation();
        }

        if (M5Cardputer.Keyboard.isKeyPressed(',')) {  // Left
            grapherState.panBy(-posStep, 0);
        } else if (M5Cardputer.Keyboard.isKeyPressed('/')) {  // Right
            grapherState.panBy(posStep, 0);
        } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {  // Down
            grapherState.panBy(0, -posStep);
        } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {  // Up
            grapherState.panBy(0, posStep);
        } else if (M5Cardputer.Keyboard.isKeyPressed('c')) {  // center position
            grapherState.resetPan();
        } else if (M5Cardputer.Keyboard.isKeyPressed('g')) {  // select function to graph, if one is alredy selected deletes it and redraws grid empty
            if (grapherState.selectedEquation() != -1) {
                Serial.print("Clearing expression...");
                grapherState.clearSelection();
            } else {
                Serial.print("\ng Pressed, Entering Seleciton Screen...");
                int slot = SelectEquationSlot();
                if (slot == -1) {
                    Serial.print("\ncancelled...");
                    M5Cardputer.Display.clear();
                    M5Cardputer.Display.drawCenterString("Cancelled...", cx, cy);
                    delay(1000);
                } else {
                    Serial.print("\nslot selected, parsing equation..");
                    if (!grapherState.graphEquation(slot)) {
                        M5Cardputer.Display.clear();
                        M5Cardputer.Display.drawCenterString("Parse error:", cx, cy - 10);
                        M5Cardputer.Display.drawCenterString(String(grapherState.lastParseError().c_str()), cx, cy + 10);
                        delay(1500);
                    }
                }
                // force a redraw next loop regardless of outcome, same
                // as the original's `goto redrawGraph`
                lastSelected = grapherState.selectedEquation() - 1;
            }
        } else if (M5Cardputer.Keyboard.isKeyPressed('h')) {
            helpScreen();
        } else if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                M5Cardputer.Display.clear();
                delay(100);
                return;
            }
        }
    }
}

int SelectEquationSlot() {
    M5Cardputer.Display.fillScreen(SystemColour);
    M5Cardputer.Display.setTextSize(2);

    int lineSelection = 1;
    int lineStep = maxy / lines;  // distance between listings
    bool screenUpdate = true;
    bool selected = false;

    while (true) {
        M5Cardputer.update();
        if (screenUpdate) {
            M5Cardputer.Display.clear();
            for (int i = 1; i <= lines; i++) {
                M5Cardputer.Display.drawLine(0, lineStep * i, maxx, lineStep * i, SystemColour);
                M5Cardputer.Display.drawString(String(i) + ':', 18, (lineStep * i) - (lineStep / 2));
                M5Cardputer.Display.drawString(String(grapherState.equationSlot(i - 1).c_str()), 44, (lineStep * i) - (lineStep / 2));
            }
            if (selected) {  // filled triangle
                M5Cardputer.Display.fillTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            } else {  // empty triangle
                M5Cardputer.Display.drawTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            }
            if (grapherState.selectedEquation() >= 0) {  // gives green checkmark if equation is alredy drawn
                M5Cardputer.Display.fillTriangle(10, (lineStep * grapherState.selectedEquation()) - (lineStep / 2), 4, (lineStep * grapherState.selectedEquation()) - (lineStep / 2) + 4, 4, (lineStep * grapherState.selectedEquation()) - (lineStep / 2) - 4, GREEN);
            }
            screenUpdate = false;
        }

        if (M5Cardputer.Keyboard.isChange()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastDebounceTime >= debounceDelay) {  // prevent acidental double click
                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    return -1;
                } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    return lineSelection - 1;  // 0-based slot index
                } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
                    if (lineSelection > 1) {
                        lineSelection--;
                    }
                } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
                    if (lineSelection < 4) {
                        lineSelection++;
                    }
                }
            }
            lastDebounceTime = currentMillis;
            screenUpdate = true;
        }
    }
}

void helpScreen() {  // for anyone that wants it will also be displayed on the cardputer' screen but condensed info
    int numLines = 5;
    int distBetwTxt = (M5Cardputer.Display.height() - 20) / numLines;
    const char* helpText[numLines] = {
        "Arrow keys to move",
        "c to reset view",
        "Esc to go back",
        "g to graph equaiton",
        "serial 4 xtra deets"};
    M5.Display.clear();
    // the following serial is just if anyone wants more details info
    delay(100);
    Serial.println("=== HELP MENU ===");  // same info will apear on screen but condensed.
    Serial.println("Arrowkeys to move [No need for holding the function key].");
    Serial.println("Escape to switch betweeen funciton editing and graphing.");
    Serial.println("When on the Graphing Screen press c to reset view to (0,0) 'center'.");
    Serial.println("When on the Graphing screen press g to select equation(s) to plot.");
    Serial.println("if in any menu you don't want to be in press esc to cancel/back out.");
    delay(100);
    Serial.println("\n=== GENERAL INFO ===");
    Serial.println("graph with automatically refresh on each change of position so there shouldn't be any need to manualy refresh or update");
    delay(100);
    Serial.println("\n== ABOUT ===");
    Serial.println("Prometheus Grapher" + (String)versionNum + "Inspired by voidos by avascik\nAuthor: SupremeEgg75\nhttps://github.com/personwithbeans/PrometheusGrapher\nString to equation parser written by Claude (shamfully)");
    for (int i = 0; i < numLines; i++) {  // yeah a for loop whats it to you, I like em.
        M5Cardputer.Display.drawString(helpText[i], 10, (distBetwTxt * i) + 10);
    }
}
