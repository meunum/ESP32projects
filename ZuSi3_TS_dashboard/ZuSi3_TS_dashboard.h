#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ZuSi3_TS_controls.h>

extern int* DigitalOutGPIOPins;
extern int* AnalogInGPIOPins;
extern int* DigitalOutGPIOData;
extern float* AnalogInGPIOData;

class ZuSi3_TS_DashBoard
{
public:
  ZuSi3_TS_DashBoard();
  void Init(int controlCount, int digitalPinCount, int analogPinCount);
  void SetConfig(String config_json);
  void SetBaureihe(String name);
  void AddControl(ZuSi3_TS_Control* control);
  int ControlCount = 0;
  ZuSi3_TS_Control** Controls;
  
private:
  JSONVar parseConfig();
  void loadHardwareConfig(JSONVar config);
  void loadBaureihenConfig(JSONVar config, String baureihe);
  String configJson = "";
  String baureihe = "default";
};