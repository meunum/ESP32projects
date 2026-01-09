#include <Arduino.h>
#include <Arduino_JSON.h>
#include <NetworkClient.h>
#include <ZuSi3Schnittstelle.h>
#include <ZuSi3_TS_controls.h>

extern int* G_DigitalOutGPIOPins;
extern int* G_AnalogInGPIOPins;
extern int* G_DigitalOutGPIOData;
extern float* G_AnalogInGPIOData;

class ZuSi3_TS_DashBoard
{
public:
	ZuSi3_TS_DashBoard();
	void Init(String config, NetworkClient *nwclient);
	void SetNetworkClient(NetworkClient *client);
	void SetConfig(String config_json);
	void SetBaureihe(String name);
	void AddControl(ZuSi3_TS_Control* control);
	void Update();
	ZuSi3_TS_Control** Controls;
	int ControlCount = 0;
	int AnalogInGPIOLength = 0;
	int DigitalOutGPIOLength = 0;
	
private:
	JSONVar parseConfig();
	void loadSystemConfig(JSONVar config);
	void loadHardwareConfig(JSONVar config);
	void loadBaureihenConfig(JSONVar config, String baureihe);
	String configJson = "";
	String baureihe = "default";
	String clientName;
	String serverAdresse;
	int serverPortnummer;
	int analogInGPIOLength = 0;
	int digitalOutGPIOLength = 0;
	int prevStufe = 0;
	int motorHold = 0;
	float prevAnalogValue = 0;
	NetworkClient *networkClient;
	Zusi3Schnittstelle *zusiClient;
};