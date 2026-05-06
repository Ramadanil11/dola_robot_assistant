/*
 * ============================================
 *   DOLA - Face Expressions & Animations
 * ============================================
 * Animasi wajah lucu untuk OLED SSD1306 128x64
 * Style: Kawaii robot eyes (inspired by Cozmo/Vector)
 */

#ifndef DOLA_FACES_H
#define DOLA_FACES_H

#include <Adafruit_SSD1306.h>

// Face types
enum DolaFace {
  FACE_IDLE,
  FACE_BLINK,
  FACE_HAPPY,
  FACE_SAD,
  FACE_ANGRY,
  FACE_SCARED,
  FACE_DIZZY,
  FACE_THINKING,
  FACE_THINKING_2,
  FACE_LISTENING,
  FACE_SPEAKING,
  FACE_SPEAKING_2,
  FACE_SLEEPY,
  FACE_SURPRISED,
  FACE_CONFUSED,
  FACE_UNCOMFORTABLE,
  FACE_CONNECTING,
  FACE_AP_MODE,
  FACE_LOVE
};

// External display reference
extern Adafruit_SSD1306 display;

// ============ DRAWING HELPERS ============

void drawEye(int x, int y, int w, int h, int pupilOffX = 0, int pupilOffY = 0) {
  // Eye outline (rounded rect)
  display.fillRoundRect(x, y, w, h, h/3, SSD1306_WHITE);
  // Pupil
  int pupilW = w / 3;
  int pupilH = h / 3;
  int px = x + w/2 - pupilW/2 + pupilOffX;
  int py = y + h/2 - pupilH/2 + pupilOffY;
  display.fillRoundRect(px, py, pupilW, pupilH, pupilH/3, SSD1306_BLACK);
}

void drawMouth(int x, int y, int type) {
  // type: 0=neutral, 1=happy, 2=sad, 3=open, 4=small open, 5=wavy
  switch (type) {
    case 0: // Neutral - small line
      display.drawLine(x - 5, y, x + 5, y, SSD1306_WHITE);
      break;
    case 1: // Happy - arc up
      display.drawCircleHelper(x, y - 3, 8, 0x08, SSD1306_WHITE); // bottom half
      display.drawCircleHelper(x, y - 3, 7, 0x08, SSD1306_WHITE);
      // Simple smile curve
      for (int i = -8; i <= 8; i++) {
        int dy = (i * i) / 12;
        display.drawPixel(x + i, y + dy, SSD1306_WHITE);
        display.drawPixel(x + i, y + dy + 1, SSD1306_WHITE);
      }
      break;
    case 2: // Sad - arc down
      for (int i = -8; i <= 8; i++) {
        int dy = -(i * i) / 12;
        display.drawPixel(x + i, y + dy + 5, SSD1306_WHITE);
        display.drawPixel(x + i, y + dy + 6, SSD1306_WHITE);
      }
      break;
    case 3: // Open mouth (speaking/surprised)
      display.fillRoundRect(x - 6, y - 3, 12, 10, 4, SSD1306_WHITE);
      display.fillRoundRect(x - 4, y - 1, 8, 6, 3, SSD1306_BLACK);
      break;
    case 4: // Small open (speaking frame 2)
      display.fillRoundRect(x - 4, y - 2, 8, 6, 3, SSD1306_WHITE);
      display.fillRoundRect(x - 2, y, 4, 3, 2, SSD1306_BLACK);
      break;
    case 5: // Wavy (dizzy/uncomfortable)
      for (int i = -10; i <= 10; i++) {
        int dy = sin(i * 0.8) * 2;
        display.drawPixel(x + i, y + dy, SSD1306_WHITE);
      }
      break;
  }
}

// ============ FACE EXPRESSIONS ============

void drawFaceIdle() {
  display.clearDisplay();
  // Two round eyes
  drawEye(25, 18, 28, 28);   // Left eye
  drawEye(75, 18, 28, 28);   // Right eye
  // Small neutral mouth
  drawMouth(64, 55, 0);
  display.display();
}

void drawFaceBlink() {
  display.clearDisplay();
  // Closed eyes (horizontal lines)
  display.fillRoundRect(25, 30, 28, 4, 2, SSD1306_WHITE);  // Left
  display.fillRoundRect(75, 30, 28, 4, 2, SSD1306_WHITE);  // Right
  // Mouth
  drawMouth(64, 55, 0);
  display.display();
}

