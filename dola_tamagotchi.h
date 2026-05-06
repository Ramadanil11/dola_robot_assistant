/*
 * ============================================
 *   DOLA - Tamagotchi Ikan
 * ============================================
 * Pet virtual ikan yang hidup di OLED Dola.
 * Fitur: Stats, Evolusi, Save/Load, Gravestone
 * 
 * Kontrol:
 *   Tap = Beri makan
 *   Double Tap = Ajak main
 *   Hold pendek (1-2s) = Tidurkan
 *   Hold lama (>3s) = Keluar
 *   Shake = Elus
 */

#ifndef DOLA_TAMAGOTCHI_H
#define DOLA_TAMAGOTCHI_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_ADXL345_U.h>
#include <Preferences.h>

// External references
extern Adafruit_SSD1306 display;
extern Adafruit_ADXL345_Unified accel;
extern Preferences preferences;
extern void playWelcomeSound(const char* filename);
extern bool textToSpeech(String text);
extern void showFace(DolaFace face);

// ============ BITMAP DATA (PROGMEM) ============
// Stage 0: Telur (16x16)
static const unsigned char PROGMEM tama_egg_1[] = {
  0x00, 0x00, // ................
  0x03, 0xC0, // ......####......
  0x07, 0xE0, // .....######.....
  0x0F, 0xF0, // ....########....
  0x0F, 0xF0, // ....########....
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF8, // ...##########...
  0x0F, 0xF0, // ....########....
  0x0F, 0xF0, // ....########....
  0x07, 0xE0, // .....######.....
  0x03, 0xC0, // ......####......
  0x00, 0x00  // ................
};

// Telur frame 2 (sedikit miring - animasi goyang)
static const unsigned char PROGMEM tama_egg_2[] = {
  0x00, 0x00, // ................
  0x01, 0xE0, // .......####.....
  0x03, 0xF0, // ......######....
  0x07, 0xF8, // .....########...
  0x0F, 0xF8, // ....#########...
  0x0F, 0xF8, // ....#########...
  0x1F, 0xF8, // ...##########...
  0x1F, 0xF0, // ...#########....
  0x1F, 0xF0, // ...#########....
  0x1F, 0xF0, // ...#########....
  0x0F, 0xF0, // ....########....
  0x0F, 0xE0, // ....#######.....
  0x0F, 0xE0, // ....#######.....
  0x07, 0xC0, // .....#####......
  0x03, 0x80, // ......###.......
  0x00, 0x00  // ................
};

// Stage 1: Baby Fish (20x12)
static const unsigned char PROGMEM tama_baby_1[] = {
  0x00, 0x00, 0x00, // ........................
  0x01, 0xC0, 0x00, // .......###..............
  0x03, 0xE0, 0x00, // ......#####.............
  0x07, 0xF0, 0x60, // .....#######....##......
  0x0D, 0xF8, 0xF0, // ....##.######...####....
  0x1F, 0xFF, 0xF0, // ...#################....
  0x1F, 0xFF, 0xF0, // ...#################....
  0x0F, 0xF8, 0xF0, // ....#########...####....
  0x07, 0xF0, 0x60, // .....#######....##......
  0x03, 0xE0, 0x00, // ......#####.............
  0x01, 0xC0, 0x00, // .......###..............
  0x00, 0x00, 0x00  // ........................
};

// Baby Fish frame 2 (posisi sedikit naik)
static const unsigned char PROGMEM tama_baby_2[] = {
  0x01, 0xC0, 0x00, // .......###..............
  0x03, 0xE0, 0x00, // ......#####.............
  0x07, 0xF0, 0x60, // .....#######....##......
  0x0D, 0xF8, 0xF0, // ....##.######...####....
  0x1F, 0xFF, 0xF0, // ...#################....
  0x1F, 0xFF, 0xF0, // ...#################....
  0x0F, 0xF8, 0xF0, // ....#########...####....
  0x07, 0xF0, 0x60, // .....#######....##......
  0x03, 0xE0, 0x00, // ......#####.............
  0x01, 0xC0, 0x00, // .......###..............
  0x00, 0x00, 0x00, // ........................
  0x00, 0x00, 0x00  // ........................
};

// Stage 2: Teen Fish (24x14)
static const unsigned char PROGMEM tama_teen_1[] = {
  0x00, 0x0E, 0x00, // .............###........
  0x00, 0x1F, 0x00, // ............#####.......
  0x03, 0xFF, 0x00, // ......##########........
  0x07, 0xFF, 0x80, // .....############.......
  0x0F, 0xFF, 0xC0, // ....##############......
  0x1B, 0xFF, 0xF8, // ...##.##############....
  0x3F, 0xFF, 0xFC, // ..####################..
  0x3F, 0xFF, 0xFC, // ..####################..
  0x1F, 0xFF, 0xF8, // ...##################...
  0x0F, 0xFF, 0xC0, // ....##############......
  0x07, 0xFF, 0x80, // .....############.......
  0x03, 0xFF, 0x00, // ......##########........
  0x00, 0x1F, 0x00, // ............#####.......
  0x00, 0x0E, 0x00  // .............###........
};

