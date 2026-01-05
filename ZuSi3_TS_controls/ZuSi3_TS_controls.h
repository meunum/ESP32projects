#pragma once
#include <Arduino.h>

extern int* DigitalOutGPIOPins;
extern int* AnalogInGPIOPins;
extern int* DigitalOutGPIOData;
extern float* AnalogInGPIOData;

class StepperMotor
{
public:
  void Init(int ena, int dir, int step);
  void Step();

  int GetEnable() { return DigitalOutGPIOData[enaIndex]; }
  void SetEnable(int value) { DigitalOutGPIOData[enaIndex] = value; }
  int GetDirection() { return DigitalOutGPIOData[dirIndex]; }
  void SetDirection(int value) { DigitalOutGPIOData[enaIndex] = value; }
private:
  int enaIndex;
  int dirIndex;
  int stepIndex;
};

class ZuSi3_TS_Control
{
public:
  virtual ~ZuSi3_TS_Control() {} // makes the class polymorphic

  String ControlName;
  String Class;
  virtual int GetWert();
  virtual void SetWert(int value);
  void SetTastaturZuordnung(int value);
  virtual void Update();
  
private:
  int tastaturZuordnung;
  int wert;
};

class ZuSi3_TS_Schalter : public ZuSi3_TS_Control
{
};

class ZuSi3_TS_Melder : public ZuSi3_TS_Control
{
};

class DynamischerStufenSchalter : public ZuSi3_TS_Schalter
{
public:
  DynamischerStufenSchalter(String name, int enaIndex, int dirIndex, int stepIndex, int analogIndex, float analogMin, float analogMax);

  virtual int GetWert();
  virtual void Update();
  void SetMinStufe(int value);
  void SetMaxStufe(int value);
  void SetMittelStellung(int value);
  void SetFunktion(int value);
  void SetFunktion2(int value);
  
private:
  int stufe;
  int minStufe;
  int maxStufe;
  int mittelStellung;
  int funktion;
  int funktion2;
  float holdSensorValue;
  float minSensorValue;
  float maxSensorValue;
  int sensorIndex;
  StepperMotor motor;
};