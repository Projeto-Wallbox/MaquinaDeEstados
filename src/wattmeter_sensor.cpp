#include "SparkFun_ACS37800_Arduino_Library.h" 
#include <Wire.h>
#include "state_machine.h"
#include "wattmeter_sensor.h"
#include <vector>
//gpio_num_t PINO_TESTE_POWER_OUTAGE = GPIO_NUM_25;

ACS37800 mySensor;            // Create an object of the ACS37800 class
WattmeterSensor myWattmeter;  // Create an object of the WattmeterSensor class
testInterrupt testOne; 

// Calculation of the average of the last samples (numSamples) stored in voltsBuffer[]
void WattmeterSensor::updateFilteredVolts(float vL1, float vL2, float vL3)
{
  // static float voltsBufferL1[numSamples];           // Buffer to store the last samples of voltage values
  // static float voltsBufferL2[numSamples]; 
  // static float voltsBufferL3[numSamples]; 
  static std::vector<float> voltsBufferL1(numSamples);
  static std::vector<float> voltsBufferL2(numSamples); 
  static std::vector<float> voltsBufferL3(numSamples); 
    
  voltsBufferL1[currentIndex] = vL1;
  voltsBufferL2[currentIndex] = vL2;
  voltsBufferL3[currentIndex] = vL3;

  currentIndex = (currentIndex + 1) % numSamples;

  // Calculate the average of the samples in the buffer
  float sumL1 = 0.0;
  float sumL2 = 0.0;
  float sumL3 = 0.0;
  
  for (int i = 0; i < numSamples; ++i) {
    sumL1 += voltsBufferL1[i];
    sumL2 += voltsBufferL2[i];
    sumL3 += voltsBufferL3[i];
  }
  filteredVoltsL1 = sumL1 / numSamples;    // Filtered value of voltage
  filteredVoltsL2 = sumL2 / numSamples;
  filteredVoltsL3 = sumL3 / numSamples;
  
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
  static std::vector<float> currentBufferL1(numSamples);
  static std::vector<float> currentBufferL2(numSamples); 
  static std::vector<float> currentBufferL3(numSamples);

  currentBufferL1[currentIndexcurrents] = aL1;
  currentBufferL2[currentIndexcurrents] = aL2;
  currentBufferL3[currentIndexcurrents] = aL3;

  currentIndexcurrents = (currentIndexcurrents + 1) % numSamplescurrents;

  // Calculate the average of the samples in the buffer
  float sumcL1 = 0.0;
  float sumcL2 = 0.0;
  float sumcL3 = 0.0;

  for (int i = 0; i < numSamplescurrents; ++i) {
    sumcL1 += currentBufferL1[i];
    sumcL2 += currentBufferL2[i];
    sumcL3 += currentBufferL3[i];

  }
  filteredCurrentsL1 = sumcL1 / numSamplescurrents;
  filteredCurrentsL2 = sumcL2 / numSamplescurrents;
  filteredCurrentsL3 = sumcL3 / numSamplescurrents;


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
  
  float instVoltsL1 = 0.0;
  float instAmpsL1 = 0.0;
  float instWattsL1 = 0.0;

  mySensor.begin(ADRESS_L1);
  mySensor.readRMS(&voltsL1, &ampsL1); // Read the RMS voltage and current
  mySensor.readInstantaneous(&instVoltsL1, &instAmpsL1, &instWattsL1); // Read the instantaneous
  powerOutage(instVoltsL1); // Call function power Outage
  Wire.endTransmission();

  // mySensor.begin(ADRESS_L2);
  // mySensor.readRMS(&voltsL2, &ampsL2); // Read the RMS voltage and current
  // Wire.endTransmission();

  // mySensor.begin(ADRESS_L3);
  // mySensor.readRMS(&voltsL3, &ampsL3); // Read the RMS voltage and current
  // Wire.endTransmission();

  //realizar o filtro
  updateFilteredVolts(voltsL1*0.810581, voltsL2, voltsL3); //*1.674*0.989
  updateFilteredCurrents(ampsL1*2.9175, ampsL2, ampsL3);

  // Serial.print(">L1VoltsFilter:");
  // Serial.println(myWattmeter.getFilteredVolts(1), 5);

  // Serial.print(">L1AmpsFilter:");
  // Serial.println(myWattmeter.getFilteredCurrents(1), 5);

  if(filteredCurrentsL1 >=1){
    PowerApparentL1 = filteredCurrentsL1 * filteredVoltsL1;
  }else{
    PowerApparentL1 = 0;
    filteredCurrentsL1 = 0;
  }
  
}

// Calculate energy in kWh
void WattmeterSensor::calculateEnergy() {
  energy += (PowerApparentL1 * 10) / (3600.0 * 1000.0);  // Calcula a energia em kWh
}

// Configuration and initialization of I2C communication and the ACS37800 Sensor
void WattmeterSensor::initWattmeter(config_wattmeter &params){
    numSamples = params.numsamples;    
    numSamplescurrents = params.numsamplescurrents;
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

    mySensor.setBypassNenable(false, true);
    // mySensor.setNumberOfSamples(1023, true);
    mySensor.setSenseRes(params.senseRes);
    mySensor.setDividerRes(params.DividerRes);

}

// Detect installation type
void WattmeterSensor::electricalInstallation(){
  //Intslacao trifasica
  if(filteredVoltsL1>200 && filteredVoltsL2>200 && filteredVoltsL3 >200){
      myInstallation = 3;
  }
  
  //Intslacao monofasica e bifasica
  if(filteredVoltsL1>200 && filteredVoltsL2<10 && filteredVoltsL3<10){
      myInstallation = 1;
  }
}

// Detects power outage (chamada a cada 0.5 ms)
// Detects power outage
void WattmeterSensor::powerOutage(float newInstVolts){
  static int cont_outage = 0;
  static int number_measurements = 5;
  
  //TODO ajustar number_measurements e o tempo de chamada
  if (newInstVolts >= -5 && newInstVolts <= 5) {
      cont_outage++;
      testOne.testeContOutage++;
  } else {
      cont_outage = 0; // Reinicia o contador se a tensão não estiver dentro do intervalo
  }

  //ocorreu uma queda de energia
  if(cont_outage >= number_measurements){  
    //gpio_set_level(PINO_TESTE_POWER_OUTAGE, true);
    testOne.FlagOutage = 1;
    powerOutageFlag = true;
  }else{
    //gpio_set_level(PINO_TESTE_POWER_OUTAGE, false);
    testOne.FlagOutage = 0;
    powerOutageFlag = false;
  }
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
    currentFilter = filteredCurrentsL1;
  }

  if(line==2){
    currentFilter = filteredCurrentsL2;
  }

  if(line==3){
    currentFilter = filteredCurrentsL3;
  }

  return currentFilter;
}

// Returns the apparent power (PowerApparent)
float WattmeterSensor::getPowerApparent() {
  return PowerApparentL1;
}

// Returns the calculated energy (energy)
float WattmeterSensor::getEnergy() {
  return energy;
}

// Returns installation type
int WattmeterSensor::getMyInstallation(){
  return myInstallation;
}

bool WattmeterSensor::getPowerOutageFlag(){
  return powerOutageFlag;
}

void WattmeterSensor::setPowerOutageFlag(bool newValue){
  powerOutageFlag = newValue;
}