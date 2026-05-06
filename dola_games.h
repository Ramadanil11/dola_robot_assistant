/*
 * ============================================
 *   DOLA - Games System (7 Games)
 * ============================================
 * 1. Tebak Angka (Voice)
 * 2. Simon Says (Sensor Gerak)
 * 3. Shake Challenge (Sensor Gerak)
 * 4. Quiz Trivia (Voice AI)
 * 5. Batu Gunting Kertas (Voice)
 * 6. Random Jokes (TTS AI)
 * 7. Tamagotchi Ikan (Touch + Sensor)
 */

#ifndef DOLA_GAMES_H
#define DOLA_GAMES_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_ADXL345_U.h>

// External references
extern Adafruit_SSD1306 display;
extern Adafruit_ADXL345_Unified accel;
extern void playWelcomeSound(const char* filename);
extern String speechToText();
extern String chatWithAI(String userMessage);
extern bool textToSpeech(String text);
extern bool recordAudio();
extern void showFace(DolaFace face);

// ============ GAME STATE ============
enum GameState {
  GAME_INIT,
  GAME_PLAYING,
  GAME_WAITING_INPUT,
  GAME_RESULT,
  GAME_OVER
};

GameState gameState = GAME_INIT;
int gameScore = 0;
int gameRound = 0;
int gameMaxRounds = 5;
bool gameActive = false;

// ============ GAME SOUNDS ============
void playGameSound(const char* type) {
  String filename = String("/") + type + ".wav";
  playWelcomeSound(filename.c_str());
}

// ============ GAME DISPLAY HELPERS ============
void showGameTitle(const char* title) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== ");
  display.print(title);
  display.print(" ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  display.display();
}

void showGameMessage(const char* title, const char* msg1, const char* msg2 = nullptr) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== ");
  display.print(title);
  display.print(" ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  display.setCursor(5, 25);
  display.print(msg1);
  if (msg2) {
    display.setCursor(5, 40);
    display.print(msg2);
  }
  display.display();
}

void showGameScore(const char* title, int score, int maxScore) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== SKOR ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(30, 20);
  display.printf("%d/%d", score, maxScore);
  
  display.setTextSize(1);
  display.setCursor(20, 45);
  if (score == maxScore) {
    display.print("SEMPURNA! Hebat~");
  } else if (score > maxScore / 2) {
    display.print("Bagus! Keren~");
  } else {
    display.print("Coba lagi ya~");
  }
  
  display.setCursor(15, 56);
  display.print("hold = kembali");
  display.display();
}

// ============ GAME 1: TEBAK ANGKA ============
int tebakTarget = 0;
int tebakAttempts = 0;

void startGameTebakAngka() {
  gameActive = true;
  gameState = GAME_INIT;
  tebakTarget = random(1, 101);  // 1-100
  tebakAttempts = 0;
  
  showGameMessage("TEBAK ANGKA", "Dola pilih angka 1-100", "Coba tebak ya~!");
  
  // TTS instruction
  textToSpeech("Aku udah pilih angka antara 1 sampai 100. Coba tebak!");
  
  gameState = GAME_WAITING_INPUT;
  showGameMessage("TEBAK ANGKA", "Bilang angka tebakan mu!", "Dola dengerin...");
}

