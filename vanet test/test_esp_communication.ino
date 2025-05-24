bool warning_received = false;

void setup() {
  Serial.begin(115200);  // For debugging
  Serial1.begin(115200); // For ESP32 communication
  
  Serial.println("ESP32 Communication Test Program Started");
}

void loop() {
  // Check for incoming messages from ESP32
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    
    Serial.print("Received command: ");
    Serial.println(command);
    
    // Process different commands
    if (command == "TAKE_FIRST_EXIT") {
      Serial.println("Processing: TAKE_FIRST_EXIT");
      Serial1.println("ACK:TAKE_FIRST_EXIT");
    } 
    else if (command == "MOVE") {
      Serial.println("Processing: MOVE");
      Serial1.println("ACK:MOVE");
    } 
    else if (command == "WARNING") {
      Serial.println("00004444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444WARNING");
      Serial1.println("ACK:WARNING");
      warning_received = true;
    }
  }
  
  // Simulate sending test commands (for testing purposes)
  if (Serial.available()) {
    String testCommand = Serial.readStringUntil('\n');
    testCommand.trim();
    
    if (testCommand == "test") {
      // Send test commands to ESP32
      Serial.println("Sending test commands...");
      
      Serial1.println("TAKE_FIRST_EXIT");
      delay(1000);
      
      Serial1.println("MOVE");
      delay(1000);
      
      Serial1.println("WARNING");
      delay(1000);
    }
  }
  
  delay(100); // Small delay for stability
} 