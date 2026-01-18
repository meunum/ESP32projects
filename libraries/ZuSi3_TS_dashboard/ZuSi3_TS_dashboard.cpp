#include <ZuSi3_TS_dashboard.h>

ZuSi3_TS_DashBoard::ZuSi3_TS_DashBoard()
{
}

void ZuSi3_TS_DashBoard::Init(String config, Client *nwclient)
{
	debug::println("ZuSi3_TS_DashBoard::Init");
	
	SetConfig(config);
	SetNetworkClient(nwclient);
	zusiClient = new Zusi3Schnittstelle(networkClient, clientName);
	zusiClient->connect();
}

void ZuSi3_TS_DashBoard::SetNetworkClient(Client *client)
{
	debug::print("ZuSi3_TS_DashBoard::SetNetworkClient IP-Adresse: "); debug::print(serverAdresse); debug::print(" Port: "); debug::println(serverPortnummer);
	
	networkClient = client;
	Serial.println("Verbinden mit ZuSi-Server ...");
	
	if(ConnectTcp())
	{		
		Serial.println(); Serial.println("Server verbunden");
	}
	else
	{
		Serial.println(); Serial.println("Verbindung nicht hergestellt!");
	}
}

bool ZuSi3_TS_DashBoard::ConnectTcp()
{
	if (networkClient->connected()) return true;
	debug::println("ZuSi3_TS_DashBoard::ConnectTcp");
	debug::print("Connecting to " + serverAdresse + ":"); debug::println(serverPortnummer);

	networkClient->stop();
	IPAddress ip;
	
	if(!ip.fromString(serverAdresse))
	{
		debug::print("Invalid ip-address");
		return false;
	}
	
	if (!networkClient->connect(ip, serverPortnummer))
	{
		debug::print("NOT connected!");
		return false;
	}

	debug::print("connected");
	return true;
}

void ZuSi3_TS_DashBoard::Update()
{
//	debug::println("ZuSi3_TS_DashBoard::Update");
	
//	if (!networkClient->connected()) ConnectTcp();

	for(int i = 0; i < ControlCount; i++)
	{
		Controls[i]->Update();
		byte stufe = Controls[i]->GetWert();
		
		if(stufe != prevStufe)
		{
			debug::print(Controls[i]->ControlName + " Stufe: "); debug::println(stufe);

			prevStufe = stufe;

			byte input[] = {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 10, 1, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 1, 0, 1, 0, 4, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 3, 0, 7, 0, 4, 0, 0, 0, 4, 0, 0, 0, 6, 0, 0, 0, 5, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
			input[48] = stufe & 0xff;
			input[49] = (stufe >> 8) & 0xff;
			networkClient->write(input, 72);
		}
	}
}

void ZuSi3_TS_DashBoard::AddControl(ZuSi3_TS_Control* control)
{
	Controls[ControlCount++] = control;
}

void ZuSi3_TS_DashBoard::SetConfig(String config_json)
{
	debug::println("ZuSi3_TS_DashBoard::SetConfig");
	
	configJson = config_json;
	JSONVar config = parseConfig();
	loadSystemConfig(config);
	loadHardwareConfig(config);
	loadBaureihenConfig(config, baureihe);
	delete config;
}

void ZuSi3_TS_DashBoard::SetBaureihe(String name)
{
	debug::println("ZuSi3_TS_DashBoard::SetBaureihe");

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
	debug::println("ZuSi3_TS_DashBoard::parseConfig");
	
	JSONVar config = JSON.parse(configJson);
	if (JSON.typeof(config) == "undefined") { debug::println("Parsing config json failed!"); return null; }
	return config;
}

void ZuSi3_TS_DashBoard::loadSystemConfig(JSONVar config)
{
	debug::println("ZuSi3_TS_DashBoard::loadSystemConfig");
	
	JSONVar systemConfig = config["ZuSi3_TS_config"]["system"];
	if (systemConfig == nullptr) { debug::println("Systemkonfiguration nicht gefunden"); return; }
	clientName = (const char*)systemConfig["clientName"];

	JSONVar serverConfig = systemConfig["server"];
	if (serverConfig == nullptr) { debug::println("Netzwerkkonfiguration nicht gefunden"); return; }
	serverAdresse = (const char*)serverConfig["ipAddresse"];
	serverPortnummer = serverConfig["portNummer"];
	
	delete systemConfig;
	delete serverConfig;
}

void ZuSi3_TS_DashBoard::loadHardwareConfig(JSONVar config)
{
	debug::println("ZuSi3_TS_DashBoard::loadHardwareConfig");
	
	JSONVar controlsConfig = config["ZuSi3_TS_config"]["hardware"]["steuerelemente"];
	if (controlsConfig == nullptr) { debug::println("Hardware-Steuerelementekonfiguration nicht gefunden"); return; }

	int controlCount = controlsConfig.length();
	int analogInPinCount = 0;
	int digitalInPinCount = 0;
	int digitalOutPinCount = 0;
	
	for (int i = 0; i < controlsConfig.length(); i++)
	{
		String klasse = controlsConfig[i]["klasse"];
		if(klasse == "DynamischerStufenSchalter")
		{
			digitalOutPinCount += 3;
			analogInPinCount++;
		}
	}
	
	Controls = new ZuSi3_TS_Control*[controlCount];

	
	int digitalOutGpioPinIndex = 0;
	int analogInGpioPinIndex = 0;
	
	for (int i = 0; i < controlsConfig.length(); i++)
	{
		JSONVar elementConfig = controlsConfig[i];
		
		debug::print("element: "); debug::printlnJV(elementConfig);
		
		String name = elementConfig["name"];
		String klasse = elementConfig["klasse"];
		
		if(klasse == "DynamischerStufenSchalter")
		{
			JSONVar gpio = elementConfig["gpio"];
			JSONVar kal = elementConfig["kalibrierung"];
			int enaIndex = gpio["ena"];
			int dirIndex = gpio["dir"];
			int stepIndex = gpio["step"];
			int sensorIndex = gpio["sensor"];
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
	debug::println("ZuSi3_TS_DashBoard::loadBaureihenConfig");
	
	JSONVar controlsConfig = config["ZuSi3_TS_config"]["baureihen"][baureihe]["steuerelemente"];
	if (controlsConfig == nullptr) { debug::println("Steuerelementekonfiguration der Baureihe nicht gefunden: " + baureihe); return; }

	for (int i = 0; i < controlsConfig.length(); i++)
	{
		JSONVar elementConfig = controlsConfig[i];

		debug::print("element: "); debug::printlnJV(elementConfig);

		String name = elementConfig["name"];
		for(int j = 0; j < ControlCount; j++)
		{
			if (Controls[j]->ControlName == name)
			{
				int tastaturZuordnung = (int) elementConfig["tastaturZuordnung"];
				Controls[j]->SetTastaturZuordnung(tastaturZuordnung);

				if(Controls[j]->Class == "DynamischerStufenSchalter")
				{
					DynamischerStufenSchalter* schalter = static_cast<DynamischerStufenSchalter*>(Controls[j]);
					JSONVar kombi = elementConfig["kombischalter"];
					
					if (kombi == nullptr | JSON.typeof(kombi) == "undefined")
					{
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
