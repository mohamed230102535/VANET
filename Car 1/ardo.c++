#include <AFMotor.h>

// Motor Pins for L293D Motor Driver
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

// Sensor Pins
const int TRIG_PIN = A5;
const int ECHO_PIN = A4;
const int IR_SENSOR = 41;

// Constants
const int STOP_DISTANCE = 20;
const int MOTOR_SPEED = 100;

void setup() {
  Serial.begin(115200);      // Debug
  Serial1.begin(115200);     // ESP32
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_SENSOR, INPUT);
  
  // Initialize motors
  motor1.setSpeed(MOTOR_SPEED);
  motor2.setSpeed(MOTOR_SPEED);
  motor3.setSpeed(MOTOR_SPEED);
  motor4.setSpeed(MOTOR_SPEED);
}

void loop() {
  // Read sensors
  int irValue = digitalRead(IR_SENSOR);
  long distance = getDistance();
  
  // Line following
  if (irValue == HIGH) {
    moveForward();
  } else {
    stopCar();
  }
  
  // Obstacle detection
  if (distance < STOP_DISTANCE && distance > 1) {
    stopCar();
    String warning = "WARN:Obstacle at " + String(distance) + " cm";
    Serial.println(warning);
    Serial1.println(warning);
  }
  
  // Handle ESP32 commands
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    if (cmd == "STOP") stopCar();
    else if (cmd == "MOVE") moveForward();
  }
  
  delay(50);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void moveForward() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void stopCar() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}