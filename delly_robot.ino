/*
 * ============================================
 *   DOLA - Robot Asisten AI Ekspresif
 * ============================================
 * Hardware:
 *   - ESP32 DevKit
 *   - INMP441 I2S Microphone
 *   - MAX98357 I2S DAC (Speaker)
 *   - OLED SSD1306 0.96" (Wajah/Ekspresi)
 *   - ADXL345 Accelerometer (Deteksi gerakan)
 *   - Touch Sensor (T0 / GPIO 4)
 * 
 * Features:
 *   - WiFi provisioning via AP + Web Portal
 *   - Voice recognition (STT) -> AI Chat -> TTS -> Speaker
 *   - Ekspresi wajah lucu & reaktif
 *   - Sensor gerak: diguncang=pusing, diangkat=takut/marah
 *   - Menu system dengan touch navigation
 *   - 7 Games: Tebak Angka, Simon Says, Shake, Quiz, BSK, Jokes, Tamagotchi
 *   - Clock (NTP WIB) + Timer
 *   - Settings: Volume, WiFi Reset
 */

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <driver/i2s.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADXL345_U.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <base64.h>
#include <Update.h>
#include "dola_faces.h"
#include "dola_touch.h"
#include "dola_memory.h"

// ============ PIN DEFINITIONS ============
// INMP441 Microphone (I2S Port 0)
#define I2S_MIC_PORT      I2S_NUM_0
#define I2S_MIC_SCK       26
#define I2S_MIC_WS        25
#define I2S_MIC_SD        33

// MAX98357 Speaker (I2S Port 1)
#define I2S_SPK_PORT      I2S_NUM_1
#define I2S_SPK_BCLK      27
#define I2S_SPK_LRC       14
#define I2S_SPK_DIN       12

// OLED Display
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64
#define OLED_RESET        -1
#define OLED_ADDR         0x3C

// ADXL345
#define ADXL345_ADDR      0x53

// ============ CONSTANTS ============
#define SAMPLE_RATE       16000
#define SAMPLE_BITS       16
#define RECORD_DURATION   5       // seconds max recording
#define BUFFER_SIZE       512
#define SILENCE_THRESHOLD 500     // threshold untuk deteksi silence
#define SILENCE_DURATION  1500    // ms silence sebelum stop recording

// AI API
const char* AI_API_KEY = "enx-4d749449c8308d60e6e713b471783037aaa3811a991c7c7fa99b0a6912c42224";
const char* STT_ENDPOINT = "https://api.enowx.com/v1/audio/transcriptions";
const char* CHAT_ENDPOINT = "https://api.enowx.com/v1/chat/completions";
const char* TTS_ENDPOINT = "https://api.enowx.com/v1/audio/speech";

// ============ OBJECTS ============
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
WebServer server(80);
Preferences preferences;

// ============ STATE ============
enum DolaState {
  STATE_AP_MODE,        // Waiting for WiFi config
  STATE_CONNECTING,     // Connecting to WiFi
  STATE_IDLE,           // Normal idle, showing face
  STATE_LISTENING,      // Recording audio
  STATE_THINKING,       // Processing AI
  STATE_SPEAKING,       // Playing audio response
  STATE_DIZZY,          // Diguncang-guncang
  STATE_SCARED,         // Diangkat tinggi
  STATE_ANGRY,          // Diangkat (marah)
  STATE_HAPPY,          // Setelah berhasil jawab
  STATE_SLEEPY,         // Idle lama
  STATE_MENU,           // Di menu / game / clock / timer
};

DolaState currentState = STATE_AP_MODE;
DolaState previousState = STATE_IDLE;

// WiFi credentials
String savedSSID = "";
String savedPassword = "";
bool wifiConnected = false;

// Audio buffer
int16_t* audioBuffer = nullptr;
size_t audioBufferSize = 0;
bool isRecording = false;

// Motion detection
float lastAccelX = 0, lastAccelY = 0, lastAccelZ = 0;
unsigned long shakeStartTime = 0;
int shakeCount = 0;
unsigned long lastMotionCheck = 0;
unsigned long liftedStartTime = 0;
bool isLifted = false;

// Expression timing
unsigned long expressionStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long idleStartTime = 0;
int blinkInterval = 3000;

// Volume control (0-150, default 100 = normal, >100 = amplified/boost)
uint8_t volumeLevel = 120;  // Sedikit di-boost supaya kencang

// SPIFFS
bool spiffsReady = false;

// Expression sound cooldown (supaya tidak spam)
unsigned long lastExprSoundTime = 0;
DolaFace lastPlayedFace = FACE_IDLE;
bool exprSoundPlayed = false;  // Flag: sudah play suara untuk ekspresi saat ini

// Forward declarations for menu/games/clock
void setVolume(uint8_t level);
void playWelcomeSound(const char* filename);
void startAPMode();
String speechToText();
String chatWithAI(String userMessage);
bool textToSpeech(String text);
bool recordAudio();
void setupSpeaker();

// Include menu, games, tamagotchi, clock, OTA (after forward declarations)
#include "dola_menu.h"
#include "dola_games.h"
#include "dola_tamagotchi.h"
#include "dola_clock.h"
#include "dola_ota.h"

