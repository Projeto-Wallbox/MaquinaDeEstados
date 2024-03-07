#include <Arduino.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_adc_cal.h"

#ifdef COMPILE_ME
#include "state_machine.h"
#endif

#ifdef COMPILE_OCPP
#include <WiFi.h>
#include <MicroOcpp.h>
#include <MicroOcpp_c.h>
#include <MicroOcpp/Core/Configuration.h>

const int connectorId = 1;
#endif

//#include "SparkFun_ACS37800_Arduino_Library.h"
//#include <Wire.h>
//ACS37800 wattmeter; // Create an object of the ACS37800 class

#ifdef COMPILE_WATT
#include "wattmeter_sensor.h"
#include <Wire.h>

#define SENSE_RES 2550
#define DIVIDER_RES 4000000
#define NUM_SAMPLES 400    
#define NUM_SAMPLES_CURRENTS 400
#define UNDER_VOLTAGE 5
#define OVER_VOLTAGE 4
#define OVER_CURRENT 13
#endif


// DEFINIR PINOS---------------------------------------------------------------------------------
//DEFINICOES DE PINOS ESP32_DEV
#ifdef ESP32_DEV
gpio_num_t PILOT_PIN = GPIO_NUM_32;		 // Pino para leitura AD do Sinal VA
gpio_num_t PINO_PROXIMIDADE = GPIO_NUM_33;	 // Pino para leitura AD do do cabo conectado
gpio_num_t PWM_PIN = GPIO_NUM_2;			 // Pino no qual é gerado o sinal PWM
gpio_num_t RELE_N = GPIO_NUM_4;			 // Pino de saida, para acionar o Relé do Neutro (N)
gpio_num_t RELE_L3 = GPIO_NUM_5;			 // Pino de saida, para acionar o Relé da fase 1 (L1)
gpio_num_t RELE_L2 = GPIO_NUM_18;			 // Pino de saida, para acionar o Relé da fase 2 (L2)
gpio_num_t RELE_L1 = GPIO_NUM_19;			 // Pino de saida, para acionar o Relé da fase 3 (L3)
gpio_num_t LED_A = GPIO_NUM_26;				 // Led de EVSE ON/OF
gpio_num_t LED_B = GPIO_NUM_15;				 // Led de carregamento
gpio_num_t LED_C = GPIO_NUM_14;				 // Led de conexao a rede Wi-fi
gpio_num_t LED_D = GPIO_NUM_27;				 // Led de erro ou falha
gpio_num_t START_RECHARGER_BT = GPIO_NUM_23; // Pino de entrada, para setar o inicio da recarga pela Estacao
adc1_channel_t CHANNEL_PILOT = ADC1_CHANNEL_4;
adc1_channel_t CHANNEL_PROXIMIDADE = ADC1_CHANNEL_5;
adc1_channel_t CHANNEL_FAULT = ADC1_CHANNEL_6;

gpio_num_t PIN_FAULT = GPIO_NUM_34;
gpio_num_t PIN_TRIG_DC = GPIO_NUM_12;
gpio_num_t PIN_TRIG_AC = GPIO_NUM_13;
#define SPEED_MODE_TIMER LEDC_HIGH_SPEED_MODE
#define PIN_SDA 21
#define PIN_SCL 22
#endif

//DEFINICOES DE PINOS ESP32-S3-DEVKITC-1
#ifdef ESP32_S3
gpio_num_t PILOT_PIN = GPIO_NUM_8;
gpio_num_t PINO_PROXIMIDADE = GPIO_NUM_7;
gpio_num_t PWM_PIN = GPIO_NUM_6;

gpio_num_t RELE_L1 = GPIO_NUM_11;     //Seria GPIO11 na PCB
gpio_num_t RELE_L2 = GPIO_NUM_12;     //Seria GPIO12 na PCB
gpio_num_t RELE_L3 = GPIO_NUM_13;     //Seria GPIO13 na PCB
gpio_num_t RELE_N = GPIO_NUM_14;       //Seria GPIO14 na PCB

gpio_num_t LED_A = GPIO_NUM_1;	// Estamos usando o led on do ESP para mostrar que a estacao ligada
gpio_num_t LED_B = GPIO_NUM_10;	// O LEDB(Estado 9), piscando (Carregando) 
gpio_num_t LED_C = GPIO_NUM_2;	
gpio_num_t LED_D = GPIO_NUM_18; 
gpio_num_t START_RECHARGER_BT = GPIO_NUM_9;
gpio_num_t PIN_FAULT = GPIO_NUM_4;
gpio_num_t PIN_TRIG_DC = GPIO_NUM_36;
gpio_num_t PIN_TRIG_AC = GPIO_NUM_38;