// Returns true if game is over
bool updateGameTebakAngka() {
  if (gameState != GAME_WAITING_INPUT) return false;
  
  showGameMessage("TEBAK ANGKA", "Bilang angka tebakan mu!", "Dola dengerin...");
  
  // Record & recognize
  if (recordAudio()) {
    String text = speechToText();
    if (text.length() > 0) {
      // Parse angka dari text
      int guess = 0;
      for (int i = 0; i < text.length(); i++) {
        if (isDigit(text[i])) {
          guess = guess * 10 + (text[i] - '0');
        }
      }
      
      tebakAttempts++;
      
      if (guess == 0) {
        textToSpeech("Hmm, Dola ga denger angkanya. Coba bilang lagi!");
        return false;
      }
      
      char buf[64];
      sprintf(buf, "Tebakan: %d", guess);
      
      if (guess == tebakTarget) {
        // BENAR!
        sprintf(buf, "BENAR! Angkanya %d!", tebakTarget);
        showGameMessage("TEBAK ANGKA", buf, "");
        char msg[100];
        sprintf(msg, "BENAR! Angkanya %d! Kamu hebat, cuma butuh %d kali tebak!", tebakTarget, tebakAttempts);
        textToSpeech(msg);
        playGameSound("game_win");
        gameState = GAME_OVER;
        
        // Show score
        char scoreBuf[32];
        sprintf(scoreBuf, "%d kali tebak", tebakAttempts);
        showGameMessage("TEBAK ANGKA", "KAMU MENANG!", scoreBuf);
        return true;
      } else if (guess < tebakTarget) {
        sprintf(buf, "Tebakan %d: KURANG!", guess);
        showGameMessage("TEBAK ANGKA", buf, "Lebih tinggi lagi~");
        textToSpeech("Kurang! Lebih tinggi lagi dong~");
      } else {
        sprintf(buf, "Tebakan %d: KEBANYAKAN!", guess);
        showGameMessage("TEBAK ANGKA", buf, "Lebih rendah~");
        textToSpeech("Kebanyakan! Turunin lagi~");
      }
      
      // Max 10 attempts
      if (tebakAttempts >= 10) {
        char msg[100];
        sprintf(msg, "Yah, udah 10 kali. Jawabannya %d. Coba lagi ya!", tebakTarget);
        textToSpeech(msg);
        playGameSound("game_lose");
        gameState = GAME_OVER;
        return true;
      }
    }
  }
  return false;
}

// ============ GAME 2: SIMON SAYS ============
enum SimonAction {
  SIMON_SHAKE,
  SIMON_TILT_LEFT,
  SIMON_TILT_RIGHT,
  SIMON_FLIP
};

const char* simonActionNames[] = {"guncang", "miringkan ke kiri", "miringkan ke kanan", "balikkan"};
int simonRound = 0;
int simonScore = 0;
bool simonSays = true;  // true = harus ikut, false = jebakan
SimonAction simonCurrentAction;
unsigned long simonWaitStart = 0;

void startGameSimonSays() {
  gameActive = true;
  gameState = GAME_INIT;
  simonRound = 0;
  simonScore = 0;
  gameMaxRounds = 8;
  
  showGameMessage("SIMON SAYS", "Ikuti perintah Dola!", "Tapi hati-hati jebakan~");
  textToSpeech("Ayo main Simon Says! Ikuti perintah Dola, tapi kalau Dola ga bilang Simon Says, jangan gerak ya!");
  delay(1000);
  
  gameState = GAME_PLAYING;
  simonNextRound();
}

void simonNextRound() {
  simonRound++;
  if (simonRound > gameMaxRounds) {
    // Game over - show score
    gameState = GAME_OVER;
    showGameScore("SIMON SAYS", simonScore, gameMaxRounds);
    char msg[100];
    sprintf(msg, "Game selesai! Skor kamu %d dari %d. ", simonScore, gameMaxRounds);
    if (simonScore >= 6) strcat(msg, "Hebat banget!");
    else strcat(msg, "Coba lagi ya~");
    textToSpeech(msg);
    if (simonScore >= gameMaxRounds / 2) playGameSound("game_win");
    else playGameSound("game_lose");
    return;
  }
  
  // Random action & whether it's "simon says" or trap
  simonCurrentAction = (SimonAction)random(0, 4);
  simonSays = (random(100) < 70);  // 70% chance simon says, 30% trap
  
  // Announce
  char msg[100];
  if (simonSays) {
    sprintf(msg, "Simon says... %s!", simonActionNames[simonCurrentAction]);
  } else {
    sprintf(msg, "%s!", simonActionNames[simonCurrentAction]);  // Jebakan! Tanpa "Simon says"
  }
  
  char displayMsg[64];
  sprintf(displayMsg, "Ronde %d/%d", simonRound, gameMaxRounds);
  showGameMessage("SIMON SAYS", displayMsg, msg);
  textToSpeech(msg);
  
  simonWaitStart = millis();
  gameState = GAME_WAITING_INPUT;
}

