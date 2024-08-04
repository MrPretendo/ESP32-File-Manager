//ESP32 File manager By Mr. Pretendo https://github.com/MrPretendo
// Version ENG 1.0

#include <WiFiManager.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

// Web server instance
WebServer server(80);

void handleRoot() {
  String html = "<html><head>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #F0F8FF; color: #333; line-height: 1.6; }";
  html += ".container { width: 100%; max-width: 1200px; padding: 20px; margin: 0 auto; }";
  html += "h1, h2 { color: #6A5ACD; margin-bottom: 20px; text-align: center; }";
  html += "form { display: flex; flex-direction: column; gap: 15px; max-width: 500px; margin: 0 auto; }";
  html += "input[type='file'], input[type='submit'] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }";
  html += "input[type='submit'] { background-color: #6A5ACD; color: white; cursor: pointer; transition: background-color 0.3s; }";
  html += "input[type='submit']:hover { background-color: #5849b6; }";
  html += "input[type='submit']:disabled { background-color: #cccccc; cursor: not-allowed; }";
  html += "ul { list-style-type: none; padding: 0; }";
  html += "li { background-color: white; margin-bottom: 10px; padding: 10px; border-radius: 4px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += "a { color: #6A5ACD; text-decoration: none; margin-left: 10px; }";
  html += ".modal { display: none; position: fixed; z-index: 1; left: 0; top: 0; width: 100%; height: 100%; overflow: auto; background-color: rgba(0,0,0,0.4); }";
  html += ".modal-content { background-color: #fefefe; margin: 15% auto; padding: 20px; border: 1px solid #888; width: 80%; max-width: 500px; border-radius: 5px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }";
  html += ".close { color: #aaa; float: right; font-size: 28px; font-weight: bold; cursor: pointer; }";
  html += ".close:hover, .close:focus { color: #000; text-decoration: none; cursor: pointer; }";
  html += "#confirmUpload { background-color: #6A5ACD; color: white; border: none; padding: 10px 20px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; transition-duration: 0.4s; cursor: pointer; border-radius: 5px; }";
  html += "#confirmUpload:hover { background-color: #5849b6; }";
  html += "</style>";
  html += "</head><body><div class='container'>";
  
  html += "<h1>ESP32 File Manager</h1>";
  
  // File upload form
  html += "<h2>Upload File</h2>";
  html += "<form id='uploadForm'>";
  html += "<input type='file' id='fileInput' name='file'>";
  html += "<input type='submit' id='uploadButton' value='Upload File' disabled>";
  html += "</form>";

  // PopUp Confirmation 
  html += "<div id='uploadModal' class='modal'>";
  html += "<div class='modal-content'>";
  html += "<span class='close'>&times;</span>";
  html += "<p>Confirm file upload</p>";
  html += "<button id='confirmUpload'>Confirm</button>";
  html += "</div>";
  html += "</div>";

  // List of files in SPIFFS
  html += "<h2>Files in SPIFFS</h2><ul>";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file) {
    if(!file.isDirectory()) {
      String fileName = String(file.name());
      if (fileName.startsWith("/")) {
        fileName = fileName.substring(1); // Remove leading slash if it exists
      }
      html += "<li>" + fileName + " - " + String(file.size()) + " bytes";
      html += " <a href='/download?file=" + fileName + "'>Download</a>";
      html += " <a href='/delete?file=" + fileName + "'>Delete</a></li>";
    }
    file = root.openNextFile();
  }
  root.close();
  html += "</ul>";
  
  // JavaScript to handle file upload
  html += "<script>";
  html += "var fileInput = document.getElementById('fileInput');";
  html += "var uploadButton = document.getElementById('uploadButton');";
  html += "var uploadForm = document.getElementById('uploadForm');";
  html += "var modal = document.getElementById('uploadModal');";
  html += "var span = document.getElementsByClassName('close')[0];";
  html += "var confirmButton = document.getElementById('confirmUpload');";
  
  html += "fileInput.onchange = function() {";
  html += "  uploadButton.disabled = !fileInput.value;";
  html += "};";
  
  html += "uploadForm.onsubmit = function(e) {";
  html += "  e.preventDefault();";
  html += "  if (fileInput.value) {";
  html += "    modal.style.display = 'block';";
  html += "  }";
  html += "};";
  
  html += "span.onclick = function() {";
  html += "  modal.style.display = 'none';";
  html += "};";
  
  html += "confirmButton.onclick = function() {";
  html += "  modal.style.display = 'none';";
  html += "  var formData = new FormData(uploadForm);";
  html += "  fetch('/upload', {method: 'POST', body: formData})";
  html += "    .then(response => {";
  html += "      if(response.ok) window.location.reload();";
  html += "      else alert('Error uploading file');";
  html += "    })";
  html += "    .catch(error => alert('Error: ' + error));";
  html += "};";
  
  html += "window.onclick = function(event) {";
  html += "  if (event.target == modal) {";
  html += "    modal.style.display = 'none';";
  html += "  }";
  html += "};";
  html += "</script>";
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  static File fsUploadFile;
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    Serial.printf("handleFileUpload Name: %s\n", filename.c_str());
    fsUploadFile = SPIFFS.open(filename, FILE_WRITE);
    if (!fsUploadFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.printf("handleFileUpload Size: %u\n", upload.totalSize);
  }
}

