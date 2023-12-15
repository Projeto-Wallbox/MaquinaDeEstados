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
extern gpio_num_t BT_ESTADO_F;   		        //Pino de entrada, para setar o estado F 


//Definição da estrutura global
struct GlobalStruct {
    int Estado_Veiculo;        //Salva o estado do veiculo
    int Corrente_Do_Cabo;      //Salva a corrente do cabo
    int Corrente_Maxima;       //Salva a corrente máxima entre Cabo e corrente definida pelo usuário
    int Razao_Ciclica;         //Salva a razao ciclica
    bool Estacao_Carregando;   //Indica se a estacao está carregando ou nao
    
    //Variaveis externas
    int Corrente_Usuario;          //Valor de corrente definido pelo usuário
    int Iniciar_Recarga;           //Se for 1, coloca a razao ciclica para o valor de corrente existente, caso contário razao em 100%
    
    //Variaveis externas só para printar na tela
    int Contador;
    int Bt_Estado;
    int Media_Piloto;
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
