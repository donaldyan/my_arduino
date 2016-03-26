// http://iot-playground.com/2-uncategorised/38-esp8266-and-arduino-ide-blink-example
// https://www.hackster.io/rayburne/esp8266-01-using-arduino-ide-67a124

#include "ESP8266WiFi.h"

//char* password = "WiFi Password/Phrase Here";
char* password = "2512825926";
//char* ssid     = "SSID of your wireless router";
char* ssid     = "ATT283";
//String MyNetworkSSID = "Repeat of SSID here"; // SSID you want to connect to Same as SSID
String MyNetworkSSID = "ATT283"; // SSID you want to connect to Same as SSID
bool Fl_MyNetwork = false; // Used to flag specific network has been found
bool Fl_NetworkUP = false; // Used to flag network connected and operational.

extern "C" {
#include "user_interface.h"
}

//
// The left and right servo
//
#define N 20
int left_servo_pin = 16;
int right_servo_pin = 14;

int servoPosition = 1000; // position in microseconds

//
// HC-SR04
//
#define SR04_TRIG_PIN   12
#define SR04_ECHO_PIN   13



void setup() {
  Serial.begin(115200);
  delay(2000); // wait for uart to settle and print Espressif blurb..
  // print out all system information
  Serial.print("Heap: "); Serial.println(system_get_free_heap_size());
  Serial.print("Boot Vers: "); Serial.println(system_get_boot_version());
  Serial.print("CPU: "); Serial.println(system_get_cpu_freq());
  Serial.println();

  //setup GPIO02 to turn on/off LED
  pinMode(2, OUTPUT);

  pinMode(left_servo_pin, OUTPUT);
  pinMode(right_servo_pin, OUTPUT);
  pinMode(SR04_ECHO_PIN, INPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);

  Serial.println("Setup done");
}


void loop()
{
#if 0
  if (!Fl_NetworkUP)
  {
    Serial.println("Starting Process Scanning...");
    Scan_Wifi_Networks();
    delay(2000);

    if (Fl_MyNetwork)
    {
      // Yep we have our network lets try and connect to it..
      Serial.println("MyNetwork has been Found....");
      Serial.println("Attempting Connection to Network..");
      Do_Connect();

      if (Fl_NetworkUP)
      {
        // Connection success
        Serial.println("Connected OK");
      }
      else
      {
        // Connection failure
        Serial.println("Not Connected");
      }
    }
    else
    {
      // Nope my network not identified in Scan
      Serial.println("Not Connected");
    }
  }
  else {
    Serial.println("Blinking LED ...");
    digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);              // wait for a second
  }
#endif
  
  //motor_servo_test();
  sr04_test();

  delay(3000);    // Wait a little before trying again
}

void Scan_Wifi_Networks()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // Need to be in dicsonected mode to Run network Scan!
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Scaning Networks Complete..");
  Serial.print(n); Serial.println(" Networks have been Found");

  if (n == 0)
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      if (MyNetworkSSID == WiFi.SSID(i))

      {
        Serial.print("My network has been Found! ");
        Fl_MyNetwork = true;
      }
      else
      {
        Serial.print("Not my network... ");
      }
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}


void Do_Connect()                  // Try to connect to the Found WIFI Network!
{
  Serial.println();
  Serial.print("Connecting to ");  // My network
  Serial.println(ssid);

  WiFi.begin(ssid, password);      // attempt login

  for (int i = 0; i < 10; i++)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Fl_NetworkUP = false;
    }
    else
    {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      Fl_NetworkUP = true;
      return;
    }
    delay(500);
    Serial.print(".");
  }
}

void sr04_test() {
  long duration, distance;
  digitalWrite(SR04_TRIG_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(SR04_TRIG_PIN, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(SR04_TRIG_PIN, LOW);
  duration = pulseIn(SR04_ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;
  //distance = ( (distance >>1) *10)/291;
  if (distance < 4) {  // This is where the LED On/Off happens/
//    digitalWrite(led,HIGH); // When the Red condition is met, the Green LED should turn off
//    digitalWrite(led2,LOW);
}
  else {
//    digitalWrite(led,LOW);
//    digitalWrite(led2,HIGH);
  }
  if (distance >= 200 || distance <= 0){
    Serial.println("Out of range");
  }
  else {
    Serial.print(distance);
    Serial.println(" cm");
  }
  delay(500);
}

void motor_servo_test() {

  //Backwarf_right();
  //Forward_right();
  //Backward_left();
  //Forward_left();
  Forward();
  delay(3000);
  Backward();
  delay(3000);
  RightRotate();
  delay(3000);
  LeftRotate();
  delay(3000);
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
    Serial.println(i);
  }
}