// Teen Fish frame 2
static const unsigned char PROGMEM tama_teen_2[] = {
  0x00, 0x00, 0x00, // ........................
  0x00, 0x0E, 0x00, // .............###........
  0x03, 0xFF, 0x00, // ......##########........
  0x07, 0xFF, 0x80, // .....############.......
  0x0F, 0xFF, 0xC0, // ....##############......
  0x1B, 0xFF, 0xF8, // ...##.##############....
  0x3F, 0xFF, 0xFC, // ..####################..
  0x3F, 0xFF, 0xFC, // ..####################..
  0x1F, 0xFF, 0xF8, // ...##################...
  0x0F, 0xFF, 0xC0, // ....##############......
  0x07, 0xFF, 0x80, // .....############.......
  0x03, 0xFF, 0x00, // ......##########........
  0x00, 0x1F, 0x00, // ............#####.......
  0x00, 0x0E, 0x00  // .............###........
};

// Stage 3: Adult Fish (32x18)
static const unsigned char PROGMEM tama_adult_1[] = {
  0x00, 0x00, 0x38, 0x00, // ....................###.........
  0x00, 0x00, 0x7C, 0x00, // ...................#####........
  0x00, 0x03, 0xFE, 0x00, // ..............#########.........
  0x00, 0x0F, 0xFF, 0x00, // ............############........
  0x00, 0x3F, 0xFF, 0x80, // ..........#############.........
  0x00, 0xFF, 0xFF, 0xC0, // ........##################......
  0x03, 0xFF, 0xFF, 0xF0, // ......######################....
  0x06, 0xFF, 0xFF, 0xF8, // .....##.#####################...
  0x0F, 0xFF, 0xFF, 0xFC, // ....##########################..
  0x1F, 0xFF, 0xFF, 0xFE, // ...############################.
  0x0F, 0xFF, 0xFF, 0xFC, // ....##########################..
  0x07, 0xFF, 0xFF, 0xF8, // .....#########################..
  0x03, 0xFF, 0xFF, 0xF0, // ......######################....
  0x00, 0xFF, 0xFF, 0xC0, // ........##################......
  0x00, 0x3F, 0xFF, 0x80, // ..........#############.........
  0x00, 0x0F, 0xFF, 0x00, // ............############........
  0x00, 0x00, 0x7C, 0x00, // ...................#####........
  0x00, 0x00, 0x38, 0x00  // ....................###.........
};

// Adult Fish frame 2 (berenang naik)
static const unsigned char PROGMEM tama_adult_2[] = {
  0x00, 0x00, 0x00, 0x00, // ................................
  0x00, 0x00, 0x38, 0x00, // ....................###.........
  0x00, 0x00, 0x7C, 0x00, // ...................#####........
  0x00, 0x03, 0xFE, 0x00, // ..............#########.........
  0x00, 0x0F, 0xFF, 0x00, // ............############........
  0x00, 0x3F, 0xFF, 0x80, // ..........#############.........
  0x00, 0xFF, 0xFF, 0xC0, // ........##################......
  0x03, 0xFF, 0xFF, 0xF0, // ......######################....
  0x06, 0xFF, 0xFF, 0xF8, // .....##.#####################...
  0x0F, 0xFF, 0xFF, 0xFC, // ....##########################..
  0x1F, 0xFF, 0xFF, 0xFE, // ...############################.
  0x0F, 0xFF, 0xFF, 0xFC, // ....##########################..
  0x07, 0xFF, 0xFF, 0xF8, // .....#########################..
  0x03, 0xFF, 0xFF, 0xF0, // ......######################....
  0x00, 0xFF, 0xFF, 0xC0, // ........##################......
  0x00, 0x3F, 0xFF, 0x80, // ..........#############.........
  0x00, 0x0F, 0xFF, 0x00, // ............############........
  0x00, 0x00, 0x7C, 0x00  // ...................#####........
};

// Dead Fish (terbalik, 24x12)
static const unsigned char PROGMEM tama_dead[] = {
  0x00, 0x1F, 0x00, // ............#####.......
  0x00, 0x3F, 0x80, // ..........#######.......
  0x03, 0xFF, 0xC0, // ......############......
  0x07, 0xFF, 0xF0, // .....##############.....
  0x0F, 0xFF, 0xF8, // ....################....
  0x1F, 0xFF, 0xFC, // ...##################...
  0x3F, 0xFF, 0xFC, // ..###################...
  0x1A, 0xFF, 0xF8, // ...#.#.#############....
  0x0F, 0xFF, 0xF0, // ....################....
  0x07, 0xFF, 0xC0, // .....##############.....
  0x01, 0xFF, 0x00, // .......#########........
  0x00, 0x7C, 0x00  // .........#####..........
};

