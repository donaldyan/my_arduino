
#include <Servo.h>

#define SM_INIT_POS  75  // initial position
#define SM_START_POS 30
#define SM_END_POS   120
#define SM_STEP      1

Servo sm_servo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int sm_pos = 71;    // variable to store the servo position

#define N 20
int servoPinOne = 5;
int servoPinTwo = 6;

int servoPosition = 1000; // position in microseconds


void setup() {
  sm_servo.attach(9);  // attaches the servo on pin 10 to the servo object
  sm_servo.write(SM_INIT_POS);
  delay(2000);
  pinMode(servoPinOne, OUTPUT);
  pinMode(servoPinTwo, OUTPUT);
  Serial.begin(9600); 

 }

void loop () {

  sm_servo_test();
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
void sm_servo_test() {
    
  for (sm_pos = SM_INIT_POS; sm_pos <= SM_END_POS; sm_pos += SM_STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(2000);
  for (sm_pos = SM_END_POS; sm_pos >= SM_START_POS; sm_pos -= SM_STEP) { // goes from 180 degrees to 0 degrees
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(2000);
  for (sm_pos = SM_START_POS; sm_pos <= SM_INIT_POS; sm_pos += SM_STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    sm_servo.write(sm_pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void Forward() {
  int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinOne, LOW);
    
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void Backward() {
  int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinOne, LOW);
    
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void RightRotate() { // vpravo
   int i = 0;
   while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(servoPinTwo, LOW);
    digitalWrite(servoPinOne, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}

void LeftRotate() { // vlavo
    int i = 0;
    while(i < N) {
    digitalWrite(servoPinOne, HIGH);
    digitalWrite(servoPinTwo, HIGH);
    delayMicroseconds(1000); // toci sa vlavo
    digitalWrite(servoPinOne, LOW);
    digitalWrite(servoPinTwo, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.println(i);
  } 
}


