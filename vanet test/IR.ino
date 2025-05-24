#include <AFMotor.h>

// Motor pins
AF_DCMotor leftMotor(1);
AF_DCMotor rightMotor(2);
AF_DCMotor leftMotor2(3);
AF_DCMotor rightMotor2(4);

// IR Sensor pins
const int LEFT_IR = 37;
const int RIGHT_IR = 39;

// Line following state variables
bool line_right = false;
bool line_left = false;
bool is_turning = false;
unsigned long turn_start_time = 0;
const unsigned long TURN_DURATION = 650; // Time for 90-degree turn in milliseconds

// Motor speeds
const int NORMAL_SPEED = 90;     // Normal forward speed
const int TURN_SPEED = 140;      // Speed during turns
const int SLOW_SPEED = 70;       // Speed for fine adjustments

void setup() {
  Serial.begin(115200);
  Serial.println("Line Following Test Program Started");
  
  // Initialize IR sensor pins
  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);

  // Set initial motor speeds
  setMotorSpeed(NORMAL_SPEED);
}

void loop() {
  // Read IR sensor values
  bool IR_Sen_R = digitalRead(RIGHT_IR);
  bool IR_Sen_L = digitalRead(LEFT_IR);

  // Print current sensor state
  Serial.print("Sensors - Left: ");
  Serial.print(IR_Sen_L);
  Serial.print(" Right: ");
  Serial.println(IR_Sen_R);

  // If we're in the middle of a turn, continue turning
  if (is_turning) {
    if (millis() - turn_start_time < TURN_DURATION) {
      // Continue turning
      if (line_right) {
        setMotorSpeed(TURN_SPEED);  // Faster speed for turns
        move_right();
        Serial.println("Completing right turn...");
      } else if (line_left) {
        setMotorSpeed(TURN_SPEED);  // Faster speed for turns
        move_left();
        Serial.println("Completing left turn...");
      }
    } else {
      // Turn complete
      is_turning = false;
      line_right = false;
      line_left = false;
      Stop_car();
      Serial.println("Turn complete");
      delay(200);  // Short delay after turn
      setMotorSpeed(NORMAL_SPEED);  // Return to normal speed
    }
    return; // Skip other logic while turning
  }

  // Normal line following logic
  if (!IR_Sen_R && !IR_Sen_L && !line_right && !line_left) {
    setMotorSpeed(NORMAL_SPEED);
    Serial.println("Condition: Moving Forward");
    move_forward();
  } else if (!IR_Sen_L && IR_Sen_R) {
    // Start right turn
    line_right = true;
    line_left = false;
    is_turning = true;
    turn_start_time = millis();
    Serial.println("Starting right turn");
    move_right();
  } else if (IR_Sen_L && !IR_Sen_R) {
    // Start left turn
    line_right = false;
    line_left = true;
    is_turning = true;
    turn_start_time = millis();
    Serial.println("Starting left turn");
    move_left();
  } else if (IR_Sen_L && IR_Sen_R) {
    line_right = true;
    line_left = true;
    Serial.println("Condition: Stop - Both sensors on line");
    Stop_car();
    delay(1000);
    line_right = false;
    line_left = false;
  }

  delay(50); // Reduced delay for more responsive control
}

void setMotorSpeed(int speed) {
  leftMotor.setSpeed(speed);
  rightMotor.setSpeed(speed);
  leftMotor2.setSpeed(speed);
  rightMotor2.setSpeed(speed);
}

void move_forward() {
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(FORWARD);
}

void move_right() {
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
  leftMotor2.run(FORWARD);
  rightMotor2.run(BACKWARD);
}

void move_left() {
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
  leftMotor2.run(BACKWARD);
  rightMotor2.run(FORWARD);
}

void Stop_car() {
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
  leftMotor2.run(RELEASE);
  rightMotor2.run(RELEASE);
} 