// ============ TAMAGOTCHI DATA STRUCTURE ============
struct TamagotchiData {
  uint8_t hunger;      // 0-100
  uint8_t happiness;   // 0-100
  uint8_t energy;      // 0-100
  uint8_t health;      // 0-100
  uint8_t stage;       // 0=egg, 1=baby, 2=teen, 3=adult
  uint32_t aliveTime;  // Total detik hidup
  uint32_t lastUpdate; // millis() terakhir update
  bool isDead;         // Pet mati?
  uint32_t bestAge;    // Rekor umur terlama (detik)
  uint16_t deathCount; // Total kali mati
};

// Gravestone entry
struct GraveEntry {
  uint8_t stage;       // Stage saat mati
  uint32_t age;        // Umur saat mati (detik)
};

// ============ TAMAGOTCHI STATE ============
enum TamaState {
  TAMA_ACTIVE,       // Pet hidup, normal
  TAMA_SLEEPING,     // Pet tidur
  TAMA_EATING,       // Animasi makan
  TAMA_PLAYING,      // Animasi main
  TAMA_PETTING,      // Animasi dielus
  TAMA_SICK,         // Pet sakit (health < 20)
  TAMA_DEAD,         // Pet mati
  TAMA_GRAVESTONE,   // Tampilan kuburan
  TAMA_HATCHING      // Telur menetas
};

TamagotchiData tamaData;
TamaState tamaState = TAMA_ACTIVE;
GraveEntry graveyard[5];  // 5 kuburan terakhir

// Timing
unsigned long tamaLastDecay = 0;
unsigned long tamaLastSave = 0;
unsigned long tamaLastAnim = 0;
unsigned long tamaActionStart = 0;
unsigned long tamaHoldStart = 0;
bool tamaAnimFrame = false;
bool tamaInitialized = false;

// Bubble positions (dekorasi air)
int bubbleX[3] = {10, 60, 100};
int bubbleY[3] = {50, 40, 55};

// ============ SAVE / LOAD ============
void saveTamagotchi() {
  preferences.putUChar("tama_hunger", tamaData.hunger);
  preferences.putUChar("tama_happy", tamaData.happiness);
  preferences.putUChar("tama_energy", tamaData.energy);
  preferences.putUChar("tama_health", tamaData.health);
  preferences.putUChar("tama_stage", tamaData.stage);
  preferences.putUInt("tama_alive", tamaData.aliveTime);
  preferences.putUInt("tama_last", millis());
  preferences.putBool("tama_dead", tamaData.isDead);
  preferences.putUInt("tama_best", tamaData.bestAge);
  preferences.putUShort("tama_deaths", tamaData.deathCount);
  
  // Save graveyard
  for (int i = 0; i < 5; i++) {
    char key[12];
    sprintf(key, "tama_gs%d", i);
    preferences.putUChar(key, graveyard[i].stage);
    sprintf(key, "tama_ga%d", i);
    preferences.putUInt(key, graveyard[i].age);
  }
  
  Serial.println("Tamagotchi saved!");
}

void loadTamagotchi() {
  tamaData.hunger = preferences.getUChar("tama_hunger", 80);
  tamaData.happiness = preferences.getUChar("tama_happy", 80);
  tamaData.energy = preferences.getUChar("tama_energy", 100);
  tamaData.health = preferences.getUChar("tama_health", 100);
  tamaData.stage = preferences.getUChar("tama_stage", 0);
  tamaData.aliveTime = preferences.getUInt("tama_alive", 0);
  tamaData.lastUpdate = preferences.getUInt("tama_last", 0);
  tamaData.isDead = preferences.getBool("tama_dead", false);
  tamaData.bestAge = preferences.getUInt("tama_best", 0);
  tamaData.deathCount = preferences.getUShort("tama_deaths", 0);
  
  // Load graveyard
  for (int i = 0; i < 5; i++) {
    char key[12];
    sprintf(key, "tama_gs%d", i);
    graveyard[i].stage = preferences.getUChar(key, 0);
    sprintf(key, "tama_ga%d", i);
    graveyard[i].age = preferences.getUInt(key, 0);
  }
  
  // Hitung decay offline (berapa lama robot mati)
  if (!tamaData.isDead && tamaData.lastUpdate > 0) {
    uint32_t now = millis();
    // Estimasi waktu offline: jika lastUpdate > now, berarti robot di-restart
    // Gunakan aliveTime sebagai referensi
    // Asumsi: setiap restart, decay 30 detik worth of stats
    // (karena kita tidak bisa tahu pasti berapa lama offline tanpa RTC)
    uint32_t offlineDecayCycles = 10; // Asumsi 5 menit offline = 10 cycles
    
    for (uint32_t i = 0; i < offlineDecayCycles; i++) {
      if (tamaData.hunger > 2) tamaData.hunger -= 2;
      else tamaData.hunger = 0;
      if (tamaData.happiness > 1) tamaData.happiness -= 1;
      else tamaData.happiness = 0;
      if (tamaData.energy > 1) tamaData.energy -= 1;
      else tamaData.energy = 0;
      
      if (tamaData.hunger < 20 && tamaData.health > 0) {
        tamaData.health -= 1;
      }
    }
    
    // Check if died while offline
    if (tamaData.health == 0) {
      tamaData.isDead = true;
    }
  }
  
  Serial.printf("Tamagotchi loaded! Stage:%d HP:%d Dead:%d\n", 
                tamaData.stage, tamaData.health, tamaData.isDead);
}