bool detectSimonAction(SimonAction expected) {
  sensors_event_t event;
  accel.getEvent(&event);
  
  float ax = event.acceleration.x;
  float ay = event.acceleration.y;
  float az = event.acceleration.z;
  float total = sqrt(ax*ax + ay*ay + az*az);
  
  switch (expected) {
    case SIMON_SHAKE:
      return (total > 15.0);  // Strong movement
    case SIMON_TILT_LEFT:
      return (ax < -6.0);
    case SIMON_TILT_RIGHT:
      return (ax > 6.0);
    case SIMON_FLIP:
      return (az < -5.0);  // Upside down
  }
  return false;
}

bool detectAnyMovement() {
  sensors_event_t event;
  accel.getEvent(&event);
  float total = sqrt(event.acceleration.x * event.acceleration.x + 
                     event.acceleration.y * event.acceleration.y + 
                     event.acceleration.z * event.acceleration.z);
  // Significant deviation from normal gravity
  return (abs(total - 9.8) > 4.0) || (abs(event.acceleration.x) > 5.0);
}

bool updateGameSimonSays() {
  if (gameState != GAME_WAITING_INPUT) return (gameState == GAME_OVER);
  
  unsigned long elapsed = millis() - simonWaitStart;
  
  // Timeout 4 seconds
  if (elapsed > 4000) {
    if (simonSays) {
      // Should have moved but didn't
      showGameMessage("SIMON SAYS", "Terlalu lambat!", "");
      textToSpeech("Terlalu lambat!");
      delay(1000);
    } else {
      // Trap - didn't move = correct!
      simonScore++;
      showGameMessage("SIMON SAYS", "Bagus! Ga ketipu~", "");
      textToSpeech("Bagus! Kamu ga ketipu!");
      delay(1000);
    }
    simonNextRound();
    return (gameState == GAME_OVER);
  }
  
  // Check for movement
  if (simonSays) {
    // Should do the action
    if (detectSimonAction(simonCurrentAction)) {
      simonScore++;
      showGameMessage("SIMON SAYS", "Benar!", "");
      textToSpeech("Benar!");
      delay(800);
      simonNextRound();
    }
  } else {
    // Trap! Should NOT move
    if (detectAnyMovement()) {
      showGameMessage("SIMON SAYS", "KETIPU! Dola ga bilang", "Simon Says~");
      textToSpeech("Haha ketipu! Dola ga bilang Simon Says~");
      delay(1000);
      simonNextRound();
    }
  }
  
  return (gameState == GAME_OVER);
}

// ============ GAME 3: SHAKE CHALLENGE ============
int shakeGameCount = 0;
unsigned long shakeGameStart = 0;
unsigned long shakeGameDuration = 5000;  // 5 detik
float shakeLastTotal = 9.8;

void startGameShakeChallenge() {
  gameActive = true;
  gameState = GAME_INIT;
  shakeGameCount = 0;
  
  showGameMessage("SHAKE CHALLENGE", "Guncang Dola sekenceng", "mungkin dalam 5 detik!");
  textToSpeech("Guncang aku sekenceng-kencengnya dalam 5 detik! Siap?");
  delay(1000);
  
  // Countdown
  for (int i = 3; i > 0; i--) {
    char buf[8];
    sprintf(buf, "%d...", i);
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(50, 20);
    display.print(i);
    display.display();
    char numStr[16];
    sprintf(numStr, "%d", i);
    textToSpeech(numStr);
    delay(800);
  }
  
  textToSpeech("MULAI!");
  playGameSound("game_start");
  shakeGameStart = millis();
  shakeGameCount = 0;
  gameState = GAME_PLAYING;
}

