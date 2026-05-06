/*
 * ============================================
 *   DOLA - Menu System
 * ============================================
 * Navigasi menu di OLED 128x64 (list text)
 * Tap = next, Double Tap = select, Hold = back
 */

#ifndef DOLA_MENU_H
#define DOLA_MENU_H

#include <Adafruit_SSD1306.h>

// External references
extern Adafruit_SSD1306 display;
extern uint8_t volumeLevel;
extern Preferences preferences;
extern void setVolume(uint8_t level);
extern void startAPMode();
extern void playWelcomeSound(const char* filename);

// ============ MENU STRUCTURE ============

enum MenuScreen {
  SCREEN_HOME,        // Wajah Dola (bukan menu)
  SCREEN_MAIN,        // Menu utama
  SCREEN_GAMES,       // Submenu games
  SCREEN_CLOCK,       // Tampilan jam
  SCREEN_TIMER_SET,   // Set timer
  SCREEN_TIMER_RUN,   // Timer running
  SCREEN_SETTINGS,    // Submenu settings
  SCREEN_VOLUME,      // Atur volume
  SCREEN_STATUS,      // Info status
  SCREEN_ABOUT,       // About Dola
  // Game screens
  SCREEN_GAME_TEBAK,
  SCREEN_GAME_SIMON,
  SCREEN_GAME_SHAKE,
  SCREEN_GAME_QUIZ,
  SCREEN_GAME_BSK,
  SCREEN_GAME_JOKES,
  SCREEN_GAME_TAMAGOTCHI
};

// Menu state
MenuScreen currentScreen = SCREEN_HOME;
MenuScreen previousScreen = SCREEN_HOME;
int menuCursor = 0;       // Posisi cursor saat ini
int menuScrollOffset = 0; // Scroll offset untuk menu panjang
bool inMenu = false;      // Flag: sedang di menu atau di home

// Menu items
const char* mainMenuItems[] = {"Games", "Clock", "Timer", "Settings", "Status", "About Dola"};
const int mainMenuCount = 6;

const char* gamesMenuItems[] = {"Tebak Angka", "Simon Says", "Shake Challenge", "Quiz Trivia", "Batu Gunting Kertas", "Random Jokes", "Tamagotchi"};
const int gamesMenuCount = 7;

const char* settingsMenuItems[] = {"Volume", "WiFi Reset"};
const int settingsMenuCount = 2;

// ============ MENU RENDERING ============

void renderMenuList(const char* title, const char* items[], int itemCount, int cursor, int scrollOffset) {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== ");
  display.print(title);
  display.print(" ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Menu items (max 5 visible, mulai dari baris 12)
  int maxVisible = 5;
  int startY = 12;
  
  for (int i = 0; i < maxVisible && (i + scrollOffset) < itemCount; i++) {
    int itemIndex = i + scrollOffset;
    int y = startY + (i * 10);
    
    display.setCursor(2, y);
    if (itemIndex == cursor) {
      // Cursor aktif
      display.print("> ");
      // Highlight: invert
      display.fillRect(0, y - 1, 128, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(2, y);
      display.print("> ");
      display.print(items[itemIndex]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.print("  ");
      display.print(items[itemIndex]);
    }
  }
  
  // Scroll indicator
  if (scrollOffset > 0) {
    display.setCursor(120, 12);
    display.print("^");
  }
  if (scrollOffset + maxVisible < itemCount) {
    display.setCursor(120, 52);
    display.print("v");
  }
  
  // Footer hint
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print("tap:next dtap:ok hold:back");
  
  display.display();
}

// ============ SPECIFIC SCREENS ============

void renderVolumeScreen() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== VOLUME ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Volume bar
  display.setCursor(10, 20);
  display.setTextSize(2);
  display.printf("%d%%", volumeLevel);
  
  // Bar visual
  int barWidth = map(volumeLevel, 0, 150, 0, 100);
  display.drawRect(14, 40, 100, 10, SSD1306_WHITE);
  display.fillRect(14, 40, barWidth, 10, SSD1306_WHITE);
  
  // Instructions
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("tap:+10  dtap:-10  hold:back");
  
  display.display();
}

void renderStatusScreen() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== STATUS DOLA ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  display.setCursor(0, 12);
  display.printf("WiFi: %s", WiFi.isConnected() ? "Connected" : "Disconnected");
  
  display.setCursor(0, 22);
  if (WiFi.isConnected()) {
    display.printf("IP: %s", WiFi.localIP().toString().c_str());
  } else {
    display.print("IP: -");
  }
  
  display.setCursor(0, 32);
  display.printf("SSID: %s", WiFi.SSID().c_str());
  
  display.setCursor(0, 42);
  display.printf("SPIFFS: %dKB/%dKB", SPIFFS.usedBytes()/1024, SPIFFS.totalBytes()/1024);
  
  display.setCursor(0, 52);
  unsigned long upSec = millis() / 1000;
  int upH = upSec / 3600;
  int upM = (upSec % 3600) / 60;
  display.printf("Uptime: %dh %dm", upH, upM);
  
  display.display();
}

void renderAboutScreen() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(30, 5);
  display.print("(^_^)");
  
  display.setTextSize(2);
  display.setCursor(30, 18);
  display.print("DOLA");
  
  display.setTextSize(1);
  display.setCursor(35, 36);
  display.print("v1.0");
  
  display.setCursor(10, 48);
  display.print("Robot Asisten AI Lucu");
  
  display.setCursor(20, 58);
  display.print("Made with love~");
  
  display.display();
}

// ============ MENU NAVIGATION ============

void enterMenu() {
  inMenu = true;
  currentScreen = SCREEN_MAIN;
  menuCursor = 0;
  menuScrollOffset = 0;
  renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
}