void resetTamagotchi() {
  tamaData.hunger = 80;
  tamaData.happiness = 80;
  tamaData.energy = 100;
  tamaData.health = 100;
  tamaData.stage = 0;
  tamaData.aliveTime = 0;
  tamaData.lastUpdate = millis();
  tamaData.isDead = false;
  // bestAge dan deathCount TIDAK di-reset
  
  tamaState = TAMA_ACTIVE;
  saveTamagotchi();
}

// ============ GRAVESTONE SYSTEM ============
void addToGraveyard(uint8_t stage, uint32_t age) {
  // Shift semua entry ke bawah
  for (int i = 4; i > 0; i--) {
    graveyard[i] = graveyard[i - 1];
  }
  // Tambah entry baru di posisi 0
  graveyard[0].stage = stage;
  graveyard[0].age = age;
}

const char* getStageName(uint8_t stage) {
  switch (stage) {
    case 0: return "Telur";
    case 1: return "Baby";
    case 2: return "Teen";
    case 3: return "Adult";
    default: return "???";
  }
}

void formatTime(uint32_t seconds, char* buf) {
  if (seconds >= 86400) {
    int days = seconds / 86400;
    int hours = (seconds % 86400) / 3600;
    sprintf(buf, "%dh %dj", days, hours);
  } else if (seconds >= 3600) {
    int hours = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    sprintf(buf, "%dj %dm", hours, mins);
  } else {
    int mins = seconds / 60;
    sprintf(buf, "%d menit", mins);
  }
}

// ============ DECAY & EVOLUTION ============
void tamaDecay() {
  if (tamaState == TAMA_SLEEPING) {
    // Saat tidur: energy naik, yang lain tetap decay tapi lebih lambat
    if (tamaData.energy < 100) tamaData.energy += 2;
    if (tamaData.hunger > 1) tamaData.hunger -= 1;
    // happiness tidak turun saat tidur
  } else {
    // Normal decay
    if (tamaData.hunger > 2) tamaData.hunger -= 2;
    else tamaData.hunger = 0;
    
    if (tamaData.happiness > 1) tamaData.happiness -= 1;
    else tamaData.happiness = 0;
    
    if (tamaData.energy > 1) tamaData.energy -= 1;
    else tamaData.energy = 0;
  }
  
  // Health logic
  if (tamaData.hunger < 20) {
    if (tamaData.health > 0) tamaData.health -= 1;
  }
  if (tamaData.happiness < 15) {
    if (tamaData.health > 0) tamaData.health -= 1;
  }
  // Recovery: semua stats bagus
  if (tamaData.hunger > 70 && tamaData.happiness > 70 && 
      tamaData.energy > 50 && tamaData.health < 100) {
    tamaData.health += 1;
  }
  
  // Update alive time (+30 detik per decay cycle)
  tamaData.aliveTime += 30;
  
  // Check death
  if (tamaData.health == 0) {
    tamaDie();
  }
  
  // Check evolution
  checkEvolution();
}

void checkEvolution() {
  uint8_t oldStage = tamaData.stage;
  
  switch (tamaData.stage) {
    case 0: // Egg -> Baby (5 menit = 300 detik, health > 50)
      if (tamaData.aliveTime >= 300 && tamaData.health > 50) {
        tamaData.stage = 1;
        tamaState = TAMA_HATCHING;
        tamaActionStart = millis();
      }
      break;
    case 1: // Baby -> Teen (1 jam = 3600 detik, health > 40)
      if (tamaData.aliveTime >= 3600 && tamaData.health > 40) {
        tamaData.stage = 2;
        tamaState = TAMA_HATCHING;
        tamaActionStart = millis();
      }
      break;
    case 2: // Teen -> Adult (3 jam = 10800 detik, health > 50)
      if (tamaData.aliveTime >= 10800 && tamaData.health > 50) {
        tamaData.stage = 3;
        tamaState = TAMA_HATCHING;
        tamaActionStart = millis();
      }
      break;
    case 3: // Adult - max stage
      break;
  }
  
  if (tamaData.stage != oldStage) {
    Serial.printf("EVOLUSI! Stage %d -> %d\n", oldStage, tamaData.stage);
    saveTamagotchi();
  }
}

void tamaDie() {
  tamaData.isDead = true;
  tamaState = TAMA_DEAD;
  
  // Update best age
  if (tamaData.aliveTime > tamaData.bestAge) {
    tamaData.bestAge = tamaData.aliveTime;
  }
  
  // Increment death count
  tamaData.deathCount++;
  
  // Add to graveyard
  addToGraveyard(tamaData.stage, tamaData.aliveTime);
  
  // Save
  saveTamagotchi();
  
  // TTS
  textToSpeech("Yah... ikan mu mati... maaf ya. Tap untuk mulai lagi~");
  playWelcomeSound("/sad.wav");
  
  Serial.printf("Ikan mati! Umur: %d detik, Stage: %d\n", tamaData.aliveTime, tamaData.stage);
}

