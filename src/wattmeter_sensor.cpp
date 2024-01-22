#include "SparkFun_ACS37800_Arduino_Library.h" 
#include <Wire.h>
#include "wattmeter_sensor.h"

ACS37800 mySensor;      // Create an object of the ACS37800 class
WattmeterSensor myWattmeter;
// //Constructor
// WattmeterSensor::WattmeterSensor()
// {
// }

void WattmeterSensor::updateFilteredVolts(float newValue)
{
  float voltsBuffer[numSamples];           // Buffer to store the last samples of voltage values

  voltsBuffer[currentIndex] = newValue;
  currentIndex = (currentIndex + 1) % numSamples;

  // Calculate the average of the samples in the buffer
  float sum = 0.0;
  for (int i = 0; i < numSamples; ++i) {
    sum += voltsBuffer[i];
  }
  filteredVolts = sum / numSamples;

  // Check if the filteredVolts is below 201 for more than 5 consecutive samples
  if (filteredVolts < 201)
  {
    voltageBelowThreshold = true;
  }
  else
  {
    voltageBelowThreshold = false;
  }

  // Check if the filteredVolts is above 231 for more than 5 consecutive samples
  if (filteredVolts > 231)
  {
    voltageAboveHighThreshold = true;
  }
  else
  {
    voltageAboveHighThreshold = false;
  }
}

void WattmeterSensor::updateFilteredCurrents(float newValue) 
{
  float currentBuffer[numSamples];         // Buffer to store the last samples of current values

  currentBuffer[currentIndexcurrents] = newValue;
  currentIndexcurrents = (currentIndexcurrents + 1) % numSamplescurrents;

  // Calculate the average of the samples in the buffer
  float sumc = 0.0;
  for (int i = 0; i < numSamplescurrents; ++i) {
    sumc += currentBuffer[i];
  }
  filteredCurrents = sumc / numSamplescurrents;

  // Check if the filteredCurrents is above 32 for more than 5 consecutive samples
  if (filteredCurrents > 32)
  {
    currentAboveThreshold = true;
  }
  else
  {
    currentAboveThreshold = false;
  }
}

void WattmeterSensor::PowerReactiveandActive()
{
  float pactive   = 0.0;
  float preactive = 0.0;
  float papparent = 0.0;
  float pfactor   = 0.0;
  bool posangle   = 0;
  bool pospf      = 0;

  mySensor.readPowerActiveReactive(&pactive, &preactive);            // Read the active and reactive power
  PowerActive = pactive;
  mySensor.readPowerFactor(&papparent, &pfactor, &posangle, &pospf); // Read the apparent power and the power factor

}

void WattmeterSensor::showRMSvalues()  
{
  float volts = 0;
  float amps = 0;

  mySensor.readRMS(&volts, &amps);

  float voltsfiltred = volts * 1;
  float ampsfiltred = amps * 11.9;
  
  updateFilteredVolts(voltsfiltred);
  updateFilteredCurrents(ampsfiltred);
 
  PowerApparent = filteredCurrents * filteredVolts;
}

void WattmeterSensor::calculateEnergy() {
  energy += (PowerApparent * 10) / (3600.0 * 1000.0);  // Calcula a energia em kWh
}

void WattmeterSensor::initWattmeter(config_wattmeter &params){ //teste
    numSamples = params.numsamples;    
    numSamplescurrents= params.numsamplescurrents;
    UnderVoltage = params.undervoltage; 
    OverVoltage = params.overvoltage;
    OverCurrent = params.overvurrent;

    Serial.begin(115200);
    Wire.begin(params.pinsda, params.pinscl);

    // pinMode(UnderVoltage, OUTPUT);
    // pinMode(OverVoltage, OUTPUT);
    // pinMode(OverCurrent, OUTPUT);

    //Initialize sensor using default I2C address
    if (mySensor.begin(0x60) == false)
    {
      Serial.print(F("ACS37800 not detected. Check connections and I2C address. Freezing..."));
    }

    //mySensor.setBypassNenable(true, true);
    mySensor.setNumberOfSamples(1023, true);
    mySensor.setSenseRes(params.senseRes);
    mySensor.setDividerRes(params.DividerRes);

}

void WattmeterSensor::setNumSamples(int newSamples){
  numSamples = newSamples;
}

void WattmeterSensor::setnumSamplescurrents(int newSamplescurrents){
  numSamplescurrents = newSamplescurrents;
}

void WattmeterSensor::setUnderVoltage(int newUnderVoltage) {
    UnderVoltage = newUnderVoltage;
}

void WattmeterSensor::setOverVoltage(int newOverVoltage) {
    OverVoltage = newOverVoltage;
}

void WattmeterSensor::setOverCurrent(int newOverCurrent) {
    OverCurrent = newOverCurrent;
}

float WattmeterSensor::getFilteredVolts(){
  return filteredVolts;
}

float WattmeterSensor::getFilteredCurrents(){
  return filteredCurrents;
}

float WattmeterSensor::getPowerApparent(){
  return PowerApparent;
}

float WattmeterSensor::getEnergy(){
  return energy;
}

