/*
 * HC-SR04 example sketch
 *
 * https://create.arduino.cc/projecthub/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-036380
 *
 * by Isaac100
 */

#include <ESP32Servo.h>
#include <XboxSeriesXControllerESP32_asukiaaa.hpp>

// global variables for base servo stuff
Servo baseServo;
Servo verticalServo;
Servo clawServo;

XboxSeriesXControllerESP32_asukiaaa::Core xboxController;
// create Servo object to control a servo
// twelve Servo objects can be created on most boards

// variable to store the servo basePosition
int basePos = 0;
bool clockwise = true;
bool toggle = false;

bool automatic = false;

const int baseServoPin = 3;
const int verticalServoPin = 2;
const int clawServoPin = 4;

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
  // base servo setup
  baseServo.attach(baseServoPin);          // attaches the servo on baseServoPin to the Servo object
  verticalServo.attach(verticalServoPin);  // attaches the servo on baseServoPin to the Servo object
  clawServo.attach(clawServoPin);          // attaches the servo on baseServoPin to the Servo object

  // ultrasonic sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // starts idk
  // Serial.begin(BAUD);  // 9600 baud
  Serial.begin(115200);
  // xboxController.begin();
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
  }else if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'a') {
      basePos-=5;
      Serial.println("A pressed turning left 5 degrees!");
    }else if (command == 'd') {
      basePos+=5;
      Serial.println("D pressed turning right 5 degrees!");
    }else if (command == 's') {
      verticalPos-=5;
    }else if (command == 'w') {
      verticalPos +=5;
    }
  }

  baseServo.write(basePos);
  verticalServo.write(verticalPos);
}