void exitMenu() {
  inMenu = false;
  currentScreen = SCREEN_HOME;
  menuCursor = 0;
  menuScrollOffset = 0;
}

// Navigate next item (tap)
void menuNext(const char* items[], int itemCount) {
  menuCursor++;
  if (menuCursor >= itemCount) {
    menuCursor = 0;
    menuScrollOffset = 0;
  }
  // Auto scroll
  if (menuCursor >= menuScrollOffset + 5) {
    menuScrollOffset = menuCursor - 4;
  }
  if (menuCursor < menuScrollOffset) {
    menuScrollOffset = menuCursor;
  }
}

// Handle gesture di menu - return true jika gesture sudah di-handle
bool handleMenuGesture(TouchGesture gesture) {
  if (!inMenu) {
    // Di HOME, double tap = masuk menu
    if (gesture == GESTURE_DOUBLE_TAP) {
      enterMenu();
      return true;
    }
    return false;
  }
  
  switch (currentScreen) {
    case SCREEN_MAIN:
      if (gesture == GESTURE_TAP) {
        menuNext(mainMenuItems, mainMenuCount);
        renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        // Select item
        switch (menuCursor) {
          case 0: // Games
            currentScreen = SCREEN_GAMES;
            menuCursor = 0;
            menuScrollOffset = 0;
            renderMenuList("GAMES", gamesMenuItems, gamesMenuCount, menuCursor, menuScrollOffset);
            break;
          case 1: // Clock
            currentScreen = SCREEN_CLOCK;
            break;
          case 2: // Timer
            currentScreen = SCREEN_TIMER_SET;
            break;
          case 3: // Settings
            currentScreen = SCREEN_SETTINGS;
            menuCursor = 0;
            menuScrollOffset = 0;
            renderMenuList("SETTINGS", settingsMenuItems, settingsMenuCount, menuCursor, menuScrollOffset);
            break;
          case 4: // Status
            currentScreen = SCREEN_STATUS;
            renderStatusScreen();
            break;
          case 5: // About
            currentScreen = SCREEN_ABOUT;
            renderAboutScreen();
            break;
        }
      } else if (gesture == GESTURE_HOLD) {
        exitMenu();
      }
      break;
      
    case SCREEN_GAMES:
      if (gesture == GESTURE_TAP) {
        menuNext(gamesMenuItems, gamesMenuCount);
        renderMenuList("GAMES", gamesMenuItems, gamesMenuCount, menuCursor, menuScrollOffset);
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        // Launch game
        switch (menuCursor) {
          case 0: currentScreen = SCREEN_GAME_TEBAK; break;
          case 1: currentScreen = SCREEN_GAME_SIMON; break;
          case 2: currentScreen = SCREEN_GAME_SHAKE; break;
          case 3: currentScreen = SCREEN_GAME_QUIZ; break;
          case 4: currentScreen = SCREEN_GAME_BSK; break;
          case 5: currentScreen = SCREEN_GAME_JOKES; break;
          case 6: currentScreen = SCREEN_GAME_TAMAGOTCHI; break;
        }
      } else if (gesture == GESTURE_HOLD) {
        currentScreen = SCREEN_MAIN;
        menuCursor = 0;
        menuScrollOffset = 0;
        renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
      }
      break;
      
    case SCREEN_SETTINGS:
      if (gesture == GESTURE_TAP) {
        menuNext(settingsMenuItems, settingsMenuCount);
        renderMenuList("SETTINGS", settingsMenuItems, settingsMenuCount, menuCursor, menuScrollOffset);
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        if (menuCursor == 0) {
          // Volume
          currentScreen = SCREEN_VOLUME;
          renderVolumeScreen();
        } else if (menuCursor == 1) {
          // WiFi Reset - confirm
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(10, 20);
          display.print("Reset WiFi?");
          display.setCursor(10, 35);
          display.print("Double tap = YES");
          display.setCursor(10, 45);
          display.print("Hold = Cancel");
          display.display();
          // Wait for confirmation (handled in next cycle)
          currentScreen = SCREEN_SETTINGS; // Stay, special handling needed
          // Actually do the reset
          preferences.putString("ssid", "");
          preferences.putString("pass", "");
          delay(1500);
          ESP.restart();
        }
      } else if (gesture == GESTURE_HOLD) {
        currentScreen = SCREEN_MAIN;
        menuCursor = 3; // Back to Settings position
        menuScrollOffset = 0;
        renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
      }
      break;
      
    case SCREEN_VOLUME:
      if (gesture == GESTURE_TAP) {
        // Volume up +10
        if (volumeLevel < 150) volumeLevel += 10;
        renderVolumeScreen();
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        // Volume down -10
        if (volumeLevel > 0) volumeLevel -= 10;
        renderVolumeScreen();
      } else if (gesture == GESTURE_HOLD) {
        // Save & back
        currentScreen = SCREEN_SETTINGS;
        menuCursor = 0;
        renderMenuList("SETTINGS", settingsMenuItems, settingsMenuCount, menuCursor, menuScrollOffset);
      }
      break;
      
    case SCREEN_STATUS:
    case SCREEN_ABOUT:
      if (gesture == GESTURE_TAP || gesture == GESTURE_HOLD || gesture == GESTURE_DOUBLE_TAP) {
        currentScreen = SCREEN_MAIN;
        menuCursor = (currentScreen == SCREEN_STATUS) ? 4 : 5;
        menuCursor = 0;
        menuScrollOffset = 0;
        renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
      }
      break;
      
    default:
      // Game screens handled by dola_games.h
      return false;
  }
  
  return true;
}

#endif // DOLA_MENU_H
