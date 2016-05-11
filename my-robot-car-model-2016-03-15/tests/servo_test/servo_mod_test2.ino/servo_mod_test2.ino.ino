#include <Servo.h>
Servo myservo;
void setup()
{
myservo.attach(9);
//myservo.write(0); //this goes backwards
myservo.write(180); // this goes forwards 
}
void loop() {
  
}
