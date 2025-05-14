#include <WiFi.h>

// RSU Connection Details
const char* RSU_IP = "192.168.4.1";
const uint16_t RSU_PORT = 8080;

// Wi-Fi settings
const char* SSID = "vanet";
const char* PASSWORD = "12345678";

WiFiClient client;
String megaData = "";

void setup() {
  Serial.begin(115200);      // Serial0 for debugging
  Serial2.begin(115200, SERIAL_8N1, 16, 17);  // Serial2 to Mega
  delay(100);

  connectToWiFi();
  Serial.println("ESP32: Started (Serial0)");
  Serial2.println("ESP32: Started (Serial2 to Mega)");
}

void loop() {
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

        if (megaData.startsWith("DIST:")) {
          // Optional: handle distance
        } else if (megaData.startsWith("WARN:")) {
          sendMessageToRSU("Warning: " + megaData.substring(5));
        } else if (megaData.startsWith("INFO:")) {
          sendMessageToRSU("Info: " + megaData.substring(5));
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

    if (receivedMessage.startsWith("Important:") || receivedMessage.startsWith("Warning:")) {
      Serial2.println("STOP");
      sendMessageToRSU("Vehicle stopped due to RSU warning");
    } else if (receivedMessage.startsWith("Move")) {
      Serial2.println("MOVE");
      sendMessageToRSU("Vehicle instructed to move by RSU");
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
