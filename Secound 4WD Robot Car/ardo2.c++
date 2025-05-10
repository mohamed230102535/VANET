#include <AFMotor.h>

// Motor Pins for L293D Motor Driver
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// IR Obstacle Avoidance Sensor Pins
const int LEFT_IR = 41;      // Left sensor (for ESP warnings)
const int MID_LEFT_IR = 37;  // Middle left sensor
const int MID_RIGHT_IR = 39; // Middle right sensor
const int RIGHT_IR = 35;     // Right sensor (for ESP warnings)

// Motor speeds (adjusted for slower, smoother motion)
const int MOTOR_SPEED = 60;
const int TURN_SPEED = 45;

// Vehicle state
bool isStopped = false;

void setup() {
  Serial.begin(115200);      // Serial0 for debugging to PC
  Serial1.begin(115200, SERIAL_8N1);     // Serial1 for ESP32 (TX1 18, RX1 19)
  
  // Set IR pins as input
  pinMode(LEFT_IR, INPUT);
  pinMode(MID_LEFT_IR, INPUT);
  pinMode(MID_RIGHT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);

  // Test IR sensors
  Serial.println("Testing IR Sensors...");
  Serial.println("Format: Left | Mid-Left | Mid-Right | Right");
  Serial.println("1 = Line/Obstacle Detected, 0 = No Detection"); 
  Serial.println("----------------------------------------");
  delay(1000);
  
  // Initialize motors at medium speed for testing
  motor1.setSpeed(80);
  motor2.setSpeed(80);
  motor3.setSpeed(80);
  motor4.setSpeed(80);
  
  // Test motors briefly
  Serial.println("Testing motors...");
  moveForward();
  delay(1000);
  stopCar();
  
  // Set normal operating speeds
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
  int midLeftIR = digitalRead(MID_LEFT_IR);
  int midRightIR = digitalRead(MID_RIGHT_IR);
  int rightIR = digitalRead(RIGHT_IR);
  
  // Debug IR sensor readings
  Serial.print("Left: ");
  Serial.print(leftIR);
  Serial.print(" | Mid-Left: ");
  Serial.print(midLeftIR);
  Serial.print(" | Mid-Right: ");
  Serial.print(midRightIR);
  Serial.print(" | Right: ");
  Serial.println(rightIR);
  
  // Line following logic using middle sensors (1 = black line detected)
  if (midLeftIR == 1 && midRightIR == 1) {
    moveForward();
    isStopped = false;
  }
  else if (midLeftIR == 1) {
    turnLeft();
    isStopped = false;
  }
  else if (midRightIR == 1) {
    turnRight();
    isStopped = false;
  }
  else {
    stopCar();
    isStopped = true;
    Serial.println("Mega: STOPPED - Lost black line completely");
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
    } 
    else if (command == "MOVE") {
      moveForward();
      isStopped = false;
      Serial.println("Mega: Moving by ESP32");
    }
    else if (command == "WARN_LEFT") {
      turnRight();  // Turn away from the warning
      isStopped = false;
      Serial.println("Mega: Turning right - ESP left warning");
    }
    else if (command == "WARN_RIGHT") {
      turnLeft();   // Turn away from the warning
      isStopped = false;
      Serial.println("Mega: Turning left - ESP right warning");
    }
  }

  delay(50);  // Faster loop for improved accuracy
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
  motor3.setSpeed(0);
  motor4.setSpeed(0);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

void turnRight() {
  motor1.setSpeed(0);
  motor2.setSpeed(0);
  motor3.setSpeed(TURN_SPEED);
  motor4.setSpeed(TURN_SPEED);
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void stopCar() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}
