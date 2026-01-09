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

char* sysConfig = "{\"system\":{\"wifi\":{\"SSID\":\"FRITZ!Box 7590 BI\",\"password\":\"28149516463916020556\"}}}";
char* dashConfig = "{\"ZuSi3_TS_config\":{\"system\":{\"clientName\":\"ZuSi3_TS_Dashboard\",\"server\":{\"ipAddresse\":\"192.168.178.65\",\"portNummer\":1436}},\"hardware\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"klasse\":\"DynamischerStufenSchalter\",\"gpio\":{\"ena\":0,\"dir\":0,\"step\":0,\"sensor\":35},\"kalibrierung\":{\"min\":685,\"max\":3415}}]},\"baureihen\":{\"default\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"verwendung\":\"Fahrstufe\",\"tastaturZuordnung\":\"1\",\"stufen\":15}]},\"BR118\":{},\"BR154\":{}}}}";

void setup() {
	Serial.begin(115200);

    pinMode(36, OUTPUT);
	digitalWrite(36, HIGH);

	ConnectWifi();

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
	
	char* configData = readFile("ZuSi3_TS_config.json");

	dashBoard.Init(configData, &wifiClient);
	
	for(int i = 0; i < dashBoard.DigitalOutGPIOLength; i++)
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
		1							// Core
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
			Serial.print("G_AnalogInGPIOData[");Serial.print(i);Serial.print("] = ");Serial.println(G_AnalogInGPIOData[i]);	delay(100);
		}
		
		dashBoard.Update();
		
		for(int i = 0; i < dashBoard.AnalogInGPIOLength; i++)
		{
			digitalWrite(G_DigitalOutGPIOPins[i], G_DigitalOutGPIOData[i]);
		}
	}
}

void ConnectWifi()
{
	char* configJson = readFile("systemConfig.json");
	JSONVar config = JSON.parse(configJson);
	if (JSON.typeof(config) == "undefined") { Serial.println("Parsing sysemConfig.json failed!"); return; }
	
	String SSID = config["system"]["wifi"]["SSID"];
	String pwd = config["system"]["wifi"]["password"];

	Serial.println();
	Serial.print("WiFi SSID: "); Serial.print(SSID); Serial.print("; password: "); Serial.println(pwd);
	Serial.print("Connecting WiFi ");

	WiFi.begin(SSID, pwd);
	
	while (WiFi.status() != WL_CONNECTED) 
	{
		delay(100);
		Serial.print(".");
	}

	Serial.println(" Connected!");
	
	while (WiFi.localIP().toString() == "0.0.0.0") { delay(100); }
	
	delete config;
	
	Serial.println("IP: " + WiFi.localIP().toString());
	Serial.print("Gateway IP: "); Serial.println(WiFi.gatewayIP());
}

char* readFile(char* name)
{
	if(name=="ZuSi3_TS_config.json") return dashConfig;
	if(name=="systemConfig.json") return sysConfig;

	File32 file;
	
	if (!file.open(name, O_READ)) {
		Serial.println(name);
		sd.errorHalt("opening file failed");
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
