#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <SdFat.h>
#include <ZuSi3_TS_dashboard.h>
//#include <esp_wifi.h>
//#include "DEV_Config.h"

#define SPI_SPEED SD_SCK_MHZ(4)
const int CS_PIN = 5;
SdFat32 sd;
WiFiClient wifiClient;
ZuSi3_TS_DashBoard dashBoard;
TaskHandle_t UpdateTaskHandle = NULL;

void setup() {
	Serial.begin(115200);
//	if (DEV_Module_Init() == 0) { } else { Serial.println("GPIO Init Fail!"); exit(0); }
//	ConnectToZuSi();

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
	
	char* configData = readFile("config.json");
	dashBoard.SetConfig(configData);
	
	for(int i = 0; i < dashBoard.AnalogInGPIOLength; i++)
	{
		pinMode(G_DigitalOutGPIOPins[i], OUTPUT);
	}
	
	xTaskCreatePinnedToCore(
		UpdateTask,				// Task function
		"UpdateTask",			// Task name
		10000,					// Stack size (bytes)
		NULL,						// Parameters
		1,							// Priority
		&UpdateTaskHandle,	// Task handle
									// Core
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
	JSONVar config = JSON.parse(configJson);
	if (JSON.typeof(config) == "undefined") { Serial.println("Parsing config json failed!"); return null; }
	
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
	
	dashBoard.InitNetwork(wifiClient);
}

char* readFile(String name)
{
	File32 file;
	
	if (!file.open(name, O_READ)) {
		sd.errorHalt("opening file '" + name + "' failed");
	}
	
	char* data;
	data = new char[file.size()];
	int n = 0;
	
	while (file.available()) 
	{
		char c = file.read();
		if((byte)c >= 32 )
			data[n++] = c;
	}
	file.close();
	
	return data;
}