bool updateGameShakeChallenge() {
  if (gameState != GAME_PLAYING) return (gameState == GAME_OVER);
  
  unsigned long elapsed = millis() - shakeGameStart;
  
  // Check shake
  sensors_event_t event;
  accel.getEvent(&event);
  float total = sqrt(event.acceleration.x * event.acceleration.x + 
                     event.acceleration.y * event.acceleration.y + 
                     event.acceleration.z * event.acceleration.z);
  
  // Detect shake (significant change from last reading)
  if (abs(total - shakeLastTotal) > 8.0) {
    shakeGameCount++;
  }
  shakeLastTotal = total;
  
  // Update display
  float timeLeft = (shakeGameDuration - elapsed) / 1000.0;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("== SHAKE! ==");
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 18);
  display.printf("Skor:%d", shakeGameCount);
  
  display.setTextSize(1);
  display.setCursor(10, 45);
  display.printf("Waktu: %.1f detik", timeLeft > 0 ? timeLeft : 0);
  
  // Progress bar
  int barW = map(elapsed, 0, shakeGameDuration, 0, 120);
  display.drawRect(4, 56, 120, 6, SSD1306_WHITE);
  display.fillRect(4, 56, barW, 6, SSD1306_WHITE);
  
  display.display();
  
  // Time's up!
  if (elapsed >= shakeGameDuration) {
    gameState = GAME_OVER;
    
    char msg[100];
    if (shakeGameCount > 50) {
      sprintf(msg, "WOW! %d guncangan! Kamu gila! Dola pusing banget!", shakeGameCount);
    } else if (shakeGameCount > 30) {
      sprintf(msg, "Keren! %d guncangan! Lumayan kenceng~", shakeGameCount);
    } else if (shakeGameCount > 15) {
      sprintf(msg, "%d guncangan. Yah biasa aja sih~", shakeGameCount);
    } else {
      sprintf(msg, "Cuma %d? Lemah banget! Masa gitu doang~", shakeGameCount);
    }
    
    showGameMessage("SHAKE CHALLENGE", "WAKTU HABIS!", "");
    textToSpeech(msg);
    
    if (shakeGameCount > 30) playGameSound("game_win");
    else playGameSound("game_lose");
    
    // Show final score
    char scoreBuf[32];
    sprintf(scoreBuf, "Skor: %d guncangan", shakeGameCount);
    showGameMessage("SHAKE CHALLENGE", "SELESAI!", scoreBuf);
    return true;
  }
  
  return false;
}

// ============ GAME 4: QUIZ TRIVIA ============
int quizScore = 0;
int quizRound = 0;
int quizMaxRounds = 5;

void startGameQuizTrivia() {
  gameActive = true;
  gameState = GAME_INIT;
  quizScore = 0;
  quizRound = 0;
  
  showGameMessage("QUIZ TRIVIA", "Dola akan kasih 5", "pertanyaan. Siap?");
  textToSpeech("Ayo main quiz! Dola kasih 5 pertanyaan, jawab pakai suara ya. Siap?");
  delay(1500);
  
  gameState = GAME_PLAYING;
  quizNextQuestion();
}

