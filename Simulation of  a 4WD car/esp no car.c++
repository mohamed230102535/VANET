#include <WiFi.h>
#include <WebServer.h>

// RSU Connection Details
const char* RSU_IP = "192.168.4.1";
const uint16_t RSU_PORT = 8080;

// Wi-Fi settings
const char* SSID = "vanet";
const char* PASSWORD = "12345678";

// Web server
WebServer server(80);

// Car simulation states
enum CarState {
    MOVING,
    STOPPED,
    WARNING
};

struct Car {
    CarState state;
    int speed;
    int distance;
    bool hasObstacle;
} car = {MOVING, 100, 50, false};

WiFiClient client;
String receivedData = "";

void setup() {
    Serial.begin(115200);
    delay(100);

    connectToWiFi();
    
    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/send-warning", HTTP_GET, handleSendWarning);
    server.on("/car-status", HTTP_GET, handleCarStatus);
    
    server.begin();
    Serial.println("Simulated Car: Started");
    Serial.println("Car State: Moving");
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();  // Handle web requests
    
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    } else if (!client.connected()) {
        connectToRSU();
    }

    // Simulate car movement and distance changes
    simulateCarMovement();

    // Check for messages from RSU
    if (client.available()) {
        String message = client.readStringUntil('\n');
        message.trim();
        handleRSUMessage(message);
    }

    delay(100);
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Car Control Panel</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += ".container{max-width:800px;margin:0 auto;}";
    html += ".card{background:white;padding:20px;margin:10px;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
    html += ".button{background:#ff4444;color:white;border:none;padding:15px 30px;border-radius:5px;cursor:pointer;font-size:16px;}";
    html += ".button:hover{background:#cc0000;}";
    html += ".status{font-size:18px;margin:10px 0;}";
    html += "</style>";
    html += "<script>";
    html += "function updateStatus() {";
    html += "  fetch('/car-status')";
    html += "    .then(response => response.text())";
    html += "    .then(data => {";
    html += "      document.getElementById('status').innerHTML = data;";
    html += "    });";
    html += "}";
    html += "setInterval(updateStatus, 1000);";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Car Control Panel</h1>";
    html += "<div class='card'>";
    html += "<h2>Car Status</h2>";
    html += "<div id='status' class='status'>Loading...</div>";
    html += "</div>";
    html += "<div class='card'>";
    html += "<h2>Controls</h2>";
    html += "<button class='button' onclick='window.location.href=\"/send-warning\"'>Send Warning</button>";
    html += "</div>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
}

void handleSendWarning() {
    sendWarningToRSU("WARN:Manual warning sent from web interface");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleCarStatus() {
    String status = "State: ";
    status += (car.state == MOVING) ? "Moving" : "Stopped";
    status += "<br>Speed: " + String(car.speed);
    status += "<br>Distance: " + String(car.distance) + " cm";
    status += "<br>Obstacle: " + String(car.hasObstacle ? "Yes" : "No");
    server.send(200, "text/html", status);
}

void simulateCarMovement() {
    // Simulate distance changes
    if (car.state == MOVING) {
        car.distance = random(10, 100);  // Random distance between 10 and 100 cm
        
        // Simulate obstacle detection
        if (car.distance < 20) {
            car.hasObstacle = true;
            stopCar();
            sendWarningToRSU("WARN:Obstacle detected at " + String(car.distance) + " cm");
        } else if (car.hasObstacle && car.distance > 30) {
            car.hasObstacle = false;
            moveCar();
            sendInfoToRSU("INFO:Obstacle cleared");
        }

        // Send distance to RSU periodically
        static unsigned long lastDistanceUpdate = 0;
        if (millis() - lastDistanceUpdate > 1000) {  // Send every second
            sendDistanceToRSU();
            lastDistanceUpdate = millis();
        }
    }
}

void handleRSUMessage(String message) {
    Serial.println("Received from RSU: " + message);

    if (message.startsWith("Important:") || message.startsWith("Warning:")) {
        stopCar();
        sendInfoToRSU("INFO:Stopped due to RSU warning");
    } else if (message == "Move") {
        moveCar();
        sendInfoToRSU("INFO:Moving by RSU command");
    }
}

void stopCar() {
    car.state = STOPPED;
    car.speed = 0;
    Serial.println("Car State: Stopped");
}

void moveCar() {
    car.state = MOVING;
    car.speed = 100;
    Serial.println("Car State: Moving");
}

void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected, IP: " + WiFi.localIP().toString());
}

void connectToRSU() {
    if (client.connect(RSU_IP, RSU_PORT)) {
        Serial.println("Connected to RSU");
        sendInfoToRSU("INFO:Car connected to RSU");
    } else {
        Serial.println("RSU connection failed");
        delay(2000);
    }
}

void sendDistanceToRSU() {
    if (client.connected()) {
        String message = "DIST:" + String(car.distance);
        client.println(message);
        Serial.println("Sent to RSU: " + message);
    }
}

void sendWarningToRSU(String warning) {
    if (client.connected()) {
        client.println(warning);
        Serial.println("Sent to RSU: " + warning);
    }
}

void sendInfoToRSU(String info) {
    if (client.connected()) {
        client.println(info);
        Serial.println("Sent to RSU: " + info);
    }
}
