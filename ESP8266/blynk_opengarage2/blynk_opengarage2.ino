#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "04fbed8623924761853ccb3d50527391";   // opengarage2
char auth[] = "9922b2579c48421f8a18a0068a6d8ccd";     // opengarage

WidgetLED led(V1);
WidgetLCD lcd(V2);
char uptime_string[16];

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
int door_open=0; 
int sr04_count_max=3; 



//************************** Just Some basic Definitions used for the Up Time LOgger ************//
long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int HighMillis=0;
int Rollover=0;

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, "ATT283", "2512825926");

//  pinMode(garage_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
  led.off();
}

void loop()
{
  int sr04_count;
  
  Blynk.run();

//  Serial.print("garage_pin: ");
//  Serial.println(garage_state);

  uptime(); //Runs the uptime script located below the main loop and reenters the main loop
  print_Uptime();

  sr04_echo = sr04_check();
  Serial.print("sr04_echo: ");
  Serial.print(sr04_echo);
  Serial.println(" cm");
//  Serial.println("#################");  
  if ( sr04_echo < 65 && door_open == 0 ) { // door open detected first time 
    sr04_count++;
    if ( sr04_count == sr04_count_max ) {  // for some reason, sr04 can randomly measured distance less then 65cm even though when it should be more than 200cm
                                          // if for sr04_count_max times, it measures less than 65cm, treat it as valid values
      door_open = 1;
      led.on();
      Blynk.email("donaldyan@gmail.com", "GARAGE DOOR OPEN!", "Garage door has been opened.");
    }
  }
  else if ( sr04_echo >= 65 && door_open == 1 ) { // door close detected first time
    door_open = 0;
    led.off();
    sr04_count = 0;
  }

  delay(1000);
}

BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, sr04_echo);
  Serial.print("READ V0: ");
  Serial.println(sr04_echo);    
}

// there seems like a probelm with float type calculation in esp8266. and wifi/socket always got disconnected when distance is more than 5cm.
// change from 29.1 to 29 instead
int sr04_check() {
  int duration, distance;

  digitalWrite(SR04_TRIG_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(SR04_TRIG_PIN, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(SR04_TRIG_PIN, LOW);
  duration = pulseIn(SR04_ECHO_PIN, HIGH);
  distance = (duration/2)/29.1;

//  Serial.print("sr04_check: ");
//  Serial.print(distance);
//  Serial.println(" cm");

  if (distance >= 200 || distance <= 0)
    Serial.println("Out of range");
  
//  delay(100);

  return distance;
}

//************************ Uptime Code - Makes a count of the total up time since last start ****************//

void uptime(){
  //** Making Note of an expected rollover *****//   
  if(millis()>=3000000000){ 
    HighMillis=1;
  }

  //** Making note of actual rollover **//
  if(millis()<=100000&&HighMillis==1){
    Rollover++;
    HighMillis=0;
  }

  long secsUp = millis()/1000;

  Second = secsUp%60;
  Minute = (secsUp/60)%60;
  Hour = (secsUp/(60*60))%24;
  Day = (Rollover*50)+(secsUp/(60*60*24));  //First portion takes care of a rollover [around 50 days]
                       
}

//******************* Prints the uptime to serial window **********************//
void print_Uptime(){
  
//  Serial.print(F("Uptime: ")); // The "F" Portion saves your SRam Space
//  Serial.print(Day);
//  Serial.print(F("  Days  "));
//  Serial.print(Hour);
//  Serial.print(F("  Hours  "));
//  Serial.print(Minute);
//  Serial.print(F("  Minutes  "));
//  Serial.print(Second);
//  Serial.println(F("  Seconds"));
  lcd.print(1,0, "Uptime:" );
  sprintf(uptime_string, "%02d:%02d:%02d:%02d", Day, Hour, Minute, Second);
  lcd.print(2,1, uptime_string);
}