void quizNextQuestion() {
  quizRound++;
  if (quizRound > quizMaxRounds) {
    gameState = GAME_OVER;
    showGameScore("QUIZ", quizScore, quizMaxRounds);
    char msg[100];
    sprintf(msg, "Quiz selesai! Skor kamu %d dari %d. ", quizScore, quizMaxRounds);
    if (quizScore == quizMaxRounds) strcat(msg, "Sempurna! Kamu jenius!");
    else if (quizScore >= 3) strcat(msg, "Lumayan pinter~");
    else strcat(msg, "Belajar lagi ya hehe~");
    textToSpeech(msg);
    if (quizScore >= 3) playGameSound("game_win");
    else playGameSound("game_lose");
    return;
  }
  
  char roundBuf[32];
  sprintf(roundBuf, "Pertanyaan %d/%d", quizRound, quizMaxRounds);
  showGameMessage("QUIZ TRIVIA", roundBuf, "Generating...");
  
  // Generate question from AI
  String prompt = "Buat 1 pertanyaan quiz trivia sederhana dalam bahasa Indonesia "
                  "(pengetahuan umum, sains, geografi, atau sejarah). "
                  "Format: PERTANYAAN: [pertanyaan]\nJAWABAN: [jawaban singkat 1-3 kata]. "
                  "Jangan terlalu susah.";
  
  String aiResponse = chatWithAI(prompt);
  
  // Parse question and answer
  String question = "";
  String answer = "";
  
  int qIdx = aiResponse.indexOf("PERTANYAAN:");
  int aIdx = aiResponse.indexOf("JAWABAN:");
  
  if (qIdx >= 0 && aIdx >= 0) {
    question = aiResponse.substring(qIdx + 11, aIdx);
    answer = aiResponse.substring(aIdx + 8);
    question.trim();
    answer.trim();
  } else {
    question = aiResponse;
    answer = "unknown";
  }
  
  // Show & speak question
  showGameMessage("QUIZ TRIVIA", roundBuf, question.c_str());
  textToSpeech(question);
  
  // Wait for answer
  showGameMessage("QUIZ TRIVIA", "Jawab sekarang!", "Dola dengerin...");
  
  if (recordAudio()) {
    String userAnswer = speechToText();
    
    if (userAnswer.length() > 0) {
      // Ask AI to check if answer is correct
      String checkPrompt = "Pertanyaan: " + question + "\n"
                           "Jawaban benar: " + answer + "\n"
                           "Jawaban user: " + userAnswer + "\n"
                           "Apakah jawaban user benar atau mendekati benar? "
                           "Jawab hanya dengan 'BENAR' atau 'SALAH' diikuti penjelasan singkat.";
      
      String checkResult = chatWithAI(checkPrompt);
      
      if (checkResult.indexOf("BENAR") >= 0) {
        quizScore++;
        showGameMessage("QUIZ", "BENAR!", answer.c_str());
        textToSpeech("Benar! Pinter~");
        delay(1000);
      } else {
        char wrongBuf[100];
        sprintf(wrongBuf, "Jawaban: %s", answer.c_str());
        showGameMessage("QUIZ", "SALAH!", wrongBuf);
        String wrongMsg = "Salah~ jawabannya " + answer;
        textToSpeech(wrongMsg);
        delay(1000);
      }
    } else {
      showGameMessage("QUIZ", "Ga kedenger!", "Skip...");
      textToSpeech("Dola ga denger, skip ya~");
      delay(1000);
    }
  }
  
  quizNextQuestion();
}

bool updateGameQuizTrivia() {
  // Quiz runs synchronously in quizNextQuestion()
  return (gameState == GAME_OVER);
}

// ============ GAME 5: BATU GUNTING KERTAS ============
int bskPlayerScore = 0;
int bskDolaScore = 0;
int bskRound = 0;
int bskMaxRounds = 3;  // Best of 3

enum BSKChoice { BSK_BATU, BSK_GUNTING, BSK_KERTAS, BSK_INVALID };

BSKChoice parseBSKChoice(String text) {
  text.toLowerCase();
  if (text.indexOf("batu") >= 0) return BSK_BATU;
  if (text.indexOf("gunting") >= 0) return BSK_GUNTING;
  if (text.indexOf("kertas") >= 0) return BSK_KERTAS;
  return BSK_INVALID;
}

const char* bskChoiceName(BSKChoice c) {
  switch (c) {
    case BSK_BATU: return "BATU";
    case BSK_GUNTING: return "GUNTING";
    case BSK_KERTAS: return "KERTAS";
    default: return "???";
  }
}

