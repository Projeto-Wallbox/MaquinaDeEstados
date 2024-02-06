#ifndef MAQUINA_DE_ESTADOS_H
#define MAQUINA_DE_ESTADOS_H
#include "driver/gpio.h"
#include <iostream>
#include <string.h>

extern gpio_num_t PILOT_PIN;      // Pino para leitura AD do Sinal VA
extern gpio_num_t PINO_PROXIMIDADE; // Pino para leitura AD do do cabo conectado
extern gpio_num_t PWM_PIN;         // Pino no qual é gerado o sinal PWM

extern gpio_num_t RELE_N;  // Pino de saida, para acionar o Relé do Neutro (N)
extern gpio_num_t RELE_L1; // Pino de saida, para acionar o Relé da fase 1 (L1)
extern gpio_num_t RELE_L2; // Pino de saida, para acionar o Relé da fase 2 (L2)
extern gpio_num_t RELE_L3; // Pino de saida, para acionar o Relé da fase 3 (L3)

extern gpio_num_t START_RECHARGER_BT; // Pino de entrada, para setar o inicio da recarga pela Estacao

extern gpio_num_t LED_A; // Led de EVSE ON/OF
extern gpio_num_t LED_B; // Led de carregamento
extern gpio_num_t LED_C; // Led de conexao a rede Wi-fi
extern gpio_num_t LED_D; // Led de erro ou falha

extern adc1_channel_t CHANNEL_PILOT; //Configuração do canal ADC para o CP
extern adc1_channel_t CHANNEL_PROXIMIDADE; //Configuração do canal ADC para o PP

// Definição da estrutura global
struct GlobalStruct
{
    int vehicleState;   // Salva o estado do veiculo
    int cableCurrent; // Salva a corrente do cabo
    int maximumCurrent;  // Salva a corrente máxima entre Cabo e corrente definida pelo usuário
    int stationCurrent; // Corrente deifinida em chargingStation(pode ser 0 ou Corrente_Maxima)
    int dutyCycle;    // Salva a razao ciclica
      

    // variaveis a serem utilizadados para interagir com OCPP
    bool mcAvailable; // estacao disponivel
    bool mcPreparing; // veículo conectado
    bool mcCharging;  // Em recarga

    bool mcSuspendedEV;   // Recarga finalizada pelo EV
    bool mcSuspendedEVSE; // Recarga finalizada pela EVSE
    bool mcFinishing;
    bool mcReserved;
    bool mcUnavailable;
    bool mcFaulted; // Foi detectado uma falha

    int historyState; // Variavel de historico para definir a transicao entre os estados "12 -> 9" e "6 -> 9"

    // Variaveis externas
    int currentSetByUser; // Valor de corrente definido pelo usuário
    int startChargingByUser;  // Se for 1, coloca a razao ciclica para o valor de corrente existente, caso contário razao em 100%

    // Variaveis externas só para printar na tela
    int Contador_C;  // salva valor utilizado para transicao 10.2
    int Contador_BT;  //Botao pressionado 
    int Ad_Proximidade; //Valor da leitura AD PP
    int Media_Piloto;
    int Bt_Estado;
    int newState;

    //Parte para falhas
    std::string typeError = "---";
};

// Declaração da variável global
extern GlobalStruct DataStruct;

int funcaoInterrupcao();
int positivaPiloto(int piloto);
int correnteCabo(int proximidade);
int defineEstado(int media_x1);
int chargingStationMain(int estado, int corrente_cabo);
void acendeLed();
void leBotao();
void dispositivoDeManobra(int acao);
void printTela();
void stateMachineControl(int state, int dutyCycle);
bool monitorFaultStatus();

#endif
