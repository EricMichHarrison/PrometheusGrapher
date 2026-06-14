// Inspired by voidos by avascik
// Author: SupremeEgg75
// Liscense: AGPL-3.0

#include <Icons.c>
#include <M5Cardputer.h>

extern bool instantBoot;
extern String versionNum;

// debounce delay
const unsigned long debounceDelay = 20;
unsigned long lastDebounceTime = 0;

int pageReturnVal = 0;

// Forward declarations for functions defined later in this file
void SplashScreen(int DrawDelayTime, int FinishDelayTime);
void OpenPage(int index);
void DisplayPage();

void setup() {
    Serial.begin(115200);
    unsigned long serialWaitStart = millis();
    while (!Serial && (millis() - serialWaitStart < 2000)) {
        delay(10);
    }
    M5Cardputer.begin();
    M5Cardputer.Display.setBrightness(100);
    Serial.print("=== STARTING ===\n");
    int textsize = M5Cardputer.Display.height() / 60;
    if (textsize == 0) {
        textsize = 1;
    }
    SplashScreen(1, 2);
}

void SplashScreen(int DrawDelayTime, int FinishDelayTime) {
    if (instantBoot) {
        Serial.println("Skipping SpashScreen\n");
        return;
    } else {
        Serial.println("Starting SpashScreen\n");
    }
    M5.Display.fillScreen(BLACK);
    M5Cardputer.Display.drawString("SupremeEgg75", 85, 102);
    // draw Version num
    M5Cardputer.Display.drawString(versionNum, 106, 88);
    // Main Logo
    // drawBitmap is deprecated; use pushImage with RGB565 data.
    M5Cardputer.Display.pushImage(60, 11, 120, 70, MainLogoIcon);
    delay(FinishDelayTime * 3000);
}

// void SplashScreen(int DrawDelayTime, int FinishDelayTime) {
//   if (instantBoot) {
//     Serial.println("Skipping SpashScreen");
//     return;
//   } else {
//     Serial.println("Starting SpashScreen");
//   }
//   M5.Display.fillScreen(BackgroundColour);
//   float a = 3;
//   float b = 4;
//   int cx = M5.Display.width() / 2;
//   int cy = M5.Display.height() / 2;
//   float scale = 30;  // adjust zoom
//   float yadj = 25;   // alter y position

//   float prevX = cx;
//   float prevY = cy;

//   M5Cardputer.Display.drawString("PromethuesOS " + versionNum, cx -
//   (M5.Display.textWidth("PrometheusOS") + M5.Display.textWidth(versionNum) + 2) / 2, cy + yadj);
//   M5Cardputer.Display.drawString("personwithbeans", cx -
//   (M5.Display.textWidth("personwithbeans")) / 2, cy + yadj + 15);

//   for (float theta = 0; theta <= 8 * PI; theta += 0.01) {

//     float r = sin((a / b) * theta);

//     float x = cx + ((r * cos(theta)) * scale);
//     float y = cy + ((r * sin(theta)) * scale) - yadj;  //top down cordinate system

//     M5.Display.drawLine(prevX, prevY, x, y, WHITE);

//     prevX = x;
//     prevY = y;
//     M5Cardputer.update();
//     if (M5Cardputer.Keyboard.isChange()) {
//       Serial.println("Manualy Skipping SpashScreen");
//       FinishDelayTime = 0;
//       break;
//     }
//     delay(DrawDelayTime);
//   }
//   delay(FinishDelayTime * 1000);
// }

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastDebounceTime >= debounceDelay) { // prevent acidental double click
            // if (M5Cardputer.Keyboard.isKeyPressed('/')) {        // right arrow
            //     if (AudioOn) {
            //         M5Cardputer.Speaker.tone(3000, 40);
            //     }
            //     if (page < 5) {
            //         page += 1;
            //     }
            // } else if (M5Cardputer.Keyboard.isKeyPressed(',')) { // left arrow
            //     if (AudioOn) {
            //         M5Cardputer.Speaker.tone(3000, 40);
            //     }
            //     if (page > 1) {
            //         page -= 1;
            //     }
            // } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
            //     OpenPage(page);
            //     prevpage = 0;  // causes screen refresh in DisplayPage()
            //     DisplayPage(); // returns and displays main menu
            //     Serial.println("Successfully Returned to Main Menu");
            // }
            lastDebounceTime = currentMillis;
        }
    }
    DisplayPage();
}