// Conversation context
String conversationHistory = "";
String dolaPersonality = "Kamu adalah Dola, robot asisten kecil yang lucu dan ekspresif. "
                          "Kamu suka bercanda, kadang manja, dan sangat setia pada pemilikmu. "
                          "Jawab dengan singkat, lucu, dan penuh ekspresi. "
                          "Gunakan bahasa Indonesia yang casual dan friendly. "
                          "Kalau ditanya nama, bilang 'Aku Dola! Robot paling imut sedunia~'. "
                          "Kamu suka bilang 'hehe', 'hihi', 'uwu' sesekali.";

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== DOLA Robot Starting ===");
  
  // Init I2C
  Wire.begin(21, 22);
  
  // Init Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED gagal!");
    while (1);
  }
  display.clearDisplay();
  display.display();
  
  // Init ADXL345
  if (!accel.begin(ADXL345_ADDR)) {
    Serial.println("ADXL345 tidak ditemukan!");
  } else {
    accel.setRange(ADXL345_RANGE_4_G);
    Serial.println("ADXL345 OK!");
  }
  
  // Init SPIFFS
  if (SPIFFS.begin(true)) {
    spiffsReady = true;
    Serial.println("SPIFFS OK!");
    Serial.printf("SPIFFS Total: %d bytes, Used: %d bytes\n", 
                  SPIFFS.totalBytes(), SPIFFS.usedBytes());
    // Load conversation memory dari SPIFFS
    loadMemory();
  } else {
    Serial.println("SPIFFS gagal! Welcome sound tidak akan berfungsi.");
  }
  
  // Init Touch Sensor
  setupTouch();
  
  // Init I2S Microphone
  setupMicrophone();
  
  // Init I2S Speaker
  setupSpeaker();
  
  // Load saved WiFi credentials
  preferences.begin("dola", false);
  savedSSID = preferences.getString("ssid", "");
  savedPassword = preferences.getString("pass", "");
  
  if (savedSSID.length() > 0) {
    // Try connecting to saved WiFi
    currentState = STATE_CONNECTING;
    showFace(FACE_CONNECTING);
    connectToWiFi();
  } else {
    // Start AP mode for WiFi provisioning
    startAPMode();
  }
  
  // Allocate audio buffer
  audioBuffer = (int16_t*)ps_malloc(SAMPLE_RATE * RECORD_DURATION * sizeof(int16_t));
  if (!audioBuffer) {
    audioBuffer = (int16_t*)malloc(SAMPLE_RATE * RECORD_DURATION * sizeof(int16_t));
  }
  
  Serial.println("=== DOLA Ready! ===");
}

// ============ MAIN LOOP ============
void loop() {
  // Handle web server in AP mode
  if (currentState == STATE_AP_MODE) {
    server.handleClient();
    animateAPMode();
    // Touch: masih bisa detect di AP mode
    readTouch();
    return;
  }
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED && currentState != STATE_AP_MODE && currentState != STATE_MENU) {
    wifiConnected = false;
    currentState = STATE_CONNECTING;
    showFace(FACE_SAD);
    playExpressionSound(FACE_SAD);
    connectToWiFi();
    return;
  }
  
  // Handle OTA web server (supaya dashboard bisa diakses kapan saja)
  server.handleClient();
  
  // Read touch sensor every cycle
  readTouch();
  
  // Handle touch gestures
  if (hasGesture()) {
    TouchGesture gesture = getGesture();
    
    // Jika sedang di menu/game/clock
    if (currentState == STATE_MENU) {
      handleMenuState(gesture);
      return;
    }
    
    // Jika di HOME (idle/sleepy/etc), double tap = masuk menu
    if (gesture == GESTURE_DOUBLE_TAP && 
        (currentState == STATE_IDLE || currentState == STATE_SLEEPY || 
         currentState == STATE_HAPPY || currentState == STATE_DIZZY ||
         currentState == STATE_SCARED || currentState == STATE_ANGRY)) {
      currentState = STATE_MENU;
      enterMenu();
      return;
    }
  }
  
  // Motion detection (every 50ms) - HANYA kalau tidak di menu
  if (currentState != STATE_MENU && millis() - lastMotionCheck > 50) {
    checkMotion();
    lastMotionCheck = millis();
  }
  
  // State machine
  switch (currentState) {
    case STATE_IDLE:
      handleIdle();
      break;
    case STATE_LISTENING:
      handleListening();
      break;
    case STATE_THINKING:
      handleThinking();
      break;
    case STATE_SPEAKING:
      handleSpeaking();
      break;
    case STATE_DIZZY:
      handleDizzy();
      break;
    case STATE_SCARED:
      handleScared();
      break;
    case STATE_ANGRY:
      handleAngry();
      break;
    case STATE_HAPPY:
      handleHappy();
      break;
    case STATE_SLEEPY:
      handleSleepy();
      break;
    case STATE_MENU:
      // Handled by touch gesture above + handleMenuState
      // Timer tetap jalan di background
      if (timerRunning) {
        showTimerRunning();
      }
      // Tamagotchi perlu continuous update (animasi, decay, shake detect)
      if (currentScreen == SCREEN_GAME_TAMAGOTCHI && tamaInitialized) {
        updateTamagotchi();
      }
      break;
    default:
      break;
  }
}

