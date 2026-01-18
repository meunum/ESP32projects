#include <debug.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <Wire.h>
#include <ADS1115_WE.h> 
#include <SdFat.h>
#include <ZuSi3_TS_dashboard.h>

#define CS_PIN 10
#define PIN_MISO 11
#define PIN_MOSI 12 
#define PIN_SCK 13
#define wire1SdaPin 15
#define wire1SclPin 16
#define ads1Adr 0x48
#define SPI_SPEED SD_SCK_MHZ(4)
const uint32_t RESPONSE_TIMEOUT = 30000;
SdFat32 sd;
WiFiClient wifiClient;
ZuSi3_TS_DashBoard dashBoard;
TwoWire wire1 = TwoWire(1);
ADS1115_WE ads1 = ADS1115_WE(&wire1, ads1Adr);
bool ads1Available = false;
String Wifi_SSID;
String Wifi_Password;
TaskHandle_t UpdateTaskHandle = NULL;
TaskHandle_t AnalogReadTaskHandle = NULL;

int AnalogInGPIOLength = 4;
int DigitalInGPIOLength = 8;
int DigitalOutGPIOLength = 8;

char* dashConfig = "{\"ZuSi3_TS_config\":{\"system\":{\"clientName\":\"ZuSi3_TS_Dashboard\",\"server\":{\"ipAddresse\":\"192.168.178.65\",\"portNummer\":1436}},\"hardware\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"klasse\":\"DynamischerStufenSchalter\",\"gpio\":{\"ena\":0,\"dir\":1,\"step\":2,\"sensor\":0},\"kalibrierung\":{\"min\":0,\"max\":4095}}]},\"baureihen\":{\"default\":{\"steuerelemente\":[{\"name\":\"stufenschalter_1\",\"verwendung\":\"Fahrstufe\",\"tastaturZuordnung\":1,\"stufen\":15}]},\"BR118\":{},\"BR154\":{}}}}";

void setup() {
	pinMode(A0, INPUT_PULLUP);

	if (digitalRead(A0) == LOW)
	{
		while (true)
		{
			delay(100);   // Board bleibt flashbar
		}
	}
	Serial.begin(115200);
	
	debug::println("setup");
	
	initGPIO();
	initSdCard();

	LoadWifiConfig();
	ConnectWifi();
	
	char* configData = readFile("ZuSi3_TS_config.json");
	dashBoard.Init(configData, &wifiClient);
/*	
	xTaskCreatePinnedToCore(
		UpdateTask,			// Task function
		"UpdateTask",		// Task name
		10000,				// Stack size (bytes)
		NULL,				// Parameters
		1,					// Priority
		&UpdateTaskHandle,	// Task handle
		1					// Core
	);
*/	
	xTaskCreatePinnedToCore(
		AnalogReadTask,			// Task function
		"AnalogReadTask",		// Task name
		10000,				// Stack size (bytes)
		NULL,				// Parameters
		1,					// Priority
		&AnalogReadTaskHandle,	// Task handle
		1					// Core
	);
}

void loop() 
{
}

void UpdateTask(void *parameter) 
{
	debug::println("UpdateTask start");
	
	while(true) 
	{
		if (WiFi.status() != WL_CONNECTED) ConnectWifi();
		
		for(int i = 0; i < DigitalInGPIOLength; i++)
		{
			G_DigitalInGPIOData[i] = digitalRead(G_DigitalInGPIOPins[i]);
		}
		
		dashBoard.Update();
		
		for(int i = 0; i < DigitalOutGPIOLength; i++)
		{
			digitalWrite(G_DigitalOutGPIOPins[i], G_DigitalOutGPIOData[i]);
		}

		delay(100);
	}
}

void AnalogReadTask(void *parameter) 
{
	debug::println("AnalogReadTask start");
	
	while(true) 
	{
		if (ads1Available)
		{
			G_AnalogInGPIOData[0] = readAdsChannel(ads1, ADS1115_COMP_0_GND);
			G_AnalogInGPIOData[1] = readAdsChannel(ads1, ADS1115_COMP_1_GND);
			G_AnalogInGPIOData[2] = readAdsChannel(ads1, ADS1115_COMP_2_GND);
			G_AnalogInGPIOData[3] = readAdsChannel(ads1, ADS1115_COMP_3_GND);
		}
	}
}

float readAdsChannel(ADS1115_WE ads, ADS1115_MUX channel) 
{
  float voltage = 0.0;
  ads.setCompareChannels(channel);
  ads.startSingleMeasurement();
  while(ads.isBusy()){delay(0);}
  voltage = ads.getResult_mV();

trace::print("AnalogRead voltage = "); trace::println(voltage);
  
  return voltage;
  
}

void initGPIO()
{
	debug::println("initGPIO");
  
	wire1.begin(wire1SdaPin, wire1SclPin);
	ads1Available = ads1.init(ads1Adr);

	if(!ads1Available) { 
		debug::println("!ADS1115 1 not connected!");
	}
	else
	{
		debug::println("ADS1115 1 connected."); 

		ads1.setVoltageRange_mV(ADS1115_RANGE_6144);
//		ads1.setMeasureMode(ADS1115_CONTINUOUS); 
	}

	G_AnalogInGPIOData = new float[AnalogInGPIOLength];
	G_DigitalInGPIOData = new int[DigitalInGPIOLength];
	G_DigitalOutGPIOData = new int[DigitalOutGPIOLength];
}

void LoadWifiConfig()
{
	char* configJson = readFile("systemConfig.json");
	JSONVar config = JSON.parse(configJson);
	if (JSON.typeof(config) == "undefined") { Serial.println("Parsing sysemConfig.json failed!"); return; }
	
	Wifi_SSID = (const char*) config["system"]["wifi"]["SSID"];
	Wifi_Password = (const char*) config["system"]["wifi"]["password"];

	Serial.println();
	Serial.print("WiFi SSID: "); Serial.print(Wifi_SSID); Serial.print("; password: "); Serial.println(Wifi_Password);
	delete config;
}

void ConnectWifi()
{
	if (WiFi.status() == WL_CONNECTED) return;
	
	WiFi.mode(WIFI_STA);
	WiFi.begin(Wifi_SSID, Wifi_Password);

	Serial.print("Connecting WiFi ");
	
	while (WiFi.status() != WL_CONNECTED) 
	{
		delay(100);
		Serial.print(".");
	}

	Serial.println(" Connected!");
	
	while (WiFi.localIP().toString() == "0.0.0.0") { delay(100); }
	wifiClient.setTimeout(RESPONSE_TIMEOUT);
	
	Serial.println("IP: " + WiFi.localIP().toString());
	Serial.print("Gateway IP: "); Serial.println(WiFi.gatewayIP());
}

void initSdCard()
{
	SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);
	if (!sd.begin(CS_PIN, SPI_SPEED)) {
		Serial.println("SD-Card error().");
		if (sd.card()->errorCode()) {
			Serial.println("SD initialization failed.");
		} else if (sd.vol()->fatType() == 0) {
			Serial.println("Can't find a valid FAT16/FAT32 partition.");
		} else {
			Serial.println("Can't determine error type");
		}
	}
	else
	{
		debug::println("SD-Card initialized");
	}
}

char* readFile(char* name)
{
	if(name=="ZuSi3_TS_config.json") return dashConfig;

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
	
	debug::print("SD-Card read file "); debug::println(name);
	
	return data;
}
