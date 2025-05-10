#include <AFMotor.h>

// Motor Pins for L293D Motor Driver
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// Ultrasonic Sensor Pins
const int TRIG_PIN = A5;
const int ECHO_PIN = A4;

// IR Sensor Pins
const int LEFT_IR = 41;   // Left IR sensor
const int RIGHT_IR = 43;  // Right IR sensor

// Threshold distances (in cm)
const int STOP_DISTANCE = 20;
const int RESUME_DISTANCE = 25;

// Motor speeds
const int MOTOR_SPEED = 100;
const int TURN_SPEED = 80;

// Vehicle state
bool isStopped = false;

void setup() {
  Serial.begin(115200);      // Serial0 for debugging to PC
  Serial1.begin(115200, SERIAL_8N1);     // Serial1 for ESP32 (TX1 18, RX1 19)
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_IR, INPUT);   // Set IR pins as input
  pinMode(RIGHT_IR, INPUT);

  // Test IR sensors
  Serial.println("Testing IR Sensors...");
  delay(1000);
  
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor3.setSpeed(MOTOR_SPEED);
  motor4.setSpeed(MOTOR_SPEED);
  
  Serial.println("Mega: Started (Serial0)");
  Serial1.println("Mega: Started (Serial1 to ESP32)");
  delay(1000); // Give ESP32 time to initialize
}

void loop() {
  // Read IR sensors
  int leftIR = digitalRead(LEFT_IR);
  int rightIR = digitalRead(RIGHT_IR);
  
  // Debug IR sensor readings
  Serial.print("Left IR: ");
  Serial.print(leftIR);
  Serial.print(" Right IR: ");
  Serial.println(rightIR);
  
  // Read distance
  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);
  
  // Add debug message before sending
  Serial.println("Mega: Sending distance to ESP32");
  Serial1.print("DIST:");
  Serial1.println(distance);
  Serial1.flush(); // Ensure data is sent

  // Check for obstacles first
  if (distance < STOP_DISTANCE && distance > 0 && !isStopped) {
    stopCar();
    isStopped = true;
    String warning = "WARN:Obstacle detected at " + String(distance) + " cm";
    Serial.println("Mega: " + warning);         // Debug to PC
    Serial1.println(warning);                   // Send to ESP32
    Serial1.flush(); // Ensure data is sent
  } 
  // If no obstacles, handle line following
  else if (distance > RESUME_DISTANCE) {
    // Line following logic
    if (leftIR == HIGH && rightIR == HIGH) {
      // Both sensors on line - move forward
      moveForward();
      isStopped = false;
      Serial.println("Mega: Moving forward - on line");
    }
    else if (leftIR == HIGH && rightIR == LOW) {
      // Left sensor on line - turn left
      turnLeft();
      isStopped = false;
      Serial.println("Mega: Turning left");
    }
    else if (leftIR == LOW && rightIR == HIGH) {
      // Right sensor on line - turn right
      turnRight();
      isStopped = false;
      Serial.println("Mega: Turning right");
    }
    else {
      // Both sensors off line - stop
      stopCar();
      isStopped = true;
      Serial.println("Mega: Stopped - off line");
      String info = "INFO:Off line";
      Serial.println("Mega: " + info);            // Debug to PC
      Serial1.println(info);                      // Send to ESP32
      Serial1.flush(); // Ensure data is sent
    }
  }

  // Listen for commands from ESP32
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    Serial.println("Mega: Received from ESP32: " + command);
    if (command == "STOP") {
      stopCar();
      isStopped = true;
      Serial.println("Mega: Stopped by ESP32");
    } else if (command == "MOVE") {
      moveForward();
      isStopped = false;
      Serial.println("Mega: Moving by ESP32");
    }
  }

  delay(100);  // Faster response for better line following
}

void moveForward() {
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor3.setSpeed(MOTOR_SPEED);
  motor4.setSpeed(MOTOR_SPEED);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void turnLeft() {
  motor1.setSpeed(TURN_SPEED);
  motor2.setSpeed(TURN_SPEED);
  motor3.setSpeed(TURN_SPEED);
  motor4.setSpeed(TURN_SPEED);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void turnRight() {
  motor1.setSpeed(TURN_SPEED);
  motor2.setSpeed(TURN_SPEED);
  motor3.setSpeed(TURN_SPEED);
  motor4.setSpeed(TURN_SPEED);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void stopCar() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}

// IR Sensor Test Code
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