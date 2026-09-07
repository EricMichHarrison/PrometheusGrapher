// Inspired by voidos by avascik
// Author: SupremeEgg75
// https://github.com/personwithbeans/PrometheusGrapher
// Liscense: AGPL-3.0

//=== TO DO ===
// -Add scrool speed adjustment
// -Add asymptote graphing support
// -multi-equation graphing
// -Add ability to position the camera at a specific cordinate
// -Add zoom and axis scaling (scaling might be tough)
// -Add boot/SplashScreen
// -Add Settings menu
// -Add custom plot colours(maybe even have a special rainbow mode)

#include "ExpressionParser.h"
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
int lineStepCount = 20; // distance between marker lines

// Screen Distance Constants
int cx = 0;
int cy = 0;
int maxx = 0;
int maxy = 0;
int posStep = 5; // controls how fast the screen moves in graph view

int selEquation = -1; // -1 is default when nothing is selected
const int lines = 4;
String lineEntries[lines] = {"x^2", "test", "test", "test"};
String tmp = "";
// Forward declarations for functions defined later in this file
void DrawGraphPage();
void FunctionMenu();
String SelectExpression();
void helpScreen();

void setup() {
    M5Cardputer.begin();
    M5Cardputer.Display.setBrightness(10); // default brightness for now, if I put in a settings menu or add aditional controls this will customizable.
    cx = M5Cardputer.Display.width() / 2;
    cy = M5Cardputer.Display.height() / 2;
    maxx = M5Cardputer.Display.width();
    maxy = M5Cardputer.Display.height();
    Serial.begin(115200);
    unsigned long serialWaitStart = millis();
    while (!Serial && (millis() - serialWaitStart < 2000)) {
        delay(10);
    }
    M5Cardputer.Display.setBrightness(100);
    Serial.print("=== STARTING ===\n");
    M5Cardputer.Display.setTextSize(2);
}

void loop() {
    DrawGraphPage();
    FunctionMenu();
}

