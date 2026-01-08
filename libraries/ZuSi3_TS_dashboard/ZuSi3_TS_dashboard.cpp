#define DEBUG
#include <ZuSi3_TS_dashboard.h>

ZuSi3_TS_DashBoard::ZuSi3_TS_DashBoard()
{
}

void ZuSi3_TS_DashBoard::Init(String config, NetworkClient *nwclient);
{
	debugPrint("ZuSi3_TS_DashBoard::Init");
	
	SetConfig(config);
	SetNetworkClient(nwclient);
	zusiClient = new Zusi3Schnittstelle(networkClient, clientName);
}

void ZuSi3_TS_DashBoard::SetNetworkClient(NetworkClient *client);
{
	debugPrint("ZuSi3_TS_DashBoard::SetNetworkClient");
	
	networkClient = client;
	Serial.print("Verbinden mit ZuSi ...");
	
	if(!networkClient->connect(serverAdresse, serverPortnummer))
	{
		Serial.println(" ZuSi-Server verbunden");
	}
	else
	{
		Serial.println(" Verbindung fehlgeschlagen!");
	}
}

void ZuSi3_TS_DashBoard::Update()
{
	for(int i = 0; i < ControlCount; i++)
	{
		Controls[i]->Update();
		byte stufe = Controls[i]->GetWert();
		
		if(stufe != prevStufe)
		{
			prevStufe = stufe;
			
			if(zusiClient != nullptr)
			{
				zusiClient->inputSchalterposition(Controls[i]->GetTastaturZuordnung(), stufe);
			}
			
			debugPrint(Controls[i]->ControlName + " Stufe: "); debugPrint(stufe);
		}
	}
}

void ZuSi3_TS_DashBoard::AddControl(ZuSi3_TS_Control* control)
{
	Controls[ControlCount++] = control;
}

void ZuSi3_TS_DashBoard::SetConfig(String config_json)
{
	debugPrint("ZuSi3_TS_DashBoard::SetConfig");
	
	configJson = config_json;
	JSONVar config = parseConfig();
	loadSystemConfig(config);
	loadHardwareConfig(config);
	loadBaureihenConfig(config, baureihe);
	delete config;
}

void ZuSi3_TS_DashBoard::SetBaureihe(String name)
{
	debugPrint("ZuSi3_TS_DashBoard::SetBaureihe");

	baureihe = name;
	
	if(configJson != "")
	{
		JSONVar config = parseConfig();
		loadBaureihenConfig(config, name);
		delete config;
	}
}

JSONVar ZuSi3_TS_DashBoard::parseConfig()
{
	debugPrint("ZuSi3_TS_DashBoard::parseConfig");
	
	JSONVar config = JSON.parse(configJson);
	if (JSON.typeof(config) == "undefined") { debugPrint("Parsing config json failed!"); return null; }
	return config;
}

void ZuSi3_TS_DashBoard::loadSystemConfig(JSONVar config)
{
	debugPrint("ZuSi3_TS_DashBoard::loadSystemConfig");
	
	JSONVar systemConfig = config["ZuSi3_TS_config"]["system"];
	if (systemConfig == nullptr) { debugPrint("Systemkonfiguration nicht gefunden"); return; }
	clientName = systemConfig["clientName"];

	JSONVar serverConfig = systemConfig["server"];
	if (serverConfig == nullptr) { debugPrint("Netzwerkkonfiguration nicht gefunden"); return; }
	serverAdresse = serverConfig["ipAddresse"];
	serverPortnummer = serverConfig["portNummer"];
	
	delete systemConfig;
	delete serverConfig;
}

