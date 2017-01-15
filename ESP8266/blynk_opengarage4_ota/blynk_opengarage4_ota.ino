#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define HOSTNAME "ESP8266-OTA-"
#define OPENGARAGE_VERSION "opengarage4_ota"           
                                                                                                               
/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

//*********************** Blynk ***************************************************************//
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//define your default values here, if there are different values in config.json, they are overwritten.
//char mqtt_server[40];
//char mqtt_port[6] = "8080";
char blynk_token[34] = "9a60b2e42d764b7ab9a83bdaa4456636";
//char auth[] = "9a60b2e42d764b7ab9a83bdaa4456636";   // opengarage3-device03 - test
//char auth[] = "04fbed8623924761853ccb3d50527391";   // opengarage2-device02 - lily
//char auth[] = "9922b2579c48421f8a18a0068a6d8ccd";   // opengarage-device01 - whirlaway

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

//V0 is Pin 14
WidgetLED led(V1);
WidgetLCD lcd(V2);
// Attach virtual serial terminal to Virtual Pin V3
WidgetTerminal terminal(V3);

WiFiManager wifiManager;
  
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
  else if (String("version") == param.asStr()) {
    terminal.println(OPENGARAGE_VERSION);
  }
  else if (String("token") == param.asStr()) {
    //terminal.println(auth);
    terminal.println(blynk_token);
  }
  else if (String("hostname") == param.asStr()) {
    terminal.println(WiFi.hostname());
  }
  else if (String("ip") == param.asStr()) {
    terminal.println(WiFi.localIP());
  }
  else if (String("reboot=y") == param.asStr()) {
    terminal.println("ESP will be rebooted in three seconds ...");
    delay(3000);
    ESP.restart();
  }
  else if (String("reset=y") == param.asStr()) {
    terminal.println("ESP wifi credential will be cleared and ESP will be rebooted ...");
    delay(2000);
    wifiManager.resetSettings();
    ESP.restart();
//    Serial.println("\nFormatting SPIFFS please wait .....");  
//    SPIFFS.format(); //clean FS, for testing
//    Serial.println("SPIFFS was formatted");
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
  WiFi.hostname(hostname);

  // Print hostname.
  Serial.println("Hostname: " + hostname);
  //Serial.println(WiFi.hostname());

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          //strcpy(mqtt_server, json["mqtt_server"]);
          //strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  //WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  //WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33); //should be 33 not 32, otherwise missing the last char

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  //WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  //wifiManager.addParameter(&custom_mqtt_server);
  //wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(HOSTNAME, "esp8266!")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  
  // start wifi manager
  //first parameter is name of local access point, second is the password
  //wifiManager.autoConnect(HOSTNAME, "esp8266!");
  // After ESP starts, it will try to connect to WiFi. If it fails it starts in Access Point mode. While in AP mode, 
  // connect to it then open a browser to the gateway IP, default 192.168.4.1, configure wifi, save and 
  // it should reboot and connect. 

  //read updated parameters
  //strcpy(mqtt_server, custom_mqtt_server.getValue());
  //strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());
  Serial.println(blynk_token);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    //json["mqtt_server"] = mqtt_server;
    //json["mqtt_port"] = mqtt_port;
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.println("");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  // Start OTA server.
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");
  ArduinoOTA.setHostname((const char *)hostname.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  //Blynk.begin(auth, "ATT283", "2512825926");
  //Blynk.config(auth);
  Blynk.config(blynk_token);

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
    wifiManager.resetSettings();
    ESP.restart();
  }
  
  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println("-------------");
  terminal.print(OPENGARAGE_VERSION); terminal.println(" sketch");
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

  // Handle OTA server.
  ArduinoOTA.handle();
//  yield();

}