// ============ MENU STATE HANDLER ============
void handleMenuState(TouchGesture gesture) {
  // Handle Tamagotchi (special: uses touch for gameplay)
  if (currentScreen == SCREEN_GAME_TAMAGOTCHI) {
    bool shouldExit = handleTamagotchiGesture(gesture);
    if (shouldExit) {
      exitTamagotchi();
      exitGame();
      currentScreen = SCREEN_GAMES;
      menuCursor = 6; // Posisi Tamagotchi di menu
      menuScrollOffset = 2;
      renderMenuList("GAMES", gamesMenuItems, gamesMenuCount, menuCursor, menuScrollOffset);
    }
    return;
  }
  
  // Handle other game screens
  if (currentScreen >= SCREEN_GAME_TEBAK && currentScreen <= SCREEN_GAME_JOKES) {
    // Hold = exit game
    if (gesture == GESTURE_HOLD) {
      exitGame();
      currentScreen = SCREEN_GAMES;
      menuCursor = 0;
      menuScrollOffset = 0;
      renderMenuList("GAMES", gamesMenuItems, gamesMenuCount, menuCursor, menuScrollOffset);
      return;
    }
    // Games handle their own input via voice/sensor, not touch (except exit)
    return;
  }
  
  // Handle clock screen
  if (currentScreen == SCREEN_CLOCK) {
    if (handleClockGesture(gesture)) {
      currentScreen = SCREEN_MAIN;
      menuCursor = 1;
      menuScrollOffset = 0;
      renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
    }
    return;
  }
  
  // Handle timer set screen
  if (currentScreen == SCREEN_TIMER_SET) {
    int result = handleTimerSetGesture(gesture);
    if (result == 1) {
      // Timer started
      currentScreen = SCREEN_TIMER_RUN;
    }
    return;
  }
  
  // Handle timer running screen
  if (currentScreen == SCREEN_TIMER_RUN) {
    if (handleTimerRunGesture(gesture)) {
      // Timer cancelled
      currentScreen = SCREEN_MAIN;
      menuCursor = 2;
      menuScrollOffset = 0;
      renderMenuList("MENU DOLA", mainMenuItems, mainMenuCount, menuCursor, menuScrollOffset);
    }
    return;
  }
  
  // Handle menu navigation
  bool handled = handleMenuGesture(gesture);
  
  // Check if we need to launch a game
  if (currentScreen >= SCREEN_GAME_TEBAK && currentScreen <= SCREEN_GAME_JOKES) {
    startGame(currentScreen);
    return;
  }
  
  // Check if we need to show clock
  if (currentScreen == SCREEN_CLOCK) {
    showClock();
    return;
  }
  
  // Check if we need to show timer set
  if (currentScreen == SCREEN_TIMER_SET) {
    timerSetMinutes = 1;
    showTimerSet();
    return;
  }
  
  // Check if exited menu
  if (!inMenu) {
    currentState = STATE_IDLE;
    idleStartTime = millis();
    resetExpressionSound();
    showFace(FACE_IDLE);
  }
}

// ============ I2S MICROPHONE SETUP ============
void setupMicrophone() {
  i2s_config_t i2s_mic_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t mic_pin_config = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
  };
  
  i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
  i2s_set_pin(I2S_MIC_PORT, &mic_pin_config);
  i2s_zero_dma_buffer(I2S_MIC_PORT);
  Serial.println("Microphone I2S OK!");
}

// ============ I2S SPEAKER SETUP ============
void setupSpeaker() {
  i2s_config_t i2s_spk_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 24000,  // TTS output biasanya 24kHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t spk_pin_config = {
    .bck_io_num = I2S_SPK_BCLK,
    .ws_io_num = I2S_SPK_LRC,
    .data_out_num = I2S_SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  i2s_driver_install(I2S_SPK_PORT, &i2s_spk_config, 0, NULL);
  i2s_set_pin(I2S_SPK_PORT, &spk_pin_config);
  i2s_zero_dma_buffer(I2S_SPK_PORT);
  Serial.println("Speaker I2S OK!");
}