void ZuSi3_TS_DashBoard::loadHardwareConfig(JSONVar config)
{
	debugPrint("ZuSi3_TS_DashBoard::loadHardwareConfig");
	
	JSONVar controlsConfig = config["ZuSi3_TS_config"]["hardware"]["steuerelemente"];
	if (controlsConfig == nullptr) { debugPrint("Hardware-Steuerelementekonfiguration nicht gefunden"); return; }
	int controlCount = controlsConfig.length();
	int digitalPinCount = 0;
	int analogPinCount = 0;
	
	for (int i = 0; i < controlsConfig.length(); i++)
	{
		String klasse = controlsConfig[i]["klasse"];
		if(klasse == "DynamischerStufenSchalter")
		{
			digitalPinCount += 3;
			analogPinCount++;
		}
	}
	
	Controls = new ZuSi3_TS_Control*[controlCount];
	G_DigitalOutGPIOPins = new int[digitalPinCount];
	G_AnalogInGPIOPins = new int[analogPinCount];
	G_DigitalOutGPIOData = new int[digitalPinCount];
	G_AnalogInGPIOData = new float[analogPinCount];
	AnalogInGPIOLength = sizeof(G_AnalogInGPIOPins) / sizeof(int);
	DigitalOutGPIOLength = sizeof(G_DigitalOutGPIOPins) / sizeof(int);
	
	int digitalGpioPinIndex = 0;
	int analogGpioPinIndex = 0;
	
	for (int i = 0; i < controlsConfig.length(); i++)
	{
		JSONVar elementConfig = controlsConfig[i];
		
		debugPrint(elementConfig);
		
		String name = elementConfig["name"];
		String klasse = elementConfig["klasse"];
		
		if(klasse == "DynamischerStufenSchalter")
		{
			JSONVar gpio = elementConfig["gpio"];
			G_DigitalOutGPIOPins[digitalGpioPinIndex] = (int) gpio["ena"]; int enaIndex = digitalGpioPinIndex++;
			G_DigitalOutGPIOPins[digitalGpioPinIndex] = (int) gpio["dir"]; int dirIndex = digitalGpioPinIndex++;
			G_DigitalOutGPIOPins[digitalGpioPinIndex] = (int) gpio["step"]; int stepIndex = digitalGpioPinIndex++;
			G_AnalogInGPIOPins[analogGpioPinIndex] = (int) gpio["sensor"]; int sensorIndex = analogGpioPinIndex++;
		
			JSONVar kal = elementConfig["kalibrierung"];
			int analogMin = (int) kal["min"];
			int analogMax = (int) kal["max"];
			DynamischerStufenSchalter* schalter = new DynamischerStufenSchalter(name, enaIndex, dirIndex, stepIndex, sensorIndex, analogMin, analogMax); 
			AddControl(schalter);
			
			delete gpio;
			delete kal;
		}
		
		delete elementConfig;
	}
	
	delete controlsConfig;
}

void ZuSi3_TS_DashBoard::loadBaureihenConfig(JSONVar config, String baureihe)
{
	debugPrint("ZuSi3_TS_DashBoard::loadBaureihenConfig");
	
	JSONVar controlsConfig = config["ZuSi3_TS_config"]["baureihen"][baureihe]["steuerelemente"];
	if (controlsConfig == nullptr) { debugPrint("Steuerelementekonfiguration der Baureihe nicht gefunden: " + baureihe); return; }

	for (int i = 0; i < controlsConfig.length(); i++)
	{
		JSONVar elementConfig = controlsConfig[i];

		debugPrint(elementConfig);

		String name = elementConfig["name"];
		for(int j = 0; j < ControlCount; j++)
		{
			if (Controls[j]->ControlName == name)
			{
				if(Controls[j]->Class == "DynamischerStufenSchalter")
				{
					DynamischerStufenSchalter* schalter = static_cast<DynamischerStufenSchalter*>(Controls[j]);
					JSONVar kombi = elementConfig["kombischalter"];
					if (kombi == nullptr | JSON.typeof(kombi) == "undefined")
					{
						schalter->SetTastaturZuordnung((int) elementConfig["tastaturZuordnung"]);
						schalter->SetMaxStufe((int) elementConfig["stufen"]);
					}
					else
					{
						schalter->SetMinStufe((int) elementConfig["minimal"]);
						schalter->SetMaxStufe((int) elementConfig["maximal"]);
						schalter->SetMittelStellung((int) elementConfig["mitte"]);
						schalter->SetFunktion((int) elementConfig["funktion"][0]);
						schalter->SetFunktion2((int) elementConfig["funktion"][1]);
					}
					
					delete kombi;
				}
			}
		}
		
		delete elementConfig;
	}
	
	delete controlsConfig;
}

void debugPrint(String text);
{
#ifdef DEBUG
		debugPrint(text);
#endif
}