// Inspired by voidos by avascik
// Author: SupremeEgg75
// Liscense: AGPL-3.0
#include <Icons.c>
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
// Other Variables
bool drawMode = true;
const int lines = 4;
String lineEntries[lines] = {"test", "test", "test", "test"};
String tmp = "";
// Forward declarations for functions defined later in this file
void DrawGraphPage();
void FunctionMenu();

void setup() {
    M5Cardputer.begin();
    M5Cardputer.Display.setBrightness(10);
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
    int textsize = M5Cardputer.Display.height() / 60;
    if (textsize == 0) {
        textsize = 1;
    }
}

void loop() {
    DrawGraphPage();
    FunctionMenu();
}

void FunctionMenu() {
    M5.Display.fillScreen(SystemColour);
    M5.Display.setTextSize(2);

    int lineSelection = 1;
    int lineStep = maxy / lines;
    bool screenUpdate = true;
    bool selected = false;

    while (true) {
        M5Cardputer.update();
        if (screenUpdate) {
            M5.Display.clear();
            for (int i = 1; i <= lines; i++) {
                M5.Display.drawLine(0, lineStep * i, maxx, lineStep * i, SystemColour);
                M5Cardputer.Display.drawString(String(i) + ':', 18, (lineStep * i) - (lineStep / 2));
                M5Cardputer.Display.drawString(lineEntries[i - 1], 44, (lineStep * i) - (lineStep / 2));
            }
            if (selected) { // filled triangle
                M5Cardputer.Display.fillTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);

            } else { // empty triangle
                M5Cardputer.Display.drawTriangle(10, (lineStep * lineSelection) - (lineStep / 2), 4, (lineStep * lineSelection) - (lineStep / 2) + 4, 4, (lineStep * lineSelection) - (lineStep / 2) - 4, SystemColour);
            }
            screenUpdate = false;
        }

        if (M5Cardputer.Keyboard.isChange()) {
            unsigned long currentMillis = millis();
            if (currentMillis - lastDebounceTime >= debounceDelay) { // prevent acidental double click
                if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                    drawMode = true;
                    return;
                } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    if (selected) {
                        selected = false;
                    } else {
                        selected = true;
                    }
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

                if (selected) {
                    if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                        if (lineEntries[lineSelection - 1].length() > 0) {
                            lineEntries[lineSelection - 1].remove(lineEntries[lineSelection - 1].length() - 1);
                        }
                    }else {
                        lineEntries[lineSelection - 1]+=(String)M5Cardputer.Keyboard.getKey();
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
    M5.Display.setBaseColor(BLACK);
    while (true) {
        M5Cardputer.update();
        if (oldPosX != posx || oldPosY != posy) {
            M5.Display.clear();
            M5Cardputer.Display.drawLine(cx - posx, 0, cx - posx, maxy, SystemColour);
            M5Cardputer.Display.drawLine(0, cy + posy, maxx, cy + posy, SystemColour);
            oldPosX = posx;
            oldPosY = posy;
            // im a pronbably a shity newbie coder so this is probably a shitty
            // way or at least unoptimized as fuck but it works
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
            delay(10);
        }
        if (M5Cardputer.Keyboard.isKeyPressed(',')) {
            posx -= posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('/')) {
            posx += posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
            posy -= posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
            posy += posStep;
        } else if (M5Cardputer.Keyboard.isKeyPressed('c')) { // center position
            posx = 0;
            posy = 0;
        } else if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                drawMode = false;
                M5.Display.clear();
                return;
            }
        }
    }
}