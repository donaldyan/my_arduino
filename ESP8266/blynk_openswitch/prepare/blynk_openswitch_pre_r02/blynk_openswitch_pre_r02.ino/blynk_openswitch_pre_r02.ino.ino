/*
 * http://community.blynk.cc/t/amazon-echo-esp8266-control-with-natural-speech-commands/10676
 * Witnessmenow's ESP8266 Wemo Emulator library is an awesome way to build an IoT device on 
 * the cheap, and its direct integration with Alexa / Echo means that voice control can
 * be done with natural sounding commands. However, it is limited by its reliance solely on
 * voice control, as it doesnt work with the official Wemo app. It also provided no affordance
 * for toggling the switch when an internet connection was unavailable. 
 * 
 * With just a bit of additional code, devices can be made controllable by the Blynk app, hardware 
 * switches, IFTTT event triggers,and of course, Alexa. Toss in OTA updates and Tzapulica's 
 * WiFiManager for easy provisioning, and you've got a really versatile, easy to use device. 
 * 
 * 
 * OTA updates are hardware dependent, but don't seem to cause any problems for devices
 * that don't support it.
 * 
 * Wemo Emulator and WiFi Manager libraries:
 * https://github.com/witnessmenow/esp8266-alexa-wemo-emulator
 * https://github.com/tzapu/WiFiManager
 * 
 * In order to control a multitude of devices with a single Blynk dashboard, each ESP8266
 * should be programmed with a unique virtual pin assignment, corresponding to a Blynk switch.
 * 
 * The onboard LED is set to ON when the relay is off. This made sense to me, if you're looking 
 * for the physical switch in a dark room. 
 * 
 * For IFTTT control, use the Maker Channel with the following settings:
 *    URL: http://blynk-cloud.com:8080/YOUR_TOKEN/V1       Substitute your own token and vitual pin 
 *    Method: PUT
 *    Content type: application/json
 *    Body: ["1"]                                          Use 1 for ON, 0 for OFF
 */
 
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>

#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

#define VPIN V1  //Use a unique virtual pin for each device using the same token / dashboard

char auth[] = "9db8f76084db4ba7a0bb141412b2c4f2"; //Get token from Blynk

//on/off callbacks
void lightOn();
void lightOff();

WemoManager wemoManager;
WemoSwitch *light = NULL;

boolean LampState = 0;
boolean SwitchReset = true;   //Flag indicating that the hardware button has been released

const int TacSwitch = 0;      //Pin for hardware momentary switch. On when grounded. Pin 0 on Sonoff
const int RelayPin = 2;      //Relay switching pin. Relay is pin 12 on the SonOff
const int LED = BUILTIN_LED;           //On / Off indicator LED. Onboard LED is 13 on Sonoff

SimpleTimer timer;

void setup()      
{
  Serial.begin(115200);

  WiFiManager wifi;   //WiFiManager intialization.
  wifi.autoConnect("FakeWemo"); //Create AP, if necessary

  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  //light = new WemoSwitch("SonOfWemo", 80, lightOn, lightOff);
  light = new WemoSwitch("test light", 80, lightOn, lightOff);
  wemoManager.addDevice(*light);

  //pinMode(RelayPin, OUTPUT);
  pinMode(LED, OUTPUT);
  //pinMode(TacSwitch, INPUT_PULLUP);
  delay(10);
  //digitalWrite(RelayPin, LOW);
  digitalWrite(LED, LOW);
  

  Blynk.config(auth);
  ArduinoOTA.begin();

  timer.setInterval(100, ButtonCheck);
}

void loop()
{
  wemoManager.serverLoop();
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}

// Toggle the relay on
void lightOn() {
    Serial.println("Switch 1 turn on ...");
    //digitalWrite(RelayPin, HIGH);
    digitalWrite(LED, HIGH);
    LampState = 1;
    Blynk.virtualWrite(VPIN, HIGH);     // Sync the Blynk button widget state
}

// Toggle the relay off
void lightOff() {
    Serial.println("Switch 1 turn off ...");
    //digitalWrite(RelayPin, LOW);
    digitalWrite(LED, LOW);
    LampState = 0;
    Blynk.virtualWrite(VPIN, LOW);      // Sync the Blynk button widget state
}

// Handle switch changes originating on the Blynk app
BLYNK_WRITE(VPIN){
  int SwitchStatus = param.asInt();

  Serial.println("Blynk switch activated");

  // For use with IFTTT, toggle the relay by sending a "2"
  if (SwitchStatus == 2){
    ToggleRelay();
  }
  else if (SwitchStatus){
    lightOn();
  }
  else lightOff();
}

// Handle hardware switch activation
void ButtonCheck(){
  // look for new button press
  boolean SwitchState = (digitalRead(TacSwitch));

  // toggle the switch if there's a new button press
  if (!SwitchState && SwitchReset == true){
    Serial.println("Hardware switch activated");
    if (LampState){
      lightOff();
    }
    else{
      lightOn();
    }

    // Flag that indicates the physical button hasn't been released
    SwitchReset = false;
    delay(50);            //debounce
  }
  else if (SwitchState){
    // reset flag the physical button release
    SwitchReset = true;
  }
}


void ToggleRelay(){
  LampState = !LampState;

  if (LampState){
    lightOn();
  }
  else lightOff();
}
