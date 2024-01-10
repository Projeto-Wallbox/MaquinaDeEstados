#include <stdio.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_adc_cal.h"
#include "state_machine.h"

#include <Arduino.h>
#include <WiFi.h>
#include <MicroOcpp.h>
#include <MicroOcpp_c.h>
#include <MicroOcpp/Core/Configuration.h>

//DEFINIR PINOS--------------------------------------------------------------------------------------------------

#ifdef ESP32_DEV
gpio_num_t PINO_PILOTO = GPIO_NUM_32;		 // Pino para leitura AD do Sinal VA
gpio_num_t PINO_PROXIMIDADE = GPIO_NUM_33;	 // Pino para leitura AD do do cabo conectado
gpio_num_t PINO_PWM = GPIO_NUM_15;			 // Pino no qual é gerado o sinal PWM
gpio_num_t RELE_N = GPIO_NUM_26;			 // Pino de saida, para acionar o Relé do Neutro (N)
gpio_num_t RELE_L1 = GPIO_NUM_19;			 // Pino de saida, para acionar o Relé da fase 1 (L1)
gpio_num_t RELE_L2 = GPIO_NUM_21;			 // Pino de saida, para acionar o Relé da fase 2 (L2)
gpio_num_t RELE_L3 = GPIO_NUM_22;			 // Pino de saida, para acionar o Relé da fase 3 (L3)
gpio_num_t LED_A = GPIO_NUM_2;				 // Led de EVSE ON/OF
gpio_num_t LED_B = GPIO_NUM_18;				 // Led de carregamento
gpio_num_t LED_C = GPIO_NUM_4;				 // Led de conexao a rede Wi-fi
gpio_num_t LED_D = GPIO_NUM_5;				 // Led de erro ou falha
gpio_num_t BT_INICIAR_RECARGA = GPIO_NUM_14; // Pino de entrada, para setar o inicio da recarga pela Estacao
#define SPEED_MODE_TIMER LEDC_HIGH_SPEED_MODE
#endif

#ifdef ESP32_S3
gpio_num_t PINO_PILOTO = GPIO_NUM_5;
gpio_num_t PINO_PROXIMIDADE = GPIO_NUM_7;
gpio_num_t PINO_PWM = GPIO_NUM_6;

// gpio_num_t RELE_N = GPIO_NUM_11;
// gpio_num_t RELE_L1 = GPIO_NUM_12;
// gpio_num_t RELE_L2 = GPIO_NUM_13;
// gpio_num_t RELE_L3 = GPIO_NUM_14;

gpio_num_t RELE_L1 = GPIO_NUM_11;
gpio_num_t RELE_L2 = GPIO_NUM_12;
gpio_num_t RELE_L3 = GPIO_NUM_46;
gpio_num_t RELE_N = GPIO_NUM_14;

gpio_num_t LED_A = GPIO_NUM_1;      //over_voltage
gpio_num_t LED_B = GPIO_NUM_2;      //under_voltage
gpio_num_t LED_C = GPIO_NUM_3;      //over_current
gpio_num_t LED_D = GPIO_NUM_10;     //fault_out
gpio_num_t BT_INICIAR_RECARGA = GPIO_NUM_9;
#define SPEED_MODE_TIMER LEDC_LOW_SPEED_MODE//LEDC_LOW_MODE_MAX
#endif

// DEFINIR VARIAVEIS--------------------------------------------------------------------------------------------
static ledc_channel_config_t ledc_channel;
static esp_adc_cal_characteristics_t adc_chars; // Fornecido pela Esressif para calibracao do ADC
int Razao_Ciclica_PWM = 1023;					// Variavel que armazena valor da razao cicllica

// FUNCAO DE INTERRUPCAO DO TIMER
void timer_callback(void *param)
{
	Razao_Ciclica_PWM = funcaoInterrupcao();
	// ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, Razao_Ciclica_PWM); // Configura razao do PWM no canal
	// ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);				 // Atualiza a configuração da razão

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, Razao_Ciclica_PWM));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

// const char *OCPP_BACKEND_URL = "ws://200.18.45.173:7589";
// const char *OCPP_CHARGE_BOX_ID = "IntrallWallbox";

// const char *ssid = "LabAT";
// const char *password = "inrilabat";
// unsigned int connectorId = 1;

void setup()
{
	Dados.Corrente_Usuario = 22; // Valor de corrente externo usuario/APP/OCPP
	Dados.Iniciar_Recarga = 0;	 // valor alterado para iniciar ou encerrar recarga usuario/APP/OCPP

	gpio_set_direction(PINO_PWM, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(LED_A, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(LED_B, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(LED_C, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(LED_D, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(RELE_N, GPIO_MODE_OUTPUT);  // Define pino como saida
	gpio_set_direction(RELE_L1, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(RELE_L2, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(RELE_L3, GPIO_MODE_OUTPUT); // Define pino como saida
	gpio_set_direction(BT_INICIAR_RECARGA, GPIO_MODE_INPUT); // Define pino como entrada

	// CONFIGURA OS CANAIS ADC ---------------------------------------------------------------------------
	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, 0, &adc_chars);
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); 
	adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

	//CONFIGURA NEW PWM
	ledc_timer_config_t ledc_timer = {
        .speed_mode       = SPEED_MODE_TIMER,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 1000, 
        .clk_cfg          = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel.channel = LEDC_CHANNEL_0;
	ledc_channel.duty = 0;
	ledc_channel.gpio_num = PINO_PWM;
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
	
	// Serial.begin(115200);
	// Serial.print(F("[main] Wait for WiFi: "));
	// WiFi.begin(ssid, password);
	// while (!WiFi.isConnected())
	// {
	// 	Serial.print('.');
	// 	delay(1000);
	// }

	// mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, "Intral Wallbox", "Intral");

	// setEnergyMeterInput([]()
	// 					{ return 10.f; });

	// setSmartChargingCurrentOutput([](float limit)
	// 							  {
  //       Serial.printf("[main] Smart Charging allows maximum charge rate: %.0f A\n", limit);
  //       return 32.f; },
	// 							  connectorId);

}

void loop()
{
	//mocpp_loop();
}
