/*
 * ============================================
 *   DOLA - Clock (NTP) & Timer
 * ============================================
 * NTP sync, timezone WIB (UTC+7)
 * Timer dengan suara custom saat selesai
 */

#ifndef DOLA_CLOCK_H
#define DOLA_CLOCK_H

#include <time.h>
#include <Adafruit_SSD1306.h>

// External references
extern Adafruit_SSD1306 display;
extern void playWelcomeSound(const char* filename);
extern void showFace(DolaFace face);
extern bool textToSpeech(String text);

// NTP Config
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;  // WIB = UTC+7
const int daylightOffset_sec = 0;

// Timer state
bool timerRunning = false;
unsigned long timerDuration = 0;     // Total duration in ms
unsigned long timerStartTime = 0;    // When timer started
int timerSetMinutes = 1;             // Minutes being set (default 1)

// Hari dalam bahasa Indonesia
const char* hariIndo[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
const char* bulanIndo[] = {"Jan", "Feb", "Mar", "Apr", "Mei", "Jun", "Jul", "Agu", "Sep", "Okt", "Nov", "Des"};

// ============ NTP SETUP ============
void setupNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP configured (WIB UTC+7)");
  
  // Wait for time sync
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    delay(500);
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    Serial.printf("Time synced: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("NTP sync failed (will retry later)");
  }
}

// ============ CLOCK DISPLAY ============
void showClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 25);
    display.print("Waktu belum sync...");
    display.setCursor(10, 40);
    display.print("Cek koneksi WiFi");
    display.display();
    return;
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Big time display
  display.setTextSize(3);
  char timeBuf[9];
  sprintf(timeBuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  
  // Center the time
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 5);
  display.print(timeBuf);
  
  // Seconds (smaller)
  display.setTextSize(2);
  char secBuf[4];
  sprintf(secBuf, ":%02d", timeinfo.tm_sec);
  display.setCursor(100, 10);
  display.print(secBuf);
  
  // Separator line
  display.drawLine(10, 32, 118, 32, SSD1306_WHITE);
  
  // Date
  display.setTextSize(1);
  char dateBuf[32];
  sprintf(dateBuf, "%s, %d %s %d", 
          hariIndo[timeinfo.tm_wday],
          timeinfo.tm_mday,
          bulanIndo[timeinfo.tm_mon],
          timeinfo.tm_year + 1900);
  
  display.getTextBounds(dateBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 38);
  display.print(dateBuf);
  
  // Footer
  display.setCursor(20, 56);
  display.print("tap/hold = kembali");
  
  display.display();
}

// ============ TIMER SET DISPLAY ============
void showTimerSet() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== SET TIMER ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Big timer display
  display.setTextSize(3);
  char timerBuf[8];
  int mins = timerSetMinutes;
  int secs = 0;
  sprintf(timerBuf, "%02d:%02d", mins, secs);
  
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timerBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 18);
  display.print(timerBuf);
  
  // Instructions
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("tap:+1min dtap:+5min");
  display.setCursor(0, 56);
  display.print("hold: START timer!");
  
  display.display();
}

// ============ TIMER RUNNING DISPLAY ============
void showTimerRunning() {
  if (!timerRunning) return;
  
  unsigned long elapsed = millis() - timerStartTime;
  long remaining = (long)timerDuration - (long)elapsed;
  
  if (remaining <= 0) {
    // Timer selesai!
    timerRunning = false;
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.print("WAKTU");
    display.setCursor(10, 30);
    display.print("HABIS!");
    display.display();
    
    // Play timer sound
    playWelcomeSound("/timer.wav");
    
    // Also TTS
    textToSpeech("Waktunya habis! Ayo ayo~");
    
    showFace(FACE_SURPRISED);
    delay(2000);
    return;
  }
  
  // Calculate remaining time
  int remSec = remaining / 1000;
  int remMin = remSec / 60;
  remSec = remSec % 60;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== TIMER ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Big countdown
  display.setTextSize(3);
  char timerBuf[8];
  sprintf(timerBuf, "%02d:%02d", remMin, remSec);
  
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timerBuf, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, 18);
  display.print(timerBuf);
  
  // Progress bar
  float progress = (float)elapsed / (float)timerDuration;
  int barW = (int)(progress * 120);
  display.drawRect(4, 48, 120, 8, SSD1306_WHITE);
  display.fillRect(4, 48, barW, 8, SSD1306_WHITE);
  
  // Footer
  display.setTextSize(1);
  display.setCursor(20, 58);
  display.print("hold = cancel");
  
  display.display();
}

// ============ TIMER CONTROLS ============
void timerAddMinute() {
  timerSetMinutes++;
  if (timerSetMinutes > 60) timerSetMinutes = 60;  // Max 60 menit
  showTimerSet();
}

void timerAddFiveMinutes() {
  timerSetMinutes += 5;
  if (timerSetMinutes > 60) timerSetMinutes = 60;
  showTimerSet();
}

void timerStart() {
  timerDuration = (unsigned long)timerSetMinutes * 60 * 1000;
  timerStartTime = millis();
  timerRunning = true;
  
  char msg[64];
  sprintf(msg, "Timer %d menit dimulai!", timerSetMinutes);
  textToSpeech(msg);
}

void timerCancel() {
  timerRunning = false;
  timerSetMinutes = 1;  // Reset
}

// Handle gesture di clock screen
bool handleClockGesture(TouchGesture gesture) {
  // Any gesture = back to menu
  if (gesture == GESTURE_TAP || gesture == GESTURE_HOLD || gesture == GESTURE_DOUBLE_TAP) {
    return true;  // Signal to go back
  }
  return false;
}

// Handle gesture di timer set screen
// Returns: 0 = handled, 1 = start timer, 2 = go back
int handleTimerSetGesture(TouchGesture gesture) {
  if (gesture == GESTURE_TAP) {
    timerAddMinute();
    return 0;
  } else if (gesture == GESTURE_DOUBLE_TAP) {
    timerAddFiveMinutes();
    return 0;
  } else if (gesture == GESTURE_HOLD) {
    timerStart();
    return 1;  // Start!
  }
  return 0;
}

// Handle gesture di timer running screen
// Returns true if timer should be cancelled
bool handleTimerRunGesture(TouchGesture gesture) {
  if (gesture == GESTURE_HOLD) {
    timerCancel();
    textToSpeech("Timer dibatalkan~");
    return true;
  }
  return false;
}

#endif // DOLA_CLOCK_H
