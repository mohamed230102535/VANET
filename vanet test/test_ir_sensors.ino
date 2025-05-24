// IR Sensor pins
const int LEFT_IR = 37;
const int RIGHT_IR = 39;

void setup() {
  Serial.begin(115200);
  Serial.println("IR Sensor Test Program Started");
  
  // Initialize IR sensor pins
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
}

void loop() {
  // Read IR sensor values
  bool IR_Sen_R = digitalRead(RIGHT_IR);
  bool IR_Sen_L = digitalRead(LEFT_IR);

  // Print sensor values
  Serial.print("Left IR Sensor: ");
  Serial.print(IR_Sen_L);
  Serial.print(" | Right IR Sensor: ");
  Serial.println(IR_Sen_R);

  // Add interpretation of sensor values
  if (!IR_Sen_L && !IR_Sen_R) {
    Serial.println("Status: Both sensors on white line");
  } else if (!IR_Sen_L && IR_Sen_R) {
    Serial.println("Status: Left sensor on white line, Right sensor off line");
  } else if (IR_Sen_L && !IR_Sen_R) {
    Serial.println("Status: Right sensor on white line, Left sensor off line");
  } else {
    Serial.println("Status: Both sensors off white line");
  }

  delay(500); // Update every 500ms
} 