void handleFileUploadComplete() {
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleFileDownload() {
  String fileName = server.arg("file");
  String path = "/" + fileName;
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    if (file) {
      server.sendHeader("Content-Type", "application/octet-stream");
      server.sendHeader("Content-Disposition", "attachment; filename=" + fileName);
      server.sendHeader("Connection", "close");
      server.streamFile(file, "application/octet-stream");
      file.close();
    } else {
      server.send(500, "text/plain", "Error opening file");
    }
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void handleDeleteFile() {
  String fileName = server.arg("file");
  String path = "/" + fileName;
  if (SPIFFS.remove(path)) {
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(500, "text/plain", "Error deleting file");
  }
}

// Functions for serial monitor
void serialDeploy() {
  Serial.println("List of files in SPIFFS:");
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Error opening root directory");
    return;
  }
  File file = root.openNextFile();
  int fileCount = 0;
  while (file) {
    if (!file.isDirectory()) {
      fileCount++;
      Serial.printf("%d. %s - %d bytes\n", fileCount, file.name(), file.size());
    }
    file = root.openNextFile();
  }
  if (fileCount == 0) {
    Serial.println("No files in SPIFFS.");
  }
  root.close();
}

void serialDeleteFile(String fileName) {
  // Ensure the file name starts with "/"
  if (!fileName.startsWith("/")) {
    fileName = "/" + fileName;
  }
  
  // Check if the file exists
  if (SPIFFS.exists(fileName)) {
    // Try to delete the file directly
    if (SPIFFS.remove(fileName)) {
      Serial.printf("File %s successfully deleted.\n", fileName.c_str());
      return;
    }
  }
  
  // If direct deletion failed, program will try the alternative method
  // This method needs to be update for the next release
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool found = false;
  
  while (file && !found) {
    String currentFileName = String(file.name());
    if (currentFileName.equals(fileName)) {
      found = true;
      file.close();
      if (SPIFFS.remove(fileName)) {
        Serial.printf("File %s successfully deleted.\n", fileName.c_str());
      } else {
        Serial.printf("Error deleting file %s.\n", fileName.c_str());
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  
  if (!found) {
    Serial.printf("File %s does not exist.\n", fileName.c_str());
  }
}

void serialShowMemory() {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;

  // Calculate 75% of total space as "safe" space
  size_t safeBytes = totalBytes * 0.75;
  size_t safeUsedBytes = (usedBytes > safeBytes) ? safeBytes : usedBytes;
  float safeUsagePercentage = (safeUsedBytes * 100.0) / safeBytes;

  Serial.println("SPIFFS Memory:");
  Serial.printf("Total: %u bytes\n", totalBytes);
  Serial.printf("Used: %u bytes\n", usedBytes);
  Serial.printf("Free: %u bytes\n", freeBytes);
  
  Serial.println("\nSafe memory usage (75% of total):");
  Serial.printf("Total safe space: %u bytes\n", safeBytes);
  Serial.printf("Used safe space: %u bytes\n", safeUsedBytes);
  Serial.printf("Safe usage percentage: %.2f%%\n", safeUsagePercentage);

  // Warning if usage exceeds 75% of safe space
  if (safeUsagePercentage > 75.0) {
    Serial.println("\nWARNING!");
    Serial.println("Storage usage exceeds 75% of safe space.");
    Serial.println("Storage may not function reliably.");
    Serial.println("It is recommended to free up space.");
  }

  Serial.println("\nNOTE: According to official Espressif documentation,");
  Serial.println("SPIFFS can only reliably use about 75% of the partition space.");
}

void setup() {
  WiFi.mode(WIFI_AP_STA); // AP + STA mode
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  // Configure WiFiManager
  WiFiManager wifiManager;
  // SSID - PASSWORD... yeah, the password is password lol
  if (!wifiManager.autoConnect("ESP32-Config", "password")) {
    Serial.println("Failed to connect and hit timeout");
  } else {
    Serial.println("Connected to WiFi");
  }

  // Keep the access point active
  WiFi.softAP("ESP32-AP", "password");

  // Configure web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, handleFileUploadComplete, handleFileUpload);
  server.on("/download", HTTP_GET, handleFileDownload);
  server.on("/delete", HTTP_GET, handleDeleteFile);
  server.begin();

  Serial.println("HTTP server started");
  Serial.print("Access point IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Station IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Available commands:");
  Serial.println("- deploy: Show list of files in SPIFFS");
  Serial.println("- delete [filename]: Delete specified file");
  Serial.println("- memory: Show available memory in SPIFFS");
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    String lowerCommand = command;
    lowerCommand.toLowerCase();

    if (lowerCommand == "deploy") {
      serialDeploy();
    } else if (lowerCommand.startsWith("delete ")) {
      String fileName = command.substring(7); // Extract the filename without converting to lowercase
      serialDeleteFile(fileName);
    } else if (lowerCommand == "memory") {
      serialShowMemory();
    }
  }
}