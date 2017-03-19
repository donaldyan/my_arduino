#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <stdio.h>
#include <ESP8266WiFi.h>
//#include "fauxmoESP.h"
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define HOSTNAME "ESP8266-OTA-"
#define OPENGARAGE_VERSION "openswitch01_ota"           
                                                                                                               
/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

//fauxmoESP fauxmo;

//*********************** Blynk ***************************************************************//
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//define your default values here, if there are different values in config.json, they are overwritten.
char blynk_token[34] = "9db8f76084db4ba7a0bb141412b2c4f2";
//char auth[] = "9db8f76084db4ba7a0bb141412b2c4f2";   // openswitch - test

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// V0: is phy Pin 2
// V2: attach LCD to virtual pin V2
WidgetLCD lcd(V2);
// V3: Attach virtual serial terminal to Virtual Pin V3
WidgetTerminal terminal(V3);

WiFiManager wifiManager;
  
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
// relay control pin
//
int relay_pin = 2;
int relay_state = 0;  /* 0: close; 1: open */

BLYNK_WRITE(V0)
{
  int pinData = param.asInt(); 
  Serial.printf("Button pressed: %d\t - ", pinData);
  if ( pinData == 1 ) {
    Serial.printf("On\n");
    digitalWrite(relay_pin, HIGH);
  }
  else {
    Serial.printf("Off\n");
    digitalWrite(relay_pin, LOW);
  }
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
  }

  // Ensure everything is sent
  terminal.flush();
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
  wifiManager.setTimeout(300);

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

  pinMode(relay_pin, OUTPUT);
  //delay(1000);digitalWrite(relay_pin, LOW);
   
  email_subject = String(WiFi.hostname());
  email_subject += " - GARAGE DOOR OPEN!";

  // Fauxmo
  //fauxmo.addDevice("light one");
  //fauxmo.addDevice("light two");
  //fauxmo.addDevice("light three");
  //fauxmo.addDevice("light four");
  
  // fauxmoESP 2.0.0 has changed the callback signature to add the device_id, this WARRANTY
  // it's easier to match devices to action without having to compare strings.
  //fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
  //  Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    //digitalWrite(relay_pin, !state);
  //  });

}

//*************** MAIN LOOP *********************************************************//
void loop()
{  
  Blynk.run();

  // Handle OTA server.
  ArduinoOTA.handle();

  // Since fauxmoESP 2.0 the library uses the "compatibility" mode by
  // default, this means that it uses WiFiUdp class instead of AsyncUDP.
  // The later requires the Arduino Core for ESP8266 staging version
  // whilst the former works fine with current stable 2.3.0 version.
  // But, since it's not "async" anymore we have to manually poll for UDP
  // packets
  //fauxmo.handle();


  //static unsigned long last = millis();
  //if (millis() - last > 5000) {
  //    last = millis();
  //    Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  //}


}


