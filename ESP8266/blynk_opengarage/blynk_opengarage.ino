#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "9922b2579c48421f8a18a0068a6d8ccd";

//
// garage control pin
//
//int garage_pin = 14;
//int garage_state = 0;  /* 0: close; 1: open */

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

//  pinMode(garage_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
}

void loop()
{
  Blynk.run();

//  Serial.print("garage_pin: ");
//  Serial.println(garage_state);

  sr04_echo = sr04_check();
  Serial.print("sr04_echo: ");
  Serial.print(sr04_echo);
  Serial.println(" cm");
  Serial.println("#################");  
//  delay(1000);
}

BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, sr04_echo);
  Serial.print("READ V0: ");
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

//void run01()
//{
//  long distance = sr04_check();
//  sr04_echo = distance;  
//}

