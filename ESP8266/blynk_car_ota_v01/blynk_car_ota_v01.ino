#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEsp8266.h>

// Blynk related definitions
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7650f8a15d8240228500c64b088be41f";
// V0 - left wheel
// V1 - right wheel
// V2&V4 - sr04_echo
// V3 - mode switch MODE1 of run_mode
// V5 - Attach to virtual serial terminal
WidgetTerminal terminal(V5);

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

//
// The left and right servo
//
#define N 20
//#define N 1
int left_servo_pin = 16;
int right_servo_pin = 14;
int servoPosition = 1000; // position in microseconds

int left_value = 100;
int right_value = 100;
// mode 0: car move is controlled by blynk
// mode 1: car move is controlled by run_01
int run_mode = 0;
#define MODE1 0   // mode 1: bit 0 = 1

//
// HC-SR04
//
#define SR04_TRIG_PIN   12
#define SR04_ECHO_PIN   13
long sr04_echo = 0;


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
  //Blynk.begin(auth, "ATT283", "2512825926");
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

  pinMode(left_servo_pin, OUTPUT);
  pinMode(right_servo_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
}

void loop()
{
  Blynk.run();

  if ( run_mode & (1<<MODE1) ) run01();
  else {
    if ( (left_value == 255) && (right_value == 255) )
      Forward();
    else if ( (left_value == 0) && (right_value == 0) )
      Backward();
    else if ( left_value == 255 && right_value != 255 )
      RightRotate();
    else if ( left_value != 0 && right_value == 0 )
      RightRotate();
    else if ( left_value != 255 && right_value == 255 )
      LeftRotate();
    else if ( left_value == 0 && right_value != 0 )
      LeftRotate();
  }
  //delay(1000);
  delay(10);
  Serial.print("Left: ");
  Serial.println(left_value);    
  Serial.print("Right: ");
  Serial.println(right_value);    
  Serial.print("Run Mode: ");
  Serial.println(run_mode);
  Serial.print("sr04_echo: ");
  Serial.print(sr04_echo);
  Serial.println(" cm");
  Serial.println("#################");  
  //Handle OTA server.
  ArduinoOTA.handle();
  //yield();

}

//Left servo
BLYNK_WRITE(V0)
{   
  left_value = param.asInt(); // Get value as integer

//  Serial.print("Left: ");
//  Serial.println(left_value);    
}

//Right servo
BLYNK_WRITE(V1)
{   
  right_value = param.asInt(); // Get value as integer

//  Serial.print("Right: ");
//  Serial.println(right_value);    
}

BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, sr04_echo);
  Serial.print("READ V2: ");
  Serial.println(sr04_echo);    
}

//Mode 1 pin
BLYNK_WRITE(V3)
{   
  int value = param.asInt(); // Get value as integer

  Serial.print("Mode 1: ");
  Serial.println(value);

  if ( value == 1 ) run_mode = run_mode | (1<<MODE1);
  else {
     run_mode = run_mode & ~(1<<MODE1);
     sr04_echo=0;
  }
  Serial.print("Run Mode: ");
  Serial.println(run_mode);
}

BLYNK_READ(V4)
{
  Blynk.virtualWrite(V4, sr04_echo);
  Serial.print("READ V4: ");
  Serial.println(sr04_echo);    
}

BLYNK_WRITE(V5)
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

void run01()
{
  long distance = sr04_check();
  sr04_echo = distance;
  
  if ( distance < 20 ) {
    RightRotate();RightRotate();
  }
  else {
    Forward();Forward();
  }    
}
void Backward() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Backward: ");
    Serial.println(i);
  }
}

void Backward_right() {
  int i = 0;
   while(i < N) {
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(20); // wait 20 milliseconds
    i++;
    Serial.print("Backward_right: ");
    Serial.println(i);
  }
}

void Backward_left() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    delay(20); // wait 20 milliseconds
    i++;
    Serial.print("Backward_left: ");
    Serial.println(i);
  }
}

void Forward() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Forward: ");
    Serial.println(i);
  }
}

void Forward_right() {
  int i = 0;
   while(i < N) {
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Forward_right: ");
    Serial.println(i);
  }
}

void Forward_left() {
  int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(left_servo_pin, LOW);

    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("Forward_left: ");
    Serial.println(i);
  }
}

void RightRotate() { // vpravo
   int i = 0;
   while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(2000); // toci sa vpravo nikto netusi preco to je tak
    digitalWrite(right_servo_pin, LOW);
    digitalWrite(left_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("RightRotate: ");
    Serial.println(i);
  }
}

void LeftRotate() { // vlavo
    int i = 0;
    while(i < N) {
    digitalWrite(left_servo_pin, HIGH);
    digitalWrite(right_servo_pin, HIGH);
    delayMicroseconds(1000); // toci sa vlavo
    digitalWrite(left_servo_pin, LOW);
    digitalWrite(right_servo_pin, LOW);
    delay(30); // wait 20 milliseconds
    i++;
    Serial.print("LeftRotate: ");
    Serial.println(i);
  }
}