// 0 = draw, 1 = player wins, -1 = dola wins
int bskWinner(BSKChoice player, BSKChoice dola) {
  if (player == dola) return 0;
  if ((player == BSK_BATU && dola == BSK_GUNTING) ||
      (player == BSK_GUNTING && dola == BSK_KERTAS) ||
      (player == BSK_KERTAS && dola == BSK_BATU)) return 1;
  return -1;
}

void startGameBSK() {
  gameActive = true;
  gameState = GAME_INIT;
  bskPlayerScore = 0;
  bskDolaScore = 0;
  bskRound = 0;
  
  showGameMessage("SUIT!", "Batu Gunting Kertas", "Best of 3!");
  textToSpeech("Ayo main suit! Best of three ya. Bilang batu, gunting, atau kertas!");
  delay(1000);
  
  gameState = GAME_PLAYING;
  bskNextRound();
}

void bskNextRound() {
  bskRound++;
  
  // Check if someone already won
  if (bskPlayerScore >= 2) {
    gameState = GAME_OVER;
    showGameMessage("SUIT!", "KAMU MENANG!", "Dola kalah huhu~");
    textToSpeech("Yah Dola kalah~ kamu jago deh!");
    playGameSound("game_win");
    return;
  }
  if (bskDolaScore >= 2) {
    gameState = GAME_OVER;
    showGameMessage("SUIT!", "DOLA MENANG!", "Hehe~ Dola jago kan~");
    textToSpeech("Hehe Dola menang! Dola emang jago~");
    playGameSound("game_lose");
    return;
  }
  if (bskRound > 5) {  // Safety: max 5 rounds (in case of draws)
    gameState = GAME_OVER;
    if (bskPlayerScore > bskDolaScore) {
      showGameMessage("SUIT!", "KAMU MENANG!", "");
      playGameSound("game_win");
    } else if (bskDolaScore > bskPlayerScore) {
      showGameMessage("SUIT!", "DOLA MENANG!", "");
      playGameSound("game_lose");
    } else {
      showGameMessage("SUIT!", "SERI!", "");
    }
    return;
  }
  
  char roundBuf[32];
  sprintf(roundBuf, "Ronde %d (Kamu:%d Dola:%d)", bskRound, bskPlayerScore, bskDolaScore);
  showGameMessage("SUIT!", roundBuf, "");
  
  // Countdown
  textToSpeech("Batu... Gunting... KERTAS!");
  
  showGameMessage("SUIT!", "Bilang pilihanmu!", "Batu/Gunting/Kertas?");
  
  // Get player choice via voice
  BSKChoice playerChoice = BSK_INVALID;
  if (recordAudio()) {
    String text = speechToText();
    playerChoice = parseBSKChoice(text);
  }
  
  if (playerChoice == BSK_INVALID) {
    showGameMessage("SUIT!", "Ga kedenger!", "Ulang ronde ini...");
    textToSpeech("Dola ga denger, ulang ya!");
    delay(1000);
    bskRound--;  // Retry this round
    bskNextRound();
    return;
  }
  
  // Dola's choice (random)
  BSKChoice dolaChoice = (BSKChoice)random(0, 3);
  
  // Determine winner
  int result = bskWinner(playerChoice, dolaChoice);
  
  char resultBuf[64];
  sprintf(resultBuf, "Kamu:%s vs Dola:%s", bskChoiceName(playerChoice), bskChoiceName(dolaChoice));
  
  if (result == 1) {
    bskPlayerScore++;
    showGameMessage("SUIT!", resultBuf, "Kamu menang ronde ini!");
    String msg = String("Kamu pilih ") + bskChoiceName(playerChoice) + ", Dola pilih " + bskChoiceName(dolaChoice) + ". Kamu menang!";
    textToSpeech(msg);
  } else if (result == -1) {
    bskDolaScore++;
    showGameMessage("SUIT!", resultBuf, "Dola menang! Hehe~");
    String msg = String("Kamu pilih ") + bskChoiceName(playerChoice) + ", Dola pilih " + bskChoiceName(dolaChoice) + ". Dola menang hehe~";
    textToSpeech(msg);
  } else {
    showGameMessage("SUIT!", resultBuf, "Seri! Ulang~");
    textToSpeech("Seri! Sama-sama " + String(bskChoiceName(playerChoice)) + "! Ulang!");
    bskRound--;  // Don't count draws
  }
  
  delay(1500);
  bskNextRound();
}

