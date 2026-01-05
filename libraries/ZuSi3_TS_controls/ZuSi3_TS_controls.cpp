#include <Arduino.h>
#include <ZuSi3_TS_controls.h>

int* G_DigitalOutGPIOPins;
int* G_AnalogInGPIOPins;
int* G_DigitalOutGPIOData;
float* G_AnalogInGPIOData;

void ZuSi3_TS_Control::SetTastaturZuordnung(int value)
{
  tastaturZuordnung = value;
}

void ZuSi3_TS_Control::Update()
{
}

int ZuSi3_TS_Control::GetWert()
{
	return -32.768;
}

void ZuSi3_TS_Control::SetWert(int value)
{
}

DynamischerStufenSchalter::DynamischerStufenSchalter(String name, int enaIndex, int dirIndex, int stepIndex, int analogIndex, float analogMin, float analogMax)
{
  Class = "DynamischerStufenSchalter";	
  ControlName = name;
  minStufe = 0;
  minSensorValue = analogMin;
  maxSensorValue = analogMax;
  sensorIndex = analogIndex;
  motor.Init(enaIndex, dirIndex, stepIndex);
}

void DynamischerStufenSchalter::Update()
{
  int newStufe = 0;
  int mapValue = 0;
  int sensorValue = G_AnalogInGPIOData[sensorIndex];

  if(sensorValue > maxSensorValue)
  {
	newStufe = maxStufe;
  }
  else if(sensorValue < minSensorValue)
  {
    newStufe = minStufe;
  }
  else
  {
    mapValue = map(sensorValue, minSensorValue, maxSensorValue, minStufe, maxStufe);
    if(mapValue < minStufe) 
	{
	  newStufe = minStufe;
	}
    else if(mapValue > maxStufe) 
	{
	  newStufe = maxStufe;
	}
    else
	{
      newStufe = mapValue;
	}
  }
  
  if (newStufe != stufe)
  {
	holdSensorValue = sensorValue;
  }
  if (abs(holdSensorValue - sensorValue) < 10)
  {
	motor.SetEnable(1);
  }
  else
  {
    motor.SetEnable(0);
  }

  stufe = newStufe;
}
  
int DynamischerStufenSchalter::GetWert()
{
  return stufe;
}

void DynamischerStufenSchalter::SetMinStufe(int value)
{
  minStufe = value;
}

void DynamischerStufenSchalter::SetMaxStufe(int value)
{
  maxStufe = value;
}

void DynamischerStufenSchalter::SetMittelStellung(int value)
{
  mittelStellung = value;
}

void DynamischerStufenSchalter::SetFunktion(int value)
{
  funktion = value;
}

void DynamischerStufenSchalter::SetFunktion2(int value)
{
  funktion2 = value;
}

void StepperMotor::Init(int ena, int dir, int step)
{
  enaIndex = ena;
  dirIndex = dir;
  stepIndex = step;
}

void StepperMotor::Step()
{
}
