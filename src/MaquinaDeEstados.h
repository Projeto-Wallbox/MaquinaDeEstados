#ifndef MAQUINA_DE_ESTADOS_H
#define MAQUINA_DE_ESTADOS_H
#include "driver/gpio.h"

extern gpio_num_t PINO_PILOTO;        			//Pino para leitura AD do Sinal VA
extern gpio_num_t PINO_PROXIMIDADE;   			//Pino para leitura AD do do cabo conectado
extern gpio_num_t PINO_PWM;           			//Pino no qual é gerado o sinal PWM
extern gpio_num_t RELE_N;   					//Pino de saida, para acionar o Relé do Neutro (N)
extern gpio_num_t RELE_L1;   					//Pino de saida, para acionar o Relé da fase 1 (L1)
extern gpio_num_t RELE_L2;   					//Pino de saida, para acionar o Relé da fase 2 (L2)
extern gpio_num_t RELE_L3;   					//Pino de saida, para acionar o Relé da fase 3 (L3)


extern gpio_num_t LED_ESTADO_B;       			//Pino de saida, com led indicando veiculo conectado
extern gpio_num_t LED_ESTADO_C;       			//Pino de saida, com led indicando veiculo fechou S2, em recarga (C2)
extern gpio_num_t LED_ESTADO_ERRO;       		//Pino de saida, com led indicando erro
extern gpio_num_t BT_INICIAR_RECARGA;   		//Pino de entrada, para setar o inicio da recarga pela Estacao 


//Definição da estrutura global
struct GlobalStruct {
    int Estado_Veiculo;
    int Corrente_Do_Cabo;
    int Corrente_Maxima;
    int Razao_Ciclica;
    bool Estacao_Carregando;
    
    //Variaveis externas
    int Corrente_Usuario;
    int Iniciar_Recarga; //Se for 1, coloca a razao ciclica para o valor de corrente existente, caso contário razao em 100%
    int Contador;
    int Finalizar_Recarga;  //Valor que indica que a estacao deve abrir dispositivo de manobra caso (Transicao 10.2)
    int Bt_Estado;
};

//Declaração da variável global
extern GlobalStruct Dados;

int funcaoInterrupcao();
int positivaPiloto(int piloto);
int correnteCabo(int proximidade);
int defineEstado(int media_x1);
int chargingStationMain(int estado, int corrente_cabo);
void acendeLed();
void leBotao();
void dispositivoDeManobra(int acao);
void printTela();

#endif
