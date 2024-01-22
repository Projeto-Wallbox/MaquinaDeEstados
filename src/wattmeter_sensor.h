#ifndef wattmeter_sensor_h
#define wattmeter_sensor_h

const int NUM_SAMPLES_DEFAULT = 200;
const int NUM_SAMPLES_CURRENTS_DEFAULT = 200;
const int UNDER_VOLTAGE_DEFAULT = 5;
const int OVER_VOLTAGE_DEFAULT = 4;
const int OVER_CURRENT_DEFAULT = 13;

struct config_wattmeter
{
    int pinscl;             // SCL pin
    int pinsda;             // SDA pin
    int senseRes;           // Detection resistor value (Ohms)
    int DividerRes;         // Voltage divider resistance value (Ohms)
    int numsamples;         // Number of samples for calculating the voltage average
    int numsamplescurrents; // Number of samples to calculate the current average
    int undervoltage;       // undervoltage value
    int overvoltage;        // overvoltage value
    int overcurrent;        // overcurrent value
};

class WattmeterSensor{
private:
    int numSamples = NUM_SAMPLES_DEFAULT;                   // Number of samples for averaging
    int numSamplescurrents = NUM_SAMPLES_CURRENTS_DEFAULT;  // Number of samples for averaging 
    int UnderVoltage = UNDER_VOLTAGE_DEFAULT;               // UnderVoltage variable
    int OverVoltage = OVER_VOLTAGE_DEFAULT;                 // OverVoltage variable
    int OverCurrent = OVER_CURRENT_DEFAULT;                 // OverCurrent variable
    int currentIndex = 0;                    // Current index in the buffer
    int currentIndexcurrents = 0;            // Current index in the buffer
    int c = 0;                               // Counter used in the loop function

    float filteredVolts;                    // Filtered value of voltage
    float filteredCurrents;                 // Filtered value of current
    float PowerApparent;                    // Variable that receives the value of the apparent power
    float PowerActive;                      // Variable that receives the value of the active power
    float energy;                           // Variable to store the accumulated energy in kWh        

    unsigned long previousPrintMillis = 0;   // Variable to store the time of the last print
    unsigned long previousCalcMillis = 0;    // Variable to store the time of the last execution of the calculation functions
    unsigned long previousMillis = 0;        // Variable to store the previous time
    unsigned long elapsedTime = 0;           // Variable to store elapsed time

    bool voltageBelowThreshold = false;      // Auxiliary variable to track if voltage is below the threshold
    bool currentAboveThreshold = false;      // Auxiliary variable to track if current is above the threshold
    bool voltageAboveHighThreshold = false;  // Auxiliary variable to track if voltage is above the high threshold
public:
    void updateFilteredVolts(float newValue);
    void updateFilteredCurrents(float newValue);
    void PowerReactiveandActive();
    void showRMSvalues();
    void calculateEnergy();
    void initWattmeter(config_wattmeter &params);  

    void setNumSamples(int newSamples);
    void setnumSamplescurrents(int newSamplescurrents);
    void setUnderVoltage(int newUnderVoltage);
    void setOverVoltage(int newOverVoltage);
    void setOverCurrent(int newOverCurrent);
    
    float getFilteredVolts() const;
    float getFilteredCurrents() const; 
    float getPowerApparent() const;
    float getEnergy() const;
};

extern WattmeterSensor myWattmeter;

#endif