void FunctionMenu() { // === Either disable editing of currently drawn functions or redraw after they have been edited.
    M5Cardputer.Display.fillScreen(SystemColour);

    int lineSelection = 1;
    int lineStep = maxy / lines; // distance between drawn lines in screen gride cordinates
    bool screenUpdate = true;
    bool selected = false;

    while (true) {
        M5Cardputer.update();
        if (screenUpdate) {
            M5Cardputer.Display.clear();
            for (int i = 1; i <= lines; i++) {
                M5Cardputer.Display.drawLine(0, lineStep * i, maxx, lineStep * i, SystemColour);
                M5Cardputer.Display.drawString(String(i) + ':', 18, (lineStep * i) - (lineStep / 2));
                M5Cardputer.Display.drawString(lineEntries[i - 1], 44, (lineStep * i) - (lineStep / 2));
            }
            if (selected) { // filled triangle
                M5Cardputer.Display.fillTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            } else { // empty triangle
                M5Cardputer.Display.drawTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            }
            if (selEquation >= 0) { // gives green checkmark if equation is alredy drawn
                M5Cardputer.Display.fillTriangle(10, (lineStep * selEquation) - (lineStep / 2), 4, (lineStep * selEquation) - (lineStep / 2) + 4, 4, (lineStep * selEquation) - (lineStep / 2) - 4, GREEN);
            }
            screenUpdate = false;
        }

        if (M5Cardputer.Keyboard.isChange()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastDebounceTime >= debounceDelay) { // prevent acidental double click

                if (selected) { // typing mode
                    if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                        if (lineEntries[lineSelection - 1].length() > 0) {
                            lineEntries[lineSelection - 1].remove(lineEntries[lineSelection - 1].length() - 1);
                        }
                    } else if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                        selected = false;
                    } else {
                        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                        for (auto i : status.word) {
                            lineEntries[lineSelection - 1] += i;
                        }
                    }
                } else { // selection mode
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
    // view variables
    int posx = 0;
    int posy = 0;
    int oldPosX = 100;
    int oldPosY = 100;
    Expression parsedExpression;

    M5Cardputer.Display.setBaseColor(BLACK);
    while (true) {
        M5Cardputer.update();
        if (oldPosX != posx || oldPosY != posy) {
            goto jumpover;
        redrawGraph:
            Serial.print("Graph Redraw...");
            M5Cardputer.Display.drawCenterString("Clearing...", cx, cy);
            delay(100);           // give time for g key to not register as pressed
            M5Cardputer.update(); // Update key states
        jumpover:
            M5Cardputer.Display.clear();
            M5Cardputer.Display.drawLine(cx - posx, 0, cx - posx, maxy, SystemColour);
            M5Cardputer.Display.drawLine(0, cy + posy, maxx, cy + posy, SystemColour);
            oldPosX = posx;
            oldPosY = posy;
            // im a probably a shity newbie coder so this is probably a shitty
            // way  to do this or at least unoptimized as fuck but it works
            for (int i = lineStepCount; i < maxx - posx;
                 i += lineStepCount) { // left
                M5Cardputer.Display.drawLine(cx - posx - i, 0, cx - posx - i, maxy, StepColour);
            }
            for (int i = lineStepCount; i < maxx + posx;
                 i += lineStepCount) { // right
                M5Cardputer.Display.drawLine(cx - posx + i, 0, cx - posx + i, maxy, StepColour);
            }
            for (int i = lineStepCount; i < maxy + posy;
                 i += lineStepCount) { // top
                M5Cardputer.Display.drawLine(0, cy + posy - i, maxx, cy + posy - i, StepColour);
            }
            for (int i = lineStepCount; i < maxy - posy;
                 i += lineStepCount) { // bottom
                M5Cardputer.Display.drawLine(0, cy + posy + i, maxx, cy + posy + i, StepColour);
            }
            // === PUT EQUATION GRAPHING DRAWER HERE ===
            if (selEquation != -1) {
                M5Cardputer.Display.drawString((String)selEquation, 8, 6);
            }
            delay(10);
        }
        if (M5Cardputer.Keyboard.isKeyPressed(',')) { // Left
            posx -= posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('/')) { // Right
            posx += posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('.')) { // Down
            posy -= posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed(';')) { // Up
            posy += posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('c')) { // center position
            posx = 0;
            posy = 0;
        } else if (M5Cardputer.Keyboard.isKeyPressed('g')) { // select function to graph, if one is alredy selected deletes it and redraws grid empty
            if (selEquation != -1) {
                Serial.print("Clearing expression...");
                freeExpression(parsedExpression); // clear mem
                selEquation = -1;
                M5Cardputer.Display.clear();
                delay(100);
                goto redrawGraph;
            }
            Serial.print("\ng Pressed, Entering Seleciton Screen...");
            String expressionString = SelectExpression();
            Serial.print("\n" + expressionString + " returned...");
            if (expressionString == "cancelled") {
                M5Cardputer.Display.clear();
                M5Cardputer.Display.drawCenterString("Cancelled...", cx, cy);
                delay(1000);
            } else {
                Serial.print("\n" + expressionString + " Selected, parsing equation..");
                parsedExpression = parseExpression(expressionString);
                // COMPLETE
            }
            goto redrawGraph;
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

String SelectExpression() {
    M5Cardputer.Display.fillScreen(SystemColour);
    M5Cardputer.Display.setTextSize(2);

    int lineSelection = 1;
    int lineStep = maxy / lines; // distance between listings
    bool screenUpdate = true;
    bool selected = false;

    while (true) {
        M5Cardputer.update();
        if (screenUpdate) {
            M5Cardputer.Display.clear();
            for (int i = 1; i <= lines; i++) {
                M5Cardputer.Display.drawLine(0, lineStep * i, maxx, lineStep * i, SystemColour);
                M5Cardputer.Display.drawString(String(i) + ':', 18, (lineStep * i) - (lineStep / 2));
                M5Cardputer.Display.drawString(lineEntries[i - 1], 44, (lineStep * i) - (lineStep / 2));
            }
            if (selected) { // filled triangle
                M5Cardputer.Display.fillTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            } else { // empty triangle
                M5Cardputer.Display.drawTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            }
            // === THIS IS MOSTLY HERE INCASE IT GOES BACK TO THE SELECTION SCREEN AND IF I ADD MULTIPLE EQUATION GRAPHING AT SOME POINT ===
            if (selEquation != -1) { // gives green checkmark if equation is alredy drawn
                M5Cardputer.Display.fillTriangle(10, (lineStep * (selEquation + 1)) - (lineStep / 2), 4, (lineStep * (selEquation + 1)) - (lineStep / 2) + 4, 4, (lineStep * (selEquation + 1)) - (lineStep / 2) - 4, GREEN);
            }
            screenUpdate = false;
        }

        if (M5Cardputer.Keyboard.isChange()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastDebounceTime >= debounceDelay) { // prevent acidental double click
                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    return ("cancelled");
                } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    selEquation = lineSelection;
                    return lineEntries[lineSelection - 1];
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

void helpScreen() { // for anyone that wants it will also be displayed on the cardputer' screen but condensed info
    int numLines = 5;
    int distBetwTxt = (M5Cardputer.Display.height() - 20) / numLines;
    const char *helpText[numLines] = {
        "Arrow keys to move",
        "c to reset view",
        "Esc to go back",
        "g to graph equaiton",
        "serial 4 xtra deets"};
    M5.Display.clear();
    // the following serial is just if anyone wants more details info
    delay(100);
    Serial.println("=== HELP MENU ==="); // same info will apear on screen but condensed.
    Serial.println("Arrowkeys to move [No need for holding the function key].");
    Serial.println("Escape to switch betweeen funciton editing and graphing.");
    Serial.println("When on the Graphing Screen press c to reset view to (0,0) 'center'.");
    Serial.println("When on the Graphing screen press g to select equation(s) to plot.");
    Serial.println("if in any menu you don't want to be in press esc to cancel/back out.");
    // Uncomment if multi funciton plotting is ever added
    //  Serial.println("If you want to select multiple use SPACE to select and then hit enter to confirm.")
    delay(100);
    Serial.println("\n=== GENERAL INFO ===");
    Serial.println("graph with automatically refresh on each change of position so there shouldn't be any need to manualy refresh or update");
    delay(100);
    Serial.println("\n== ABOUT ===");
    Serial.println("Prometheus Grapher" + (String)versionNum + "Inspired by voidos by avascik\nAuthor: SupremeEgg75\nhttps://github.com/personwithbeans/PrometheusGrapher\nString to equation parser written by Claude (shamfully)");
    for (int i = 0; i < numLines; i++) { // yeah a for loop whats it to you, I like em.
        M5Cardputer.Display.drawString(helpText[i], 10, (distBetwTxt * i) + 10);
    }
}