// ============ ACTIONS ============
void tamaFeed() {
  if (tamaData.isDead) return;
  if (tamaData.hunger >= 100) {
    // Sudah kenyang
    textToSpeech("Ikan nya udah kenyang nih~");
    return;
  }
  
  tamaData.hunger += 20;
  if (tamaData.hunger > 100) tamaData.hunger = 100;
  tamaData.happiness += 5;
  if (tamaData.happiness > 100) tamaData.happiness = 100;
  
  tamaState = TAMA_EATING;
  tamaActionStart = millis();
  
  Serial.printf("Feed! Hunger: %d\n", tamaData.hunger);
}

void tamaPlay() {
  if (tamaData.isDead) return;
  if (tamaData.energy < 10) {
    textToSpeech("Ikan nya capek, tidurin dulu~");
    return;
  }
  
  tamaData.happiness += 15;
  if (tamaData.happiness > 100) tamaData.happiness = 100;
  tamaData.energy -= 10;
  if (tamaData.energy < 0) tamaData.energy = 0;
  
  tamaState = TAMA_PLAYING;
  tamaActionStart = millis();
  
  Serial.printf("Play! Happiness: %d, Energy: %d\n", tamaData.happiness, tamaData.energy);
}

void tamaSleep() {
  if (tamaData.isDead) return;
  
  tamaState = TAMA_SLEEPING;
  tamaActionStart = millis();
  
  Serial.println("Ikan tidur... zzz");
}

void tamaWakeUp() {
  if (tamaState == TAMA_SLEEPING) {
    tamaState = TAMA_ACTIVE;
    // Bonus energy saat bangun
    tamaData.energy += 30;
    if (tamaData.energy > 100) tamaData.energy = 100;
    Serial.println("Ikan bangun!");
  }
}

void tamaPet() {
  if (tamaData.isDead) return;
  
  tamaData.happiness += 10;
  if (tamaData.happiness > 100) tamaData.happiness = 100;
  
  tamaState = TAMA_PETTING;
  tamaActionStart = millis();
  
  Serial.printf("Pet! Happiness: %d\n", tamaData.happiness);
}

// ============ RENDERING ============
void renderBubbles() {
  // Animasi gelembung naik
  for (int i = 0; i < 3; i++) {
    display.drawCircle(bubbleX[i], bubbleY[i], 1, SSD1306_WHITE);
    bubbleY[i] -= 1;
    if (bubbleY[i] < 10) {
      bubbleY[i] = 55 + random(5);
      bubbleX[i] = random(5, 123);
    }
  }
}

void renderStatsBar() {
  // Baris atas: 4 stat bars
  int barW = 25;
  int barH = 5;
  int y = 1;
  
  // Hunger (H)
  display.setCursor(0, y);
  display.setTextSize(1);
  display.print("H");
  display.drawRect(7, y, barW, barH, SSD1306_WHITE);
  display.fillRect(7, y, map(tamaData.hunger, 0, 100, 0, barW), barH, SSD1306_WHITE);
  
  // Happiness (P)
  display.setCursor(35, y);
  display.print("P");
  display.drawRect(42, y, barW, barH, SSD1306_WHITE);
  display.fillRect(42, y, map(tamaData.happiness, 0, 100, 0, barW), barH, SSD1306_WHITE);
  
  // Energy (E)
  display.setCursor(70, y);
  display.print("E");
  display.drawRect(77, y, barW, barH, SSD1306_WHITE);
  display.fillRect(77, y, map(tamaData.energy, 0, 100, 0, barW), barH, SSD1306_WHITE);
  
  // Health (heart icon + number)
  display.setCursor(105, y);
  display.print("\x03"); // Heart character
  display.printf("%d", tamaData.health);
}

void renderFish() {
  int centerX = 40;
  int centerY = 30;
  
  switch (tamaData.stage) {
    case 0: // Egg
      if (tamaAnimFrame) {
        display.drawBitmap(centerX, centerY, tama_egg_1, 16, 16, SSD1306_WHITE);
      } else {
        display.drawBitmap(centerX + 1, centerY, tama_egg_2, 16, 16, SSD1306_WHITE);
      }
      break;
      
    case 1: // Baby
      if (tamaAnimFrame) {
        display.drawBitmap(centerX - 2, centerY, tama_baby_1, 24, 12, SSD1306_WHITE);
      } else {
        display.drawBitmap(centerX - 2, centerY - 2, tama_baby_2, 24, 12, SSD1306_WHITE);
      }
      // Mata
      display.drawPixel(centerX + 4, centerY + 4, SSD1306_BLACK);
      break;
      
    case 2: // Teen
      if (tamaAnimFrame) {
        display.drawBitmap(centerX - 4, centerY - 2, tama_teen_1, 24, 14, SSD1306_WHITE);
      } else {
        display.drawBitmap(centerX - 4, centerY, tama_teen_2, 24, 14, SSD1306_WHITE);
      }
      // Mata
      display.fillCircle(centerX + 3, centerY + 5, 2, SSD1306_BLACK);
      display.drawPixel(centerX + 3, centerY + 5, SSD1306_WHITE);
      break;
      
    case 3: // Adult
      if (tamaAnimFrame) {
        display.drawBitmap(centerX - 8, centerY - 4, tama_adult_1, 32, 18, SSD1306_WHITE);
      } else {
        display.drawBitmap(centerX - 8, centerY - 2, tama_adult_2, 32, 18, SSD1306_WHITE);
      }
      // Mata (lebih detail)
      display.fillCircle(centerX + 2, centerY + 4, 2, SSD1306_BLACK);
      display.drawPixel(centerX + 2, centerY + 4, SSD1306_WHITE);
      // Sisik pattern
      for (int i = 0; i < 3; i++) {
        display.drawPixel(centerX + 8 + (i * 4), centerY + 2 + (i % 2), SSD1306_BLACK);
        display.drawPixel(centerX + 10 + (i * 4), centerY + 5 + (i % 2), SSD1306_BLACK);
      }
      break;
  }
}