void drawFaceHappy() {
  display.clearDisplay();
  // Happy eyes (upside down arcs / ^_^ style)
  // Left eye - arc
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(39 + i, 28 + dy, SSD1306_WHITE);
    display.drawPixel(39 + i, 29 + dy, SSD1306_WHITE);
    display.drawPixel(39 + i, 27 + dy, SSD1306_WHITE);
  }
  // Right eye - arc
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(89 + i, 28 + dy, SSD1306_WHITE);
    display.drawPixel(89 + i, 29 + dy, SSD1306_WHITE);
    display.drawPixel(89 + i, 27 + dy, SSD1306_WHITE);
  }
  // Big smile
  drawMouth(64, 50, 1);
  // Blush marks
  display.fillCircle(20, 40, 3, SSD1306_WHITE);
  display.fillCircle(108, 40, 3, SSD1306_WHITE);
  display.display();
}

void drawFaceSad() {
  display.clearDisplay();
  // Sad eyes (droopy)
  drawEye(25, 22, 28, 24, 0, 3);  // Left eye, pupil down
  drawEye(75, 22, 28, 24, 0, 3);  // Right eye, pupil down
  // Eyebrow droop
  display.drawLine(28, 16, 48, 18, SSD1306_WHITE);
  display.drawLine(80, 18, 100, 16, SSD1306_WHITE);
  // Sad mouth
  drawMouth(64, 55, 2);
  // Tear drop
  display.fillCircle(55, 48, 2, SSD1306_WHITE);
  display.display();
}

void drawFaceAngry() {
  display.clearDisplay();
  // Angry eyes (narrowed with angry eyebrows)
  display.fillRoundRect(25, 25, 28, 18, 5, SSD1306_WHITE);  // Left
  display.fillRoundRect(75, 25, 28, 18, 5, SSD1306_WHITE);  // Right
  // Pupils
  display.fillRoundRect(34, 29, 10, 10, 3, SSD1306_BLACK);
  display.fillRoundRect(84, 29, 10, 10, 3, SSD1306_BLACK);
  // Angry eyebrows (V shape)
  display.fillTriangle(25, 22, 53, 15, 53, 22, SSD1306_WHITE);
  display.fillTriangle(75, 22, 103, 22, 75, 15, SSD1306_WHITE);
  // Black out top of eyes for angry look
  display.fillTriangle(25, 18, 53, 22, 25, 28, SSD1306_BLACK);
  display.fillTriangle(103, 18, 75, 22, 103, 28, SSD1306_BLACK);
  // Angry mouth
  display.drawLine(54, 55, 64, 58, SSD1306_WHITE);
  display.drawLine(64, 58, 74, 55, SSD1306_WHITE);
  // Steam marks
  display.drawLine(10, 10, 14, 5, SSD1306_WHITE);
  display.drawLine(14, 10, 18, 5, SSD1306_WHITE);
  display.drawLine(110, 10, 114, 5, SSD1306_WHITE);
  display.drawLine(114, 10, 118, 5, SSD1306_WHITE);
  display.display();
}

void drawFaceScared() {
  display.clearDisplay();
  // Big scared eyes (very round, small pupils)
  display.fillRoundRect(20, 12, 35, 35, 15, SSD1306_WHITE);  // Left
  display.fillRoundRect(73, 12, 35, 35, 15, SSD1306_WHITE);  // Right
  // Tiny pupils (scared)
  display.fillCircle(37, 29, 4, SSD1306_BLACK);
  display.fillCircle(90, 29, 4, SSD1306_BLACK);
  // Highlight in eyes
  display.fillCircle(33, 22, 3, SSD1306_BLACK);
  display.fillCircle(86, 22, 3, SSD1306_BLACK);
  display.fillCircle(34, 23, 1, SSD1306_WHITE);
  display.fillCircle(87, 23, 1, SSD1306_WHITE);
  // Open mouth (scared)
  display.fillRoundRect(54, 50, 20, 12, 6, SSD1306_WHITE);
  display.fillRoundRect(57, 53, 14, 7, 4, SSD1306_BLACK);
  // Sweat drop
  display.fillCircle(112, 20, 3, SSD1306_WHITE);
  display.fillTriangle(112, 14, 110, 20, 114, 20, SSD1306_WHITE);
  display.display();
}