adc1_channel_t CHANNEL_PILOT = ADC1_CHANNEL_7;
adc1_channel_t CHANNEL_PROXIMIDADE = ADC1_CHANNEL_6;
adc1_channel_t CHANNEL_FAULT = ADC1_CHANNEL_3;

#define SPEED_MODE_TIMER LEDC_LOW_SPEED_MODE
#define PIN_SDA 42
#define PIN_SCL 39

#endif

// DEFINIR VARIAVEIS ME-----------------------------------------------------------------------------
#ifdef COMPILE_ME 
static ledc_channel_config_t ledc_channel;
static esp_adc_cal_characteristics_t adc_chars; // Fornecido pela Esressif para calibracao do ADC
int Razao_Ciclica_PWM = 1023;					// Variavel que armazena valor da razao cicllica

// FUNCAO DE INTERRUPCAO DA ME
void timer_callback(void *param)
{
	Razao_Ciclica_PWM = funcaoInterrupcao();
	ESP_ERROR_CHECK(ledc_set_duty(SPEED_MODE_TIMER, LEDC_CHANNEL_0, Razao_Ciclica_PWM));
	ESP_ERROR_CHECK(ledc_update_duty(SPEED_MODE_TIMER, LEDC_CHANNEL_0));
}
#endif

// TASK DO MONITOR DE CORRENTE RESIDUAL
#ifdef COMPILE_D_RES_CURR
void monitorCurrentTask(void *pvParameters) {	
		while (1) {
			int adPinFault = adc1_get_raw(CHANNEL_FAULT); 
  		DataStruct.statePinDC = gpio_get_level(PIN_TRIG_DC);
  		DataStruct.statePinAC = gpio_get_level(PIN_TRIG_AC);

			// Serial.print(">adPinFault: ");
			// Serial.println(adPinFault);

			// Serial.print(">statePinDC: ");
			// Serial.println(DataStruct.statePinDC);

			// Serial.print(">statePinAC: ");
			// Serial.println(DataStruct.statePinAC);
			
			// UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
			// Serial.print("Espaço livre mínimo da pilha: ");
			// Serial.println(uxHighWaterMark);

        vTaskDelay(pdMS_TO_TICKS(1)); // Espera por 100 ms
    }
}
#endif

// TASK DO WATTIMETRO
#ifdef COMPILE_WATT
void wattmeterTask(void *pvParameters) {
    int cont_pot = 0;
		int cont_meterValue = 0;
		int cont_defineIstallation = 0;

		while (1) {
				cont_pot++;
				cont_meterValue++;
				
        myWattmeter.showRMSvalues();

				//em um segundo pega os valores RMS e define o tipo de instalação
        if(cont_defineIstallation<=2000){
					myWattmeter.electricalInstallation();
					cont_defineIstallation++;
				}
  
        if(cont_pot==1000){
          myWattmeter.calculateEnergy(); 
          cont_pot = 0;
        }

				//myWattmeter.getFilteredVolts(1)
				if(cont_meterValue==10000){
					//myWattmeter.electricalInstallation();

#ifdef COMPILE_OCPP
          addMeterValueInput([](){return myWattmeter.getFilteredVolts(1);}, "Voltage","V",nullptr, nullptr,connectorId);
#endif
        }
        // UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.print("Espaço livre mínimo da pilha: ");
        // Serial.println(uxHighWaterMark);

        vTaskDelay(pdMS_TO_TICKS(1)); // Espera por 0.5 ms
    }
}
#endif

// ############### OCPP
#ifdef COMPILE_OCPP
const char *OCPP_BACKEND_URL = "ws://200.18.75.25:3223"; //servidor
const char *OCPP_CHARGE_BOX_ID = "IntrallWallbox";

const char *ssid = "LabAT";
const char *password = "inrilabat";
#endif

void setup()

