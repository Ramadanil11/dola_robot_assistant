# DOLA - Robot Asisten AI Ekspresif

DOLA adalah robot AI ekspresif berbasis ESP32. Fitur: voice assistant (Whisper+GPT+TTS), 19 ekspresi wajah OLED, deteksi gerakan & sentuhan, 7 game (Tamagotchi Ikan, Quiz, Simon Says, dll), jam NTP, timer, memory percakapan, OTA firmware update, dan auto-save ke NVS/SPIFFS.

---

## Daftar Isi

- [Fitur](#fitur)
- [Hardware yang Dibutuhkan](#hardware-yang-dibutuhkan)
- [Wiring Diagram](#wiring-diagram)
- [Software & Library](#software--library)
- [Instalasi & Setup](#instalasi--setup)
- [Konfigurasi API](#konfigurasi-api)
- [Upload SPIFFS (Sound Files)](#upload-spiffs-sound-files)
- [Cara Penggunaan](#cara-penggunaan)
- [Sistem Menu & Navigasi](#sistem-menu--navigasi)
- [Game: Tamagotchi Ikan](#game-tamagotchi-ikan)
- [OTA Update](#ota-update)
- [Struktur File](#struktur-file)
- [Troubleshooting](#troubleshooting)
- [Power Supply](#power-supply)

---

## Fitur

- **Voice Assistant AI** — Ngobrol pakai suara dengan pipeline STT (Whisper) → AI Chat (GPT-4o-mini) → TTS (Nova)
- **19 Ekspresi Wajah** — Pixel art di OLED: idle, happy, sad, angry, scared, dizzy, thinking, listening, speaking, sleepy, surprised, confused, uncomfortable, love, connecting, AP mode, blink, thinking_2, speaking_2
- **Deteksi Gerakan** — Diguncang = pusing, diangkat = takut/marah, dimiringkan = tidak nyaman
- **Touch Sensor** — Tap, double tap, hold untuk navigasi dan interaksi
- **7 Game Interaktif:**
  1. Tebak Angka (Voice)
  2. Simon Says (Sensor Gerak)
  3. Shake Challenge (Sensor Gerak)
  4. Quiz Trivia (Voice + AI)
  5. Batu Gunting Kertas (Voice)
  6. Random Jokes (AI + TTS)
  7. Tamagotchi Ikan (Touch + Sensor) — Pet virtual dengan evolusi, save system, dan gravestone
- **Jam Digital** — NTP sync (WIB/UTC+7) dengan nama hari & bulan Indonesia
- **Countdown Timer** — 1-60 menit dengan alarm suara
- **Conversation Memory** — Menyimpan 20 pesan terakhir ke SPIFFS, bertahan saat restart
- **WiFi Provisioning** — Setup WiFi via AP mode + web portal
- **OTA Firmware Update** — Update firmware & SPIFFS via web dashboard browser
- **Volume Control** — Software volume 0-150% (dengan boost)
- **Expression Sounds** — Setiap ekspresi bisa memicu file WAV dari SPIFFS
- **Auto-Save** — Semua data tersimpan di NVS/SPIFFS, aman saat power off

---

## Hardware yang Dibutuhkan

| Komponen | Spesifikasi | Jumlah | Keterangan |
|----------|-------------|--------|------------|
| ESP32 DevKit V1 | 38 pin, 4MB Flash | 1 | Otak utama |
| INMP441 | I2S MEMS Microphone | 1 | Input suara |
| MAX98357A | I2S DAC + Amplifier | 1 | Output suara |
| Speaker | 3W 4Ohm / 8Ohm | 1 | Untuk audio output |
| OLED SSD1306 | 0.96" 128x64 I2C | 1 | Display wajah & UI |
| ADXL345 | 3-Axis Accelerometer I2C | 1 | Deteksi gerakan |
| TTP223 | Capacitive Touch Sensor | 1 | Input sentuh |
| Breadboard / PCB | - | 1 | Untuk rangkaian |
| Kabel Jumper | Male-Male, Male-Female | ~20 | Koneksi antar komponen |
| Power Supply | 5V 1A minimum | 1 | USB / Baterai |

### Opsional:
- Casing 3D print atau akrilik
- Baterai 18650 + TP4056 + Boost converter 5V (untuk portable)
- Tombol power on/off

---

## Wiring Diagram

### INMP441 Microphone → ESP32 (I2S Port 0)

| INMP441 Pin | ESP32 Pin | Keterangan |
|-------------|-----------|------------|
| VDD | 3.3V | Power |
| GND | GND | Ground |
| SCK | GPIO 26 | Serial Clock |
| WS | GPIO 25 | Word Select (L/R) |
| SD | GPIO 33 | Serial Data |
| L/R | GND | Left channel (connect to GND) |

### MAX98357A Speaker → ESP32 (I2S Port 1)

| MAX98357 Pin | ESP32 Pin | Keterangan |
|--------------|-----------|------------|
| VIN | 5V | Power |
| GND | GND | Ground |
| BCLK | GPIO 27 | Bit Clock |
| LRC | GPIO 14 | Left/Right Clock |
| DIN | GPIO 12 | Data In |
| GAIN | - | Biarkan floating (15dB) atau hubungkan ke GND (9dB) |

### OLED SSD1306 → ESP32 (I2C)

| OLED Pin | ESP32 Pin | Keterangan |
|----------|-----------|------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data |
| SCL | GPIO 22 | I2C Clock |

Alamat I2C: `0x3C`

### ADXL345 Accelerometer → ESP32 (I2C)

| ADXL345 Pin | ESP32 Pin | Keterangan |
|-------------|-----------|------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data (shared dengan OLED) |
| SCL | GPIO 22 | I2C Clock (shared dengan OLED) |
| CS | 3.3V | High = I2C mode |
| SDO | GND | Address = 0x53 |

Alamat I2C: `0x53`

### TTP223 Touch Sensor → ESP32

| TTP223 Pin | ESP32 Pin | Keterangan |
|------------|-----------|------------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SIG/OUT | GPIO 4 (Touch0) | Signal output |

---

## Software & Library

### Arduino IDE Setup:

1. **Install Arduino IDE** (versi 1.8.x atau 2.x)
2. **Tambah ESP32 Board Manager:**
   - Buka File → Preferences
   - Di "Additional Board Manager URLs", tambahkan:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Buka Tools → Board → Board Manager → cari "esp32" → Install

3. **Pilih Board:**
   - Tools → Board → ESP32 Arduino → "ESP32 Dev Module"

4. **Settings Board:**
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB (32Mb)
   - Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
   - PSRAM: Disabled (kecuali board kamu punya PSRAM)

### Library yang Dibutuhkan:

Install via Arduino IDE → Sketch → Include Library → Manage Libraries:

| Library | Versi | Keterangan |
|---------|-------|------------|
| Adafruit GFX Library | ≥1.11.0 | Graphics primitives |
| Adafruit SSD1306 | ≥2.5.0 | OLED driver |
| Adafruit ADXL345 | ≥1.3.0 | Accelerometer driver |
| Adafruit Unified Sensor | ≥1.1.0 | Dependency untuk ADXL345 |
| ArduinoJson | ≥6.21.0 | JSON parsing |
| WiFi | (built-in ESP32) | WiFi connectivity |
| WebServer | (built-in ESP32) | HTTP server |
| HTTPClient | (built-in ESP32) | HTTP client |
| SPIFFS | (built-in ESP32) | File system |
| Preferences | (built-in ESP32) | NVS storage |
| Update | (built-in ESP32) | OTA update |

---

## Instalasi & Setup

### Langkah 1: Clone Repository

```bash
git clone https://github.com/Ramadanil11/delly_robot_assistant.git
cd delly_robot_assistant
```

### Langkah 2: Buka di Arduino IDE

Buka file `delly_robot.ino` di Arduino IDE.

### Langkah 3: Install Library

Install semua library yang tercantum di bagian [Software & Library](#software--library).

### Langkah 4: Konfigurasi API Key

Edit file `delly_robot.ino`, cari bagian ini dan ganti dengan API key kamu sendiri:

```cpp
const char* AI_API_KEY = "YOUR_API_KEY_HERE";
const char* STT_ENDPOINT = "https://api.enowx.com/v1/audio/transcriptions";
const char* CHAT_ENDPOINT = "https://api.enowx.com/v1/chat/completions";
const char* TTS_ENDPOINT = "https://api.enowx.com/v1/audio/speech";
```

### Langkah 5: Upload Firmware

1. Hubungkan ESP32 ke komputer via USB
2. Pilih port COM yang benar di Tools → Port
3. Klik Upload (→)
4. Tunggu sampai selesai

### Langkah 6: Setup WiFi

1. Setelah upload, Dola akan masuk **AP Mode**
2. Di HP/laptop, connect ke WiFi: `Dola-Setup` (password: `dola1234`)
3. Buka browser, akses `192.168.4.1`
4. Pilih WiFi rumah kamu dan masukkan password
5. Dola akan restart dan connect ke WiFi

### Langkah 7: Selesai!

Dola siap digunakan. Coba bicara ke arah mikrofon untuk mulai ngobrol.

---

## Konfigurasi API

DOLA menggunakan API yang kompatibel dengan format OpenAI untuk STT, Chat, dan TTS.

### Endpoint yang Digunakan:

| Fungsi | Endpoint | Model |
|--------|----------|-------|
| Speech-to-Text | `/v1/audio/transcriptions` | whisper-1 |
| Chat AI | `/v1/chat/completions` | gpt-4o-mini |
| Text-to-Speech | `/v1/audio/speech` | tts-1 (voice: nova) |

### Cara Mendapatkan API Key:

1. Daftar di provider API yang kompatibel OpenAI
2. Generate API key
3. Masukkan ke variabel `AI_API_KEY` di code

### Mengubah Kepribadian Dola:

Edit variabel `dolaPersonality` di `delly_robot.ino`:

```cpp
String dolaPersonality = "Kamu adalah Dola, robot asisten kecil yang lucu...";
```

Kamu bisa ubah kepribadian, bahasa, atau gaya bicara Dola sesuai keinginan.

---

## Upload SPIFFS (Sound Files)

DOLA menggunakan file WAV di SPIFFS untuk suara ekspresi dan welcome sound.

### Format File WAV:
- Format: PCM WAV
- Sample Rate: 16000Hz atau 22050Hz
- Bit Depth: 16-bit
- Channel: Mono (1 channel)
- Ukuran: Maksimal total ~1.5MB (limit SPIFFS)

### Daftar File Sound (Opsional):

Letakkan file-file ini di folder `data/`:

```
data/
├── welcome.wav        → Suara saat pertama nyala
├── idle.wav           → Suara idle (sesekali)
├── happy.wav          → Suara senang
├── sad.wav            → Suara sedih
├── angry.wav          → Suara marah
├── scared.wav         → Suara takut
├── dizzy.wav          → Suara pusing
├── thinking.wav       → Suara berpikir
├── listening.wav      → Suara mulai mendengar
├── sleepy.wav         → Suara ngantuk
├── surprised.wav      → Suara kaget
├── confused.wav       → Suara bingung
├── uncomfortable.wav  → Suara tidak nyaman
├── love.wav           → Suara sayang
├── connecting.wav     → Suara connecting
├── ap_mode.wav        → Suara AP mode
├── game_start.wav     → Suara mulai game
├── game_win.wav       → Suara menang
└── game_lose.wav      → Suara kalah
```

### Cara Upload SPIFFS:

**Menggunakan Arduino IDE 1.x:**
1. Install plugin "ESP32 Sketch Data Upload"
2. Letakkan file WAV di folder `data/` (sejajar dengan `.ino`)
3. Tools → ESP32 Sketch Data Upload

**Menggunakan Arduino IDE 2.x:**
1. Install plugin SPIFFS upload untuk IDE 2.x
2. Atau gunakan OTA Web Dashboard (setelah Dola terkoneksi WiFi)

**Menggunakan OTA Dashboard:**
1. Buka browser, akses `http://[IP_DOLA]`
2. Upload file SPIFFS via web interface

> **Catatan:** File sound bersifat opsional. Dola tetap berfungsi penuh tanpa file sound, hanya tidak ada efek suara ekspresi.

---

## Cara Penggunaan

### Ngobrol dengan Dola:
1. Pastikan Dola dalam state **IDLE** (wajah normal)
2. Bicara dengan suara cukup keras ke arah mikrofon
3. Dola akan mendeteksi suara → wajah berubah ke "listening"
4. Setelah selesai bicara (1.5 detik silence), Dola memproses → wajah "thinking"
5. Dola menjawab via speaker → wajah "speaking"
6. Selesai → wajah "happy" → kembali ke "idle"

### Gesture Touch Sensor:

| Gesture | Cara | Durasi |
|---------|------|--------|
| Tap | Sentuh & lepas cepat | < 300ms |
| Double Tap | Sentuh 2x cepat | Jeda < 400ms |
| Hold | Sentuh & tahan | > 800ms |

### Reaksi Gerakan:

| Gerakan | Reaksi Dola |
|---------|-------------|
| Guncang kuat | Pusing (mata spiral) → kembali normal setelah 3 detik |
| Angkat/jatuhkan | Takut atau marah (random) → kembali normal setelah 4 detik |
| Miringkan | Tidak nyaman (wajah berubah) |

### Mode Tidur:
- Setelah 30 detik idle tanpa interaksi, Dola masuk mode tidur (wajah sleepy)
- Bicara keras untuk membangunkan Dola

---

## Sistem Menu & Navigasi

### Masuk Menu:
- **Double Tap** saat di home/idle → masuk menu utama

### Navigasi Menu:
- **Tap** = Pindah ke item berikutnya
- **Double Tap** = Pilih/masuk item
- **Hold** = Kembali ke menu sebelumnya

### Struktur Menu:

```
Menu Utama
├── Games
│   ├── Tebak Angka
│   ├── Simon Says
│   ├── Shake Challenge
│   ├── Quiz Trivia
│   ├── Batu Gunting Kertas
│   ├── Random Jokes
│   └── Tamagotchi
├── Clock
├── Timer
├── Settings
│   ├── Volume (tap: +10, double tap: -10, hold: save & back)
│   └── WiFi Reset
├── Status (info WiFi, IP, SPIFFS, uptime)
└── About Dola
```

---

## Game: Tamagotchi Ikan

### Deskripsi:
Pet virtual ikan yang hidup di layar OLED Dola. Rawat ikan kamu supaya tetap hidup dan berevolusi!

### Kontrol:

| State | Tap | Double Tap | Hold | Shake |
|-------|-----|------------|------|-------|
| Aktif | Beri makan | Ajak main | Tidurkan | Elus |
| Tidur | Bangunkan | Bangunkan | Keluar game | - |
| Mati | Mulai lagi | Lihat kuburan | Keluar game | - |
| Kuburan | Kembali | Kembali | Kembali | - |

### Stats:

| Stat | Deskripsi | Decay Rate |
|------|-----------|------------|
| Hunger (H) | Tingkat kenyang | -2 per 30 detik |
| Happiness (P) | Tingkat kesenangan | -1 per 30 detik |
| Energy (E) | Tingkat energi | -1 per 30 detik |
| Health (♥) | Kesehatan | -1 jika hunger<20 atau happiness<15 |

### Evolusi:

| Stage | Bentuk | Syarat |
|-------|--------|--------|
| 0 - Telur | Bulat goyang | Awal |
| 1 - Baby | Ikan kecil | 5 menit + health > 50 |
| 2 - Teen | Ikan medium + sirip | 1 jam + health > 40 |
| 3 - Adult | Ikan besar detail | 3 jam + health > 50 |

### Sistem Kematian:
- Ikan mati jika Health mencapai 0
- Penyebab: terlalu lapar (hunger < 20) atau terlalu sedih (happiness < 15) dalam waktu lama
- Warning "!" muncul saat health < 20

### Setelah Mati:
- **Tap** = Rebirth (mulai dari telur baru)
- **Double Tap** = Lihat kuburan (5 ikan terakhir yang mati)
- **Hold** = Keluar

### Fitur Tambahan:
- **Highscore** — Rekor umur terlama tersimpan
- **Death Counter** — Jumlah total kali ikan mati
- **Gravestone** — Daftar 5 ikan terakhir yang mati (stage + umur)
- **Auto-Save** — Stats tersimpan otomatis setiap 60 detik ke NVS
- **Offline Decay** — Saat robot dimatikan, stats berkurang saat dinyalakan lagi

### Tips:
- Rajin kasih makan (tap) supaya hunger tidak turun di bawah 20
- Ajak main (double tap) supaya happiness tetap tinggi
- Tidurin (hold) kalau energy rendah
- Elus (shake) untuk bonus happiness
- Jangan matikan robot terlalu lama, ikan bisa mati kelaparan!

---

## OTA Update

Setelah Dola terkoneksi WiFi, kamu bisa mengakses web dashboard untuk update firmware tanpa kabel USB.

### Akses Dashboard:
1. Cari IP Dola di menu Status, atau cek Serial Monitor
2. Buka browser: `http://[IP_DOLA]`

### Fitur Dashboard:
- Upload firmware baru (.bin)
- Upload file SPIFFS (.bin)
- Lihat info device (IP, SSID, uptime, memory)
- Reset WiFi
- Manage conversation memory

---

## Struktur File

```
delly_robot_assistant/
│
├── delly_robot.ino          → Main sketch: setup(), loop(), state machine,
│                              WiFi, audio recording, AI communication,
│                              motion detection, state handlers
│
├── dola_faces.h             → 19 ekspresi wajah sebagai bitmap PROGMEM
│                              + fungsi showFace() untuk render ke OLED
│
├── dola_touch.h             → Touch sensor driver (TTP223 / Touch0)
│                              Gesture detection: tap, double tap, hold
│
├── dola_memory.h            → Conversation memory system
│                              Save/load 20 pesan terakhir ke SPIFFS (JSON)
│                              buildChatPayload() untuk context AI
│
├── dola_menu.h              → Menu system dengan navigasi touch
│                              Render menu list di OLED
│                              Handle gesture per screen
│
├── dola_games.h             → 6 game: Tebak Angka, Simon Says, Shake,
│                              Quiz Trivia, BSK, Random Jokes
│                              Game dispatcher (startGame/updateGame)
│
├── dola_tamagotchi.h        → Game Tamagotchi Ikan
│                              Bitmap ikan (4 stage × 2 frame)
│                              Stats, decay, evolusi, save/load NVS
│                              Gravestone system, highscore
│
├── dola_clock.h             → NTP clock (WIB) + countdown timer
│                              Tampilan jam di OLED
│                              Timer dengan alarm
│
├── dola_ota.h               → OTA web dashboard
│                              HTTP server untuk firmware update
│                              SPIFFS upload, WiFi reset, memory management
│
├── data/                    → Folder SPIFFS (file WAV sound)
│   └── (*.wav files)
│
└── README.md                → Dokumentasi ini
```

---

## Troubleshooting

### Dola tidak bisa connect WiFi:
- Pastikan SSID dan password benar
- Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
- Jika gagal, Dola otomatis kembali ke AP Mode → setup ulang

### OLED tidak menyala:
- Cek koneksi I2C (SDA=GPIO21, SCL=GPIO22)
- Pastikan alamat I2C benar (0x3C)
- Coba scan I2C dengan sketch I2C Scanner

### Mikrofon tidak mendeteksi suara:
- Cek koneksi INMP441 (SCK=26, WS=25, SD=33)
- Pastikan pin L/R terhubung ke GND (left channel)
- Bicara lebih keras atau dekatkan ke mikrofon
- Cek Serial Monitor untuk debug level audio

### Speaker tidak bersuara:
- Cek koneksi MAX98357 (BCLK=27, LRC=14, DIN=12)
- Pastikan VIN terhubung ke 5V (bukan 3.3V)
- Cek volume di Settings (default 120%)
- Pastikan speaker terhubung ke output MAX98357

### Accelerometer tidak terdeteksi:
- Cek koneksi I2C (shared dengan OLED)
- Pastikan CS pin ADXL345 terhubung ke 3.3V (I2C mode)
- Pastikan SDO terhubung ke GND (address 0x53)

### AI tidak merespons:
- Pastikan WiFi terkoneksi (cek di menu Status)
- Pastikan API key valid
- Cek Serial Monitor untuk error code HTTP
- Pastikan endpoint API accessible

### Tamagotchi data hilang:
- Data tersimpan di NVS (non-volatile), seharusnya tidak hilang
- Jika flash di-erase total, data akan reset
- Gunakan OTA update (bukan full flash erase) untuk menjaga data

### Upload gagal:
- Pastikan board "ESP32 Dev Module" dipilih
- Tekan tombol BOOT di ESP32 saat upload dimulai
- Coba turunkan Upload Speed ke 115200
- Pastikan driver USB-to-Serial terinstall (CP2102/CH340)

---

## Power Supply

### Kebutuhan Minimum:
- Tegangan: **5V**
- Arus: **1A minimum** (rekomendasi 1.5A)

### Konsumsi Daya:

| Mode | Konsumsi |
|------|----------|
| Idle (OLED + WiFi standby) | ~270mA |
| Ngobrol (WiFi + Speaker) | ~550mA |
| Peak (semua aktif) | ~700mA |

### Opsi Power Supply:

| Opsi | Keterangan |
|------|------------|
| USB Power Bank | 5V/1A minimum, pastikan tidak auto-off |
| Adaptor USB 5V/1.5A | Untuk penggunaan statis |
| 18650 + TP4056 + Boost 5V | Portable & rechargeable |
| 2x 18650 + BMS + Buck 5V | Kapasitas lebih besar |

### Estimasi Daya Tahan Baterai:

| Kapasitas | Penggunaan Normal | Penggunaan Berat |
|-----------|-------------------|------------------|
| 1500mAh | 3-4 jam | 2-2.5 jam |
| 3000mAh | 6-8 jam | 4-5 jam |
| 5000mAh | 10-12 jam | 7-8 jam |

---

## Lisensi

Project ini dibuat untuk keperluan edukasi dan personal. Silakan gunakan, modifikasi, dan kembangkan sesuai kebutuhan.

---

## Kontributor

- **Ramadanil** — Creator & Developer

---

*Made with love~ (^_^)*
