#include <WiFi.h>
#include <vector>
#include <algorithm>
#include <list>

// RSU Wi-Fi (Access Point)
const char* ssid = "vanet";
const char* password = "12345678";

// RSU Communication Settings
const uint16_t RSU_PORT = 8080;
const uint16_t WEB_PORT = 80;
WiFiServer vehicleServer(RSU_PORT);
WiFiServer webServer(WEB_PORT);

// Data structures
struct Vehicle {
  WiFiClient client;
  String ip;
  unsigned long lastSeen;
};

std::list<Vehicle> connectedVehicles;
std::vector<String> importantMessages;
const unsigned long VEHICLE_TIMEOUT = 30000;

std::vector<String> recentMessages;  // Keep for web
const int MAX_RECENT = 10;           // Only store last 10


void setup() {
  Serial.begin(115200);
  delay(100);

  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Failed to start AP!");
    while (true) delay(1000);
  }

  IPAddress ip = WiFi.softAPIP();
  Serial.println("ESP32 Access Point Started!");
  Serial.print("AP IP Address: ");
  Serial.println(ip);

  vehicleServer.begin();
  webServer.begin();
  Serial.println("Vehicle and web servers started.");
}

void loop() {
  handleNewVehicleConnections();
  handleExistingVehicleMessages();  // ✅ NEW: Listen to all connected vehicles
  broadcastImportantMessages();
  handleWebRequests();
  cleanupDisconnectedVehicles();
  delay(10);
}

// ✅ Accept new connections
void handleNewVehicleConnections() {
  WiFiClient client = vehicleServer.available();
  if (!client) return;

  String vehicleIP = client.remoteIP().toString();
  Serial.println("Vehicle trying to connect: " + vehicleIP);

  for (auto it = connectedVehicles.begin(); it != connectedVehicles.end(); ++it) {
    if (it->ip == vehicleIP) {
      if (!it->client.connected()) {
        it->client.stop();
        connectedVehicles.erase(it);
        break;
      }
      return;
    }
  }

  Vehicle newVehicle{client, vehicleIP, millis()};
  connectedVehicles.push_back(newVehicle);
  Serial.println("New vehicle registered: " + vehicleIP);
}

// ✅ Handle data from all connected vehicles
void handleExistingVehicleMessages() {
  for (auto& vehicle : connectedVehicles) {
    if (!vehicle.client.connected()) continue;

    while (vehicle.client.available()) {
      String receivedData = vehicle.client.readStringUntil('\n');
      receivedData.trim();
      Serial.println("Received from " + vehicle.ip + ": " + receivedData);

      vehicle.lastSeen = millis();  // Update activity time

      if (receivedData.startsWith("Important:") || receivedData.startsWith("Warning:")) {
        importantMessages.push_back(receivedData);
      }
    }
  }
}

// ✅ Broadcast all important messages to all vehicles
void broadcastImportantMessages() {
  if (importantMessages.empty()) return;

  for (const String& message : importantMessages) {
    Serial.println("Broadcasting: " + message);
    
    // Broadcast to all vehicles
    for (auto& vehicle : connectedVehicles) {
      if (vehicle.client.connected()) {
        vehicle.client.println(message);
      }
    }

    // Store in recent messages
    recentMessages.push_back(message);
    if (recentMessages.size() > MAX_RECENT) {
      recentMessages.erase(recentMessages.begin());  // Remove oldest
    }
  }

  importantMessages.clear();  // Still clear this
}


// Web interface stuff (unchanged)
void handleWebRequests() {
  WiFiClient webClient = webServer.available();
  if (!webClient) return;

  Serial.println("Web client connected.");
  unsigned long timeout = millis() + 1000;
  while (!webClient.available() && millis() < timeout) {
    delay(1);
  }

  if (!webClient.available()) {
    webClient.stop();
    return;
  }

  String req = webClient.readStringUntil('\r');
  webClient.read(); // discard newline

  String html = generateWebPage();
  sendHttpResponse(webClient, html);
}

String generateWebPage() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'><title>RSU Dashboard</title>";
  html += "<style>body{font-family:Arial;}</style></head><body>";
  html += "<h2>Connected Vehicles (" + String(connectedVehicles.size()) + ")</h2><ul>";
  for (auto& vehicle : connectedVehicles) {
    if (vehicle.client.connected()) {
      html += "<li>" + vehicle.ip + " (Last seen: " + String((millis() - vehicle.lastSeen) / 1000) + "s ago)</li>";
    }
  }
  html += "</ul>";

  html += "<h2>Latest Warnings & Messages</h2><ul>";
for (const String& msg : recentMessages) {
    html += "<li>" + msg + "</li>";
  }
  html += "</ul></body></html>";
  return html;
}

void sendHttpResponse(WiFiClient& client, const String& html) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.print(html);
  delay(1);
  client.stop();
}

// ✅ Remove timed-out or disconnected vehicles
void cleanupDisconnectedVehicles() {
  unsigned long now = millis();
  connectedVehicles.remove_if([&](Vehicle& v) {
    if (!v.client.connected() || (now - v.lastSeen > VEHICLE_TIMEOUT)) {
      Serial.println("Removing vehicle: " + v.ip);
      v.client.stop();
      return true;
    }
    return false;
  });
}
