// http://iot-playground.com/2-uncategorised/38-esp8266-and-arduino-ide-blink-example
// https://www.hackster.io/rayburne/esp8266-01-using-arduino-ide-67a124
// https://www.arduino.cc/en/Tutorial/WiFiWebServer

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

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(2000); // wait for uart to settle and print Espressif blurb..
  // print out all system information
  Serial.print("Heap: "); Serial.println(system_get_free_heap_size());
  Serial.print("Boot Vers: "); Serial.println(system_get_boot_version());
  Serial.print("CPU: "); Serial.println(system_get_cpu_freq());
  Serial.println();

  // setup GPIO02 to turn on/off LED
  pinMode(2, OUTPUT);

  Serial.println("Setup done");
}


void loop()
{
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
        server.begin();
      }
      else
      {
        // Connection failure
        Serial.println("Not Connected");
        delay(3000);    // Wait a little before trying again
      }
    }
    else
    {
      // Nope my network not identified in Scan
      Serial.println("Not Connected");
      delay(3000);    // Wait a little before trying again
    }
  }
  else {
    Serial.println("Blinking LED ...");
    digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);              // wait for a second

    // listen for incoming clients
    WiFiClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 5");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            // output the value of each analog input pin
            for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
              //int sensorReading = analogRead(analogChannel);
              client.print("analog input ");
              client.print(analogChannel);
              client.print(" is ");
              //client.print(sensorReading);
              client.print(" high.");
              client.println("<br />");       
            }
            client.println("</html>");
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } 
          else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      delay(1);
    
      // close the connection:
      client.stop();
      Serial.println("client disonnected");
    }
  }
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


