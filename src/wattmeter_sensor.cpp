#include "SparkFun_ACS37800_Arduino_Library.h" 
#include <Wire.h>
#include "wattmeter_sensor.h"

ACS37800 mySensor;            // Create an object of the ACS37800 class
WattmeterSensor myWattmeter;  // Create an object of the WattmeterSensor class

// Calculation of the average of the last samples (numSamples) stored in voltsBuffer[]
void WattmeterSensor::updateFilteredVolts(float vL1, float vL2, float vL3)
{
  float voltsBufferL1[numSamples];           // Buffer to store the last samples of voltage values
  float voltsBufferL2[numSamples]; 
  float voltsBufferL3[numSamples]; 
  
  // voltsBuffer[currentIndex] = newValue;
  // currentIndex = (currentIndex + 1) % numSamples;

  // // Calculate the average of the samples in the buffer
  // float sum = 0.0;
  // for (int i = 0; i < numSamples; ++i) {
  //   sum += voltsBuffer[i];
  // }
  // filteredVolts = sum / numSamples;

  // // Check if the filteredVolts is below 201 for more than 5 consecutive samples
  // if (filteredVolts < 201)
  // {
  //   voltageBelowThreshold = true;
  // }
  // else
  // {
  //   voltageBelowThreshold = false;
  // }

  // // Check if the filteredVolts is above 231 for more than 5 consecutive samples
  // if (filteredVolts > 231)
  // {
  //   voltageAboveHighThreshold = true;
  // }
  // else
  // {
  //   voltageAboveHighThreshold = false;
  // }
}

// Calculate the average of the last samples (numSamplescurrents) stored in currentBuffer[]
void WattmeterSensor::updateFilteredCurrents(float aL1, float aL2, float aL3) 
{
  // float currentBuffer[numSamplescurrents];         // Buffer to store the last samples of current values

  // currentBuffer[currentIndexcurrents] = newValue;
  // currentIndexcurrents = (currentIndexcurrents + 1) % numSamplescurrents;

  // // Calculate the average of the samples in the buffer
  // float sumc = 0.0;
  // for (int i = 0; i < numSamplescurrents; ++i) {
  //   sumc += currentBuffer[i];
  // }
  // filteredCurrentsL1 = sumc / numSamplescurrents;

  // // Check if the filteredCurrents is above 32 for more than 5 consecutive samples
  // if (filteredCurrents > 32)
  // {
  //   currentAboveThreshold = true;
  // }
  // else
  // {
  //   currentAboveThreshold = false;
  // }
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
  float voltsL1 = 0.0;
  float ampsL1 = 0.0;

  float voltsL2 = 0.0;
  float ampsL2 = 0.0;

  float voltsL3 = 0.0;
  float ampsL3 = 0.0;
  
  mySensor.begin(ADRESS_L1);
  mySensor.readRMS(&voltsL1, &ampsL1); // Read the RMS voltage and current
  Wire.endTransmission();

  mySensor.begin(ADRESS_L2);
  mySensor.readRMS(&voltsL2, &ampsL2); // Read the RMS voltage and current
  Wire.endTransmission();

  mySensor.begin(ADRESS_L3);
  mySensor.readRMS(&voltsL3, &ampsL3); // Read the RMS voltage and current
  Wire.endTransmission();

  //realizar o filtro
  updateFilteredVolts(voltsL1, voltsL2, voltsL3);
  updateFilteredCurrents(ampsL1, ampsL2, ampsL3);

  //ai nao precisa esta parte de baixo, pois tem updateFilteredVolts(voltsfiltred);
  filteredVoltsL1 = voltsL1;
  filteredVoltsL2 = voltsL2;
  filteredVoltsL3 = voltsL3;
  filteredCurrentsL1 = ampsL1;
  filteredCurrentsL2 = ampsL2;
  filteredCurrentsL3 = ampsL3;

  // float volts = 0;
  // float amps = 0;

  // mySensor.readRMS(&volts, &amps);

  // float voltsfiltred = volts * 1;
  // float ampsfiltred = amps * 1;
  
  
 
  // PowerApparent = filteredCurrents * filteredVolts;
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
//L1 - Parametro 1
//L2 - Parametro 2
//L3 - Parametro 3
float WattmeterSensor::getFilteredVolts(int line) {
  float voltsFilter;
  if(line==1){
    voltsFilter = filteredVoltsL1;
  }
  if(line==2){
    voltsFilter = filteredVoltsL2;
  }
  if(line==3){
    voltsFilter = filteredVoltsL3;
  }
  return voltsFilter;
}

// Returns the filtered current value (filteredCurrents)
//L1 - Parametro 1
//L2 - Parametro 2
//L3 - Parametro 3
float WattmeterSensor::getFilteredCurrents(int line) {
  float currentFilter;
  if(line==1){
    currentFilter = filteredVoltsL1;
  }

  if(line==2){
    currentFilter = filteredVoltsL2;
  }

  if(line==3){
    currentFilter = filteredVoltsL3;
  }

  return currentFilter;
}

// Returns the apparent power (PowerApparent)
float WattmeterSensor::getPowerApparent() {
  return PowerApparent;
}

// Returns the calculated energy (energy)
float WattmeterSensor::getEnergy() {
  return energy;
}

