#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7650f8a15d8240228500c64b088be41f";

//
// The left and right servo
//
#define N 20
//#define N 1
int left_servo_pin = 16;
int right_servo_pin = 14;
int servoPosition = 1000; // position in microseconds

int left_value = 100;
int right_value = 100;
// mode 0: car move is controlled by blynk
// mode 1: car move is controlled by run_01
int run_mode = 0;
#define MODE1 0   // mode 1: bit 0 = 1

//
// HC-SR04
//
#define SR04_TRIG_PIN   12
#define SR04_ECHO_PIN   13
long sr04_echo = 0;

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, "ATT283", "2512825926");

  pinMode(left_servo_pin, OUTPUT);
  pinMode(right_servo_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
}

void loop()
{
  Blynk.run();

  if ( run_mode & (1<<MODE1) ) run01();
  else {
    if ( (left_value == 255) && (right_value == 255) )
      Forward();
    else if ( (left_value == 0) && (right_value == 0) )
      Backward();
    else if ( left_value == 255 && right_value != 255 )
      RightRotate();
    else if ( left_value != 0 && right_value == 0 )
      RightRotate();
    else if ( left_value != 255 && right_value == 255 )
      LeftRotate();
    else if ( left_value == 0 && right_value != 0 )
      LeftRotate();
  }
//  delay(1000);
  delay(10);
  Serial.print("Left: ");
  Serial.println(left_value);    
  Serial.print("Right: ");
  Serial.println(right_value);    
  Serial.print("Run Mode: ");
  Serial.println(run_mode);
  Serial.print("sr04_echo: ");
  Serial.print(sr04_echo);
  Serial.println(" cm");
  Serial.println("#################");  
}

//Left servo
BLYNK_WRITE(V0)
{   
  left_value = param.asInt(); // Get value as integer

//  Serial.print("Left: ");
//  Serial.println(left_value);    
}

//Right servo
BLYNK_WRITE(V1)
{   
  right_value = param.asInt(); // Get value as integer

//  Serial.print("Right: ");
//  Serial.println(right_value);    
}

BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, sr04_echo);
  Serial.print("READ V2: ");
  Serial.println(sr04_echo);    
}

//Left servo
BLYNK_WRITE(V3)
{   
  int value = param.asInt(); // Get value as integer

  Serial.print("Mode 1: ");
  Serial.println(value);

  if ( value == 1 ) run_mode = run_mode | (1<<MODE1);
  else {
     run_mode = run_mode & ~(1<<MODE1);
     sr04_echo=0;
  }
  Serial.print("Run Mode: ");
  Serial.println(run_mode);
}

BLYNK_READ(V4)
{
  Blynk.virtualWrite(V4, sr04_echo);
  Serial.print("READ V4: ");
  Serial.println(sr04_echo);    
}

long sr04_check() {
  long duration, distance;

  digitalWrite(SR04_TRIG_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(SR04_TRIG_PIN, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(SR04_TRIG_PIN, LOW);
  duration = pulseIn(SR04_ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;

  Serial.print("sr04_check: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance >= 200 || distance <= 0)
    Serial.println("Out of range");
  
  delay(100);

  return distance;
}

void run01()
{
  long distance = sr04_check();
  sr04_echo = distance;
  
  if ( distance < 20 ) {
    RightRotate();RightRotate();
  }
  else {
    Forward();Forward();
  }    
}
void Backward() {
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
    Serial.print("Backward: ");
    Serial.println(i);
  }
}

void Backward_right() {
  int i = 0;
   while(i < N) {
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(20); // wait 20 milliseconds
    i++;
    Serial.print("Backward_right: ");
    Serial.println(i);
  }
}

void Backward_left() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    delay(20); // wait 20 milliseconds
    i++;
    Serial.print("Backward_left: ");
    Serial.println(i);
  }
}

void Forward() {
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
    Serial.print("Forward: ");
    Serial.println(i);
  }
}

void Forward_right() {
  int i = 0;
   while(i < N) {
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Forward_right: ");
    Serial.println(i);
  }
}

void Forward_left() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Forward_left: ");
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
    Serial.print("RightRotate: ");
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
    Serial.print("LeftRotate: ");
    Serial.println(i);
  }
}

