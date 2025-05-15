#include <AFMotor.h>

// Motor pins
AF_DCMotor leftMotor(1);
AF_DCMotor rightMotor(2);
AF_DCMotor leftMotor2(3);
AF_DCMotor rightMotor2(4);

// IR Sensor pins
const int LEFT_IR = 41;
const int MID_LEFT_IR = 37;
const int MID_RIGHT_IR = 39;
const int RIGHT_IR = 35;

// Motor speeds
const int NORMAL_SPEED = 100;
const int TURN_SPEED = 80;
const int APPROACH_SPEED = 70;
const int PIVOT_REVERSE_SPEED = 50;
const int STOP_SPEED = 0;

// State flags
bool lookingForExit = false;
bool turning = false;
bool shouldExit = false;
bool exited = false;

void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
  
  pinMode(LEFT_IR, INPUT);
  pinMode(MID_LEFT_IR, INPUT);
  pinMode(MID_RIGHT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);
  
  leftMotor.setSpeed(NORMAL_SPEED);
  rightMotor.setSpeed(NORMAL_SPEED);
  leftMotor2.setSpeed(NORMAL_SPEED);
  rightMotor2.setSpeed(NORMAL_SPEED);
  
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  leftMotor2.run(RELEASE);
  rightMotor2.run(RELEASE);
  
  Serial.println("Arduino: Setup complete");
  Serial.println("Arduino: Waiting for commands from ESP32...");
}

void loop() {
  bool leftIR = digitalRead(LEFT_IR);
  bool midLeftIR = digitalRead(MID_LEFT_IR);
  bool midRightIR = digitalRead(MID_RIGHT_IR);
  bool rightIR = digitalRead(RIGHT_IR);

  Serial.println("\n--- IR Sensor Readings ---");
  Serial.print("Left IR: "); Serial.println(leftIR);
  Serial.print("Middle Left IR: "); Serial.println(midLeftIR);
  Serial.print("Middle Right IR: "); Serial.println(midRightIR);
  Serial.print("Right IR: "); Serial.println(rightIR);

  if (shouldExit && !exited) {
    if (leftIR) {
      Serial.println("Emergency exit left detected!");
      takeLeftTurn();
      exited = true;
      stopAndFindNewRoute();
      Serial1.println("TOOK_EXIT:LEFT");
    } else if (rightIR) {
      Serial.println("Emergency exit right detected!");
      takeRightTurn();
      exited = true;
      stopAndFindNewRoute();
      Serial1.println("TOOK_EXIT:RIGHT");
    } else {
      followLine(midLeftIR, midRightIR);
    }
  } else if (lookingForExit) {
    if (midLeftIR || midRightIR) {
      if (leftIR) {
        Serial.println("Left intersection detected!");
        takeLeftTurn();
        Serial1.println("TOOK_EXIT:LEFT");
        lookingForExit = false;
      } else if (rightIR) {
        Serial.println("Right intersection detected!");
        takeRightTurn();
        Serial1.println("TOOK_EXIT:RIGHT");
        lookingForExit = false;
      } else {
        followLine(midLeftIR, midRightIR);
      }
    }
  } else {
    followLine(midLeftIR, midRightIR);
  }

  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    Serial.println("Arduino: Received from ESP32: '" + command + "'");

    if (command == "TAKE_FIRST_EXIT") {
      lookingForExit = true;
      Serial.println("Arduino: Now looking for next intersection");
      Serial1.println("ACK:TAKE_FIRST_EXIT");
    } else if (command == "MOVE") {
      lookingForExit = false;
      turning = false;
      Serial.println("Arduino: Resuming normal line following");
      Serial1.println("ACK:MOVE");
    } else if (command == "WARNING") {
      shouldExit = true;
      exited = false;
      Serial.println("Arduino: WARNING received! Searching for nearest exit.");
      Serial1.println("ACK:WARNING");
    }
  }

  delay(50);
}

