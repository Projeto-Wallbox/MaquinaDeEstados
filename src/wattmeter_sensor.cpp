#include "SparkFun_ACS37800_Arduino_Library.h" 
#include <Wire.h>
#include "wattmeter_sensor.h"

ACS37800 mySensor;            // Create an object of the ACS37800 class
WattmeterSensor myWattmeter;  // Create an object of the WattmeterSensor class

// Calculation of the average of the last samples (numSamples) stored in voltsBuffer[]
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

// Calculate the average of the last samples (numSamplescurrents) stored in currentBuffer[]
void WattmeterSensor::updateFilteredCurrents(float newValue) 
{
  float currentBuffer[numSamplescurrents];         // Buffer to store the last samples of current values

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

// Reads register 0x21 and stores it in PowerActive
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

/* Reads voltage and current in the register,
calculates the average, saves the filtered values ​​in filteredVolts
and filteredCurrents*/
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

// Calculate energy in kWh
void WattmeterSensor::calculateEnergy() {
  energy += (PowerApparent * 10) / (3600.0 * 1000.0);  // Calcula a energia em kWh
}

// Configuration and initialization of I2C communication and the ACS37800 Sensor
void WattmeterSensor::initWattmeter(config_wattmeter &params){
    numSamples = params.numsamples;    
    numSamplescurrents= params.numsamplescurrents;
    UnderVoltage = params.undervoltage; 
    OverVoltage = params.overvoltage;
    OverCurrent = params.overcurrent;

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

// Change the value of numSamples
void WattmeterSensor::setNumSamples(int newSamples){
  numSamples = newSamples;
}

// Change the value of numSamplescurrents
void WattmeterSensor::setnumSamplescurrents(int newSamplescurrents){
  numSamplescurrents = newSamplescurrents;
}

// Change the value of UnderVoltage
void WattmeterSensor::setUnderVoltage(int newUnderVoltage) {
    UnderVoltage = newUnderVoltage;
}

// Change the OverVoltage value
void WattmeterSensor::setOverVoltage(int newOverVoltage) {
    OverVoltage = newOverVoltage;
}

// Change the OverCurrent value
void WattmeterSensor::setOverCurrent(int newOverCurrent) {
    OverCurrent = newOverCurrent;
}

// Returns the filtered voltage value (filteredVolts)
float WattmeterSensor::getFilteredVolts() const{
  return filteredVolts;
}

// Returns the filtered current value (filteredCurrents)
float WattmeterSensor::getFilteredCurrents() const{
  return filteredCurrents;
}

// Returns the apparent power (PowerApparent)
float WattmeterSensor::getPowerApparent() const{
  return PowerApparent;
}

// Returns the calculated energy (energy)
float WattmeterSensor::getEnergy() const{
  return energy;
}

