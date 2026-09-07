//###############################
//#        THIS IS A WIP        #
//#  NOT CURRENTLY IMPLEMENTED  #
//###############################


#include <M5Cardputer.h>
#include <Settings.h>
#include <SettingsList.h>

int enteriesPerPage = 6;
int cursor = 1;
int prevCursor = 0;
extern int xmargin;  //meant to be local variables
extern int ymargin;  //meant to be local variables
extern int TopBarMargin;
bool pageChange = false;
int optionStep = 0;

void ConfigApp() {
  
  bool Active = true;  //is app active? if so then keep open

  optionStep = M5.Display.height() / enteriesPerPage;
  if (optionStep <= 0) {
    optionStep = 1;
  }

  int page = 1;
  int maxNumPages = 3;
  while (Active) {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isChange()) {  //Keyboard input
      unsigned long currentMillis = millis();
      if (currentMillis - lastDebounceTime >= debounceDelay) {  //prevent acidental double click

        if (M5Cardputer.Keyboard.isKeyPressed(';')) {  //up arrow | Previous option
          if (AudioOn) {
            M5Cardputer.Speaker.tone(3000, 40);
          }
          if (cursor < enteriesPerPage) {
            cursor += 1;
          }
        } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {  //down arrow | Next option
          if (AudioOn) {
            M5Cardputer.Speaker.tone(3000, 40);
          }
          if (cursor > 0) {
            cursor -= 1;
          }
        } else if (M5Cardputer.Keyboard.isKeyPressed('/')) {  //right arrow | Next page
          if (AudioOn) {
            M5Cardputer.Speaker.tone(3000, 40);
          }
          if (page < maxNumPages) {
            page -= 1;
            pageChange = true;
          }
        } else if (M5Cardputer.Keyboard.isKeyPressed(',')) {  //left arrow | Prevous page
          if (AudioOn) {
            M5Cardputer.Speaker.tone(3000, 40);
          }
          if (page > 1) {
            page -= 1;
            pageChange = true;
          }
        } else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                    Serial.println("Returning to Main Menu");
          if (AudioOn) {
            M5Cardputer.Speaker.tone(2000, 40);
          }
          Active = false;
        }
        lastDebounceTime = currentMillis;
      }



      if (prevCursor != cursor || pageChange) {  //Page Draw
        M5Cardputer.Display.clear();
        M5Cardputer.Display.drawString("Settings", xmargin + ((M5.Display.textWidth("Settings")) / 2) - 5, TopBarMargin / 2);
        int maxX = M5.Display.width();
        int maxY = M5.Display.height();
        int xBoxPoints[] = { xmargin,
                             xmargin,
                             maxX - xmargin,
                             maxX - xmargin,
                             xmargin };  //bottom left, top left, top right, bottom right
        int yBoxPoints[] = { maxY - ymargin,
                             TopBarMargin,
                             TopBarMargin,
                             maxY - ymargin,
                             maxY - ymargin };  //bottom left, top left, top right, bottom right
        for (int i = 0; i < 4; i += 1) {
          M5.Display.drawLine(xBoxPoints[i], yBoxPoints[i], xBoxPoints[i + 1], yBoxPoints[i + 1], WHITE);
        }
        for (int i = 0; i < enteriesPerPage; i += 1) {  //[TMP] draw row positions
          M5.Display.drawString("-", xmargin + 3, optionStep * i + TopBarMargin);
        }
        M5.Display.drawString("<", maxX - xmargin - 2, cursor * optionStep + TopBarMargin);
        prevCursor = cursor;
        pageChange = false;
      }
    }
  }  //end while(Active)
  return;
}  // fuction end
