//Gestor de archivos ESP32 por Mr. Pretendo https://github.com/MrPretendo
// Version Español 1.0

#include <WiFiManager.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

// Instancia del servidor web
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
  
  html += "<h1>Gestor de archivos ESP32 </h1>";
  
  // Formulario para subir archivos
  html += "<h2>Subir Archivo</h2>";
  html += "<form id='uploadForm'>";
  html += "<input type='file' id='fileInput' name='file'>";
  html += "<input type='submit' id='uploadButton' value='Subir Archivo' disabled>";
  html += "</form>";

  // PopUp para confirmación
  html += "<div id='uploadModal' class='modal'>";
  html += "<div class='modal-content'>";
  html += "<span class='close'>&times;</span>";
  html += "<p>Confirma la carga del archivo</p>";
  html += "<button id='confirmUpload'>Confirmar</button>";
  html += "</div>";
  html += "</div>";

  // Lista de archivos en SPIFFS
  html += "<h2>Archivos en SPIFFS</h2><ul>";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file) {
    if(!file.isDirectory()) {
      String fileName = String(file.name());
      if (fileName.startsWith("/")) {
        fileName = fileName.substring(1); // Elimina la barra inicial si existe
      }
      html += "<li>" + fileName + " - " + String(file.size()) + " bytes";
      html += " <a href='/download?file=" + fileName + "'>Descargar</a>";
      html += " <a href='/delete?file=" + fileName + "'>Eliminar</a></li>";
    }
    file = root.openNextFile();
  }
  root.close();
  html += "</ul>";
  
  // JavaScript para manejar la subida de archivos
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
  html += "      else alert('Error al subir el archivo');";
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
      server.send(500, "text/plain", "Error al abrir el archivo");
    }
  } else {
    server.send(404, "text/plain", "Archivo no encontrado");
  }
}

void handleDeleteFile() {
  String fileName = server.arg("file");
  String path = "/" + fileName;
  if (SPIFFS.remove(path)) {
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(500, "text/plain", "Error al eliminar el archivo");
  }
}

// Funciones para el monitor serie
void serialDeploy() {
  Serial.println("Lista de archivos en SPIFFS:");
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Error al abrir el directorio raíz");
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
    Serial.println("No hay archivos en SPIFFS.");
  }
  root.close();
}

void serialDeleteFile(String fileName) {
  // Asegurarse de que el nombre del archivo comience con "/"
  if (!fileName.startsWith("/")) {
    fileName = "/" + fileName;
  }
  
  // Verificar si el archivo existe
  if (SPIFFS.exists(fileName)) {
    // Intenta eliminar el archivo directamente
    if (SPIFFS.remove(fileName)) {
      Serial.printf("Archivo %s eliminado con éxito.\n", fileName.c_str());
      return;
    }
  }
  
  // Si no se pudo eliminar directamente, se intentará el método alternativo
  // Nota: Problemas con esta parte del código: Delete no servía en monitor serie
  // Solo así funcionó, intentar corregir después.
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool found = false;
  
  while (file && !found) {
    String currentFileName = String(file.name());
    if (currentFileName.equals(fileName)) {
      found = true;
      file.close();
      if (SPIFFS.remove(fileName)) {
        Serial.printf("Archivo %s eliminado con éxito.\n", fileName.c_str());
      } else {
        Serial.printf("Error al eliminar el archivo %s.\n", fileName.c_str());
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  
  if (!found) {
    Serial.printf("El archivo %s no existe.\n", fileName.c_str());
  }
}

void serialShowMemory() {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;

  // Calculamos el 75% del espacio total como el espacio "seguro"
  size_t safeBytes = totalBytes * 0.75;
  size_t safeUsedBytes = (usedBytes > safeBytes) ? safeBytes : usedBytes;
  float safeUsagePercentage = (safeUsedBytes * 100.0) / safeBytes;

  Serial.println("Memoria SPIFFS:");
  Serial.printf("Total: %u bytes\n", totalBytes);
  Serial.printf("Usado: %u bytes\n", usedBytes);
  Serial.printf("Libre: %u bytes\n", freeBytes);
  
  Serial.println("\nUso seguro de memoria (75% del total):");
  Serial.printf("Espacio seguro total: %u bytes\n", safeBytes);
  Serial.printf("Usado del espacio seguro: %u bytes\n", safeUsedBytes);
  Serial.printf("Porcentaje de uso seguro: %.2f%%\n", safeUsagePercentage);

  // Advertencia si el uso supera el 75% del espacio seguro
  if (safeUsagePercentage > 75.0) {
    Serial.println("\n¡ADVERTENCIA!");
    Serial.println("El uso de almacenamiento supera el 75% del espacio seguro.");
    Serial.println("El almacenamiento puede no funcionar con fiabilidad.");
    Serial.println("Se recomienda liberar espacio.");
  }

  Serial.println("\nNOTA: Según la documentación oficial de Espressif,");
  Serial.println("SPIFFS solo puede utilizar de forma fiable alrededor");
  Serial.println("del 75% del espacio de partición.");
}

void setup() {
  WiFi.mode(WIFI_AP_STA); // Modo AP + STA
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Configurar WiFiManager
  WiFiManager wifiManager;
  //el SSID es ESP32-Config y la contraseña es "password"... sí, la contraseña es contraseña lol
  if (!wifiManager.autoConnect("ESP32-Config", "password")) {
    Serial.println("Falló la conexión y expiró el tiempo de espera");
  } else {
    Serial.println("Conectado al WiFi");
  }

  // Mantener el punto de acceso activo
  WiFi.softAP("ESP32-AP", "password");

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, handleFileUploadComplete, handleFileUpload);
  server.on("/download", HTTP_GET, handleFileDownload);
  server.on("/delete", HTTP_GET, handleDeleteFile);
  server.begin();

  Serial.println("Servidor HTTP iniciado");
  Serial.print("IP del punto de acceso: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("IP de la estación: ");
  Serial.println(WiFi.localIP());
  Serial.println("Comandos disponibles:");
  Serial.println("- deploy: Muestra la lista de archivos en SPIFFS");
  Serial.println("- delete [nombrearchivo]: Elimina el archivo especificado");
  Serial.println("- memory: Muestra la memoria disponible en SPIFFS");
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
      String fileName = command.substring(7); // Extrae el nombre del archivo sin convertir a minúsculas
      serialDeleteFile(fileName);
    } else if (lowerCommand == "memory") {
      serialShowMemory();
    }
  }
}