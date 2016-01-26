
#include <Servo.h>

//
// The middle servo(SM) driving HC-SR04
//
#define SM_PIN       9
#define SM_INIT_POS  75  // initial position 
#define SM_START_POS 30
#define SM_END_POS   120
#define SM_STEP      1

Servo sm_servo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int sm_pos = 0;    // variable to store the servo position

//
// The left and right servo
//
#define N 20
int left_servo_pin = 5; 
int right_servo_pin = 6; 

int servoPosition = 1000; // position in microseconds

//
// HC-SR04
//
#define SR04_TRIG_PIN   7 
#define SR04_ECHO_PIN   8 
#define led 11
#define led2 10
#define trigPin 7
#define echoPin 8

void setup() {
  sm_servo.attach(SM_PIN);  // attaches the servo on pin SM_PIN to the servo object
  sm_servo.write(SM_INIT_POS);
  delay(2000);

  pinMode(left_servo_pin, OUTPUT);
  pinMode(right_servo_pin, OUTPUT);

  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);

  Serial.begin(9600); 

 }

void loop () {

  sm_servo_test();

  motor_servo_test();

  //sr04_test();
}

void sm_servo_test() {

  sr04_test(); delay(1000);  
  for (sm_pos = SM_INIT_POS; sm_pos <= SM_END_POS; sm_pos += SM_STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(1000); sr04_test(); delay(1000);
  
  for (sm_pos = SM_END_POS; sm_pos >= SM_START_POS; sm_pos -= SM_STEP) { // goes from 180 degrees to 0 degrees
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(1000); sr04_test(); delay(1000);
  for (sm_pos = SM_START_POS; sm_pos <= SM_INIT_POS; sm_pos += SM_STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void sr04_test() {
  long duration, distance;
  digitalWrite(SR04_TRIG_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(SR04_TRIG_PIN, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(SR04_TRIG_PIN, LOW);
  duration = pulseIn(SR04_ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;
  //distance = ( (distance >>1) *10)/291;
  if (distance < 4) {  // This is where the LED On/Off happens/
//    digitalWrite(led,HIGH); // When the Red condition is met, the Green LED should turn off
//    digitalWrite(led2,LOW);
}
  else {
//    digitalWrite(led,LOW);
//    digitalWrite(led2,HIGH);
  }
  if (distance >= 200 || distance <= 0){
    Serial.println("Out of range");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);
}

void motor_servo_test() {

  Forward();
  Forward();
  Forward();
  delay(2000);

  Backward();
  Backward();
  Backward();
  delay(2000);

  RightRotate();
  delay(2000);
  
  LeftRotate();
  delay(2000);

}
void Forward() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);
    
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void Backward() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);
    
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void RightRotate() { // vpravo
   int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    digitalWrite(left_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void LeftRotate() { // vlavo
    int i = 0;
    while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vlavo
    digitalWrite(left_servo_pin, LOW);
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  } 
}