{
	Serial.begin(115200);
#ifdef COMPILE_WATT
	config_wattmeter my_config = {
		.pinscl = PIN_SCL,  
    .pinsda = PIN_SDA,   
    .senseRes = SENSE_RES,
    .DividerRes = DIVIDER_RES,
    .numsamples = NUM_SAMPLES,      
    .numsamplescurrents = NUM_SAMPLES_CURRENTS, 
    .undervoltage = UNDER_VOLTAGE,
    .overvoltage = OVER_VOLTAGE, 
    .overcurrent= OVER_CURRENT,
	};

	myWattmeter.initWattmeter(my_config);
#endif

#ifdef COMPILE_OCPP
		Serial.print(F("[main] Wait for WiFi: "));

    WiFi.begin(ssid, password);
		if(WiFi.status() == WL_CONNECTED) Serial.print(F("Conected\n"));
		    WiFi.begin(ssid, password);
    while (!WiFi.isConnected())
    {
        Serial.print('.');
        delay(1000);
     }
		Serial.print(F("Connected.\n"));
#endif

#ifdef COMPILE_ME
	DataStruct.currentSetByUser = 32; // Valor de corrente externo usuario/APP/OCPP
	DataStruct.enableButton = true;
	DataStruct.startChargingByUser = 0;	 // valor alterado para iniciar ou encerrar recarga usuario/APP/OCPP

	gpio_set_direction(PWM_PIN, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(PILOT_PIN, GPIO_MODE_INPUT);		
	gpio_set_direction(PINO_PROXIMIDADE, GPIO_MODE_INPUT);
	gpio_set_direction(LED_A, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(LED_C, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(LED_D, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(RELE_N, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(RELE_L1, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(RELE_L2, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(RELE_L3, GPIO_MODE_OUTPUT);			 // Define pino como saida
	gpio_set_direction(START_RECHARGER_BT, GPIO_MODE_INPUT); // Define pino como entrada
	gpio_set_direction(GPIO_NUM_37, GPIO_MODE_OUTPUT); // Define pino como saida
	
	// CONFIGURA OS CANAIS ADC ---------------------------------------------------------------------------
	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, 0, &adc_chars);
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(CHANNEL_PILOT, ADC_ATTEN_DB_11);  //pino CP
	adc1_config_channel_atten(CHANNEL_PROXIMIDADE, ADC_ATTEN_DB_11);  //pino PP
	adc1_config_channel_atten(CHANNEL_FAULT, ADC_ATTEN_DB_11);  //pino PP


	// CONFIGURA NEW PWM
	ledc_timer_config_t ledc_timer = {
		.speed_mode = SPEED_MODE_TIMER,
		.duty_resolution = LEDC_TIMER_10_BIT,
		.timer_num = LEDC_TIMER_0,
		.freq_hz = 1000,
		.clk_cfg = LEDC_AUTO_CLK};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel.channel = LEDC_CHANNEL_0;
	ledc_channel.duty = 0;
	ledc_channel.gpio_num = PWM_PIN;
	ledc_channel.speed_mode = SPEED_MODE_TIMER;
	ledc_channel.hpoint = 0;
	ledc_channel.timer_sel = LEDC_TIMER_0;
	ledc_channel_config(&ledc_channel);

	// CONFIGURA O TIMER E INTERRUPCAOO PRINCIPAL-------------------------------------------------------------------------
	const esp_timer_create_args_t my_timer_args = {
		.callback = &timer_callback,
		.name = "My Timer"};
	esp_timer_handle_t timer_handler;
	ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
	ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 167)); // 167 u,f= 6kHz P/ler 6 amostras de um ciclo PWM
#endif

#ifdef COMPILE_D_RES_CURR
gpio_set_direction(PIN_FAULT, GPIO_MODE_INPUT);
gpio_set_direction(PIN_TRIG_AC, GPIO_MODE_INPUT);
gpio_set_direction(PIN_TRIG_AC, GPIO_MODE_INPUT);
adc1_config_channel_atten(CHANNEL_FAULT, ADC_ATTEN_DB_11); 
#endif

#ifdef COMPILE_OCPP
	mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, "Intral Wallbox", "Intral");

	setEnergyMeterInput([]()
						{ return 10.f; });

	setSmartChargingCurrentOutput([](float limit)
								  {
	      Serial.printf("[main] Smart Charging allows maximum charge rate: %.0f A\n", limit);
	      return 32.f; },
								  connectorId);
#endif

#ifdef COMPILE_WATT
	xTaskCreate(wattmeterTask, "Wattmeter Task", 10000, NULL, 1, NULL);
#endif

#ifdef COMPILE_D_RES_CURR
//	xTaskCreate(monitorCurrentTask, "Wattmeter Task", 10000, NULL, 2, NULL);
#endif
}


void loop()
{
#ifdef COMPILE_OCPP
	mocpp_loop();
#endif

	if (Serial.available() > 0) { // Verifica se há dados disponíveis para leitura
			int incomingByte = Serial.read() - '0'; // Lê o byte disponível e converte para int
			bool value = (incomingByte != 0); // Converte o valor lido para true se for diferente de zero, false se for zero

			if (value) {
				DataStruct.statePinDC = 1;
				//DataStruct.enableButton = true;
				//DataStruct.startChargingByUser = true;
			} 
			if(value == false){
					DataStruct.statePinDC = 0;
					//DataStruct.enableButton = false;
					//DataStruct.startChargingByUser = false;
			}
		}
}