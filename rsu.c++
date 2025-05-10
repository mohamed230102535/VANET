#include <WiFi.h>
#include <vector>
#include <list>

// RSU Configuration
const char* ssid = "vanet";
const char* password = "12345678";
const uint16_t RSU_PORT = 8080;
const uint16_t WEB_PORT = 80;
const unsigned long VEHICLE_TIMEOUT = 30000;  // 30 seconds

// Server instances
WiFiServer vehicleServer(RSU_PORT);
WiFiServer webServer(WEB_PORT);

// Vehicle data structure
struct Vehicle {
    WiFiClient client;
    String ip;
    unsigned long lastSeen;
    String lastMessage;
};

// Data storage
std::list<Vehicle> connectedVehicles;
std::vector<String> importantMessages;
std::vector<String> recentMessages;
const int MAX_RECENT = 10;

void setup() {
    Serial.begin(115200);
    delay(100);

    // Initialize WiFi AP
    if (!WiFi.softAP(ssid, password)) {
        Serial.println("Failed to start AP!");
        while (true) {
            delay(1000);
            Serial.println("Retrying AP setup...");
        }
    }

    IPAddress ip = WiFi.softAPIP();
    Serial.println("ESP32 Access Point Started!");
    Serial.print("AP IP Address: ");
    Serial.println(ip);

    // Start servers
    vehicleServer.begin();
    webServer.begin();
    Serial.println("Vehicle and web servers started.");
}

void loop() {
    handleNewVehicleConnections();
    handleExistingVehicleMessages();
    broadcastImportantMessages();
    handleWebRequests();
    cleanupDisconnectedVehicles();
    delay(10);
}

void handleNewVehicleConnections() {
    WiFiClient client = vehicleServer.available();
    if (!client) return;

    String vehicleIP = client.remoteIP().toString();
    Serial.println("Vehicle trying to connect: " + vehicleIP);

    // Check for existing vehicle
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

    // Add new vehicle
    Vehicle newVehicle{
        client,
        vehicleIP,
        millis(),
        "Initial connection"
    };
    connectedVehicles.push_back(newVehicle);
    Serial.println("New vehicle registered: " + vehicleIP);
}

void handleExistingVehicleMessages() {
    for (auto& vehicle : connectedVehicles) {
        if (!vehicle.client.connected()) continue;

        while (vehicle.client.available()) {
            String receivedData = vehicle.client.readStringUntil('\n');
            receivedData.trim();
            
            // Update vehicle status
            vehicle.lastSeen = millis();
            vehicle.lastMessage = receivedData;

            // Process message
            if (isImportantMessage(receivedData)) {
                importantMessages.push_back(receivedData);
            }

            // Add to recent messages
            recentMessages.push_back(receivedData);
            if (recentMessages.size() > MAX_RECENT) {
                recentMessages.erase(recentMessages.begin());
            }

            Serial.println("Received from " + vehicle.ip + ": " + receivedData);
        }
    }
}

void broadcastImportantMessages() {
    if (importantMessages.empty()) return;

    for (const String& message : importantMessages) {
        Serial.println("Broadcasting: " + message);
        
        for (auto& vehicle : connectedVehicles) {
            if (vehicle.client.connected()) {
                vehicle.client.println(message);
                vehicle.lastMessage = "Broadcast: " + message;
            }
        }
    }

    importantMessages.clear();
}

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
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta http-equiv='refresh' content='5'>";
    html += "<title>RSU Dashboard</title>";
    html += "<style>";
    html += "body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += ".container{max-width:800px;margin:0 auto;}";
    html += ".card{background:white;padding:20px;margin:10px;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
    html += "table{width:100%;border-collapse:collapse;}";
    html += "th,td{padding:8px;text-align:left;border-bottom:1px solid #ddd;}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h1>RSU Dashboard</h1>";
    
    // Connected Vehicles
    html += "<div class='card'><h2>Connected Vehicles (" + String(connectedVehicles.size()) + ")</h2>";
    html += "<table><tr><th>IP</th><th>Last Seen</th><th>Last Message</th></tr>";
    for (auto& vehicle : connectedVehicles) {
        if (vehicle.client.connected()) {
            html += "<tr>";
            html += "<td>" + vehicle.ip + "</td>";
            html += "<td>" + String((millis() - vehicle.lastSeen) / 1000) + "s ago</td>";
            html += "<td>" + vehicle.lastMessage + "</td>";
            html += "</tr>";
        }
    }
    html += "</table></div>";

    // Recent Messages
    html += "<div class='card'><h2>Recent Messages</h2>";
    html += "<table><tr><th>Message</th></tr>";
    for (const String& msg : recentMessages) {
        html += "<tr><td>" + msg + "</td></tr>";
    }
    html += "</table></div>";

    html += "</div></body></html>";
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

bool isImportantMessage(const String& message) {
    return message.startsWith("Important:") || 
           message.startsWith("Warning:") ||
           message.startsWith("WARN:");
}