void followLine(bool midLeftIR, bool midRightIR) {
  if (midLeftIR && midRightIR) {
    int speedToUse = shouldExit ? APPROACH_SPEED : NORMAL_SPEED;
    Serial.println(shouldExit ? "Going straight (warning mode) - slower" : "Going straight - normal");
    
    leftMotor.setSpeed(speedToUse);
    rightMotor.setSpeed(speedToUse);
    leftMotor2.setSpeed(speedToUse);
    rightMotor2.setSpeed(speedToUse);
    
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
    leftMotor2.run(FORWARD);
    rightMotor2.run(FORWARD);
  } else if (midLeftIR && !midRightIR) {
    Serial.println("Adjusting right - middle-left sensor on line");
    leftMotor.setSpeed(NORMAL_SPEED);
    rightMotor.setSpeed(TURN_SPEED);
    leftMotor2.setSpeed(NORMAL_SPEED);
    rightMotor2.setSpeed(TURN_SPEED);
    
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
    leftMotor2.run(FORWARD);
    rightMotor2.run(FORWARD);
  } else if (!midLeftIR && midRightIR) {
    Serial.println("Adjusting left - middle-right sensor on line");
    leftMotor.setSpeed(TURN_SPEED);
    rightMotor.setSpeed(NORMAL_SPEED);
    leftMotor2.setSpeed(TURN_SPEED);
    rightMotor2.setSpeed(NORMAL_SPEED);
    
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
    leftMotor2.run(FORWARD);
    rightMotor2.run(FORWARD);
  } else {
    Serial.println("Stopping - line lost");
    leftMotor.setSpeed(STOP_SPEED);
    rightMotor.setSpeed(STOP_SPEED);
    leftMotor2.setSpeed(STOP_SPEED);
    rightMotor2.setSpeed(STOP_SPEED);
    
    leftMotor.run(RELEASE);
    rightMotor.run(RELEASE);
    leftMotor2.run(RELEASE);
    rightMotor2.run(RELEASE);
  }
}

void takeLeftTurn() {
  turning = true;
  stopMotors();
  delay(500);

  leftMotor.setSpeed(PIVOT_REVERSE_SPEED);
  leftMotor2.setSpeed(PIVOT_REVERSE_SPEED);
  rightMotor.setSpeed(TURN_SPEED);
  rightMotor2.setSpeed(TURN_SPEED);

  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
  leftMotor2.run(BACKWARD);
  rightMotor2.run(FORWARD);

  while (true) {
    if (digitalRead(MID_LEFT_IR) || digitalRead(MID_RIGHT_IR)) break;
    delay(50);
  }

  resumeMotors();
  turning = false;
}

void takeRightTurn() {
  turning = true;
  stopMotors();
  delay(500);

  rightMotor.setSpeed(PIVOT_REVERSE_SPEED);
  rightMotor2.setSpeed(PIVOT_REVERSE_SPEED);
  leftMotor.setSpeed(TURN_SPEED);
  leftMotor2.setSpeed(TURN_SPEED);

  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(BACKWARD);

  while (true) {
    if (digitalRead(MID_LEFT_IR) || digitalRead(MID_RIGHT_IR)) break;
    delay(50);
  }

  resumeMotors();
  turning = false;
}

void stopMotors() {
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  leftMotor2.run(RELEASE);
  rightMotor2.run(RELEASE);
}

void resumeMotors() {
  leftMotor.setSpeed(NORMAL_SPEED);
  rightMotor.setSpeed(NORMAL_SPEED);
  leftMotor2.setSpeed(NORMAL_SPEED);
  rightMotor2.setSpeed(NORMAL_SPEED);
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(FORWARD);
}

void stopAndFindNewRoute() {
  stopMotors();
  delay(500);
  Serial.println("Trying to realign with new track...");

  for (int i = 0; i < 2; i++) {
    // Try left rotation
    leftMotor.setSpeed(PIVOT_REVERSE_SPEED);
    leftMotor2.setSpeed(PIVOT_REVERSE_SPEED);
    rightMotor.setSpeed(TURN_SPEED);
    rightMotor2.setSpeed(TURN_SPEED);
    
    leftMotor.run(BACKWARD);
    rightMotor.run(FORWARD);
    leftMotor2.run(BACKWARD);
    rightMotor2.run(FORWARD);
    
    for (int j = 0; j < 20; j++) {
      if (digitalRead(MID_LEFT_IR) || digitalRead(MID_RIGHT_IR)) {
        Serial.println("New track found!");
        resumeMotors();
        shouldExit = false;
        exited = false;
        return;
      }
      delay(100);
    }

    // Try right rotation
    leftMotor.run(FORWARD);
    rightMotor.run(BACKWARD);
    leftMotor2.run(FORWARD);
    rightMotor2.run(BACKWARD);
    delay(1000);
  }

  Serial.println("Failed to find new track. Stopping.");
  stopMotors();
}
