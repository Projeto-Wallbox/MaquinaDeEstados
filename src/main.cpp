#include <stdio.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"
#include "MaquinaDeEstados.h"

//DEFINIR PINOS--------------------------------------------------------------------------------------------------
gpio_num_t PINO_PILOTO = GPIO_NUM_32;        					//Pino para leitura AD do Sinal VA
gpio_num_t PINO_PROXIMIDADE = GPIO_NUM_33;   					//Pino para leitura AD do do cabo conectado
gpio_num_t PINO_PWM = GPIO_NUM_15;           					//Pino no qual é gerado o sinal PWM
gpio_num_t RELE_N = GPIO_NUM_18;   										//Pino de saida, para acionar o Relé do Neutro (N)
gpio_num_t RELE_L1 = GPIO_NUM_19;   									//Pino de saida, para acionar o Relé da fase 1 (L1)
gpio_num_t RELE_L2 = GPIO_NUM_21;   									//Pino de saida, para acionar o Relé da fase 2 (L2)
gpio_num_t RELE_L3 = GPIO_NUM_22;   									//Pino de saida, para acionar o Relé da fase 3 (L3)


gpio_num_t LED_ESTADO_B = GPIO_NUM_2;       					//Pino de saida, led indicando veiculo conectado
gpio_num_t LED_ESTADO_C = GPIO_NUM_4;       					//Pino de saida, led indicando veiculo fechou S2, em recarga (C2)
gpio_num_t LED_ESTADO_ERRO = GPIO_NUM_5;       				//Pino de saida, com led indicando erro

gpio_num_t BT_INICIAR_RECARGA = GPIO_NUM_14;   				//Pino de entrada, para setar o inicio da recarga pela Estacao 
gpio_num_t BT_ESTADO_F = GPIO_NUM_34;   				      //Pino de entrada, para alterar erros pela Estacao 


//DEFINIR VARIAVEIS--------------------------------------------------------------------------------------------
static ledc_channel_config_t ledc_channel;        
static esp_adc_cal_characteristics_t adc_chars;  			//Fornecido pela Esressif para calibracao do ADC
int Razao_Ciclica_PWM = 1023;  												//Variavel que armazena valor da razao cicllica

//FUNCAO DE INTERRUPCAO DO TIMER
void timer_callback(void *param){
	Razao_Ciclica_PWM = funcaoInterrupcao();  
	ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, Razao_Ciclica_PWM);  //Configura razao do PWM no canal
	ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);                  //Atualiza a configuração da razão
}

void setup()
{
	Dados.Corrente_Usuario = 22;                  //Valor de corrente externo usuario/APP/OCPP
	Dados.Iniciar_Recarga = 0;                    //valor alterado para iniciar ou encerrar recarga usuario/APP/OCPP

	gpio_set_direction(PINO_PWM, GPIO_MODE_OUTPUT);  					//Define pino como saida
	gpio_set_direction(LED_ESTADO_C, GPIO_MODE_OUTPUT);  			//Define pino como saida
	gpio_set_direction(LED_ESTADO_B, GPIO_MODE_OUTPUT);  			//Define pino como saida
	gpio_set_direction(LED_ESTADO_ERRO, GPIO_MODE_OUTPUT);    //Define pino como saida
	gpio_set_direction(RELE_N, GPIO_MODE_OUTPUT);  						//Define pino como saida
	gpio_set_direction(RELE_L1, GPIO_MODE_OUTPUT);  					//Define pino como saida
	gpio_set_direction(RELE_L2, GPIO_MODE_OUTPUT);    				//Define pino como saida
	gpio_set_direction(RELE_L3, GPIO_MODE_OUTPUT);    				//Define pino como saida
	gpio_set_direction(BT_INICIAR_RECARGA, GPIO_MODE_INPUT);  //Define pino como entrada
	gpio_set_direction(BT_ESTADO_F, GPIO_MODE_INPUT);  				//Define pino como entrada


	//CONFIGURA OS CANAIS ADC ---------------------------------------------------------------------------
	esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_12Bit, 0, &adc_chars);
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); //Pino 32, piloto
	adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11); //Pino 33, proximidade

	//CONFIGURA PWM ----------------------------------------------------------------------------------------
	ledc_timer_config_t ledc_timer = {
			   	.speed_mode = LEDC_HIGH_SPEED_MODE,
          .duty_resolution = LEDC_TIMER_10_BIT,
          .timer_num = LEDC_TIMER_0,
	        .freq_hz = 1000,
	        .clk_cfg = LEDC_AUTO_CLK,
		};

	ledc_timer_config(&ledc_timer);
	ledc_channel.channel = LEDC_CHANNEL_0;
	ledc_channel.duty = 0;
	ledc_channel.gpio_num = PINO_PWM;
	ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel.hpoint = 0;
	ledc_channel.timer_sel = LEDC_TIMER_0;
	ledc_channel_config(&ledc_channel);

	//CONFIGURA O TIMER E INTERRUPCAOO PRINCIPAL-------------------------------------------------------------------------
	const esp_timer_create_args_t my_timer_args = {
		.callback = &timer_callback,
		.name = "My Timer"};
	esp_timer_handle_t timer_handler;
	ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
	ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 167));     //167 u,f= 6kHz P/ler 6 amostras de um ciclo PWM
}

void loop(){

}
