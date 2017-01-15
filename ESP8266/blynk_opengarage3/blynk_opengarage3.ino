
#define BLYNK_PRINT Serial        // Comment this out to disable prints and save space
#include <stdio.h>
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WiFiUdp.h>
#include <FS.h>
#include <SimpleTimer.h>

#define HOSTNAME "ESP8266-"

/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

//*********************** Blynk ***************************************************************//
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "9a60b2e42d764b7ab9a83bdaa4456636";   // opengarage3
//char auth[] = "04fbed8623924761853ccb3d50527391";   // opengarage2
char auth[] = "9922b2579c48421f8a18a0068a6d8ccd";     // opengarage

//V0 is Pin 14
WidgetLED led(V1);
WidgetLCD lcd(V2);
// Attach virtual serial terminal to Virtual Pin V3
WidgetTerminal terminal(V3);

SimpleTimer timer;

char uptime_string[16];
String email_subject;

//************************** Just Some basic Definitions used for the Up Time LOgger ************//
long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int HighMillis=0;
int Rollover=0;

//
// garage control pin
//
//int garage_pin = 14;
int garage_state = 0;  /* 0: close; 1: open */

//
// HC-SR04
//
#define SR04_TRIG_PIN   12
#define SR04_ECHO_PIN   13
long sr04_echo = 0;
int door_open=0; 
int sr04_count_max=4; 
int sr04_count;
int door_open_dist = 65;


BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, sr04_echo);
  Serial.print("READ V0: ");
  Serial.println(sr04_echo);    
}

BLYNK_WRITE(V3)
{
  char buf[32];
  char *ptr=buf;

  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } 
  else if (String("sr04") == param.asStr()) {
    terminal.print("sr04_count_max=");terminal.println(sr04_count_max);
    terminal.print("sr04_count=");terminal.println(sr04_count);
  }
  else if (String("token") == param.asStr()) {
    terminal.println(auth);
  }
  else if (String("hostname") == param.asStr()) {
    terminal.println(WiFi.hostname());
  }
  else if (String("ip") == param.asStr()) {
    terminal.println(WiFi.localIP());
  }
  else {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
    if ( 0 == strncmp("sr04", param.asStr(), 4) ) {
      strncpy(buf, param.asStr(),31); buf[31]='\0';
      ptr +=5; // "sr04=10" or "sr04 10" both can set sr04_count_max value
      sr04_count_max=atoi(ptr);
    }
    else if ( 0 == strncmp("dist", param.asStr(), 4) ) {
      strncpy(buf, param.asStr(),31); buf[31]='\0';
      ptr +=5; // "dist=40" or "sr04 40" both can set door_open_dist value in cm
      door_open_dist=atoi(ptr);
    }
  }

  // Ensure everything is sent
  terminal.flush();
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
  Serial.println(uptime_string);
}

//******************************  Boot Setup **************************************//
void setup()
{
  Serial.begin(115200);

  delay(100);

  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);

  // Print hostname.
  Serial.println("Hostname: " + hostname);
  //Serial.println(WiFi.hostname());

  // start wifi manager
  WiFiManager wifiManager;
  //first parameter is name of local access point, second is the password
  wifiManager.autoConnect(HOSTNAME, "esp8266!");
  // After ESP starts, it will try to connect to WiFi. If it fails it starts in Access Point mode. While in AP mode, 
  // connect to it then open a browser to the gateway IP, default 192.168.4.1, configure wifi, save and 
  // it should reboot and connect. 
  
  //Blynk.begin(auth, "ATT283", "2512825926");
  Blynk.config(auth);

  unsigned long startTime = millis();
  while (Blynk.connect() == false && millis() - startTime < 60000) {
    // Wait until connected
    Serial.write("Waiting ... for Blynk ...");
    delay(500);
  }
  Serial.println();
  delay(5000);
  // Check Blynk connection
  if (Blynk.connect() == false ) {
    Serial.write("Can not connect with Blynk ... rebooting ...");
    delay(3000);
    ESP.restart(); 
  }
  
  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("-------------");
  terminal.println("opengarage3 sketch");
  terminal.flush();

  //pinMode(garage_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
  led.off();

  email_subject = String(WiFi.hostname());
  email_subject += " - GARAGE DOOR OPEN!";

  timer.setInterval(2000, check_door_status);
}

void check_door_status()
{
  uptime(); //Runs the uptime script located below the main loop and reenters the main loop
  print_Uptime();

  sr04_echo = sr04_check();
  Serial.print("sr04_echo: ");
  Serial.print(sr04_echo);
  Serial.println(" cm");
//  Serial.println("#################");  
  if ( sr04_echo < door_open_dist && door_open == 0 ) { // door open detected first time 
    sr04_count++;
    if ( sr04_count >= sr04_count_max ) {  // for some reason, sr04 can randomly measured distance less then 65cm even though when it should be more than 200cm
                                          // if for sr04_count_max times, it measures less than 65cm, treat it as valid values
      door_open = 1;
      led.on();
      
      Blynk.email("donaldyan@gmail.com",  email_subject.c_str(), "Garage door has been opened.");
      //Blynk.email("donaldyan@yahoo.com",  "GARAGE DOOR 2 OPEN!", "Garage door has been opened.");
      Serial.println("email sent!");      
    }
  }
  else if ( sr04_echo >= door_open_dist && door_open == 1 ) { // door close detected first time
    door_open = 0;
    led.off();
    sr04_count = 0;
  } else {
    sr04_count = 0;
  }
}

//*************** MAIN LOOP *********************************************************//
void loop()
{  
  Blynk.run();
  timer.run();

//  Serial.print("garage_pin: ");
//  Serial.println(garage_state);

//  delay(1000);

//  yield();

}