void renderInfoBar() {
  int y = 57;
  display.setCursor(0, y);
  display.setTextSize(1);
  
  // Age
  char timeBuf[16];
  formatTime(tamaData.aliveTime, timeBuf);
  display.printf("%s ", getStageName(tamaData.stage));
  display.print(timeBuf);
  
  // Stage indicator di kanan
  display.setCursor(100, y);
  for (int i = 0; i <= 3; i++) {
    if (i <= tamaData.stage) display.print("\x04"); // Filled diamond
    else display.print(".");
  }
}

void renderTamagotchiActive() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Stats bar di atas
  renderStatsBar();
  
  // Garis pemisah
  display.drawLine(0, 8, 127, 8, SSD1306_WHITE);
  
  // Background bubbles
  renderBubbles();
  
  // Ikan
  renderFish();
  
  // Warning jika sakit
  if (tamaData.health < 20 && tamaData.health > 0) {
    // Flash warning
    if ((millis() / 500) % 2 == 0) {
      display.setCursor(90, 20);
      display.setTextSize(2);
      display.print("!");
      display.setTextSize(1);
    }
  }
  
  // Info bar di bawah
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  renderInfoBar();
  
  display.display();
}

void renderTamagotchiSleeping() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Header
  display.setCursor(35, 0);
  display.setTextSize(1);
  display.print("Zzz...");
  
  display.drawLine(0, 8, 127, 8, SSD1306_WHITE);
  
  // Ikan (diam, tidak animasi)
  // Gambar ikan di posisi bawah (tidur di dasar)
  int centerX = 40;
  int centerY = 35;
  
  switch (tamaData.stage) {
    case 0: display.drawBitmap(centerX, centerY, tama_egg_1, 16, 16, SSD1306_WHITE); break;
    case 1: display.drawBitmap(centerX - 2, centerY, tama_baby_1, 24, 12, SSD1306_WHITE); break;
    case 2: display.drawBitmap(centerX - 4, centerY, tama_teen_1, 24, 14, SSD1306_WHITE); break;
    case 3: display.drawBitmap(centerX - 8, centerY, tama_adult_1, 32, 18, SSD1306_WHITE); break;
  }
  
  // Zzz animation
  static int zzzFrame = 0;
  int zx = 70;
  int zy = 25 - (zzzFrame % 3) * 5;
  display.setCursor(zx, zy);
  display.print("z");
  display.setCursor(zx + 8, zy - 5);
  display.print("Z");
  display.setCursor(zx + 16, zy - 10);
  display.print("z");
  zzzFrame++;
  
  // Energy bar (menunjukkan progress tidur)
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.printf("Energy: %d%%  tap=bangun", tamaData.energy);
  
  display.display();
}

void renderTamagotchiEating() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  renderStatsBar();
  display.drawLine(0, 8, 127, 8, SSD1306_WHITE);
  
  // Ikan + makanan
  renderFish();
  
  // Animasi makanan (titik-titik di depan mulut)
  int foodX = 30;
  int foodY = 33;
  if ((millis() / 200) % 2 == 0) {
    display.fillCircle(foodX, foodY, 2, SSD1306_WHITE);
    display.fillCircle(foodX - 5, foodY + 3, 1, SSD1306_WHITE);
    display.fillCircle(foodX - 3, foodY - 2, 1, SSD1306_WHITE);
  }
  
  // Text
  display.setCursor(75, 30);
  display.print("Nyam~!");
  
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  renderInfoBar();
  display.display();
}