// ============ WIFI AP MODE & PROVISIONING ============
void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Dola-Setup", "dola1234");
  
  Serial.println("AP Mode started!");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSaveWiFi);
  server.on("/scan", handleScanWiFi);
  server.begin();
  
  currentState = STATE_AP_MODE;
  showFace(FACE_AP_MODE);
  playExpressionSound(FACE_AP_MODE);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Dola Setup</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh; display: flex; align-items: center; justify-content: center;
      padding: 20px;
    }
    .container {
      background: white; border-radius: 20px; padding: 40px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3); max-width: 400px; width: 100%;
    }
    .logo { text-align: center; margin-bottom: 30px; }
    .logo h1 { font-size: 2.5em; color: #667eea; }
    .logo p { color: #888; font-size: 0.9em; }
    .dola-face { font-size: 4em; text-align: center; margin: 10px 0; }
    input, select {
      width: 100%; padding: 15px; margin: 10px 0; border: 2px solid #eee;
      border-radius: 10px; font-size: 1em; transition: border-color 0.3s;
    }
    input:focus, select:focus { border-color: #667eea; outline: none; }
    button {
      width: 100%; padding: 15px; background: linear-gradient(135deg, #667eea, #764ba2);
      color: white; border: none; border-radius: 10px; font-size: 1.1em;
      cursor: pointer; margin-top: 20px; transition: transform 0.2s;
    }
    button:hover { transform: scale(1.02); }
    button:active { transform: scale(0.98); }
    .scan-btn { 
      background: linear-gradient(135deg, #f093fb, #f5576c); 
      margin-top: 10px; font-size: 0.9em; padding: 10px;
    }
    #networks { display: none; margin: 10px 0; }
    .status { text-align: center; margin-top: 15px; color: #888; font-size: 0.9em; }
  </style>
</head>
<body>
  <div class="container">
    <div class="logo">
      <div class="dola-face">( o_o )</div>
      <h1>DOLA</h1>
      <p>Robot Asisten AI Kamu</p>
    </div>
    <form action="/save" method="POST">
      <button type="button" class="scan-btn" onclick="scanWifi()">Scan WiFi Networks</button>
      <select id="networks" name="ssid_select" onchange="document.getElementById('ssid').value=this.value">
      </select>
      <input type="text" id="ssid" name="ssid" placeholder="Nama WiFi (SSID)" required>
      <input type="password" name="password" placeholder="Password WiFi">
      <button type="submit">Hubungkan Dola ke WiFi!</button>
    </form>
    <div class="status" id="status"></div>
  </div>
  <script>
    function scanWifi() {
      document.getElementById('status').innerText = 'Scanning...';
      fetch('/scan').then(r=>r.json()).then(data => {
        let sel = document.getElementById('networks');
        sel.innerHTML = '<option value="">-- Pilih WiFi --</option>';
        data.forEach(n => {
          sel.innerHTML += '<option value="'+n.ssid+'">'+n.ssid+' ('+n.rssi+'dBm)</option>';
        });
        sel.style.display = 'block';
        document.getElementById('status').innerText = 'Ditemukan ' + data.length + ' jaringan';
      });
    }
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleSaveWiFi() {
  savedSSID = server.arg("ssid");
  savedPassword = server.arg("password");
  
  // Save to NVS
  preferences.putString("ssid", savedSSID);
  preferences.putString("pass", savedPassword);
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Dola - Connecting</title>
  <style>
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh; display: flex; align-items: center; justify-content: center;
      color: white; text-align: center;
    }
    .container { padding: 40px; }
    h1 { font-size: 2em; margin-bottom: 20px; }
    .face { font-size: 4em; animation: bounce 1s infinite; }
    @keyframes bounce { 0%,100%{transform:translateY(0)} 50%{transform:translateY(-10px)} }
  </style>
</head>
<body>
  <div class="container">
    <div class="face">( ^_^ )</div>
    <h1>Dola sedang menghubungkan...</h1>
    <p>WiFi: )rawliteral" + savedSSID + R"rawliteral(</p>
    <p>Kamu bisa disconnect dari Dola-Setup sekarang!</p>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
  
  delay(2000);
  server.stop();
  WiFi.softAPdisconnect(true);
  
  currentState = STATE_CONNECTING;
  connectToWiFi();
}

void handleScanWiFi() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void connectToWiFi() {
  Serial.println("Connecting to: " + savedSSID);
  showFace(FACE_CONNECTING);
  playExpressionSound(FACE_CONNECTING);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    // Animate connecting face
    if (attempts % 4 == 0) showFace(FACE_CONNECTING);
    if (attempts % 4 == 2) showFace(FACE_BLINK);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    currentState = STATE_HAPPY;
    expressionStartTime = millis();
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
    showFace(FACE_HAPPY);
    
    // Setup NTP clock
    setupNTP();
    
    // Setup OTA Web Dashboard
    setupOTAServer();
    
    // Play welcome sound!
    playWelcomeSound("/welcome.wav");
    
    delay(1000);
    currentState = STATE_IDLE;
    idleStartTime = millis();
  } else {
    Serial.println("\nWiFi gagal! Kembali ke AP mode...");
    // Reset credentials
    preferences.putString("ssid", "");
    preferences.putString("pass", "");
    startAPMode();
  }
}

// ============ AUDIO RECORDING ============
bool recordAudio() {
  Serial.println("Recording...");
  audioBufferSize = 0;
  size_t maxSamples = SAMPLE_RATE * RECORD_DURATION;
  
  int16_t tempBuffer[BUFFER_SIZE];
  size_t bytesRead;
  unsigned long silenceStart = 0;
  bool speechDetected = false;
  
  unsigned long recordStart = millis();
  
  while (audioBufferSize < maxSamples) {
    i2s_read(I2S_MIC_PORT, tempBuffer, sizeof(tempBuffer), &bytesRead, portMAX_DELAY);
    int samplesRead = bytesRead / sizeof(int16_t);
    
    // Check audio level
    int32_t maxLevel = 0;
    for (int i = 0; i < samplesRead; i++) {
      int32_t absVal = abs(tempBuffer[i]);
      if (absVal > maxLevel) maxLevel = absVal;
    }
    
    // Detect speech
    if (maxLevel > SILENCE_THRESHOLD) {
      speechDetected = true;
      silenceStart = 0;
    } else if (speechDetected) {
      if (silenceStart == 0) silenceStart = millis();
      if (millis() - silenceStart > SILENCE_DURATION) {
        Serial.println("Silence detected, stopping recording");
        break;
      }
    }
    
    // Copy to main buffer
    for (int i = 0; i < samplesRead && audioBufferSize < maxSamples; i++) {
      audioBuffer[audioBufferSize++] = tempBuffer[i];
    }
    
    // Timeout
    if (millis() - recordStart > (RECORD_DURATION * 1000)) break;
  }
  
  Serial.printf("Recorded %d samples\n", audioBufferSize);
  return speechDetected && audioBufferSize > SAMPLE_RATE; // At least 1 second
}

// ============ AI COMMUNICATION ============
String speechToText() {
  if (audioBufferSize == 0) return "";
  
  HTTPClient http;
  http.begin(STT_ENDPOINT);
  http.addHeader("Authorization", String("Bearer ") + AI_API_KEY);
  http.setTimeout(30000);
  
  // Create WAV header + data
  size_t dataSize = audioBufferSize * sizeof(int16_t);
  size_t wavSize = 44 + dataSize;
  uint8_t* wavBuffer = (uint8_t*)malloc(wavSize);
  if (!wavBuffer) {
    Serial.println("WAV buffer allocation failed!");
    return "";
  }
  
  // WAV header
  memcpy(wavBuffer, "RIFF", 4);
  uint32_t fileSize = wavSize - 8;
  memcpy(wavBuffer + 4, &fileSize, 4);
  memcpy(wavBuffer + 8, "WAVE", 4);
  memcpy(wavBuffer + 12, "fmt ", 4);
  uint32_t fmtSize = 16;
  memcpy(wavBuffer + 16, &fmtSize, 4);
  uint16_t audioFormat = 1; // PCM
  memcpy(wavBuffer + 20, &audioFormat, 2);
  uint16_t numChannels = 1;
  memcpy(wavBuffer + 22, &numChannels, 2);
  uint32_t sampleRate = SAMPLE_RATE;
  memcpy(wavBuffer + 24, &sampleRate, 4);
  uint32_t byteRate = SAMPLE_RATE * 2;
  memcpy(wavBuffer + 28, &byteRate, 4);
  uint16_t blockAlign = 2;
  memcpy(wavBuffer + 32, &blockAlign, 2);
  uint16_t bitsPerSample = 16;
  memcpy(wavBuffer + 34, &bitsPerSample, 2);
  memcpy(wavBuffer + 36, "data", 4);
  memcpy(wavBuffer + 40, &dataSize, 4);
  memcpy(wavBuffer + 44, audioBuffer, dataSize);
  
  // Multipart form data
  String boundary = "----DolaFormBoundary";
  String bodyStart = "--" + boundary + "\r\n"
                     "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n"
                     "Content-Type: audio/wav\r\n\r\n";
  String bodyEnd = "\r\n--" + boundary + "\r\n"
                   "Content-Disposition: form-data; name=\"model\"\r\n\r\n"
                   "whisper-1\r\n"
                   "--" + boundary + "\r\n"
                   "Content-Disposition: form-data; name=\"language\"\r\n\r\n"
                   "id\r\n"
                   "--" + boundary + "--\r\n";
  
  size_t totalSize = bodyStart.length() + wavSize + bodyEnd.length();
  uint8_t* postData = (uint8_t*)malloc(totalSize);
  if (!postData) {
    free(wavBuffer);
    return "";
  }
  
  memcpy(postData, bodyStart.c_str(), bodyStart.length());
  memcpy(postData + bodyStart.length(), wavBuffer, wavSize);
  memcpy(postData + bodyStart.length() + wavSize, bodyEnd.c_str(), bodyEnd.length());
  
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  
  int httpCode = http.POST(postData, totalSize);
  
  free(wavBuffer);
  free(postData);
  
  String result = "";
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, response);
    result = doc["text"].as<String>();
    Serial.println("STT Result: " + result);
  } else {
    Serial.printf("STT Error: %d\n", httpCode);
    Serial.println(http.getString());
  }
  
  http.end();
  return result;
}

String chatWithAI(String userMessage) {
  HTTPClient http;
  http.begin(CHAT_ENDPOINT);
  http.addHeader("Authorization", String("Bearer ") + AI_API_KEY);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);
  
  // Build payload dengan conversation memory (history + pesan baru)
  String jsonStr = buildChatPayload(dolaPersonality, userMessage);
  
  int httpCode = http.POST(jsonStr);
  String result = "";
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument resDoc(8192);
    deserializeJson(resDoc, response);
    result = resDoc["choices"][0]["message"]["content"].as<String>();
    Serial.println("AI Response: " + result);
    
    // Simpan percakapan ke memory (persistent!)
    addToMemory("user", userMessage);
    addToMemory("assistant", result);
  } else {
    Serial.printf("Chat Error: %d\n", httpCode);
    Serial.println(http.getString());
    result = "Aduh, Dola lagi error nih... coba lagi ya!";
  }
  
  http.end();
  return result;
}

