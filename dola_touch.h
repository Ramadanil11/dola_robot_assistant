/*
 * ============================================
 *   DOLA - Touch Sensor Gesture Detection
 * ============================================
 * Menggunakan module TTP223 capacitive touch
 * Pin: VCC=3.3V, GND=GND, SIG=GPIO4
 * Output: HIGH saat disentuh, LOW saat tidak
 * Gesture: Tap, Double Tap, Hold
 */

#ifndef DOLA_TOUCH_H
#define DOLA_TOUCH_H

// TTP223 Touch pin
#define TOUCH_PIN         4     // GPIO 4 (SIG dari TTP223)

// Timing constants
#define TAP_MAX_DURATION    300   // Max ms untuk dianggap tap
#define DOUBLE_TAP_WINDOW   400   // Max ms antara 2 tap untuk double tap
#define HOLD_MIN_DURATION   1000  // Min ms untuk dianggap hold

// Gesture types
enum TouchGesture {
  GESTURE_NONE,
  GESTURE_TAP,
  GESTURE_DOUBLE_TAP,
  GESTURE_HOLD
};

// State tracking
bool touchActive = false;
unsigned long touchStartTime = 0;
unsigned long touchEndTime = 0;
int tapCount = 0;
bool waitingForDoubleTap = false;
unsigned long lastTapTime = 0;
TouchGesture currentGesture = GESTURE_NONE;
bool gestureReady = false;

// ============ FUNCTIONS ============

void setupTouch() {
  pinMode(TOUCH_PIN, INPUT);  // TTP223 output sebagai input di ESP32
  Serial.println("TTP223 Touch Sensor (GPIO4) OK!");
}

// Baca dan detect gesture - panggil di loop() setiap cycle
TouchGesture readTouch() {
  // TTP223: HIGH = disentuh, LOW = tidak disentuh
  bool isTouched = (digitalRead(TOUCH_PIN) == HIGH);
  unsigned long now = millis();
  
  // Reset gesture setiap cycle
  currentGesture = GESTURE_NONE;
  
  // Touch mulai (rising edge)
  if (isTouched && !touchActive) {
    touchActive = true;
    touchStartTime = now;
  }
  
  // Touch masih aktif - cek hold
  if (isTouched && touchActive) {
    if (now - touchStartTime >= HOLD_MIN_DURATION) {
      // HOLD detected!
      currentGesture = GESTURE_HOLD;
      gestureReady = true;
      touchActive = false;
      tapCount = 0;
      waitingForDoubleTap = false;
      return GESTURE_HOLD;
    }
  }
  
  // Touch selesai (falling edge)
  if (!isTouched && touchActive) {
    touchActive = false;
    touchEndTime = now;
    unsigned long duration = touchEndTime - touchStartTime;
    
    // Kalau durasi pendek = tap
    if (duration < TAP_MAX_DURATION) {
      tapCount++;
      lastTapTime = now;
      waitingForDoubleTap = true;
    }
  }
  
  // Cek apakah double tap window sudah lewat
  if (waitingForDoubleTap && !touchActive) {
    if (now - lastTapTime > DOUBLE_TAP_WINDOW) {
      if (tapCount >= 2) {
        currentGesture = GESTURE_DOUBLE_TAP;
        gestureReady = true;
      } else if (tapCount == 1) {
        currentGesture = GESTURE_TAP;
        gestureReady = true;
      }
      tapCount = 0;
      waitingForDoubleTap = false;
    }
  }
  
  return currentGesture;
}

// Cek apakah ada gesture baru
bool hasGesture() {
  return gestureReady;
}

// Ambil gesture dan reset flag
TouchGesture getGesture() {
  TouchGesture g = currentGesture;
  gestureReady = false;
  currentGesture = GESTURE_NONE;
  return g;
}

#endif // DOLA_TOUCH_H