void renderTamagotchiPlaying() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  renderStatsBar();
  display.drawLine(0, 8, 127, 8, SSD1306_WHITE);
  
  // Ikan lompat-lompat (posisi berubah cepat)
  int offsetY = ((millis() / 150) % 4) * 3 - 6;
  int centerX = 40;
  int centerY = 30 + offsetY;
  
  switch (tamaData.stage) {
    case 0: display.drawBitmap(centerX, centerY, tama_egg_1, 16, 16, SSD1306_WHITE); break;
    case 1: display.drawBitmap(centerX - 2, centerY, tama_baby_1, 24, 12, SSD1306_WHITE); break;
    case 2: display.drawBitmap(centerX - 4, centerY, tama_teen_1, 24, 14, SSD1306_WHITE); break;
    case 3: display.drawBitmap(centerX - 8, centerY, tama_adult_1, 32, 18, SSD1306_WHITE); break;
  }
  
  // Splash effect
  display.setCursor(75, 25);
  display.print("Splash!");
  
  // Water drops
  for (int i = 0; i < 4; i++) {
    int dx = random(70, 110);
    int dy = random(30, 50);
    display.drawPixel(dx, dy, SSD1306_WHITE);
  }
  
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  renderInfoBar();
  display.display();
}

void renderTamagotchiPetting() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  renderStatsBar();
  display.drawLine(0, 8, 127, 8, SSD1306_WHITE);
  
  renderFish();
  
  // Heart animation
  int hx = 65 + ((millis() / 300) % 3) * 5;
  int hy = 20 - ((millis() / 300) % 3) * 3;
  display.setCursor(hx, hy);
  display.print("\x03"); // Heart
  display.setCursor(hx + 10, hy + 5);
  display.print("\x03");
  
  display.setCursor(80, 35);
  display.print("Suka~!");
  
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  renderInfoBar();
  display.display();
}

void renderTamagotchiDead() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Header R.I.P
  display.setCursor(35, 0);
  display.setTextSize(1);
  display.print("R.I.P");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // Dead fish (terbalik)
  display.drawBitmap(40, 15, tama_dead, 24, 12, SSD1306_WHITE);
  
  // X eyes overlay
  display.setCursor(48, 18);
  display.print("x");
  
  // Gelembung terakhir
  display.drawCircle(60, 13, 1, SSD1306_WHITE);
  
  // Info
  display.setCursor(0, 30);
  char timeBuf[16];
  formatTime(tamaData.aliveTime, timeBuf);
  display.printf("Umur: %s", timeBuf);
  
  display.setCursor(0, 39);
  char bestBuf[16];
  formatTime(tamaData.bestAge, bestBuf);
  display.printf("Rekor: %s", bestBuf);
  
  // Check if new record
  if (tamaData.aliveTime >= tamaData.bestAge && tamaData.aliveTime > 0) {
    display.print(" NEW!");
  }
  
  display.setCursor(0, 48);
  display.printf("Mati ke-%d", tamaData.deathCount);
  
  // Instructions
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.print("tap:lagi dtap:kubur hold:exit");
  
  display.display();
}

void renderGravestone() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Header
  display.setCursor(20, 0);
  display.setTextSize(1);
  display.print("KUBURAN IKAN");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  // List 5 kuburan terakhir
  int y = 12;
  bool hasEntry = false;
  
  for (int i = 0; i < 5; i++) {
    if (graveyard[i].age > 0) {
      hasEntry = true;
      char timeBuf[16];
      formatTime(graveyard[i].age, timeBuf);
      
      display.setCursor(0, y);
      display.printf("%d.%s %s", i + 1, getStageName(graveyard[i].stage), timeBuf);
      
      // Mark best
      if (graveyard[i].age == tamaData.bestAge) {
        display.print(" *");
      }
      
      y += 9;
    }
  }
  
  if (!hasEntry) {
    display.setCursor(20, 25);
    display.print("Belum ada kuburan");
  }
  
  // Footer
  display.drawLine(0, 55, 127, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.printf("Total mati:%d  tap=back", tamaData.deathCount);
  
  display.display();
}

void renderHatching() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Animasi evolusi
  display.setCursor(25, 5);
  display.setTextSize(1);
  display.print("!! EVOLUSI !!");
  
  // Flash effect
  if ((millis() / 200) % 2 == 0) {
    display.fillRect(20, 20, 88, 30, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(30, 30);
    display.setTextSize(1);
    display.printf("-> %s!", getStageName(tamaData.stage));
    display.setTextColor(SSD1306_WHITE);
  } else {
    renderFish();
  }
  
  display.setCursor(15, 55);
  display.setTextSize(1);
  display.printf("Ikan mu jadi %s!", getStageName(tamaData.stage));
  
  display.display();
}

// ============ MAIN TAMAGOTCHI FUNCTIONS ============
void startTamagotchi() {
  loadTamagotchi();
  tamaInitialized = true;
  tamaLastDecay = millis();
  tamaLastSave = millis();
  tamaLastAnim = millis();
  tamaAnimFrame = false;
  tamaHoldStart = 0;
  
  if (tamaData.isDead) {
    tamaState = TAMA_DEAD;
  } else {
    tamaState = TAMA_ACTIVE;
  }
  
  Serial.println("Tamagotchi started!");
}

