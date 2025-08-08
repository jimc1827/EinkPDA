/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include "globals.h"

String currentJournal = "";
String bufferEditingFile = editingFile;

void JOURNAL_INIT() {
  CurrentAppState = JOURNAL;
  CurrentJournalState = J_MENU;
  forceSlowFullUpdate = true;
  newState = true;
  CurrentKBState = NORMAL;
  bufferEditingFile = editingFile;
}

// File Operations
void loadJournal() {
  editingFile = currentJournal;
  loadFile();
}

void saveJournal() {
  editingFile = currentJournal;
  saveFile();
}

// Functions
void drawJMENU() {
  enum Box {
    width = 4,
    height = 4
  };

  enum Margin {
    left = 91,
    top = 50
  };

  enum Spacing {
    horizontal = 7,
    vertical = 9
  };

  SDActive = true;
  setCpuFrequencyMhz(240);
  delay(50);

  // Display background
  drawStatusBar("Type:YYYYMMDD or (T)oday");
  display.drawBitmap(0, 0, _journal, 320, 218, GxEPD_BLACK);

  // Update current progress graph
  DateTime now = rtc.now();
  String year = String(now.year());

  // Files are in the format "/journal/YYYYMMDD.txt"
  for (int monthIndex = 1; monthIndex <= 12; monthIndex++) {

    int numDays = daysInMonth(now.year(), monthIndex);

    for (int dayIndex = 1; dayIndex <= numDays; dayIndex++) {
      int x = Margin::left +   ((dayIndex - 1) * Spacing::horizontal);
      int y = Margin::top  + ((monthIndex - 1) * Spacing::vertical);

      String day   = paddedNumber(dayIndex, 2);
      String month = paddedNumber(monthIndex, 2);

      String fileCode = "/journal/" + year + month + day + ".txt";

      if (SD_MMC.exists(fileCode)) display.fillRect(x, y, Box::width, Box::height, GxEPD_BLACK);
    }
  }

  if (SAVE_POWER) setCpuFrequencyMhz(POWER_SAVE_FREQ);
  SDActive = false;
}

void JMENUCommand(String command) {
  bool validInput = true;

  String fileName = "";

  SDActive = true;
  setCpuFrequencyMhz(240);
  delay(50);

  command.toLowerCase();
  command.trim();

  if (command == "t") {
    DateTime now = rtc.now();

    String year  = String(now.year());
    String month = paddedNumber(now.month(), 2);
    String day   = paddedNumber(now.day(),   2);

    fileName = "/journal/" + year + month + day + ".txt";
  }
  // command in the form "YYYYMMDD"
  else if (command.length() == 8 && command.toInt() > 0) {
    int yearIndex = command.substring(0, 4).toInt();
    int monthIndex = command.substring(4, 6).toInt();
    int dayIndex = command.substring(6, 8).toInt();

    if (validDate(yearIndex, monthIndex, dayIndex)) {
      String year = String(yearIndex);
      String month = paddedNumber(monthIndex, 2);
      String day = paddedNumber(dayIndex, 2);
      fileName = "/journal/" + year + month + day + ".txt";
    } else {
      validInput = false;
    }
  }
  // command in the form "jan 1"
  else {
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1 && spaceIndex < command.length() - 1) {

      String monthStr = command.substring(0, spaceIndex);
      String dayStr = command.substring(spaceIndex + 1);

      String monthMap = "janfebmaraprmayjunjulaugsepoctnovdec";

      int yearIndex  = rtc.now().year();
      int monthIndex = (monthMap.indexOf(monthStr) / 3) + 1;
      int dayIndex   = dayStr.toInt();

      if (validDate(yearIndex, monthIndex, dayIndex)) {
        String year  = String(yearIndex);
        String month = paddedNumber(monthIndex, 2);
        String day   = paddedNumber(dayIndex, 2);

        fileName = "/journal/" + year + month + day + ".txt";
      } else {
        validInput = false;
      }
    }
  }

  if (validInput) {
    // If file doesn't exist, create it
    if (!SD_MMC.exists(fileName)) {
      File f = SD_MMC.open(fileName, FILE_WRITE);
      if (f) f.close();
    }

    currentJournal = fileName;

    // Load file
    editingFile = currentJournal;
    loadJournal();

    dynamicScroll = 0;
    newLineAdded = true;
    CurrentJournalState = J_TXT;
  } else {
    oledLine("Invalid command format");
    delay(50);
  }

  if (SAVE_POWER) setCpuFrequencyMhz(POWER_SAVE_FREQ);
  SDActive = false;
}

