#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <ZuSi3_TS_dashboard.h>
#include "SdFat.h"
//#include <esp_wifi.h>
//#include "DEV_Config.h"

#define SPI_SPEED SD_SCK_MHZ(4)
const int CS_PIN = 5;
SdFat32 sd;
File32 configFile;
char* configData;

IPAddress HOST(192,168,178,65);
const int PORT = 1436;
//const String SSID = "Wokwi-GUEST";
//const String WPA2PWD =  "";
const String SSID = "FRITZ!Box 7590 BI";
const String WPA2PWD =  "28149516463916020556";
WiFiClient client;

ZuSi3_TS_DashBoard dashBoard;
TaskHandle_t UpdateTaskHandle = NULL;

void setup() {
  Serial.begin(115200);
//  if (DEV_Module_Init() == 0) { } else { Serial.println("GPIO Init Fail!"); exit(0); }
//  ConnectToZuSi();

  if (!sd.begin(CS_PIN, SPI_SPEED)) {
    Serial.println("Error sd.begin().");
    if (sd.card()->errorCode()) {
      Serial.println("SD initialization failed.");
    } else if (sd.vol()->fatType() == 0) {
      Serial.println("Can't find a valid FAT16/FAT32 partition.");
    } else {
      Serial.println("Can't determine error type");
    }
  }

  if (!configFile.open("config.json", O_READ)) {
    sd.errorHalt("opening config.json failed");
  }

  int size = configFile.size();
  configData = new char[size];
  int n = 0;
  
  while (configFile.available()) 
  {
    char c = configFile.read();
    if((byte)c >= 32 )
      configData[n++] = c;
  }
  configFile.close();
  
  dashBoard.SetConfig(configData);

  for(int i = 0; i < dashBoard.AnalogInGPIOLength; i++)
  {
    pinMode(G_DigitalOutGPIOPins[i], OUTPUT);
  }

  xTaskCreatePinnedToCore(
    UpdateTask,        // Task function
    "UpdateTask",      // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &UpdateTaskHandle, // Task handle
    1                  // Core
  );
}

void loop() 
{
}

void UpdateTask(void *parameter) {
  while(true) 
  {
    for(int i = 0; i < dashBoard.AnalogInGPIOLength; i++)
    {
      G_AnalogInGPIOData[i] = analogRead(G_AnalogInGPIOPins[i]);
    }

    dashBoard.Update();

    for(int i = 0; i < dashBoard.AnalogInGPIOLength; i++)
    {
      digitalWrite(G_DigitalOutGPIOPins[i], G_DigitalOutGPIOData[i]);
    }
  }
}

void ConnectToZuSi()
{
  Serial.print("Connecting WiFi ");

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

  if(!client.connect(HOST, PORT))
  {
    Serial.println(" Connected");
  }
  else
  {
    Serial.println(" not Connected!");
  }
}
