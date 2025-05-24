#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// RSU Connection Details
const char* RSU_IP = "192.168.4.1";
const uint16_t RSU_PORT = 8080;

// Wi-Fi settings
const char* SSID = "vanet";
const char* PASSWORD = "12345678";

WiFiClient client;
SoftwareSerial megaSerial(D6, D5); // RX, TX (D6 = GPIO12, D5 = GPIO14)
String megaData = "";

void setup() {
  Serial.begin(115200);         // For debugging
  megaSerial.begin(9600);       // Communication with Arduino Mega

  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  } else if (!client.connected()) {
    connectToRSU();
  }

  // Read from Mega
  while (megaSerial.available()) {
    char c = megaSerial.read();
    if (c == '\n') {
      megaData.trim();
      if (megaData.length() > 0) {
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
      Serial.println("ESP8266: Warning received from RSU - Instructing Arduino to take first exit");
      megaSerial.println("TAKE_FIRST_EXIT");
      sendMessageToRSU("Vehicle looking for first exit");
    } else if (receivedMessage.startsWith("Move")) {
      megaSerial.println("MOVE");
      sendMessageToRSU("Vehicle resuming normal operation");
    }
  }

  delay(100);
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
