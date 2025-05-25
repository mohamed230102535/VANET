#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// RSU Connection Details
const char* RSU_IP = "192.168.4.1";
const uint16_t RSU_PORT = 8080;

// Wi-Fi settings
const char* SSID = "vanet";
const char* PASSWORD = "12345678";

// Define SoftwareSerial pins (RX, TX)
// D6 (GPIO12) as RX, D7 (GPIO13) as TX for NodeMCU
SoftwareSerial megaSerial(12, 13); // RX, TX

WiFiClient client;
String megaData = "";

void setup() {
  Serial.begin(115200);      // Hardware serial for debugging
  megaSerial.begin(115200);  // Software serial to Mega
  delay(100);

  connectToWiFi();
  Serial.println("ESP8266: Started (Serial)");
  megaSerial.println("ESP8266: Started (To Mega)");
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
        Serial.println("ESP8266: From Mega: " + megaData);

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
    Serial.println("ESP8266: From RSU: " + receivedMessage);

    if (receivedMessage.startsWith("Important:") || receivedMessage.startsWith("Warning:")) {
      megaSerial.println("STOP");
      sendMessageToRSU("Vehicle stopped due to RSU warning");
    } else if (receivedMessage.startsWith("Move")) {
      megaSerial.println("MOVE");
      sendMessageToRSU("Vehicle instructed to move by RSU");
    }
  }

  delay(100);  // Responsive but avoids flooding
}

void connectToWiFi() {
  WiFi.begin(SSID, PASSWORD);
  Serial.println("ESP8266: Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nESP8266: Connected, IP: " + WiFi.localIP().toString());
}

void connectToRSU() {
  if (client.connect(RSU_IP, RSU_PORT)) {
    Serial.println("ESP8266: Connected to RSU");
  } else {
    Serial.println("ESP8266: RSU connection failed");
    delay(2000);
  }
}

void sendMessageToRSU(String message) {
  if (client.connected()) {
    client.println(message);
    Serial.println("ESP8266: Sent to RSU: " + message);
  } else {
    Serial.println("ESP8266: Cannot send, RSU not connected");
  }
}