/*
 * ============================================
 *   DOLA - Conversation Memory System
 * ============================================
 * Menyimpan 20 pesan terakhir (10 bolak-balik)
 * Persistent: disimpan ke SPIFFS (/memory.json)
 * Tetap ingat walau restart!
 */

#ifndef DOLA_MEMORY_H
#define DOLA_MEMORY_H

#include <SPIFFS.h>
#include <ArduinoJson.h>

// ============ CONFIG ============
#define MAX_MEMORY_MESSAGES  20    // 10 user + 10 assistant
#define MEMORY_FILE          "/memory.json"

// ============ STRUCT ============
struct ChatMessage {
  String role;      // "user" atau "assistant"
  String content;   // Isi pesan
};

// ============ STATE ============
ChatMessage chatHistory[MAX_MEMORY_MESSAGES];
int chatHistoryCount = 0;

// ============ FUNCTIONS ============

// Load memory dari SPIFFS saat boot
void loadMemory() {
  if (!SPIFFS.exists(MEMORY_FILE)) {
    Serial.println("Memory file tidak ada, mulai fresh.");
    chatHistoryCount = 0;
    return;
  }
  
  File file = SPIFFS.open(MEMORY_FILE, "r");
  if (!file) {
    Serial.println("Gagal buka memory file!");
    chatHistoryCount = 0;
    return;
  }
  
  // Parse JSON
  DynamicJsonDocument doc(16384);  // 16KB buffer untuk 20 pesan
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("Memory JSON parse error: %s\n", error.c_str());
    chatHistoryCount = 0;
    return;
  }
  
  // Load messages
  JsonArray messages = doc["messages"].as<JsonArray>();
  chatHistoryCount = 0;
  
  for (JsonObject msg : messages) {
    if (chatHistoryCount >= MAX_MEMORY_MESSAGES) break;
    chatHistory[chatHistoryCount].role = msg["role"].as<String>();
    chatHistory[chatHistoryCount].content = msg["content"].as<String>();
    chatHistoryCount++;
  }
  
  Serial.printf("Memory loaded: %d pesan\n", chatHistoryCount);
}

// Simpan memory ke SPIFFS
void saveMemory() {
  DynamicJsonDocument doc(16384);
  JsonArray messages = doc.createNestedArray("messages");
  
  for (int i = 0; i < chatHistoryCount; i++) {
    JsonObject msg = messages.createNestedObject();
    msg["role"] = chatHistory[i].role;
    msg["content"] = chatHistory[i].content;
  }
  
  File file = SPIFFS.open(MEMORY_FILE, "w");
  if (!file) {
    Serial.println("Gagal simpan memory!");
    return;
  }
  
  serializeJson(doc, file);
  file.close();
  
  Serial.printf("Memory saved: %d pesan\n", chatHistoryCount);
}

// Tambah pesan ke memory
void addToMemory(String role, String content) {
  // Kalau penuh, hapus 2 pesan terlama (1 user + 1 assistant)
  if (chatHistoryCount >= MAX_MEMORY_MESSAGES) {
    // Shift semua ke kiri (hapus 2 terlama)
    for (int i = 0; i < chatHistoryCount - 2; i++) {
      chatHistory[i] = chatHistory[i + 2];
    }
    chatHistoryCount -= 2;
  }
  
  // Tambah pesan baru
  chatHistory[chatHistoryCount].role = role;
  chatHistory[chatHistoryCount].content = content;
  chatHistoryCount++;
  
  // Auto-save ke SPIFFS
  saveMemory();
}

// Hapus semua memory
void clearMemory() {
  chatHistoryCount = 0;
  
  // Hapus file
  if (SPIFFS.exists(MEMORY_FILE)) {
    SPIFFS.remove(MEMORY_FILE);
  }
  
  Serial.println("Memory cleared!");
}

// Dapatkan jumlah pesan tersimpan
int getMemoryCount() {
  return chatHistoryCount;
}

// Build messages array untuk API (termasuk system prompt + history)
String buildChatPayload(String systemPrompt, String userMessage) {
  DynamicJsonDocument doc(16384);
  doc["model"] = "gpt-4o-mini";
  doc["max_tokens"] = 150;
  
  JsonArray messages = doc.createNestedArray("messages");
  
  // System prompt
  JsonObject sysMsg = messages.createNestedObject();
  sysMsg["role"] = "system";
  sysMsg["content"] = systemPrompt;
  
  // History (semua pesan sebelumnya)
  for (int i = 0; i < chatHistoryCount; i++) {
    JsonObject histMsg = messages.createNestedObject();
    histMsg["role"] = chatHistory[i].role;
    histMsg["content"] = chatHistory[i].content;
  }
  
  // Pesan user saat ini
  JsonObject userMsg = messages.createNestedObject();
  userMsg["role"] = "user";
  userMsg["content"] = userMessage;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr;
}

#endif // DOLA_MEMORY_H