bool textToSpeech(String text) {
  HTTPClient http;
  http.begin(TTS_ENDPOINT);
  http.addHeader("Authorization", String("Bearer ") + AI_API_KEY);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(30000);
  
  DynamicJsonDocument doc(4096);
  doc["model"] = "tts-1";
  doc["input"] = text;
  doc["voice"] = "nova";  // Suara yang cute
  doc["response_format"] = "pcm";
  doc["speed"] = 1.1;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  
  int httpCode = http.POST(jsonStr);
  
  if (httpCode == 200) {
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buf[1024];
    size_t bytesWritten;
    
    while (http.connected() && stream->available()) {
      int len = stream->readBytes(buf, sizeof(buf));
      if (len > 0) {
        // Apply volume control
        applyVolume((int16_t*)buf, len / 2);
        i2s_write(I2S_SPK_PORT, buf, len, &bytesWritten, portMAX_DELAY);
      }
    }
    
    // Flush remaining audio
    i2s_zero_dma_buffer(I2S_SPK_PORT);
    http.end();
    return true;
  } else {
    Serial.printf("TTS Error: %d\n", httpCode);
    http.end();
    return false;
  }
}

// ============ VOLUME CONTROL ============
void applyVolume(int16_t* buffer, size_t numSamples) {
  for (size_t i = 0; i < numSamples; i++) {
    int32_t sample = buffer[i];
    // Apply gain: volumeLevel 100 = 1.0x, 150 = 1.5x, 50 = 0.5x
    sample = (sample * volumeLevel) / 100;
    // Clamp to prevent clipping
    if (sample > 32767) sample = 32767;
    if (sample < -32768) sample = -32768;
    buffer[i] = (int16_t)sample;
  }
}

