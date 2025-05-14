#include <AFMotor.h>

// Motor pins
AF_DCMotor leftMotor(1);
AF_DCMotor rightMotor(2);

// IR Sensor pins
const int LEFT_IR = 41;
const int MID_LEFT_IR = 37;
const int MID_RIGHT_IR = 39;
const int RIGHT_IR = 35;

// Motor speeds
const int NORMAL_SPEED = 150;
const int TURN_SPEED = 130;
const int STOP_SPEED = 0;

// States
bool lookingForExit = false;
bool turning = false;

void setup() {
  // Initialize serial communication with ESP32
  Serial1.begin(115200);
  // Debug serial
  Serial.begin(115200);
  
  // Set up IR sensor pins
  pinMode(LEFT_IR, INPUT);
  pinMode(MID_LEFT_IR, INPUT);
  pinMode(MID_RIGHT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  
  // Set initial motor speed
  leftMotor.setSpeed(NORMAL_SPEED);
  rightMotor.setSpeed(NORMAL_SPEED);
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  
  Serial.println("Arduino: Setup complete");
  Serial.println("Arduino: Waiting for commands from ESP32...");
}

void loop() {
  // Read IR sensor values
  bool leftIR = digitalRead(LEFT_IR);
  bool midLeftIR = digitalRead(MID_LEFT_IR);
  bool midRightIR = digitalRead(MID_RIGHT_IR);
  bool rightIR = digitalRead(RIGHT_IR);
  
  // Check for commands from ESP32
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    
    // Print received command for debugging
    Serial.println("Arduino: Received from ESP32: '" + command + "'");
    
    if (command == "TAKE_FIRST_EXIT") {
      lookingForExit = true;
      Serial.println("Arduino: Command accepted - Looking for first exit");
      // Echo back to ESP32 that we received the command
      Serial1.println("ACK:TAKE_FIRST_EXIT");
    }
    else if (command == "MOVE") {
      lookingForExit = false;
      turning = false;
      Serial.println("Arduino: Command accepted - Resuming normal operation");
      // Echo back to ESP32 that we received the command
      Serial1.println("ACK:MOVE");
    }
    else {
      Serial.println("Arduino: Unknown command received");
    }
  }
  
  // If we're looking for an exit and not currently turning
  if (lookingForExit && !turning) {
    // Check for left exit (left sensor detects line, middle sensors don't)
    if (leftIR && !midLeftIR && !midRightIR) {
      turning = true;
      Serial.println("Arduino: Left exit detected - Starting left turn");
      takeLeftTurn();
      Serial.println("Arduino: Left turn completed");
      Serial1.println("TOOK_EXIT:LEFT");
      lookingForExit = false;
    }
    // Check for right exit (right sensor detects line, middle sensors don't)
    else if (rightIR && !midLeftIR && !midRightIR) {
      turning = true;
      Serial.println("Arduino: Right exit detected - Starting right turn");
      takeRightTurn();
      Serial.println("Arduino: Right turn completed");
      Serial1.println("TOOK_EXIT:RIGHT");
      lookingForExit = false;
    }
    else {
      // No exit detected, continue line following
      followLine(midLeftIR, midRightIR);
    }
  }
  else if (!turning) {
    // Normal line following operation
    followLine(midLeftIR, midRightIR);
  }
  
  delay(50); // Small delay to prevent overwhelming
}

void followLine(bool midLeftIR, bool midRightIR) {
  if (midLeftIR && midRightIR) {
    // Both middle sensors on line - go straight
    leftMotor.setSpeed(NORMAL_SPEED);
    rightMotor.setSpeed(NORMAL_SPEED);
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
  }
  else if (midLeftIR && !midRightIR) {
    // Drifting right - correct left
    leftMotor.setSpeed(TURN_SPEED);
    rightMotor.setSpeed(NORMAL_SPEED);
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
  }
  else if (!midLeftIR && midRightIR) {
    // Drifting left - correct right
    leftMotor.setSpeed(NORMAL_SPEED);
    rightMotor.setSpeed(TURN_SPEED);
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
  }
  else {
    // Lost line - stop
    leftMotor.setSpeed(STOP_SPEED);
    rightMotor.setSpeed(STOP_SPEED);
    leftMotor.run(RELEASE);
    rightMotor.run(RELEASE);
  }
}

void takeLeftTurn() {
  // Stop first
  leftMotor.setSpeed(STOP_SPEED);
  rightMotor.setSpeed(STOP_SPEED);
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  delay(500);
  
  // Turn left until middle sensors find the line
  leftMotor.setSpeed(TURN_SPEED);
  rightMotor.setSpeed(TURN_SPEED);
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
  
  while (true) {
    if (digitalRead(MID_LEFT_IR) || digitalRead(MID_RIGHT_IR)) {
      break;
    }
    delay(50);
  }
  
  // Reset state
  turning = false;
  leftMotor.setSpeed(NORMAL_SPEED);
  rightMotor.setSpeed(NORMAL_SPEED);
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
}

void takeRightTurn() {
  // Stop first
  leftMotor.setSpeed(STOP_SPEED);
  rightMotor.setSpeed(STOP_SPEED);
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  delay(500);
  
  // Turn right until middle sensors find the line
  leftMotor.setSpeed(TURN_SPEED);
  rightMotor.setSpeed(TURN_SPEED);
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
  
  while (true) {
    if (digitalRead(MID_LEFT_IR) || digitalRead(MID_RIGHT_IR)) {
      break;
    }
    delay(50);
  }
  
  // Reset state
  turning = false;
  leftMotor.setSpeed(NORMAL_SPEED);
  rightMotor.setSpeed(NORMAL_SPEED);
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
}