void drawFaceDizzy() {
  display.clearDisplay();
  // Spiral eyes (X_X or @_@ style)
  static int spiralAngle = 0;
  spiralAngle += 30;
  
  // Left spiral eye
  int cx1 = 39, cy1 = 28;
  for (int r = 3; r <= 12; r += 3) {
    int startAngle = spiralAngle + r * 10;
    for (int a = startAngle; a < startAngle + 270; a += 15) {
      float rad = a * PI / 180.0;
      int px = cx1 + cos(rad) * r;
      int py = cy1 + sin(rad) * r;
      display.drawPixel(px, py, SSD1306_WHITE);
    }
  }
  
  // Right spiral eye
  int cx2 = 89, cy2 = 28;
  for (int r = 3; r <= 12; r += 3) {
    int startAngle = -spiralAngle + r * 10;
    for (int a = startAngle; a < startAngle + 270; a += 15) {
      float rad = a * PI / 180.0;
      int px = cx2 + cos(rad) * r;
      int py = cy2 + sin(rad) * r;
      display.drawPixel(px, py, SSD1306_WHITE);
    }
  }
  
  // Wavy mouth
  drawMouth(64, 55, 5);
  
  // Stars around head
  int starX = 64 + cos(spiralAngle * PI / 180.0) * 30;
  int starY = 8 + sin(spiralAngle * PI / 180.0) * 5;
  display.drawPixel(starX, starY, SSD1306_WHITE);
  display.drawLine(starX-2, starY, starX+2, starY, SSD1306_WHITE);
  display.drawLine(starX, starY-2, starX, starY+2, SSD1306_WHITE);
  
  int star2X = 64 + cos((spiralAngle + 120) * PI / 180.0) * 25;
  int star2Y = 8 + sin((spiralAngle + 120) * PI / 180.0) * 5;
  display.drawPixel(star2X, star2Y, SSD1306_WHITE);
  display.drawLine(star2X-2, star2Y, star2X+2, star2Y, SSD1306_WHITE);
  display.drawLine(star2X, star2Y-2, star2X, star2Y+2, SSD1306_WHITE);
  
  display.display();
}

void drawFaceThinking() {
  display.clearDisplay();
  // Eyes looking up-right
  drawEye(25, 18, 28, 28, 5, -5);   // Left eye, pupil up-right
  drawEye(75, 18, 28, 28, 5, -5);   // Right eye, pupil up-right
  // Thinking mouth (small to the side)
  display.fillCircle(75, 55, 3, SSD1306_WHITE);
  display.fillCircle(75, 55, 1, SSD1306_BLACK);
  // Thought bubble dots
  display.fillCircle(100, 10, 2, SSD1306_WHITE);
  display.fillCircle(108, 5, 3, SSD1306_WHITE);
  display.fillCircle(118, 3, 4, SSD1306_WHITE);
  display.display();
}

void drawFaceThinking2() {
  display.clearDisplay();
  // Eyes looking up-left
  drawEye(25, 18, 28, 28, -5, -5);   // Left eye, pupil up-left
  drawEye(75, 18, 28, 28, -5, -5);   // Right eye, pupil up-left
  // Thinking mouth
  display.fillCircle(53, 55, 3, SSD1306_WHITE);
  display.fillCircle(53, 55, 1, SSD1306_BLACK);
  // Thought bubble dots (other side)
  display.fillCircle(28, 10, 2, SSD1306_WHITE);
  display.fillCircle(20, 5, 3, SSD1306_WHITE);
  display.fillCircle(10, 3, 4, SSD1306_WHITE);
  display.display();
}

void drawFaceListening() {
  display.clearDisplay();
  // Wide attentive eyes
  drawEye(22, 15, 32, 32, 0, 0);   // Left eye bigger
  drawEye(72, 15, 32, 32, 0, 0);   // Right eye bigger
  // Small open mouth (attentive)
  display.fillRoundRect(59, 52, 10, 8, 4, SSD1306_WHITE);
  display.fillRoundRect(61, 54, 6, 4, 2, SSD1306_BLACK);
  // Sound wave indicators on sides
  display.drawCircleHelper(5, 32, 8, 0x01, SSD1306_WHITE);
  display.drawCircleHelper(5, 32, 12, 0x01, SSD1306_WHITE);
  display.drawCircleHelper(5, 32, 16, 0x01, SSD1306_WHITE);
  display.drawCircleHelper(123, 32, 8, 0x02, SSD1306_WHITE);
  display.drawCircleHelper(123, 32, 12, 0x02, SSD1306_WHITE);
  display.drawCircleHelper(123, 32, 16, 0x02, SSD1306_WHITE);
  display.display();
}