void setVolume(uint8_t level) {
  // level: 0 = mute, 50 = quiet, 100 = normal, 120 = loud, 150 = max boost
  volumeLevel = level;
  Serial.printf("Volume set to: %d%%\n", level);
}

// ============ WELCOME SOUND (SPIFFS WAV PLAYBACK) ============
void playWelcomeSound(const char* filename) {
  if (!spiffsReady) {
    Serial.println("SPIFFS not ready, skipping welcome sound");
    return;
  }
  
  if (!SPIFFS.exists(filename)) {
    Serial.printf("File %s tidak ditemukan di SPIFFS!\n", filename);
    Serial.println("Pastikan sudah upload file via ESP32 Sketch Data Upload");
    return;
  }
  
  File audioFile = SPIFFS.open(filename, "r");
  if (!audioFile) {
    Serial.println("Gagal membuka file audio!");
    return;
  }
  
  Serial.printf("Playing: %s (%d bytes)\n", filename, audioFile.size());
  
  // Parse WAV header
  uint8_t wavHeader[44];
  audioFile.read(wavHeader, 44);
  
  // Verify RIFF header
  if (wavHeader[0] != 'R' || wavHeader[1] != 'I' || wavHeader[2] != 'F' || wavHeader[3] != 'F') {
    Serial.println("Bukan file WAV yang valid!");
    audioFile.close();
    return;
  }
  
  // Extract WAV info
  uint16_t audioFormat = wavHeader[20] | (wavHeader[21] << 8);
  uint16_t numChannels = wavHeader[22] | (wavHeader[23] << 8);
  uint32_t fileSampleRate = wavHeader[24] | (wavHeader[25] << 8) | (wavHeader[26] << 16) | (wavHeader[27] << 24);
  uint16_t bitsPerSample = wavHeader[34] | (wavHeader[35] << 8);
  uint32_t dataSize = wavHeader[40] | (wavHeader[41] << 8) | (wavHeader[42] << 16) | (wavHeader[43] << 24);
  
  Serial.printf("WAV Info: %dHz, %d-bit, %d channel(s), %d bytes data\n", 
                fileSampleRate, bitsPerSample, numChannels, dataSize);
  
  // Verify format
  if (audioFormat != 1) { // Must be PCM
    Serial.println("Format tidak didukung! Hanya PCM WAV.");
    audioFile.close();
    return;
  }
  
  // Reconfigure I2S speaker to match file sample rate
  i2s_driver_uninstall(I2S_SPK_PORT);
  
  i2s_config_t i2s_spk_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = fileSampleRate,
    .bits_per_sample = (i2s_bits_per_sample_t)bitsPerSample,
    .channel_format = (numChannels == 2) ? I2S_CHANNEL_FMT_RIGHT_LEFT : I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t spk_pin_config = {
    .bck_io_num = I2S_SPK_BCLK,
    .ws_io_num = I2S_SPK_LRC,
    .data_out_num = I2S_SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  i2s_driver_install(I2S_SPK_PORT, &i2s_spk_config, 0, NULL);
  i2s_set_pin(I2S_SPK_PORT, &spk_pin_config);
  i2s_zero_dma_buffer(I2S_SPK_PORT);
  
  // Stream audio from file to I2S (chunk by chunk)
  uint8_t buf[1024];
  size_t bytesWritten;
  size_t totalPlayed = 0;
  
  while (audioFile.available() && totalPlayed < dataSize) {
    int bytesToRead = min((size_t)sizeof(buf), dataSize - totalPlayed);
    int bytesRead = audioFile.read(buf, bytesToRead);
    
    if (bytesRead <= 0) break;
    
    // Apply volume control
    if (bitsPerSample == 16) {
      applyVolume((int16_t*)buf, bytesRead / 2);
    } else if (bitsPerSample == 8) {
      // Convert 8-bit to volume-adjusted (simple scale)
      for (int i = 0; i < bytesRead; i++) {
        int16_t sample = ((int16_t)buf[i] - 128) * 256; // 8-bit to 16-bit
        sample = (sample * volumeLevel) / 100;
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;
        buf[i] = (uint8_t)((sample / 256) + 128); // back to 8-bit
      }
    }
    
    i2s_write(I2S_SPK_PORT, buf, bytesRead, &bytesWritten, portMAX_DELAY);
    totalPlayed += bytesRead;
  }
  
  // Flush & close
  i2s_zero_dma_buffer(I2S_SPK_PORT);
  audioFile.close();
  
  // Restore I2S to default TTS sample rate (24kHz)
  i2s_driver_uninstall(I2S_SPK_PORT);
  setupSpeaker();
  
  Serial.printf("Welcome sound selesai! (%d bytes played)\n", totalPlayed);
}

// ============ EXPRESSION SOUND SYSTEM ============
const char* getExpressionSoundFile(DolaFace face) {
  switch (face) {
    case FACE_IDLE:          return "/idle.wav";
    case FACE_HAPPY:         return "/happy.wav";
    case FACE_SAD:           return "/sad.wav";
    case FACE_ANGRY:         return "/angry.wav";
    case FACE_SCARED:        return "/scared.wav";
    case FACE_DIZZY:         return "/dizzy.wav";
    case FACE_THINKING:      return "/thinking.wav";
    case FACE_LISTENING:     return "/listening.wav";
    case FACE_SLEEPY:        return "/sleepy.wav";
    case FACE_SURPRISED:     return "/surprised.wav";
    case FACE_CONFUSED:      return "/confused.wav";
    case FACE_UNCOMFORTABLE: return "/uncomfortable.wav";
    case FACE_LOVE:          return "/love.wav";
    case FACE_CONNECTING:    return "/connecting.wav";
    case FACE_AP_MODE:       return "/ap_mode.wav";
    default:                 return nullptr;
  }
}

// Play suara ekspresi - dengan cooldown supaya tidak spam
// Hanya play SEKALI per perubahan ekspresi
void playExpressionSound(DolaFace face) {
  // Skip kalau face sama dengan yang terakhir diplay (anti-spam)
  if (face == lastPlayedFace && exprSoundPlayed) {
    return;
  }
  
  // Cooldown minimum 500ms antar suara
  if (millis() - lastExprSoundTime < 500) {
    return;
  }
  
  const char* filename = getExpressionSoundFile(face);
  if (filename == nullptr) return;
  
  // Cek file ada di SPIFFS
  if (!spiffsReady || !SPIFFS.exists(filename)) {
    Serial.printf("Sound file %s tidak ada, skip.\n", filename);
    return;
  }
  
  Serial.printf("Expression sound: %s\n", filename);
  
  // Play file (reuse fungsi playWelcomeSound yang sudah handle WAV parsing)
  playWelcomeSound(filename);
  
  // Update cooldown tracking
  lastExprSoundTime = millis();
  lastPlayedFace = face;
  exprSoundPlayed = true;
}

// Reset flag saat ekspresi berubah (supaya ekspresi baru bisa play suara)
void resetExpressionSound() {
  exprSoundPlayed = false;
}

// ============ MOTION DETECTION ============
void checkMotion() {
  sensors_event_t event;
  accel.getEvent(&event);
  
  float deltaX = abs(event.acceleration.x - lastAccelX);
  float deltaY = abs(event.acceleration.y - lastAccelY);
  float deltaZ = abs(event.acceleration.z - lastAccelZ);
  float totalDelta = deltaX + deltaY + deltaZ;
  
  lastAccelX = event.acceleration.x;
  lastAccelY = event.acceleration.y;
  lastAccelZ = event.acceleration.z;
  
  // Detect shake (guncangan)
  if (totalDelta > 15.0) {
    shakeCount++;
    if (shakeCount > 5 && currentState != STATE_DIZZY) {
      previousState = currentState;
      currentState = STATE_DIZZY;
      expressionStartTime = millis();
      resetExpressionSound();
      Serial.println("DOLA: Aduhh pusing!!");
    }
  } else {
    if (shakeCount > 0) shakeCount--;
  }
  
  // Detect lifted (diangkat) - Z axis significantly different from gravity
  float totalAccel = sqrt(event.acceleration.x * event.acceleration.x + 
                          event.acceleration.y * event.acceleration.y + 
                          event.acceleration.z * event.acceleration.z);
  
  // Jika percepatan total jauh dari 9.8 (free fall atau diangkat cepat)
  if (totalAccel < 5.0) { // Mendekati free fall = diangkat
    if (!isLifted) {
      isLifted = true;
      liftedStartTime = millis();
    }
    if (millis() - liftedStartTime > 500 && currentState != STATE_SCARED && currentState != STATE_ANGRY) {
      previousState = currentState;
      resetExpressionSound();
      // Random antara scared atau angry
      if (random(2) == 0) {
        currentState = STATE_SCARED;
        Serial.println("DOLA: HUAAA TAKUT KETINGGIAN!!");
      } else {
        currentState = STATE_ANGRY;
        Serial.println("DOLA: HEH! TURUNIN AKU!!");
      }
      expressionStartTime = millis();
    }
  } else if (totalAccel > 8.0 && totalAccel < 12.0) {
    // Normal gravity - robot di posisi normal
    if (isLifted && currentState == STATE_SCARED) {
      currentState = STATE_HAPPY;
      expressionStartTime = millis();
      Serial.println("DOLA: Fyuh... aman~");
    }
    isLifted = false;
  }
  
  // Detect tilt (dimiringkan)
  if (abs(event.acceleration.x) > 7.0 || abs(event.acceleration.y) > 7.0) {
    if (currentState == STATE_IDLE) {
      // Dola merasa tidak nyaman kalau dimiringkan
      showFace(FACE_UNCOMFORTABLE);
      playExpressionSound(FACE_UNCOMFORTABLE);
    }
  }
}

// ============ STATE HANDLERS ============
void handleIdle() {
  // Check for voice input (simple threshold detection)
  int16_t tempBuf[BUFFER_SIZE];
  size_t bytesRead;
  i2s_read(I2S_MIC_PORT, tempBuf, sizeof(tempBuf), &bytesRead, 100);
  
  int samplesRead = bytesRead / sizeof(int16_t);
  int32_t maxLevel = 0;
  for (int i = 0; i < samplesRead; i++) {
    if (abs(tempBuf[i]) > maxLevel) maxLevel = abs(tempBuf[i]);
  }
  
  // If loud enough, start recording
  if (maxLevel > SILENCE_THRESHOLD * 2) {
    currentState = STATE_LISTENING;
    resetExpressionSound();
    showFace(FACE_LISTENING);
    playExpressionSound(FACE_LISTENING);
    Serial.println("Voice detected! Listening...");
    return;
  }
  
  // Idle animations
  unsigned long now = millis();
  
  // Blink randomly
  if (now - lastBlinkTime > blinkInterval) {
    showFace(FACE_BLINK);
    delay(150);
    showFace(FACE_IDLE);
    lastBlinkTime = now;
    blinkInterval = random(2000, 5000);
  }
  
  // Random idle sound (sesekali, 1 dari 200 loop cycle ~ setiap ~20 detik)
  static unsigned long lastIdleSound = 0;
  if (now - lastIdleSound > 20000 && random(100) == 0) {
    playExpressionSound(FACE_IDLE);
    lastIdleSound = now;
  }
  
  // Go sleepy after 30 seconds idle
  if (now - idleStartTime > 30000) {
    currentState = STATE_SLEEPY;
    resetExpressionSound();
    expressionStartTime = now;
  }
  
  showFace(FACE_IDLE);
}

void handleListening() {
  showFace(FACE_LISTENING);
  
  bool hasAudio = recordAudio();
  
  if (hasAudio) {
    currentState = STATE_THINKING;
    resetExpressionSound();
    showFace(FACE_THINKING);
    playExpressionSound(FACE_THINKING);
    
    // STT
    String text = speechToText();
    
    if (text.length() > 0) {
      // Chat with AI
      String response = chatWithAI(text);
      
      if (response.length() > 0) {
        // TTS
        currentState = STATE_SPEAKING;
        resetExpressionSound();
        showFace(FACE_SPEAKING);
        textToSpeech(response);
        
        // Happy after responding
        currentState = STATE_HAPPY;
        resetExpressionSound();
        expressionStartTime = millis();
        showFace(FACE_HAPPY);
        playExpressionSound(FACE_HAPPY);
      }
    } else {
      // Didn't understand
      resetExpressionSound();
      showFace(FACE_CONFUSED);
      playExpressionSound(FACE_CONFUSED);
      delay(1500);
    }
  }
  
  currentState = STATE_IDLE;
  resetExpressionSound();
  idleStartTime = millis();
  showFace(FACE_IDLE);
}

void handleThinking() {
  // Animated thinking face
  static int thinkFrame = 0;
  if (millis() % 500 < 250) {
    showFace(FACE_THINKING);
  } else {
    showFace(FACE_THINKING_2);
  }
}

void handleSpeaking() {
  // Animated speaking face
  if (millis() % 300 < 150) {
    showFace(FACE_SPEAKING);
  } else {
    showFace(FACE_SPEAKING_2);
  }
}

void handleDizzy() {
  // Play suara pusing sekali di awal
  playExpressionSound(FACE_DIZZY);
  
  // Spiral eyes animation
  showFace(FACE_DIZZY);
  
  if (millis() - expressionStartTime > 3000) {
    currentState = STATE_IDLE;
    resetExpressionSound();
    idleStartTime = millis();
    showFace(FACE_IDLE);
  }
}

void handleScared() {
  // Play suara takut sekali di awal
  playExpressionSound(FACE_SCARED);
  
  showFace(FACE_SCARED);
  
  if (millis() - expressionStartTime > 4000) {
    currentState = STATE_IDLE;
    resetExpressionSound();
    idleStartTime = millis();
  }
}

void handleAngry() {
  // Play suara marah sekali di awal
  playExpressionSound(FACE_ANGRY);
  
  showFace(FACE_ANGRY);
  
  if (millis() - expressionStartTime > 3000) {
    currentState = STATE_IDLE;
    resetExpressionSound();
    idleStartTime = millis();
  }
}

void handleHappy() {
  showFace(FACE_HAPPY);
  
  if (millis() - expressionStartTime > 2000) {
    currentState = STATE_IDLE;
    resetExpressionSound();
    idleStartTime = millis();
  }
}

void handleSleepy() {
  static unsigned long lastZzz = 0;
  static bool sleepSoundPlayed = false;
  
  // Play suara ngantuk sekali saat masuk state sleepy
  if (!sleepSoundPlayed) {
    playExpressionSound(FACE_SLEEPY);
    sleepSoundPlayed = true;
  }
  
  if (millis() - lastZzz > 2000) {
    showFace(FACE_SLEEPY);
    lastZzz = millis();
  }
  
  // Wake up on sound
  int16_t tempBuf[BUFFER_SIZE];
  size_t bytesRead;
  i2s_read(I2S_MIC_PORT, tempBuf, sizeof(tempBuf), &bytesRead, 100);
  
  int samplesRead = bytesRead / sizeof(int16_t);
  int32_t maxLevel = 0;
  for (int i = 0; i < samplesRead; i++) {
    if (abs(tempBuf[i]) > maxLevel) maxLevel = abs(tempBuf[i]);
  }
  
  if (maxLevel > SILENCE_THRESHOLD * 3) {
    // Wake up!
    sleepSoundPlayed = false;
    resetExpressionSound();
    showFace(FACE_SURPRISED);
    playExpressionSound(FACE_SURPRISED);
    delay(500);
    currentState = STATE_LISTENING;
    Serial.println("Dola woke up!");
  }
}

void animateAPMode() {
  static unsigned long lastFrame = 0;
  static int frame = 0;
  
  if (millis() - lastFrame > 1000) {
    if (frame % 2 == 0) {
      showFace(FACE_AP_MODE);
    } else {
      showFace(FACE_BLINK);
    }
    frame++;
    lastFrame = millis();
  }
}
