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
        // If Arduino reports it took an exit
        if (megaData.startsWith("TOOK_EXIT:")) {
          sendMessageToRSU("Vehicle took " + megaData.substring(10) + " exit");
        } else if (megaData.startsWith("STATUS:")) {
          sendMessageToRSU("Vehicle " + megaData.substring(7));
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

    if (receivedMessage.startsWith("Warning:")) {
      Serial.println("ESP32: Warning received from RSU - Instructing Arduino to take first exit");
      // Tell Arduino to take the first exit it sees
      Serial2.println("TAKE_FIRST_EXIT");
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
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void connectToRSU() {
  if (!client.connect(RSU_IP, RSU_PORT)) {
    delay(2000);
  }
}

void sendMessageToRSU(String message) {
  if (client.connected()) {
    client.println(message);
  }
}
