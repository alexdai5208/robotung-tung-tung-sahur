#include <ESP32Servo.h>
#include <Bluepad32.h>

ControllerPtr controller = nullptr;

// global variables for base servo stuff
Servo baseServo;
Servo verticalServo;
Servo clawServo;

// variable to store the servo basePosition
int basePos = 0;
bool clockwise = true;
bool toggle = false;

bool change = false;

const int baseServoPin = 18;
const int verticalServoPin = 19;
const int clawServoPin = 21;

// 95 to 180 is the degrees 180 is straight up
int verticalPos = 180;
int clawPos = 0;

// global variables for ultrasonic sensor stuff
const int trigPin = 23;
const int echoPin = 22;
const int LOW_RANGE = 5;
const int HIGH_RANGE = 13;

float duration, distance;

int num_detections = 0;

void onConnectedController(ControllerPtr ctl) {
  controller = ctl;
  Serial.println("Controller connected!");
}

void onDisconnectedController(ControllerPtr ctl) {
  if (controller == ctl) {
    controller = nullptr;
    Serial.println("Controller disconnected.");
  }
}

void setup() {
  // base servo setup
  baseServo.attach(baseServoPin);
  verticalServo.attach(verticalServoPin);
  clawServo.attach(clawServoPin);

  // ultrasonic sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {
  BP32.update();

  if (!controller || !controller->isConnected()) return;

  if (controller->a()) {
    if (num_detections < 5) {
      if (basePos <= 0) clockwise = true;
      else if (basePos >= 180) clockwise = false;

      if (num_detections == 0) {
        change = true;
        if (clockwise) basePos += 5;
        else basePos -= 5;
      }

      delay(100);

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

      if (num_detections > 0) {
        Serial.print("Distance: ");
        Serial.println(distance);
        Serial.println("Num Detections: ");
        Serial.println(num_detections);
      }
    } else {
      delay(1000);

      clawPos = 0;
      verticalPos = 95;

      verticalServo.write(verticalPos);
      clawServo.write(clawPos);

      delay(1000);

      clawPos = 85;
      clawServo.write(clawPos);

      delay(2000);

      verticalPos = 180;
      verticalServo.write(verticalPos);

      delay(2000);
    }
  }else {
    int rx = controller->axisRX();
    int ly = controller->axisY();
    if (rx > 10 || rx < -10) {
      change = true;
    }else if (ly >10 || ly < -10) {
      change = true;
    }

    basePos += (rx / 100);
    verticalPos += (ly /100);

    if (controller->y()) {
      clawPos = 85;
      change = true;
    }else if (controller->x()) {
      clawPos = 0;
      change = true;
    }
  }

  if (Serial.available() > 0) {
    char command = Serial.read();
    change = true;

    Serial.print("Command received: ");
    Serial.println(command);

    if (command == 'a') {
      basePos += 5;
      Serial.println("A pressed turning left 5 degrees!");
    } else if (command == 'd') {
      basePos -= 5;
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
    } else if (command == 'z') {
      // automatic = true;
      change = true;
      clawPos = 0;
      verticalPos = 180;
      basePos = 0;
      num_detections = 0;
      Serial.println("Z pressed opening claw!");
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
  } else if (clawPos > 90) {
    clawPos = 90;
  }

  if (change) {
    if (abs(basePos - baseServo.read()) > 2) {
      baseServo.write(basePos);
    }
    verticalServo.write(verticalPos);
    clawServo.write(clawPos);
    change = false;
  }

  delay(50);
}