bool updateGameBSK() {
  return (gameState == GAME_OVER);
}

// ============ GAME 6: RANDOM JOKES ============
void startGameJokes() {
  gameActive = true;
  gameState = GAME_PLAYING;
  
  showGameMessage("JOKES", "Dola mau cerita", "joke nih~ dengerin ya!");
  
  // Generate joke from AI
  String prompt = "Ceritakan 1 joke atau lelucon lucu dalam bahasa Indonesia. "
                  "Bisa joke receh, pun, atau cerita pendek lucu. "
                  "Maksimal 2-3 kalimat. Harus lucu dan cocok untuk semua umur.";
  
  String joke = chatWithAI(prompt);
  
  if (joke.length() > 0) {
    // Show on display (truncated if too long)
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("== JOKE TIME ==");
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
    
    // Word wrap joke on display
    display.setCursor(0, 14);
    int charCount = 0;
    for (int i = 0; i < joke.length() && charCount < 120; i++) {
      display.print(joke[i]);
      charCount++;
      if (charCount % 21 == 0) {
        display.println();
      }
    }
    display.display();
    
    // Speak the joke
    textToSpeech(joke);
    
    showFace(FACE_HAPPY);
    delay(2000);
  } else {
    showGameMessage("JOKES", "Aduh, Dola lupa", "joke-nya... hehe");
    textToSpeech("Aduh Dola lupa joke-nya. Maaf ya hehe~");
  }
  
  gameState = GAME_OVER;
  showGameMessage("JOKES", "Lucu kan? Hehe~", "hold = kembali");
}

bool updateGameJokes() {
  return (gameState == GAME_OVER);
}

// ============ GAME DISPATCHER ============
// Forward declaration for Tamagotchi (defined in dola_tamagotchi.h)
extern void startTamagotchi();
extern bool updateTamagotchi();

void startGame(MenuScreen gameScreen) {
  if (gameScreen != SCREEN_GAME_TAMAGOTCHI) {
    playGameSound("game_start");
  }
  
  switch (gameScreen) {
    case SCREEN_GAME_TEBAK: startGameTebakAngka(); break;
    case SCREEN_GAME_SIMON: startGameSimonSays(); break;
    case SCREEN_GAME_SHAKE: startGameShakeChallenge(); break;
    case SCREEN_GAME_QUIZ:  startGameQuizTrivia(); break;
    case SCREEN_GAME_BSK:   startGameBSK(); break;
    case SCREEN_GAME_JOKES: startGameJokes(); break;
    case SCREEN_GAME_TAMAGOTCHI: startTamagotchi(); break;
    default: break;
  }
}

// Update game loop - returns true when game is over
bool updateGame(MenuScreen gameScreen) {
  switch (gameScreen) {
    case SCREEN_GAME_TEBAK: return updateGameTebakAngka();
    case SCREEN_GAME_SIMON: return updateGameSimonSays();
    case SCREEN_GAME_SHAKE: return updateGameShakeChallenge();
    case SCREEN_GAME_QUIZ:  return updateGameQuizTrivia();
    case SCREEN_GAME_BSK:   return updateGameBSK();
    case SCREEN_GAME_JOKES: return updateGameJokes();
    case SCREEN_GAME_TAMAGOTCHI: return updateTamagotchi();
    default: return true;
  }
}

void exitGame() {
  gameActive = false;
  gameState = GAME_INIT;
  gameScore = 0;
  gameRound = 0;
}

#endif // DOLA_GAMES_H