void drawFaceSpeaking() {
  display.clearDisplay();
  // Happy eyes while speaking
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(39 + i, 25 + dy, SSD1306_WHITE);
    display.drawPixel(39 + i, 26 + dy, SSD1306_WHITE);
  }
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(89 + i, 25 + dy, SSD1306_WHITE);
    display.drawPixel(89 + i, 26 + dy, SSD1306_WHITE);
  }
  // Open mouth (speaking)
  drawMouth(64, 52, 3);
  display.display();
}

void drawFaceSpeaking2() {
  display.clearDisplay();
  // Happy eyes while speaking
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(39 + i, 25 + dy, SSD1306_WHITE);
    display.drawPixel(39 + i, 26 + dy, SSD1306_WHITE);
  }
  for (int i = -12; i <= 12; i++) {
    int dy = (i * i) / 10;
    display.drawPixel(89 + i, 25 + dy, SSD1306_WHITE);
    display.drawPixel(89 + i, 26 + dy, SSD1306_WHITE);
  }
  // Small mouth (speaking frame 2)
  drawMouth(64, 52, 4);
  display.display();
}

void drawFaceSleepy() {
  display.clearDisplay();
  // Droopy half-closed eyes
  display.fillRoundRect(25, 30, 28, 10, 5, SSD1306_WHITE);  // Left
  display.fillRoundRect(75, 30, 28, 10, 5, SSD1306_WHITE);  // Right
  // Eyelids (top half black)
  display.fillRect(25, 25, 28, 8, SSD1306_BLACK);
  display.fillRect(75, 25, 28, 8, SSD1306_BLACK);
  // Small mouth
  drawMouth(64, 55, 0);
  // Zzz
  static int zzzFrame = 0;
  zzzFrame = (zzzFrame + 1) % 3;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (zzzFrame >= 0) { display.setCursor(100, 10); display.print("z"); }
  if (zzzFrame >= 1) { display.setCursor(107, 5); display.print("z"); }
  if (zzzFrame >= 2) { display.setCursor(114, 0); display.print("Z"); }
  display.display();
}

void drawFaceSurprised() {
  display.clearDisplay();
  // Very big round eyes
  display.fillRoundRect(18, 10, 38, 38, 18, SSD1306_WHITE);  // Left
  display.fillRoundRect(72, 10, 38, 38, 18, SSD1306_WHITE);  // Right
  // Big pupils
  display.fillCircle(37, 29, 8, SSD1306_BLACK);
  display.fillCircle(91, 29, 8, SSD1306_BLACK);
  // Highlight
  display.fillCircle(33, 24, 3, SSD1306_WHITE);
  display.fillCircle(87, 24, 3, SSD1306_WHITE);
  // Big O mouth
  display.fillCircle(64, 55, 6, SSD1306_WHITE);
  display.fillCircle(64, 55, 3, SSD1306_BLACK);
  // Exclamation mark
  display.fillRect(118, 5, 3, 10, SSD1306_WHITE);
  display.fillRect(118, 18, 3, 3, SSD1306_WHITE);
  display.display();
}

void drawFaceConfused() {
  display.clearDisplay();
  // One eye bigger than other
  drawEye(25, 20, 28, 28, 3, 0);    // Left normal
  drawEye(78, 22, 22, 22, -2, 0);   // Right smaller
  // Raised eyebrow
  display.drawLine(75, 15, 102, 18, SSD1306_WHITE);
  display.drawLine(75, 16, 102, 19, SSD1306_WHITE);
  // Confused mouth (wavy)
  drawMouth(64, 55, 5);
  // Question mark
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(110, 5);
  display.print("?");
  display.display();
}

