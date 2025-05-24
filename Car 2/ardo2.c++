//the code is for arduino mega car2 
#include <AFMotor.h>

// Motor pins
AF_DCMotor leftMotor(1);
AF_DCMotor rightMotor(2);
AF_DCMotor leftMotor2(3);
AF_DCMotor rightMotor2(4);

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

bool warning_received = false; // Flag for ESP warning

void setup(){
	Serial.begin(115200);
	pinMode(LEFT_IR, INPUT);
	pinMode(RIGHT_IR, INPUT);
	setMotorSpeed(NORMAL_SPEED);
	Serial1.begin(115200); // ESP32 communication
}

void loop(){
	bool IR_Sen_R = digitalRead(RIGHT_IR);
	bool IR_Sen_L = digitalRead(LEFT_IR);

	ESP_RECIV_Message(); // Check for ESP32 messages

	// If we're in the middle of a turn, continue turning
	if (is_turning) {
		if (millis() - turn_start_time < TURN_DURATION) {
			// Continue turning
			if (line_right) {
				setMotorSpeed(TURN_SPEED);
				move_right();
			} else if (line_left) {
				setMotorSpeed(TURN_SPEED);
				move_left();
			}
		} else {
			// Turn complete
			is_turning = false;
			line_right = false;
			line_left = false;
			Stop_car();
			delay(200);
			setMotorSpeed(NORMAL_SPEED);
			warning_received = false; // Reset warning only after turn is complete
		}
		return; // Skip other logic while turning
	}

	// Normal line following logic
	if (!IR_Sen_R && !IR_Sen_L && !line_right && !line_left) {
		setMotorSpeed(NORMAL_SPEED);
		move_forward();
	} else if (!IR_Sen_L && IR_Sen_R && warning_received) {
		// Start right turn only if warning received
		line_right = true;
		line_left = false;
		is_turning = true;
		turn_start_time = millis();
		move_right();
	} else if (IR_Sen_L && !IR_Sen_R && warning_received) {
		// Start left turn only if warning received
		line_right = false;
		line_left = true;
		is_turning = true;
		turn_start_time = millis();
		move_left();
	} else if (IR_Sen_L && IR_Sen_R) {
		line_right = true;
		line_left = true;
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

void ESP_RECIV_Message() {
	if (Serial1.available()) {
		String command = Serial1.readStringUntil('\n');
		command.trim();

		if (command == "TAKE_FIRST_EXIT") {
			Serial.println("Arduino: Warning command received - Will take first exit");
			Serial1.println("TOOK_EXIT:FIRST");
			warning_received = true;
		} else if (command == "MOVE") {
			Serial1.println("STATUS:MOVING");
			warning_received = false;
		}
	}
}