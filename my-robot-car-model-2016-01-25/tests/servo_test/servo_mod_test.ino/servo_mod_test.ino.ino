
#include <Servo.h>

//
// The left and right servo
//
#define N 20
int left_servo_pin = 9; 

int servoPosition = 1000; // position in microseconds

void setup() {

  pinMode(left_servo_pin, OUTPUT);

  Serial.begin(115200); 

 }

void loop () {

  motor_servo_test();

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


}
void Forward() {
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

void Backward() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);
    
    delay(20); // wait 20 milliseconds
    i++;
    Serial.println(i);
  }
}


