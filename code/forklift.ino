#include <Stepper.h>
#include <Servo.h>

const int stepsPerRevolution = 2048;

Stepper myStepper = Stepper(stepsPerRevolution, 7, 5, 6, 4);
Servo myServo;

const int trigPin = 9;  
const int echoPin = 10;
const int dropItPin = 8;
const int forkliftFloorPin = 12;
float duration, distance;

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);
  pinMode(forkliftFloorPin, INPUT);
  pinMode(dropItPin, INPUT);
  pinMode(11, OUTPUT);
	Serial.begin(9600);
  myServo.attach(2);
}

void loop() {

  distance = 10.0;
  myServo.write(0);

  // Calibrate Forklift
  while (digitalRead(forkliftFloorPin) == HIGH){
    myStepper.setSpeed(10);
    myStepper.step(50);
    delay(500);
  }
  while (digitalRead(forkliftFloorPin) != HIGH){
    myStepper.setSpeed(10);
    myStepper.step(-50);
    delay(500);
  }
  myStepper.step(50);

  // Wait until the can is close
  while (distance >= 2.5) {
    senseDistance();
    digitalWrite(11, HIGH);
    delay(50);
    digitalWrite(11, LOW);
    delay(50);
  }

  // Turn on indicator light
  digitalWrite(11, HIGH);

  // Let wheels drive forward into the can
  delay(2000);

  // Lock gate
  myServo.write(90);
  // Raise forklift
  myStepper.setSpeed(5);
  myStepper.step(600);

  // Wait for button press
  while (digitalRead(dropItPin) != HIGH) {
    delay(100);
  }

  // Drop that thing
  myStepper.setSpeed(10);
  myStepper.step(-600);
  myServo.write(0);

  // Wait until the can is far
  while (distance <= 2.5 || distance >= 10.0) {
    senseDistance();
    delay(100);
  }

  // Turn off indicator light
  digitalWrite(11, LOW);
}

void senseDistance() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin, LOW);  
	delayMicroseconds(2);  
	digitalWrite(trigPin, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  Serial.print("Distance: ");  
	Serial.println(distance);
}
