#ifndef wattmeter_sensor_h
#define wattmeter_sensor_h

const int NUM_SAMPLES_DEFAULT = 200;
const int NUM_SAMPLES_CURRENTS_DEFAULT = 200;
const int UNDER_VOLTAGE_DEFAULT = 5;
const int OVER_VOLTAGE_DEFAULT = 4;
const int OVER_CURRENT_DEFAULT = 13;

struct config_wattmeter
{
    int pinscl;   // pino SCL
    int pinsda;   // pino SDA
    int senseRes;
    int DividerRes;
    int numsamples;      // Number of samples for averaging
    int numsamplescurrents; // Number of samples for averaging
    int undervoltage; 
    int overvoltage;
    int overvurrent;
};

class WattmeterSensor{
private:
    int numSamples = NUM_SAMPLES_DEFAULT;              // Number of samples for averaging
    int numSamplescurrents = NUM_SAMPLES_CURRENTS_DEFAULT;      // Number of samples for averaging 
    int UnderVoltage = UNDER_VOLTAGE_DEFAULT;
    int OverVoltage = OVER_VOLTAGE_DEFAULT;
    int OverCurrent = OVER_CURRENT_DEFAULT;
    int currentIndex = 0;                    // Current index in the buffer
    int currentIndexcurrents = 0;            // Current index in the buffer
    int c = 0;                               // Counter used in the loop function

    float filteredVolts;               // Filtered value of voltage
    float filteredCurrents;            // Filtered value of current
    float PowerApparent;                     // Variável que recebe o valor da potencia aparente
    float PowerActive;                       // Variável que recebe o valor da potencia ativa
    float energy;                      // Variável para armazenar a energia acumulada em kWh        

    unsigned long previousPrintMillis = 0;   // Variável para armazenar o tempo da última impressão
    unsigned long previousCalcMillis = 0;    // Variável para armazenar o tempo da última execução das funções de cálculo
    unsigned long previousMillis = 0;        // Variável para armazenar o tempo anterior
    unsigned long elapsedTime = 0;           // Variável para armazenar o tempo decorrido

    bool voltageBelowThreshold = false;      // Auxiliary variable to track if voltage is below the threshold
    bool currentAboveThreshold = false;      // Auxiliary variable to track if current is above the threshold
    bool voltageAboveHighThreshold = false;  // Auxiliary variable to track if voltage is above the high threshold
public:
    void updateFilteredVolts(float newValue);  //Fu
    void updateFilteredCurrents(float newValue);
    void PowerReactiveandActive();
    void showRMSvalues();
    void calculateEnergy();
    void initWattmeter(config_wattmeter &params);  //função que inicilaiza o wattimetro

    void setNumSamples(int newSamples);
    void setnumSamplescurrents(int newSamplescurrents);
    void setUnderVoltage(int newUnderVoltage);
    void setOverVoltage(int newOverVoltage);
    void setOverCurrent(int newOverCurrent);
    
    float getFilteredVolts();
    float getFilteredCurrents(); 
    float getPowerApparent();
    float getEnergy();
};

extern WattmeterSensor myWattmeter;


#endif