// Loops
void processKB_JOURNAL() {
  int currentMillis = millis();
  char inchar;

  if (currentMillis - KBBounceMillis < KB_COOLDOWN)
    return;

  switch (CurrentJournalState) {
    case J_MENU:
      inchar = updateKeypress();
      // HANDLE INPUTS
      switch (inchar) {
        case EMPTY:
          break;
        case BKSP:
          if (currentLine.length() > 0)
            currentLine.remove(currentLine.length() - 1);
          break;
        case 12:
          editingFile = bufferEditingFile;
          CurrentAppState = HOME;
          currentLine = "";
          newState = true;
          CurrentKBState = NORMAL;
          break;
        case CR:
          JMENUCommand(currentLine);
          currentLine = "";
          break;
        case SHIFT_KEY:
          CurrentKBState = (CurrentKBState == SHIFT) ? NORMAL : SHIFT;
          break;
        case FN_KEY:
          CurrentKBState = (CurrentKBState == FUNC) ? NORMAL : FUNC;
          break;
        case ESC:
          currentLine = "";
          break;
        default: // Normal character received
          currentLine += inchar;
          if (inchar != SPACE && !isDigit(inchar) && CurrentKBState != NORMAL)
            CurrentKBState = NORMAL;
          break;
      }

      currentMillis = millis();
      //Make sure oled only updates at OLED_MAX_FPS
      if (currentMillis - OLEDFPSMillis >= (1000/OLED_MAX_FPS)) {
        OLEDFPSMillis = currentMillis;
        oledLine(currentLine, false);
      }
      break;

    case J_TXT:
      inchar = updateKeypress();
      // SET MAXIMUMS AND FONT
      setTXTFont(currentFont);

      // UPDATE SCROLLBAR
      updateScrollFromTouch();

      // HANDLE INPUTS
      switch (inchar) {
        case EMPTY:
          break;
        case LOAD:
          loadJournal();
          CurrentKBState = NORMAL;
          newLineAdded = true;
          break;
        case SAVE:
          saveJournal();
          CurrentKBState = NORMAL;
          newLineAdded = true;
          break;
        case BKSP:
          if (currentLine.length() > 0)
            currentLine.remove(currentLine.length() - 1);
          break;
        case TAB:
          currentLine += "    ";
          break;
        case 12:
          JOURNAL_INIT();
          break;
        case CR:
          allLines.push_back(currentLine);
          currentLine = "";
          newLineAdded = true;
          break;
        case 14: // Font Switcher
          CurrentTXTState = FONT;
          CurrentKBState = FUNC;
          newState = true;
          break;
        case SHIFT_KEY:
          CurrentKBState = (CurrentKBState == SHIFT) ? NORMAL : SHIFT;
          break;
        case FN_KEY:
          CurrentKBState = (CurrentKBState == FUNC) ? NORMAL : FUNC;
          break;
        case LEFT:
          break;
        case ESC:
          allLines.clear();
          currentLine = "";
          oledWord("Clearing...");
          doFull = true;
          newLineAdded = true;
          delay(300);
          break;
        case RIGHT:
          break;
        default:
          currentLine += inchar;
          if (inchar != SPACE && !isDigit(inchar) && CurrentKBState != NORMAL)
            CurrentKBState = NORMAL;
          break;
      }

      currentMillis = millis();
      //Make sure oled only updates at 60fps
      if (currentMillis - OLEDFPSMillis >= (1000/60)) {
        OLEDFPSMillis = currentMillis;
        // ONLY SHOW OLEDLINE WHEN NOT IN SCROLL MODE
        if (lastTouch == -1) {
          oledLine(currentLine);
          prev_dynamicScroll = dynamicScroll;
        }
        else oledScroll();
      }

      if (currentLine.length() > 0) {
        int16_t x1, y1;
        uint16_t charWidth, charHeight;
        display.getTextBounds(currentLine, 0, 0, &x1, &y1, &charWidth, &charHeight);

        if (charWidth >= display.width()-5) {
          // If currentLine ends with a space, just start a new line
          if (currentLine.endsWith(" ")) {
            allLines.push_back(currentLine);
            currentLine = "";
          }
          // If currentLine ends with a letter, we are in the middle of a word
          else {
            int lastSpace = currentLine.lastIndexOf(' ');
            String partialWord;

            if (lastSpace != -1) {
              partialWord = currentLine.substring(lastSpace + 1);
              currentLine = currentLine.substring(0, lastSpace);  // Strip partial word
              allLines.push_back(currentLine);
              currentLine = partialWord;  // Start new line with the partial word
            }
            // No spaces found, whole line is a single word
            else {
              allLines.push_back(currentLine);
              currentLine = "";
            }
          }
          newLineAdded = true;
        }
      }

      break;
  }
}

void einkHandler_JOURNAL() {
  switch (CurrentJournalState) {
    case J_MENU:
      if (newState) {
        newState = false;

        drawJMENU();

        multiPassRefesh(2);
      }
      break;
    case J_TXT:
      if (newState && doFull) {
        display.fillScreen(GxEPD_WHITE);
        refresh();
      }
      if (newLineAdded && !newState) {
        einkTextDynamic(true);
        refresh();
      }

      newState = false;
      newLineAdded = false;
      break;
  }
}