void drawFaceUncomfortable() {
  display.clearDisplay();
  // Squished eyes
  display.fillRoundRect(25, 28, 28, 12, 5, SSD1306_WHITE);
  display.fillRoundRect(75, 28, 28, 12, 5, SSD1306_WHITE);
  // Pupils off-center
  display.fillRoundRect(30, 30, 8, 8, 3, SSD1306_BLACK);
  display.fillRoundRect(90, 30, 8, 8, 3, SSD1306_BLACK);
  // Uncomfortable eyebrows
  display.drawLine(25, 22, 40, 25, SSD1306_WHITE);
  display.drawLine(88, 25, 103, 22, SSD1306_WHITE);
  // Wavy mouth
  drawMouth(64, 55, 5);
  // Sweat
  display.fillCircle(110, 25, 2, SSD1306_WHITE);
  display.fillTriangle(110, 20, 108, 25, 112, 25, SSD1306_WHITE);
  display.display();
}

void drawFaceConnecting() {
  display.clearDisplay();
  // Normal eyes
  drawEye(25, 18, 28, 28, 0, 0);
  drawEye(75, 18, 28, 28, 0, 0);
  // Neutral mouth
  drawMouth(64, 55, 0);
  // WiFi icon animation
  static int wifiFrame = 0;
  wifiFrame = (wifiFrame + 1) % 4;
  int cx = 64, cy = 5;
  if (wifiFrame >= 1) display.drawCircleHelper(cx, cy + 10, 6, 0x03, SSD1306_WHITE);
  if (wifiFrame >= 2) display.drawCircleHelper(cx, cy + 10, 10, 0x03, SSD1306_WHITE);
  if (wifiFrame >= 3) display.drawCircleHelper(cx, cy + 10, 14, 0x03, SSD1306_WHITE);
  display.fillCircle(cx, cy + 10, 2, SSD1306_WHITE);
  display.display();
}

void drawFaceAPMode() {
  display.clearDisplay();
  // Cute waiting eyes
  drawEye(25, 15, 28, 28, 0, 0);
  drawEye(75, 15, 28, 28, 0, 0);
  // Small smile
  drawMouth(64, 52, 1);
  // Text at bottom
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 56);
  display.print("Connect: Dola-Setup");
  display.display();
}

void drawFaceLove() {
  display.clearDisplay();
  // Heart eyes
  // Left heart
  display.fillCircle(32, 24, 8, SSD1306_WHITE);
  display.fillCircle(44, 24, 8, SSD1306_WHITE);
  display.fillTriangle(24, 28, 52, 28, 38, 42, SSD1306_WHITE);
  // Right heart
  display.fillCircle(82, 24, 8, SSD1306_WHITE);
  display.fillCircle(94, 24, 8, SSD1306_WHITE);
  display.fillTriangle(74, 28, 102, 28, 88, 42, SSD1306_WHITE);
  // Happy mouth
  drawMouth(64, 55, 1);
  // Blush
  display.fillCircle(20, 45, 3, SSD1306_WHITE);
  display.fillCircle(108, 45, 3, SSD1306_WHITE);
  display.display();
}

// ============ MAIN FACE DISPATCHER ============
void showFace(DolaFace face) {
  switch (face) {
    case FACE_IDLE:          drawFaceIdle(); break;
    case FACE_BLINK:         drawFaceBlink(); break;
    case FACE_HAPPY:         drawFaceHappy(); break;
    case FACE_SAD:           drawFaceSad(); break;
    case FACE_ANGRY:         drawFaceAngry(); break;
    case FACE_SCARED:        drawFaceScared(); break;
    case FACE_DIZZY:         drawFaceDizzy(); break;
    case FACE_THINKING:      drawFaceThinking(); break;
    case FACE_THINKING_2:    drawFaceThinking2(); break;
    case FACE_LISTENING:     drawFaceListening(); break;
    case FACE_SPEAKING:      drawFaceSpeaking(); break;
    case FACE_SPEAKING_2:    drawFaceSpeaking2(); break;
    case FACE_SLEEPY:        drawFaceSleepy(); break;
    case FACE_SURPRISED:     drawFaceSurprised(); break;
    case FACE_CONFUSED:      drawFaceConfused(); break;
    case FACE_UNCOMFORTABLE: drawFaceUncomfortable(); break;
    case FACE_CONNECTING:    drawFaceConnecting(); break;
    case FACE_AP_MODE:       drawFaceAPMode(); break;
    case FACE_LOVE:          drawFaceLove(); break;
  }
}

#endif // DOLA_FACES_H
