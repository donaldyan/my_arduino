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
int left_servo_pin = 16;
int right_servo_pin = 14;
int servoPosition = 1000; // position in microseconds

int left_value = 100;
int right_value = 100;

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, "ATT283", "2512825926");

  pinMode(left_servo_pin, OUTPUT);
  pinMode(right_servo_pin, OUTPUT);
}

void loop()
{
  Blynk.run();

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

//Left servo
BLYNK_WRITE(V0)
{   
  left_value = param.asInt(); // Get value as integer

  Serial.print("Left: ");
  Serial.println(left_value);    
}

//Right servo
BLYNK_WRITE(V1)
{   
  right_value = param.asInt(); // Get value as integer

  Serial.print("Right: ");
  Serial.println(right_value);    
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

