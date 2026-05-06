# DOLA - Expressive AI Robot Assistant

DOLA is an expressive AI robot based on ESP32. Features: voice assistant (Whisper+GPT+TTS), 19 OLED facial expressions, motion & touch detection, 7 interactive games (Fish Tamagotchi, Quiz, Simon Says, etc.), NTP clock, timer, conversation memory, OTA firmware update, and auto-save to NVS/SPIFFS.

---

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Software & Libraries](#software--libraries)
- [Installation & Setup](#installation--setup)
- [API Configuration](#api-configuration)
- [Upload SPIFFS (Sound Files)](#upload-spiffs-sound-files)
- [How to Use](#how-to-use)
- [Menu System & Navigation](#menu-system--navigation)
- [Game: Fish Tamagotchi](#game-fish-tamagotchi)
- [OTA Update](#ota-update)
- [File Structure](#file-structure)
- [Troubleshooting](#troubleshooting)
- [Power Supply](#power-supply)

---

## Features

- **AI Voice Assistant** — Talk using voice with full pipeline: STT (Whisper) → AI Chat (GPT-4o-mini) → TTS (Nova)
- **19 Facial Expressions** — Pixel art on OLED: idle, happy, sad, angry, scared, dizzy, thinking, listening, speaking, sleepy, surprised, confused, uncomfortable, love, connecting, AP mode, blink, thinking_2, speaking_2
- **Motion Detection** — Shake = dizzy, lift = scared/angry, tilt = uncomfortable
- **Touch Sensor** — Tap, double tap, hold for navigation and interaction
- **7 Interactive Games:**
  1. Guess the Number (Voice)
  2. Simon Says (Motion Sensor)
  3. Shake Challenge (Motion Sensor)
  4. Quiz Trivia (Voice + AI)
  5. Rock Paper Scissors (Voice)
  6. Random Jokes (AI + TTS)
  7. Fish Tamagotchi (Touch + Sensor) — Virtual pet with evolution, save system, and gravestone
- **Digital Clock** — NTP synced (WIB/UTC+7) with Indonesian day & month names
- **Countdown Timer** — 1-60 minutes with audio alarm
- **Conversation Memory** — Stores last 20 messages to SPIFFS, persists across restarts
- **WiFi Provisioning** — WiFi setup via AP mode + beautiful web portal
- **OTA Firmware Update** — Update firmware & SPIFFS via browser web dashboard
- **Volume Control** — Software volume 0-150% (with boost capability)
- **Expression Sounds** — Each expression can trigger a WAV file from SPIFFS
- **Auto-Save** — All data stored in NVS/SPIFFS, safe during power off

---

## Hardware Requirements

| Component | Specification | Qty | Notes |
|-----------|---------------|-----|-------|
| ESP32 DevKit V1 | 38 pin, 4MB Flash | 1 | Main controller |
| INMP441 | I2S MEMS Microphone | 1 | Voice input |
| MAX98357A | I2S DAC + Amplifier | 1 | Audio output |
| Speaker | 3W 4Ohm / 8Ohm | 1 | For audio playback |
| OLED SSD1306 | 0.96" 128x64 I2C | 1 | Face display & UI |
| ADXL345 | 3-Axis Accelerometer I2C | 1 | Motion detection |
| TTP223 | Capacitive Touch Sensor | 1 | Touch input |
| Breadboard / PCB | - | 1 | Circuit assembly |
| Jumper Wires | Male-Male, Male-Female | ~20 | Component connections |
| Power Supply | 5V 1A minimum | 1 | USB / Battery |

### Optional:
- 3D printed or acrylic case
- 18650 battery + TP4056 + 5V Boost converter (for portable use)
- Power on/off switch

---

## Wiring Diagram

### INMP441 Microphone → ESP32 (I2S Port 0)

| INMP441 Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VDD | 3.3V | Power |
| GND | GND | Ground |
| SCK | GPIO 26 | Serial Clock |
| WS | GPIO 25 | Word Select (L/R) |
| SD | GPIO 33 | Serial Data |
| L/R | GND | Left channel (connect to GND) |

### MAX98357A Speaker → ESP32 (I2S Port 1)

| MAX98357 Pin | ESP32 Pin | Description |
|--------------|-----------|-------------|
| VIN | 5V | Power |
| GND | GND | Ground |
| BCLK | GPIO 27 | Bit Clock |
| LRC | GPIO 14 | Left/Right Clock |
| DIN | GPIO 12 | Data In |
| GAIN | - | Leave floating (15dB) or connect to GND (9dB) |

### OLED SSD1306 → ESP32 (I2C)

| OLED Pin | ESP32 Pin | Description |
|----------|-----------|-------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data |
| SCL | GPIO 22 | I2C Clock |

I2C Address: `0x3C`

### ADXL345 Accelerometer → ESP32 (I2C)

| ADXL345 Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data (shared with OLED) |
| SCL | GPIO 22 | I2C Clock (shared with OLED) |
| CS | 3.3V | High = I2C mode |
| SDO | GND | Address = 0x53 |

I2C Address: `0x53`

### TTP223 Touch Sensor → ESP32

| TTP223 Pin | ESP32 Pin | Description |
|------------|-----------|-------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SIG/OUT | GPIO 4 (Touch0) | Signal output |

---

## Software & Libraries

### Arduino IDE Setup:

1. **Install Arduino IDE** (version 1.8.x or 2.x)
2. **Add ESP32 Board Manager:**
   - Open File → Preferences
   - In "Additional Board Manager URLs", add:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Open Tools → Board → Board Manager → search "esp32" → Install

3. **Select Board:**
   - Tools → Board → ESP32 Arduino → "ESP32 Dev Module"

4. **Board Settings:**
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB (32Mb)
   - Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
   - PSRAM: Disabled (unless your board has PSRAM)

### Required Libraries:

Install via Arduino IDE → Sketch → Include Library → Manage Libraries:

| Library | Version | Description |
|---------|---------|-------------|
| Adafruit GFX Library | ≥1.11.0 | Graphics primitives |
| Adafruit SSD1306 | ≥2.5.0 | OLED driver |
| Adafruit ADXL345 | ≥1.3.0 | Accelerometer driver |
| Adafruit Unified Sensor | ≥1.1.0 | Dependency for ADXL345 |
| ArduinoJson | ≥6.21.0 | JSON parsing |
| WiFi | (built-in ESP32) | WiFi connectivity |
| WebServer | (built-in ESP32) | HTTP server |
| HTTPClient | (built-in ESP32) | HTTP client |
| SPIFFS | (built-in ESP32) | File system |
| Preferences | (built-in ESP32) | NVS storage |
| Update | (built-in ESP32) | OTA update |

---

## Installation & Setup

### Step 1: Clone Repository

```bash
git clone https://github.com/Ramadanil11/delly_robot_assistant.git
cd delly_robot_assistant
```

### Step 2: Open in Arduino IDE

Open the file `delly_robot.ino` in Arduino IDE.

### Step 3: Install Libraries

Install all libraries listed in the [Software & Libraries](#software--libraries) section.

### Step 4: Configure API Key

Edit `delly_robot.ino`, find this section and replace with your own API key:

```cpp
const char* AI_API_KEY = "YOUR_API_KEY_HERE";
const char* STT_ENDPOINT = "https://api.enowx.com/v1/audio/transcriptions";
const char* CHAT_ENDPOINT = "https://api.enowx.com/v1/chat/completions";
const char* TTS_ENDPOINT = "https://api.enowx.com/v1/audio/speech";
```

### Step 5: Upload Firmware

1. Connect ESP32 to computer via USB
2. Select the correct COM port in Tools → Port
3. Click Upload (→)
4. Wait until complete

### Step 6: WiFi Setup

1. After upload, DOLA enters **AP Mode**
2. On your phone/laptop, connect to WiFi: `Dola-Setup` (password: `dola1234`)
3. Open browser, navigate to `192.168.4.1`
4. Select your home WiFi and enter the password
5. DOLA will restart and connect to your WiFi

### Step 7: Done!

DOLA is ready to use. Try speaking towards the microphone to start chatting.

---

## API Configuration

DOLA uses OpenAI-compatible APIs for STT, Chat, and TTS.

### Endpoints Used:

| Function | Endpoint | Model |
|----------|----------|-------|
| Speech-to-Text | `/v1/audio/transcriptions` | whisper-1 |
| AI Chat | `/v1/chat/completions` | gpt-4o-mini |
| Text-to-Speech | `/v1/audio/speech` | tts-1 (voice: nova) |

### Getting an API Key:

1. Register at an OpenAI-compatible API provider
2. Generate an API key
3. Insert it into the `AI_API_KEY` variable in the code

### Changing DOLA's Personality:

Edit the `dolaPersonality` variable in `delly_robot.ino`:

```cpp
String dolaPersonality = "You are Dola, a cute and expressive little robot assistant...";
```

You can customize the personality, language, or speaking style as you wish.

---

## Upload SPIFFS (Sound Files)

DOLA uses WAV files stored in SPIFFS for expression sounds and welcome audio.

### WAV File Format:
- Format: PCM WAV
- Sample Rate: 16000Hz or 22050Hz
- Bit Depth: 16-bit
- Channels: Mono (1 channel)
- Total Size: Maximum ~1.5MB (SPIFFS limit)

### Sound File List (Optional):

Place these files in the `data/` folder:

```
data/
├── welcome.wav        → Sound when first powered on
├── idle.wav           → Occasional idle sound
├── happy.wav          → Happy expression sound
├── sad.wav            → Sad expression sound
├── angry.wav          → Angry expression sound
├── scared.wav         → Scared expression sound
├── dizzy.wav          → Dizzy expression sound
├── thinking.wav       → Thinking expression sound
├── listening.wav      → Listening start sound
├── sleepy.wav         → Sleepy expression sound
├── surprised.wav      → Surprised expression sound
├── confused.wav       → Confused expression sound
├── uncomfortable.wav  → Uncomfortable expression sound
├── love.wav           → Love expression sound
├── connecting.wav     → WiFi connecting sound
├── ap_mode.wav        → AP mode sound
├── game_start.wav     → Game start sound
├── game_win.wav       → Game win sound
└── game_lose.wav      → Game lose sound
```

### How to Upload SPIFFS:

**Using Arduino IDE 1.x:**
1. Install the "ESP32 Sketch Data Upload" plugin
2. Place WAV files in the `data/` folder (same level as `.ino`)
3. Tools → ESP32 Sketch Data Upload

**Using Arduino IDE 2.x:**
1. Install the SPIFFS upload plugin for IDE 2.x
2. Or use the OTA Web Dashboard (after DOLA is connected to WiFi)

**Using OTA Dashboard:**
1. Open browser, navigate to `http://[DOLA_IP]`
2. Upload SPIFFS files via the web interface

> **Note:** Sound files are optional. DOLA works fully without them — you just won't have expression sound effects.

---

## How to Use

### Talking to DOLA:
1. Make sure DOLA is in **IDLE** state (normal face displayed)
2. Speak loud enough towards the microphone
3. DOLA detects voice → face changes to "listening"
4. After you stop speaking (1.5s silence), DOLA processes → face shows "thinking"
5. DOLA responds via speaker → face shows "speaking"
6. Done → face shows "happy" → returns to "idle"

### Touch Sensor Gestures:

| Gesture | How To | Duration |
|---------|--------|----------|
| Tap | Touch & release quickly | < 300ms |
| Double Tap | Touch twice quickly | Gap < 400ms |
| Hold | Touch & keep holding | > 800ms |

### Motion Reactions:

| Motion | DOLA's Reaction |
|--------|-----------------|
| Shake hard | Dizzy (spiral eyes) → returns to normal after 3 seconds |
| Lift/drop | Scared or angry (random) → returns to normal after 4 seconds |
| Tilt | Uncomfortable (face changes) |

### Sleep Mode:
- After 30 seconds idle without interaction, DOLA enters sleep mode (sleepy face)
- Speak loudly to wake DOLA up

---

## Menu System & Navigation

### Enter Menu:
- **Double Tap** while at home/idle → enters main menu

### Menu Navigation:
- **Tap** = Move to next item
- **Double Tap** = Select/enter item
- **Hold** = Go back to previous menu

### Menu Structure:

```
Main Menu
├── Games
│   ├── Guess the Number
│   ├── Simon Says
│   ├── Shake Challenge
│   ├── Quiz Trivia
│   ├── Rock Paper Scissors
│   ├── Random Jokes
│   └── Tamagotchi
├── Clock
├── Timer
├── Settings
│   ├── Volume (tap: +10, double tap: -10, hold: save & back)
│   └── WiFi Reset
├── Status (WiFi info, IP, SPIFFS, uptime)
└── About Dola
```

---

## Game: Fish Tamagotchi

### Description:
A virtual fish pet that lives on DOLA's OLED screen. Take care of your fish to keep it alive and watch it evolve!

### Controls:

| State | Tap | Double Tap | Hold | Shake |
|-------|-----|------------|------|-------|
| Active | Feed | Play | Sleep | Pet/Stroke |
| Sleeping | Wake up | Wake up | Exit game | - |
| Dead | Restart (new egg) | View graveyard | Exit game | - |
| Graveyard | Go back | Go back | Go back | - |

### Stats:

| Stat | Description | Decay Rate |
|------|-------------|------------|
| Hunger (H) | Fullness level | -2 per 30 seconds |
| Happiness (P) | Happiness level | -1 per 30 seconds |
| Energy (E) | Energy level | -1 per 30 seconds |
| Health (♥) | Overall health | -1 if hunger<20 or happiness<15 |

### Evolution:

| Stage | Appearance | Requirement |
|-------|------------|-------------|
| 0 - Egg | Round wobbling egg | Starting state |
| 1 - Baby | Small fish | 5 minutes alive + health > 50 |
| 2 - Teen | Medium fish with fins | 1 hour alive + health > 40 |
| 3 - Adult | Large detailed fish | 3 hours alive + health > 50 |

### Death System:
- Fish dies when Health reaches 0
- Causes: too hungry (hunger < 20) or too sad (happiness < 15) for extended periods
- Warning "!" appears when health < 20

### After Death:
- **Tap** = Rebirth (start from new egg)
- **Double Tap** = View graveyard (last 5 dead fish)
- **Hold** = Exit to Games menu

### Additional Features:
- **Highscore** — Longest lifespan record is saved
- **Death Counter** — Total number of times fish has died
- **Gravestone** — List of last 5 dead fish (stage + age)
- **Auto-Save** — Stats automatically saved every 60 seconds to NVS
- **Offline Decay** — When robot is powered off, stats decrease upon next power on

### Tips:
- Feed regularly (tap) to keep hunger above 20
- Play often (double tap) to keep happiness high
- Let it sleep (hold) when energy is low
- Pet/stroke (shake) for bonus happiness
- Don't leave the robot powered off too long — your fish might starve!

---

## OTA Update

After DOLA is connected to WiFi, you can access the web dashboard to update firmware without a USB cable.

### Access Dashboard:
1. Find DOLA's IP in the Status menu, or check Serial Monitor
2. Open browser: `http://[DOLA_IP]`

### Dashboard Features:
- Upload new firmware (.bin)
- Upload SPIFFS files (.bin)
- View device info (IP, SSID, uptime, memory)
- Reset WiFi credentials
- Manage conversation memory

---

## File Structure

```
delly_robot_assistant/
│
├── delly_robot.ino          → Main sketch: setup(), loop(), state machine,
│                              WiFi, audio recording, AI communication,
│                              motion detection, state handlers
│
├── dola_faces.h             → 19 facial expressions as PROGMEM bitmaps
│                              + showFace() function to render on OLED
│
├── dola_touch.h             → Touch sensor driver (TTP223 / Touch0)
│                              Gesture detection: tap, double tap, hold
│
├── dola_memory.h            → Conversation memory system
│                              Save/load last 20 messages to SPIFFS (JSON)
│                              buildChatPayload() for AI context
│
├── dola_menu.h              → Menu system with touch navigation
│                              Render menu list on OLED
│                              Handle gestures per screen
│
├── dola_games.h             → 6 games: Guess Number, Simon Says, Shake,
│                              Quiz Trivia, Rock Paper Scissors, Random Jokes
│                              Game dispatcher (startGame/updateGame)
│
├── dola_tamagotchi.h        → Fish Tamagotchi game
│                              Fish bitmaps (4 stages × 2 animation frames)
│                              Stats, decay, evolution, NVS save/load
│                              Gravestone system, highscore
│
├── dola_clock.h             → NTP clock (WIB) + countdown timer
│                              Clock display on OLED
│                              Timer with alarm
│
├── dola_ota.h               → OTA web dashboard
│                              HTTP server for firmware updates
│                              SPIFFS upload, WiFi reset, memory management
│
├── data/                    → SPIFFS folder (WAV sound files)
│   └── (*.wav files)
│
└── README.md                → This documentation
```

---

## Troubleshooting

### DOLA can't connect to WiFi:
- Make sure SSID and password are correct
- Ensure WiFi is 2.4GHz (ESP32 does not support 5GHz)
- If connection fails, DOLA automatically returns to AP Mode → setup again

### OLED not turning on:
- Check I2C connections (SDA=GPIO21, SCL=GPIO22)
- Verify I2C address is correct (0x3C)
- Try scanning I2C with an I2C Scanner sketch

### Microphone not detecting sound:
- Check INMP441 connections (SCK=26, WS=25, SD=33)
- Make sure L/R pin is connected to GND (left channel)
- Speak louder or move closer to the microphone
- Check Serial Monitor for audio level debug info

### Speaker not producing sound:
- Check MAX98357 connections (BCLK=27, LRC=14, DIN=12)
- Make sure VIN is connected to 5V (not 3.3V)
- Check volume in Settings (default 120%)
- Ensure speaker is connected to MAX98357 output terminals

### Accelerometer not detected:
- Check I2C connections (shared with OLED)
- Make sure ADXL345 CS pin is connected to 3.3V (I2C mode)
- Make sure SDO is connected to GND (address 0x53)

### AI not responding:
- Ensure WiFi is connected (check in Status menu)
- Verify API key is valid
- Check Serial Monitor for HTTP error codes
- Make sure API endpoint is accessible from your network

### Tamagotchi data lost:
- Data is stored in NVS (non-volatile), should not be lost
- If flash is fully erased, data will reset
- Use OTA update (not full flash erase) to preserve data

### Upload fails:
- Make sure "ESP32 Dev Module" board is selected
- Press the BOOT button on ESP32 when upload starts
- Try lowering Upload Speed to 115200
- Make sure USB-to-Serial driver is installed (CP2102/CH340)

---

## Power Supply

### Minimum Requirements:
- Voltage: **5V**
- Current: **1A minimum** (1.5A recommended)

### Power Consumption:

| Mode | Consumption |
|------|-------------|
| Idle (OLED + WiFi standby) | ~270mA |
| Chatting (WiFi + Speaker) | ~550mA |
| Peak (all active) | ~700mA |

### Power Supply Options:

| Option | Notes |
|--------|-------|
| USB Power Bank | 5V/1A minimum, make sure it doesn't auto-off |
| USB Adapter 5V/1.5A | For stationary use |
| 18650 + TP4056 + 5V Boost | Portable & rechargeable |
| 2x 18650 + BMS + 5V Buck | Larger capacity |

### Battery Life Estimates:

| Capacity | Normal Use | Heavy Use |
|----------|------------|-----------|
| 1500mAh | 3-4 hours | 2-2.5 hours |
| 3000mAh | 6-8 hours | 4-5 hours |
| 5000mAh | 10-12 hours | 7-8 hours |

---

## License

This project is made for educational and personal purposes. Feel free to use, modify, and develop it according to your needs.

---

## Contributor

- **Ramadanil** — Creator & Developer

---

*Made with brainnn~ (^_^)*
