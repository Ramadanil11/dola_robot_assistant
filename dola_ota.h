/*
 * ============================================
 *   DOLA - OTA Update & Web Dashboard
 * ============================================
 * Custom OTA page (tanpa library tambahan)
 * Fitur: Update Firmware, Update SPIFFS, WiFi info, Memory management
 * Akses via browser: http://[IP Dola]/
 */

#ifndef DOLA_OTA_H
#define DOLA_OTA_H

#include <WebServer.h>
#include <Update.h>
#include <SPIFFS.h>

// External references
extern WebServer server;
extern String savedSSID;
extern int getMemoryCount();
extern void clearMemory();

// OTA state
bool otaInProgress = false;
int otaProgress = 0;

// ============ WEB DASHBOARD HTML ============
String getDashboardHTML() {
  unsigned long upSec = millis() / 1000;
  int upH = upSec / 3600;
  int upM = (upSec % 3600) / 60;
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>DOLA Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh; padding: 20px;
    }
    .container {
      max-width: 500px; margin: 0 auto;
    }
    .header {
      text-align: center; color: white; margin-bottom: 20px;
    }
    .header .face { font-size: 3em; }
    .header h1 { font-size: 2em; margin: 5px 0; }
    .header p { opacity: 0.8; font-size: 0.9em; }
    .card {
      background: white; border-radius: 15px; padding: 25px;
      margin-bottom: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2);
    }
    .card h2 { 
      font-size: 1.1em; color: #667eea; margin-bottom: 15px;
      padding-bottom: 8px; border-bottom: 2px solid #f0f0f0;
    }
    .info-row { 
      display: flex; justify-content: space-between; 
      padding: 5px 0; font-size: 0.9em;
    }
    .info-row .label { color: #888; }
    .info-row .value { font-weight: bold; color: #333; }
    .status-dot {
      display: inline-block; width: 8px; height: 8px; 
      border-radius: 50%; margin-right: 5px;
    }
    .status-online { background: #4CAF50; }
    .status-offline { background: #f44336; }
    input[type="file"] {
      width: 100%; padding: 10px; margin: 10px 0;
      border: 2px dashed #ddd; border-radius: 8px;
      font-size: 0.9em; cursor: pointer;
    }
    input[type="file"]:hover { border-color: #667eea; }
    button, .btn {
      width: 100%; padding: 12px; border: none; border-radius: 8px;
      font-size: 1em; cursor: pointer; margin-top: 10px;
      transition: transform 0.2s, opacity 0.2s;
      display: block; text-align: center; text-decoration: none;
    }
    button:hover, .btn:hover { transform: scale(1.02); }
    button:active, .btn:active { transform: scale(0.98); }
    .btn-primary { background: linear-gradient(135deg, #667eea, #764ba2); color: white; }
    .btn-danger { background: linear-gradient(135deg, #f093fb, #f5576c); color: white; }
    .btn-secondary { background: #eee; color: #666; }
    .progress-container { 
      display: none; margin-top: 10px; 
    }
    .progress-bar {
      width: 100%; height: 20px; background: #eee; border-radius: 10px;
      overflow: hidden;
    }
    .progress-fill {
      height: 100%; background: linear-gradient(90deg, #667eea, #764ba2);
      width: 0%; transition: width 0.3s;
      border-radius: 10px;
    }
    .progress-text { text-align: center; margin-top: 5px; font-size: 0.85em; color: #888; }
    .warning { 
      background: #fff3cd; border: 1px solid #ffc107; 
      border-radius: 8px; padding: 10px; margin-top: 10px;
      font-size: 0.85em; color: #856404;
    }
    .success {
      background: #d4edda; border: 1px solid #28a745;
      border-radius: 8px; padding: 10px; margin-top: 10px;
      font-size: 0.85em; color: #155724; display: none;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="face">( ^_^ )</div>
      <h1>DOLA</h1>
      <p>Robot Asisten AI Dashboard</p>
    </div>

    <!-- Status Card -->
    <div class="card">
      <h2>Status</h2>
      <div class="info-row">
        <span class="label">WiFi</span>
        <span class="value"><span class="status-dot status-online"></span>)rawliteral" + savedSSID + R"rawliteral(</span>
      </div>
      <div class="info-row">
        <span class="label">IP Address</span>
        <span class="value">)rawliteral" + WiFi.localIP().toString() + R"rawliteral(</span>
      </div>
      <div class="info-row">
        <span class="label">SPIFFS</span>
        <span class="value">)rawliteral" + String(SPIFFS.usedBytes()/1024) + "KB / " + String(SPIFFS.totalBytes()/1024) + R"rawliteral(KB</span>
      </div>
      <div class="info-row">
        <span class="label">Uptime</span>
        <span class="value">)rawliteral" + String(upH) + "h " + String(upM) + R"rawliteral(m</span>
      </div>
      <div class="info-row">
        <span class="label">Free Heap</span>
        <span class="value">)rawliteral" + String(ESP.getFreeHeap()/1024) + R"rawliteral(KB</span>
      </div>
    </div>

    <!-- OTA Firmware Card -->
    <div class="card">
      <h2>Update Firmware</h2>
      <p style="font-size:0.85em;color:#888;margin-bottom:10px;">
        Upload file .bin untuk update program Dola
      </p>
      <form id="firmwareForm" method="POST" action="/update-firmware" enctype="multipart/form-data">
        <input type="file" name="firmware" accept=".bin" id="firmwareFile">
        <button type="submit" class="btn-primary" onclick="startUpload('firmware')">Upload Firmware</button>
      </form>
      <div class="progress-container" id="firmwareProgress">
        <div class="progress-bar"><div class="progress-fill" id="firmwareFill"></div></div>
        <div class="progress-text" id="firmwareText">Uploading... 0%</div>
      </div>
      <div class="warning">
        Dola akan restart otomatis setelah update selesai.
      </div>
    </div>

    <!-- OTA SPIFFS Card -->
    <div class="card">
      <h2>Update File Suara (SPIFFS)</h2>
      <p style="font-size:0.85em;color:#888;margin-bottom:10px;">
        Upload file spiffs.bin untuk update semua file suara
      </p>
      <form id="spiffsForm" method="POST" action="/update-spiffs" enctype="multipart/form-data">
        <input type="file" name="spiffs" accept=".bin" id="spiffsFile">
        <button type="submit" class="btn-primary" onclick="startUpload('spiffs')">Upload SPIFFS</button>
      </form>
      <div class="progress-container" id="spiffsProgress">
        <div class="progress-bar"><div class="progress-fill" id="spiffsFill"></div></div>
        <div class="progress-text" id="spiffsText">Uploading... 0%</div>
      </div>
    </div>

    <!-- Memory Card -->
    <div class="card">
      <h2>Conversation Memory</h2>
      <div class="info-row">
        <span class="label">Pesan tersimpan</span>
        <span class="value">)rawliteral" + String(getMemoryCount()) + " / " + String(MAX_MEMORY_MESSAGES) + R"rawliteral(</span>
      </div>
      <p style="font-size:0.85em;color:#888;margin:10px 0;">
        Dola mengingat percakapan terakhir. Hapus memory jika ingin Dola mulai fresh.
      </p>
      <button class="btn-danger" onclick="if(confirm('Hapus semua memory Dola?')){fetch('/clear-memory',{method:'POST'}).then(()=>location.reload())}">
        Hapus Memory
      </button>
      <div class="success" id="memorySuccess">Memory berhasil dihapus!</div>
    </div>

    <!-- WiFi Reset Card -->
    <div class="card">
      <h2>WiFi Settings</h2>
      <div class="info-row">
        <span class="label">SSID saat ini</span>
        <span class="value">)rawliteral" + savedSSID + R"rawliteral(</span>
      </div>
      <button class="btn-secondary" onclick="if(confirm('Reset WiFi? Dola akan restart ke mode AP Setup.')){fetch('/reset-wifi',{method:'POST'}).then(()=>alert('Dola akan restart...'))}">
        Reset WiFi
      </button>
    </div>

    <!-- Footer -->
    <div style="text-align:center;color:rgba(255,255,255,0.6);margin-top:20px;font-size:0.8em;">
      DOLA v1.0 | Made with love~
    </div>
  </div>

  <script>
    function startUpload(type) {
      document.getElementById(type + 'Progress').style.display = 'block';
    }
    
    // XHR upload with progress
    document.getElementById('firmwareForm').addEventListener('submit', function(e) {
      e.preventDefault();
      uploadFile('firmware', '/update-firmware', document.getElementById('firmwareFile').files[0]);
    });
    
    document.getElementById('spiffsForm').addEventListener('submit', function(e) {
      e.preventDefault();
      uploadFile('spiffs', '/update-spiffs', document.getElementById('spiffsFile').files[0]);
    });
    
    function uploadFile(type, url, file) {
      if (!file) { alert('Pilih file dulu!'); return; }
      
      var formData = new FormData();
      formData.append(type, file);
      
      var xhr = new XMLHttpRequest();
      
      document.getElementById(type + 'Progress').style.display = 'block';
      
      xhr.upload.addEventListener('progress', function(e) {
        if (e.lengthComputable) {
          var percent = Math.round((e.loaded / e.total) * 100);
          document.getElementById(type + 'Fill').style.width = percent + '%';
          document.getElementById(type + 'Text').innerText = 'Uploading... ' + percent + '%';
        }
      });
      
      xhr.addEventListener('load', function() {
        document.getElementById(type + 'Fill').style.width = '100%';
        document.getElementById(type + 'Text').innerText = 'Selesai! Dola restart...';
        setTimeout(function() { location.reload(); }, 5000);
      });
      
      xhr.addEventListener('error', function() {
        document.getElementById(type + 'Text').innerText = 'Error! Coba lagi.';
      });
      
      xhr.open('POST', url);
      xhr.send(formData);
    }
  </script>
</body>
</html>
)rawliteral";
  
  return html;
}

// ============ OTA HANDLERS ============

void handleDashboard() {
  server.send(200, "text/html", getDashboardHTML());
}

void handleFirmwareUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("OTA Firmware Start: %s\n", upload.filename.c_str());
    otaInProgress = true;
    
    // Begin OTA update
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write chunk
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
    otaProgress = (upload.totalSize > 0) ? (upload.currentSize * 100 / upload.totalSize) : 0;
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA Firmware Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    otaInProgress = false;
  }
}

void handleFirmwareUploadDone() {
  if (Update.hasError()) {
    server.send(500, "text/plain", "Update GAGAL! Error: " + String(Update.errorString()));
  } else {
    server.send(200, "text/plain", "Update BERHASIL! Dola restart...");
    delay(1000);
    ESP.restart();
  }
}

void handleSPIFFSUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("OTA SPIFFS Start: %s\n", upload.filename.c_str());
    otaInProgress = true;
    
    // Begin SPIFFS update
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA SPIFFS Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
    otaInProgress = false;
  }
}

void handleSPIFFSUploadDone() {
  if (Update.hasError()) {
    server.send(500, "text/plain", "SPIFFS Update GAGAL!");
  } else {
    server.send(200, "text/plain", "SPIFFS Update BERHASIL! Dola restart...");
    delay(1000);
    ESP.restart();
  }
}

void handleClearMemory() {
  clearMemory();
  server.send(200, "text/plain", "Memory cleared!");
}

void handleResetWiFi() {
  server.send(200, "text/plain", "WiFi reset! Restarting...");
  delay(1000);
  
  // Clear WiFi credentials
  Preferences prefs;
  prefs.begin("dola", false);
  prefs.putString("ssid", "");
  prefs.putString("pass", "");
  prefs.end();
  
  ESP.restart();
}

// ============ SETUP OTA WEB SERVER ============
void setupOTAServer() {
  // Dashboard
  server.on("/", HTTP_GET, handleDashboard);
  
  // Firmware OTA
  server.on("/update-firmware", HTTP_POST, handleFirmwareUploadDone, handleFirmwareUpload);
  
  // SPIFFS OTA
  server.on("/update-spiffs", HTTP_POST, handleSPIFFSUploadDone, handleSPIFFSUpload);
  
  // Memory management
  server.on("/clear-memory", HTTP_POST, handleClearMemory);
  
  // WiFi reset
  server.on("/reset-wifi", HTTP_POST, handleResetWiFi);
  
  server.begin();
  Serial.println("OTA Dashboard ready!");
  Serial.printf("Buka browser: http://%s/\n", WiFi.localIP().toString().c_str());
}

#endif // DOLA_OTA_H