// Returns true when exiting game
bool updateTamagotchi() {
  unsigned long now = millis();
  
  // Animation frame toggle (setiap 600ms)
  if (now - tamaLastAnim > 600) {
    tamaAnimFrame = !tamaAnimFrame;
    tamaLastAnim = now;
  }
  
  // Decay setiap 30 detik (hanya saat aktif/tidur)
  if ((tamaState == TAMA_ACTIVE || tamaState == TAMA_SLEEPING || tamaState == TAMA_SICK) 
      && now - tamaLastDecay > 30000) {
    tamaDecay();
    tamaLastDecay = now;
  }
  
  // Auto-save setiap 60 detik
  if (now - tamaLastSave > 60000) {
    saveTamagotchi();
    tamaLastSave = now;
  }
  
  // Check sick state
  if (tamaState == TAMA_ACTIVE && tamaData.health < 20 && tamaData.health > 0) {
    tamaState = TAMA_SICK;
  }
  if (tamaState == TAMA_SICK && tamaData.health >= 20) {
    tamaState = TAMA_ACTIVE;
  }
  
  // Handle action animations (kembali ke active setelah 2 detik)
  if ((tamaState == TAMA_EATING || tamaState == TAMA_PLAYING || tamaState == TAMA_PETTING) 
      && now - tamaActionStart > 2000) {
    tamaState = TAMA_ACTIVE;
  }
  
  // Handle hatching animation (3 detik)
  if (tamaState == TAMA_HATCHING && now - tamaActionStart > 3000) {
    tamaState = TAMA_ACTIVE;
    char msg[64];
    sprintf(msg, "Yeay! Ikan mu berevolusi jadi %s!", getStageName(tamaData.stage));
    textToSpeech(msg);
  }
  
  // Handle sleeping (auto wake up setelah energy penuh atau 30 detik)
  if (tamaState == TAMA_SLEEPING) {
    if (tamaData.energy >= 100 || (now - tamaActionStart > 30000)) {
      tamaWakeUp();
    }
  }
  
  // Detect shake (elus) via accelerometer
  sensors_event_t event;
  accel.getEvent(&event);
  float totalAccel = sqrt(event.acceleration.x * event.acceleration.x + 
                          event.acceleration.y * event.acceleration.y + 
                          event.acceleration.z * event.acceleration.z);
  if (totalAccel > 15.0 && tamaState == TAMA_ACTIVE && !tamaData.isDead) {
    tamaPet();
  }
  
  // Render based on state
  switch (tamaState) {
    case TAMA_ACTIVE:
    case TAMA_SICK:
      renderTamagotchiActive();
      break;
    case TAMA_SLEEPING:
      renderTamagotchiSleeping();
      break;
    case TAMA_EATING:
      renderTamagotchiEating();
      break;
    case TAMA_PLAYING:
      renderTamagotchiPlaying();
      break;
    case TAMA_PETTING:
      renderTamagotchiPetting();
      break;
    case TAMA_DEAD:
      renderTamagotchiDead();
      break;
    case TAMA_GRAVESTONE:
      renderGravestone();
      break;
    case TAMA_HATCHING:
      renderHatching();
      break;
  }
  
  return false; // Never auto-exit
}

// Handle touch gesture for Tamagotchi
// Returns true if should exit game
bool handleTamagotchiGesture(TouchGesture gesture) {
  unsigned long now = millis();
  
  switch (tamaState) {
    case TAMA_ACTIVE:
    case TAMA_SICK:
      if (gesture == GESTURE_TAP) {
        tamaFeed();
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        tamaPlay();
      } else if (gesture == GESTURE_HOLD) {
        // Hold: cek durasi - pendek = tidur, lama = exit
        tamaSleep();
      }
      break;
      
    case TAMA_SLEEPING:
      if (gesture == GESTURE_TAP || gesture == GESTURE_DOUBLE_TAP) {
        tamaWakeUp();
      } else if (gesture == GESTURE_HOLD) {
        // Exit game
        saveTamagotchi();
        tamaInitialized = false;
        return true;
      }
      break;
      
    case TAMA_DEAD:
      if (gesture == GESTURE_TAP) {
        // Rebirth
        resetTamagotchi();
        textToSpeech("Telur baru! Rawat yang baik ya~");
      } else if (gesture == GESTURE_DOUBLE_TAP) {
        // Show gravestone
        tamaState = TAMA_GRAVESTONE;
      } else if (gesture == GESTURE_HOLD) {
        // Exit
        tamaInitialized = false;
        return true;
      }
      break;
      
    case TAMA_GRAVESTONE:
      if (gesture == GESTURE_TAP || gesture == GESTURE_HOLD || gesture == GESTURE_DOUBLE_TAP) {
        tamaState = TAMA_DEAD;
      }
      break;
      
    case TAMA_EATING:
    case TAMA_PLAYING:
    case TAMA_PETTING:
    case TAMA_HATCHING:
      // Ignore input during animations
      if (gesture == GESTURE_HOLD) {
        saveTamagotchi();
        tamaInitialized = false;
        return true;
      }
      break;
  }
  
  return false;
}

void exitTamagotchi() {
  if (tamaInitialized) {
    saveTamagotchi();
    tamaInitialized = false;
  }
}

#endif // DOLA_TAMAGOTCHI_H
