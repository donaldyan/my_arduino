/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

#define INIT_POS  70  // initial position
#define START_POS 0
#define END_POS   150
#define STEP      1

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 71;    // variable to store the servo position

void setup() {
  myservo.attach(10);  // attaches the servo on pin 9 to the servo object
  myservo.write(INIT_POS);
  delay(2000);
 }

void loop () {

  loop1();
//  loop2();
}
void loop1() {
    
  for (pos = INIT_POS; pos <= END_POS; pos += STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = END_POS; pos >= START_POS; pos -= STEP) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = START_POS; pos <= INIT_POS; pos += STEP) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
}

void loop2() {

  //myservo.write(75);
  //delay(3000);
    
  for (pos = 0; pos <= 150; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 150; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(5000);
}

