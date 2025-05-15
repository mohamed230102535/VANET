#include <WiFi.h>
#include <WebServer.h>

// RSU Connection Details
const char* RSU_IP = "192.168.4.1";
const uint16_t RSU_PORT = 8080;

// Wi-Fi settings
const char* SSID = "vanet";
const char* PASSWORD = "12345678";

WiFiClient client;
String megaData = "";

// Web server object on port 80
WebServer server(80);
String lastRsuMessage = "No messages received yet.";

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 RSU Messages</title>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f4; color: #333; }";
  html += ".container { background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { color: #0056b3; }";
  html += "p { font-size: 1.2em; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>RSU Message</h1>";
  html += "<p>" + lastRsuMessage + "</p>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);      // Serial0 for debugging
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Serial2 to Mega
  delay(100);

  connectToWiFi();
  Serial.println("ESP32: Started (Serial0)");
  Serial2.println("ESP32: Started (Serial2 to Mega)");

  server.on("/", handleRoot);
  server.begin();
  Serial.println("ESP32: HTTP server started. Access at http://" + WiFi.localIP().toString() + "/");
}

void loop() {
  server.handleClient();

  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  } else if (!client.connected()) {
    connectToRSU();
  }

  // Read from Mega
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      megaData.trim();
      if (megaData.length() > 0) {
        Serial.println("ESP32: From Mega: " + megaData);
        
        // If Arduino reports it took an exit
        if (megaData.startsWith("TOOK_EXIT:")) {
          sendMessageToRSU("Vehicle took " + megaData.substring(10) + " exit");
        }
      }
      megaData = "";
    } else {
      megaData += c;
    }
  }

  // Listen for RSU messages
  if (client.available()) {
    String receivedMessage = client.readStringUntil('\n');
    receivedMessage.trim();
    Serial.println("ESP32: From RSU: " + receivedMessage);
    lastRsuMessage = receivedMessage;

    if (receivedMessage.startsWith("Warning:")) {
      // Tell Arduino to take the first exit it sees
      Serial2.println("TAKE_FIRST_EXIT");
      Serial.println("ESP32: Instructed Arduino to take first exit");
      sendMessageToRSU("Vehicle looking for first exit");
    } else if (receivedMessage.startsWith("Move")) {
      Serial2.println("MOVE");
      sendMessageToRSU("Vehicle resuming normal operation");
    }
  }

  delay(100);  // Responsive but avoids flooding
}

void connectToWiFi() {
  WiFi.begin(SSID, PASSWORD);
  Serial.println("ESP32: Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nESP32: Connected, IP: " + WiFi.localIP().toString());
}

void connectToRSU() {
  if (client.connect(RSU_IP, RSU_PORT)) {
    Serial.println("ESP32: Connected to RSU");
  } else {
    Serial.println("ESP32: RSU connection failed");
    delay(2000);
  }
}

void sendMessageToRSU(String message) {
  if (client.connected()) {
    client.println(message);
    Serial.println("ESP32: Sent to RSU: " + message);
  } else {
    Serial.println("ESP32: Cannot send, RSU not connected");
  }
}
