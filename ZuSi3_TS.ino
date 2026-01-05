#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ZuSi3_TS_dashboard.h>
//#include "DEV_Config.h"

const char* configJson = "{\"ZuSi3_TS_config\":{\"hardware\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"klasse\":\"DynamischerStufenSchalter\",\"gpio\":{\"ena\":23,\"dir\":10,\"step\":11,\"sensor\":36},\"kalibrierung\":{\"min\":685,\"max\":3415}}]},\"baureihen\":{\"default\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"verwendung\":\"Fahrstufe\",\"tastaturZuordnung\":\"1\",\"stufen\":15}]},\"BR118\":{},\"BR154\":{}}}}";

const byte HELLO[] = {0,0,0,0,1,0,0,0,0,0,1,0,4,0,0,0,1,0,2,0,4,0,0,0,2,0,2,0,21,0,0,0,3,0,90,117,83,105,51,95,84,83,95,84,101,115,116,99,108,105,101,110,116,5,0,0,0,4,0,51,46,53,255,255,255,255,255,255,255,255};
const byte NEEDED_DATA[] = {0,0,0,0,2,0,0,0,0,0,3,0,0,0,0,0,10,0,4,0,0,0,1,0,1,0,255,255,255,255,255,255,255,255,255,255,255,255};
byte BEFEHL[] = {0,0,0,0,2,0,0,0,0,0,10,1,0,0,0,0,1,0,4,0,0,0,1,0,1,0,4,0,0,0,2,0,0,0,4,0,0,0,3,0,7,0,4,0,0,0,4,0,0,0,4,0,0,0,5,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255};

IPAddress HOST(192,168,178,65);
const int PORT = 1436;
//const String SSID = "Wokwi-GUEST";
//const String WPA2PWD =  "";
const String SSID = "FRITZ!Box 7590 BI";
const String WPA2PWD =  "28149516463916020556";

WiFiClient client;
ZuSi3_TS_DashBoard dashBoard;

int analogGPIOPinsLength = 0;
int analogGPIODataLength = 0;
int prevStufe = 0;
float prevAnalogValue = 0;
int motorHold = 0;

void setup() {
  Serial.begin(115200);
//  if (DEV_Module_Init() == 0) { } else { Serial.println("GPIO Init Fail!"); exit(0); }

/*  Serial.print("Connecting WiFi ");

  WiFi.begin(SSID, WPA2PWD);
  WiFi.mode(WIFI_STA);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  while (WiFi.localIP().toString() == "0.0.0.0") { delay(100); }

  Serial.println("IP: " + WiFi.localIP().toString());
  Serial.print("Gateway IP: "); Serial.println(WiFi.gatewayIP());
  Serial.print("Connecting to host ...");

  client.setTimeout(20000);
  
  if(!client.connect(HOST, PORT)())
  {
    Serial.println(" Connected");
  }
  else
  {
    Serial.println(" not Connected!");
  }

  for(int i=0; i<sizeof(HELLO); i++) 
  {
    client.write(HELLO[i]);
  }
  client.flush();
  for(int i=0; i<sizeof(NEEDED_DATA); i++)
  {
    client.write(NEEDED_DATA[i]);
  }
  client.flush();
*/
  dashBoard.SetConfig(configJson);
  analogGPIOPinsLength = sizeof(AnalogInGPIOPins) / sizeof(int);
  analogGPIODataLength = sizeof(AnalogInGPIOData) / sizeof(float);
  pinMode(23, OUTPUT);
}

void loop() 
{
  if(analogGPIODataLength > 0 & analogGPIOPinsLength > 0)
  {
    ZuSi3_TS_Control* schalter = dashBoard.Controls[0]; 

    AnalogInGPIOData[0] = analogRead(AnalogInGPIOPins[0]);
    schalter->Update();
    digitalWrite(DigitalOutGPIOPins[0], DigitalOutGPIOData[0]);

    byte stufe = schalter->GetWert();

    if(stufe != prevStufe)
    {
      prevStufe = stufe;
      BEFEHL[48] = stufe;

   Serial.print("Stufe: ");  Serial.println(stufe);

/*      client.flush();
      for(int i=0; i<sizeof(BEFEHL); i++)
      {
        client.write(BEFEHL[i]);
      }
*/
    }
  }
}
