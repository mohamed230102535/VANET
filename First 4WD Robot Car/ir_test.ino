// IR Sensor Test Code
const int LEFT_IR = 41;   // Left IR sensor
const int RIGHT_IR = 43;  // Right IR sensor

void setup() {
  Serial.begin(115200);
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  
  Serial.println("IR Sensor Test Started");
  Serial.println("Place sensors over black line to test");
  Serial.println("Format: Left IR | Right IR");
  Serial.println("------------------------");
}

void loop() {
  // Read IR sensors
  int leftIR = digitalRead(LEFT_IR);
  int rightIR = digitalRead(RIGHT_IR);
  
  // Print readings
  Serial.print("Left IR: ");
  Serial.print(leftIR);
  Serial.print(" | Right IR: ");
  Serial.println(rightIR);
  
  // Add visual indicator
  if (leftIR == HIGH) {
    Serial.println("Left IR: Detecting Black Line");
  } else {
    Serial.println("Left IR: Not on Black Line");
  }
  
  if (rightIR == HIGH) {
    Serial.println("Right IR: Detecting Black Line");
  } else {
    Serial.println("Right IR: Not on Black Line");
  }
  
  Serial.println("------------------------");
  delay(1000);  // Update every second
} 