#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>

/**
 * @brief mDNS and OTA Constants
 * @{
 */
#define HOSTNAME "ESP8266-OTA-" ///< Hostename. The setup function adds the Chip ID at the end.
/// @}

/**
 * @brief Default WiFi connection information.
 * @{
 */
const char* ap_default_ssid = "esp8266"; ///< Default SSID.
const char* ap_default_psk = "esp8266esp8266"; ///< Default PSK.
/// @}

/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

//*********************** Blynk ***************************************************************//
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "04fbed8623924761853ccb3d50527391";   // opengarage2
//char auth[] = "9922b2579c48421f8a18a0068a6d8ccd";     // opengarage

#define DOORBELL_PIN 14 
WidgetLED led(V1);
WidgetLCD lcd(V2);
// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V3);

char uptime_string[16];

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
//int garage_state = 0;  /* 0: close; 1: open */

int doorbell=0; 
int doorbell_pressed=0;


/**
 * @brief Read WiFi connection information from file system.
 * @param ssid String pointer for storing SSID.
 * @param pass String pointer for storing PSK.
 * @return True or False.
 * 
 * The config file have to containt the WiFi SSID in the first line
 * and the WiFi PSK in the second line.
 * Line seperator can be \r\n (CR LF) \r or \n.
 */
bool loadConfig(String *ssid, String *pass)
{
  // open file for reading.
  File configFile = SPIFFS.open("/cl_conf.txt", "r");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt.");

    return false;
  }

  // Read content from config file.
  String content = configFile.readString();
  configFile.close();
  
  content.trim();

  // Check if ther is a second line available.
  int8_t pos = content.indexOf("\r\n");
  uint8_t le = 2;
  // check for linux and mac line ending.
  if (pos == -1)
  {
    le = 1;
    pos = content.indexOf("\n");
    if (pos == -1)
    {
      pos = content.indexOf("\r");
    }
  }

  // If there is no second line: Some information is missing.
  if (pos == -1)
  {
    Serial.println("Infvalid content.");
    Serial.println(content);

    return false;
  }

  // Store SSID and PSK into string vars.
  *ssid = content.substring(0, pos);
  *pass = content.substring(pos + le);

  ssid->trim();
  pass->trim();

#ifdef SERIAL_VERBOSE
  Serial.println("----- file content -----");
  Serial.println(content);
  Serial.println("----- file content -----");
  Serial.println("ssid: " + *ssid);
  Serial.println("psk:  " + *pass);
#endif

  return true;
} // loadConfig


/**
 * @brief Save WiFi SSID and PSK to configuration file.
 * @param ssid SSID as string pointer.
 * @param pass PSK as string pointer,
 * @return True or False.
 */
bool saveConfig(String *ssid, String *pass)
{
  // Open config file for writing.
  File configFile = SPIFFS.open("/cl_conf.txt", "w");
  if (!configFile)
  {
    Serial.println("Failed to open cl_conf.txt for writing");

    return false;
  }

  // Save SSID and PSK.
  configFile.println(*ssid);
  configFile.println(*pass);

  configFile.close();
  
  return true;
} // saveConfig

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
}

//******************************  Boot Setup **************************************//
void setup()
{
  String station_ssid = "ATT283";
  String station_psk = "2512825926";

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


  // Initialize file system.
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load wifi connection information.
  if (! loadConfig(&station_ssid, &station_psk))
  {
    station_ssid = "";
    station_psk = "";

    Serial.println("No WiFi connection information available.");
  }

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  // ... Compare file config with sdk config.
  if (WiFi.SSID() != station_ssid || WiFi.psk() != station_psk)
  {
    Serial.println("WiFi config changed.");

    // ... Try to connect to WiFi station.
    WiFi.begin(station_ssid.c_str(), station_psk.c_str());

    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());

    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial);
  }
  else
  {
    // ... Begin with sdk config.
    WiFi.begin();
  }

  Serial.println("Wait for WiFi connection.");

  // ... Give ESP 60 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 60000)
  {
    Serial.write('.');
    //Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();

  // Check connection
  if(WiFi.status() == WL_CONNECTED)
  {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Can not connect to WiFi station. Go into AP mode.");
    
    // Go into software AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

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

  Blynk.config(auth);

  startTime = millis();
  while (Blynk.connect() == false && millis() - startTime < 60000) {
    // Wait until connected
    Serial.write("Waiting ... for Blynk ...");
    delay(500);
  }
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
  terminal.println("opengarage2_ota sketch");
  terminal.flush();

  // Send e-mail when your hardware gets connected to Blynk Server
  // Just put the recepient's "e-mail address", "Subject" and the "message body"
  Blynk.email("your_email@mail.com", "Subject", "My Blynk project is online.");

  // Setting the doorbell button
  pinMode(DOORBELL_PIN, INPUT_PULLUP);
  // Attach doorbell pin interrupt to our handler
  attachInterrupt(digitalPinToInterrupt(DOORBELL_PIN), emailOnButtonPress, CHANGE);
  
  led.off();
}

void emailOnButtonPress()
{
  // *** WARNING: You are limited to send ONLY ONE E-MAIL PER MINUTE! ***

  // Let's send an e-mail when you press the button
  // connected to digital pin 2 on your Arduino

  int isButtonPressed = !digitalRead(DOORBELL_PIN); // Invert state, since button is "Active LOW"

  if (isButtonPressed) // You can write any condition to trigger e-mail sending
  {
    if ( doorbell_pressed == 0 ) {
      Serial.println("Doorbell is pressed."); // This can be seen in the Serial Monitor
      Blynk.email("donaldyan@gmail.com", "Subject: Doorbell Logger", "Someone at the door just pushed the doorbell...");
      doorbell = 1;
      led.on();
    }
    doorbell_pressed++;    
  }
  else {
    doorbell_pressed=0;
    led.off();
  }
}

//*************** MAIN LOOP *********************************************************//
void loop()
{  
  Blynk.run();

  uptime(); //Runs the uptime script located below the main loop and reenters the main loop
  print_Uptime();

  // Handle OTA server.
  ArduinoOTA.handle();

  delay(1000);
}



