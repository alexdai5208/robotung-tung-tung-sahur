#include <ESP32Servo.h>
// #include <BLEGamepadClient.h>

// XboxController controller;

// global variables for base servo stuff
Servo baseServo;
Servo verticalServo;
Servo clawServo;

// create Servo object to control a servo
// twelve Servo objects can be created on most boards

// variables
int basePos = 0;
bool clockwise = true;
bool toggle = false;
bool change = false;
bool automatic = false;

const int baseServoPin = 6;
const int verticalServoPin = 5;
const int clawServoPin = 8;

// 95 to 180 is the degrees 180 is straight up
int verticalPos = 180;
int clawPos = 0;

// global variables for ultrasonic sensor stuff
const int trigPin = 16;
const int echoPin = 17;
const int LOW_RANGE = 15;
const int HIGH_RANGE = 30;

// baud for something
const int BAUD = 9600;

float duration, distance;

int num_detections = 0;

void setup() {
  Serial.begin(115200);
  
  // --- FIX FOR ISSUE 3: TIMER ALLOCATION ---
  // Allocate hardware timers to prevent signal overlap
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Set standard PWM frequency for servos (50Hz)
  baseServo.setPeriodHertz(50);
  verticalServo.setPeriodHertz(50);
  clawServo.setPeriodHertz(50);
  // -----------------------------------------

  // base servo setup
  baseServo.attach(baseServoPin);
  verticalServo.attach(verticalServoPin);
  clawServo.attach(clawServoPin);

  // ultrasonic sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Serial.begin(BAUD);  // 9600 baud
}

void loop() {
  if (automatic) {
    while (num_detections < 8) {
      // base servo code to search for objects
      // turns base if it doesn't initally detect something before
      if (num_detections == 0) {
        // sets servo direction of movement based on current servo orientation
        if (basePos <= 0) clockwise = true;
        else if (basePos >= 180) clockwise = false;

        if (clockwise) basePos += 5;
        else basePos -= 5;

        baseServo.write(basePos);
        delay(10);
      }

      // ultrasonic sensor code to detect item in range
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // math to detect distance
      duration = pulseIn(echoPin, HIGH);
      distance = (duration * .0343) / 2;

      // detect object within specified range and time
      if (distance >= LOW_RANGE && distance <= HIGH_RANGE) num_detections++;
      else num_detections = 0;

      Serial.print("Distance: ");
      Serial.println(distance);
      Serial.println("Num Detections: ");
      Serial.println(num_detections);
      delay(200);
    }

    num_detections = 0;
  } else if (Serial.available() > 0) {
    char command = Serial.read();
    change = true;

    Serial.print("Command received: ");
    Serial.println(command);

    if (command == 'a') {
      basePos -= 5;
      Serial.println("A pressed turning left 5 degrees!");
    } else if (command == 'd') {
      basePos += 5;
      Serial.println("D pressed turning right 5 degrees!");
    } else if (command == 's') {
      verticalPos -= 5;
      Serial.println("S pressed moving vertical down!");
    } else if (command == 'w') {
      verticalPos += 5;
      Serial.println("W pressed moving vertical up!");
    } else if (command == 'c') {
      clawPos = 90;
      Serial.println("C pressed closing claw!");
    } else if (command == 'o') {
      clawPos = 0;
      Serial.println("O pressed opening claw!");
    }
  }

  if (basePos < 0) {
    basePos = 0;
  } else if (basePos > 180) {
    basePos = 180;
  }

  if (verticalPos > 180) {
    verticalPos = 180;
  } else if (verticalPos < 95) {
    verticalPos = 95;
  }

  if (clawPos < 0) {
    clawPos = 0;
  } else if (clawPos > 180) {
    clawPos = 180;
  }

  if (change) {
    baseServo.write(basePos);
    verticalServo.write(verticalPos);
    clawServo.write(clawPos);
    Serial.printf("Base value is %d", basePos);
    Serial.printf("Vertical value is %d", verticalPos);
    Serial.printf("Claw value is %d", clawPos);
    change = false;
  }
  
}
