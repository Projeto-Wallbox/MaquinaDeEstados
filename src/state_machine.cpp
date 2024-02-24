//***** Bibliotecas *****//	
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "state_machine.h"
#include "wattmeter_sensor.h"
#include <Wire.h>  //Teste para plot

#ifdef COMPILE_OCPP
#include <Arduino.h>
#include <WiFi.h>
#include <MicroOcpp.h>
#include <MicroOcpp_c.h>
#include <MicroOcpp/Core/Configuration.h>

const int connectorId = 1;
#endif


GlobalStruct DataStruct; //Inicializa estrutura de dados (VER MELHOR FORMA)

//Funcao que deve ser chamada na Interrupcao
int funcaoInterrupcao()
{ 
	// Esta funcao e chamada a cada 16,6 us
	// Variaveis para os contadores das interrupcoes
	static int cont_principal = 0;
	static int cont_atualiza = 0;
	static int cont_interfaceUsuario = 0;

	// Variaveis da Leitura do Piloto
	static int medida_piloto = 4000;
	static int media_piloto = 4000;

	// Variaveis da Funcao principal da Estacao
	static int medida_proximidade = 0;
	static int estado_veiculo = 12;
	static int cabo_conectado = 0;
	static int razao_ciclica = 1000;
	static int corrente_maxima;

	// measure_one++;
	cont_principal++;
	cont_atualiza++;
	cont_interfaceUsuario++;

	// a cada 166 us (6kHz)
	DataStruct.statePinDC = gpio_get_level(PIN_TRIG_DC);
  DataStruct.statePinAC = gpio_get_level(PIN_TRIG_AC);

	medida_piloto = adc1_get_raw(CHANNEL_PILOT);	 // Leitura do piloto (1).																			
	media_piloto = positivaPiloto(medida_piloto); // Calcula a média dos sinais (2)
	DataStruct.vehicleState = estado_veiculo;  //atualiza estado na struct
	monitorFaultStatus();
	stateMachineControl(estado_veiculo, razao_ciclica);
	
	if (cont_principal >= 6) // a cada 1 ms (1kHz)
	{
		//TESTE DE LEITURA DO ESTADO E0
	  if(DataStruct.vehicleState == 0){
			//Colocar razão ciclica para 100%
			//Acionar o circuito de alimentação extra via capacitor/bateria
			//printf("ACIONAR BATERIA");   /// em teste
		}

		cont_principal = 0;
		medida_proximidade = adc1_get_raw(CHANNEL_PROXIMIDADE);     // Faz a leitura analogica do proximidade (3)
		cabo_conectado = correnteCabo(medida_proximidade);		 // Identificacao do Cabo (4)
		estado_veiculo = defineEstado(media_piloto);           // Determina o Estado (5)
	
		if(DataStruct.currentSetByUser < cabo_conectado && DataStruct.currentSetByUser<=32){ //logica de maior corrente suportada 
			corrente_maxima = DataStruct.currentSetByUser;
		}else if(cabo_conectado<32){
			corrente_maxima = cabo_conectado;
			}else{corrente_maxima = 32;}
	
		razao_ciclica = chargingStationMain(estado_veiculo, corrente_maxima); // MAQUINAS DE ESTADOS, CONTATORA E CALCULO DA RAZAO CICLICA    (6 e 7)
	}
	else
	{
		if (cont_atualiza >= 600) // a cada 100 ms ( 10 Hz)
		{
			//Atuzaliza os dados da estrura "maquinaDeEstados"
			leBotao();
			DataStruct.cableCurrent = cabo_conectado;
			DataStruct.maximumCurrent = corrente_maxima;
			DataStruct.dutyCycle = razao_ciclica;
			DataStruct.Media_Piloto = media_piloto;
			DataStruct.Ad_Proximidade = medida_proximidade;
			DataStruct.vehicleState = estado_veiculo;
			cont_atualiza = 0;

		}
		else
		{
			if (cont_interfaceUsuario >= 6000) // a cada 1000 ms (1 Hz)
			{ 
				//acendeLed();
				printTela();
				cont_interfaceUsuario = 0;
			}
		}
	}
	return razao_ciclica;
}

//Função que calcula média dos sinais lidos no AD Piloto
int positivaPiloto(int piloto)
{
	static int j=0;											//Lógica de determinação do valor máximo de um período
	static int media[5]={0,0,0,0,0};			//5 maiores valores medidos em um período
	static int media_piloto=4000;
	
	if(j>=120)		//Quando atinge 120 amostras reinicia o processo de coleta de dados
	{
		media[0]=0;
		media[1]=0;
		media[2]=0;
		media[3]=0;
		media[4]=0;
		j=0;
		media_piloto=media_piloto;
	}
	else							//Quando ainda não atingiu 120 amostras continua o processo de amostragem
	{
		j++;
		//Se a amostra for maior armazene em um dos espaços
		if((piloto>media[0])||(piloto>media[1])||(piloto>media[2])||(piloto>media[3])||(piloto>media[4]))
		{
			if(piloto>media[0])
			{
				media[0]=piloto;
			}
			else
			{	
				if(piloto>media[1])
				{
					media[1]=piloto;
				}
				else
				{		
					if(piloto>media[2])
					{
						media[2]=piloto;
					}		
					else
					{
						if(piloto>media[3])
						{
							media[3]=piloto;
						}
						else
						{
							if(piloto>media[4])
							{
								media[4]=piloto;
							}
							else
							{
								media[0]=media[0];
								media[1]=media[1];
								media[2]=media[2];
								media[3]=media[3];
								media[4]=media[4];
							}
						}
					}
				}
			}
		}
		else				//Se for menor ou igual não armazene
		{
			media_piloto=media_piloto;
		}
		if(j<120)		//Antes da amostra 120 não atualize a média do piloto, ou seja, não atualize a média dentro de 20 ms da última atualização
		{
			media_piloto=media_piloto;
		}
		else				//Na amostra 120 atualize a média piloto, ou seja, 20 ms após a última atualização
		{
			media_piloto=(media[0]+media[1]+media[2]+media[3]+media[4])/5;
		}
	}
	return media_piloto;
	
}

//Funcao que define o cabo conectado, de acordo com a Leitura AD Proximidade
int correnteCabo(int proximidade) {
	static int corrente_cabo=0;
	
	//Fórmula de cálculo: Exemplo cabo 32 A (Nominal 220 ohms, 164 a 308 ohms), para 164 ohms temos: {[(164)/(164+470)]*4095}
	
	if(proximidade<(464))															//Menor que 60 ohms (curto-circuito)
	{
		corrente_cabo=0;
	}
	else
	{
		if(proximidade>(595)&&(proximidade<(940))) 				//Entre 80 e 140 ohms (nominal 100 ohms)
		{
			corrente_cabo=63;
		}
		else
		{
			if(proximidade>(1059)&&(proximidade<(1622)))			//Entre 164 e 308 ohms (nominal 220 ohms)
			{
				corrente_cabo=32;
			}
			else
			{
				if(proximidade>(1882)&&(proximidade<(2726)))			//Entre 400 e 936 ohms (nominal 680 ohms)
				{
					corrente_cabo=20;
				}
				else
				{
					if(proximidade>(2869)&&(proximidade<(3439))) 		//Entre 1100 e 2460 ohms (nominal 1500 ohms)
					{
						corrente_cabo=13;
					}
					else
					{
						if(proximidade>(3707))													//Maior que 4500 ohms (circuito aberto)
						{
							corrente_cabo=0;
						}
						else
						{
							corrente_cabo=0;
						}
					}
				}
			}
		}
	}
	return corrente_cabo;
}

//Funcao que define o Estado de acordo com a media dos valores AD Piloto
int defineEstado(int media_x1)
{	static int estado=12;		//Inicia do Estado A
	
	//Fórmula de cálcula: Exemplo estado C (+5 a +7 V), para 5 V temos: {[(5+12)/(24)]*4095}

	if(media_x1>3924)				//Maior que +11 V
	{
		estado=12;		
	}
	else
	{
		if((media_x1>3412)&&(media_x1<3900))		//Entre +8 V e +10 V
		{
		estado=9;
		}
		else
		{
			if((media_x1>2900)&&(media_x1<3300))		//Entre +5 V e +7 V
			{
				estado=6;
			}
			else
			{
				if((media_x1>2389)&&(media_x1<2730))		//Entre +2 V e +4 V
				{
					estado=3;
				}
				else
				{
					if((media_x1>1877)&&(media_x1<2219))		//Entre -1 V e +1 V
					{
						estado=0;
					}
					else
					{
						if(media_x1<1800)												//Entre -1 V e -12 V
						{
							estado=-12;
						}
						else
						{
							estado=estado;					//Se está na faixa de nenhum estado, mantenha no estado anterior.
						}
					}
				}
			}
		}
	}
	return estado;
}

//Funcao que contém a logica de comutacao do dispositivo de manobra
int chargingStationMain(int estado, int corrente_max)
{	//Entra nessa lógica 1 vez a cada ciclo do PWM (1 kHz)
	
	static int razao=1023;											//Razão cíclica do PWM
	static int k=0;															//Contador do bloqueio da contatora
	static int m=0;												      //Contador do bloqueio da razão cíclica
	static int corrente_da_estacao=0;			      //Corrente máxima que a estaçao irá fornecer
	static int cont = 0;
	static bool estado_F=false;									//Altera o valor da razão cíclica para 0 quando true (estação com algum erro/não estiver pronta)
	static bool estadoDispositivoManobra=false;	//Estado do dispositivo de manobra
	static bool bloqueio_contatora=false;				//Variável que bloqueia a contatora por 6 segundo caso entre no modo ventilação
	static bool bloqueio_razao_ciclica=false;		//Variável que bloqueia a alteração da razão cíclica por 5 segundos
	static bool iniciar_recarga=false;					//Variável para autorizar o inicio de recarga

	
	iniciar_recarga = DataStruct.startChargingByUser;
	DataStruct.Contador_C = cont;
	DataStruct.stationCurrent = corrente_da_estacao; 
//******Lógica que decide qual será a corrente máxima da estação e o estado da contatora*********************
	//Lógica para os Estados: A, B, E e F ou cabo desconectado
	if((estado==12)||(estado==9)||(estado==0)||(estado==-12)||(corrente_max==0))			
	{
		estadoDispositivoManobra=false;
		dispositivoDeManobra(estadoDispositivoManobra);
		bloqueio_contatora=false;
		k=0;
		//cont = 0;
		if(estado == 9 && iniciar_recarga== true)		//Estado B2
		{
			corrente_da_estacao=corrente_max;
		}
		else
		{// Estado B1
			corrente_da_estacao=0;
		}

		if(estado==9){cont = 0;}
		if(estado==12){DataStruct.startChargingByUser = 0;}
	}
	//Lógica para os Estados C e D com cabo conectado
	else
	{	
		//Lógica para o Estado C
		if(estado==6 && iniciar_recarga == true)
		{
			k=0;
			corrente_da_estacao=corrente_max;
			if(razao!=1023 && iniciar_recarga == true)			//A contatora não pode fechar antes de o PWM indicar a estação como pronta
			{
				cont = 0;
				bloqueio_contatora=true;
				estadoDispositivoManobra=true;
				dispositivoDeManobra(estadoDispositivoManobra);
			}             
		}
		//Lógica para o Estado D
		else
		{ // Lógica de abertura da contatora, não responta ao término da recarga (Transição 10.2)
			if(cont >= 6000){   //Minimo 6 segundos
							estadoDispositivoManobra = 0;
							dispositivoDeManobra(estadoDispositivoManobra);
					}else{
							estadoDispositivoManobra = 1;
							dispositivoDeManobra(estadoDispositivoManobra);
							corrente_da_estacao=0;
							cont++;
					}
			//Lógica para transição do Estado C para o D com a contatora já fechada
			if((bloqueio_contatora==true)&&(estado==3))
			{
				k++;
				corrente_da_estacao=0;
				estadoDispositivoManobra=true;
				dispositivoDeManobra(estadoDispositivoManobra);
				if(k>=6000)			//Lógica para fazer esperar 6 segundos após a transição do Estado C para o D com a contatora já fechada
				{
					bloqueio_contatora=false;
					k=0;
				}
			}
			//Lógica para 6 segundos após a transição do Estado C para o D ou contatora aberta durante a transição
			else
			{

				k=0;
				bloqueio_contatora=false;
				//corrente_da_estacao=0; 
				//estadoDispositivoManobra=false;
				//dispositivoDeManobra(estadoDispositivoManobra);
			}
		}
	}

	//************Codificação da corrente máxima da estação através da razão cíclica do PWM***********************
	//Lógica caso a estação não esteja pronta para fornecer energia
	//Lógica do bloqueio da razão cíclica
	if(bloqueio_razao_ciclica==true)
	{
		m++;
		if(m>=5000)
		{
			bloqueio_razao_ciclica=false;
			m=0;
		}
	}

	//Lógica caso a razão cíclica deva ser atualizada
	if(bloqueio_razao_ciclica==false)
	{	
		if(corrente_da_estacao>=6 && corrente_da_estacao<=32){
				razao = ((corrente_da_estacao/0.6)*(1023))/100;
		}else{razao = 1023;}
		
		bloqueio_razao_ciclica=true;
	}

	//Mantém a razão cíclica em 0%, enquando o erro persistir
	if(DataStruct.state_F == 1){
		razao = 0;
	}
	return razao;
}

//Funcao para controle dos led indicadores ()
void acendeLed(){
	static bool ledB = false;
	static bool ledEstadoErro = false;

//LED A - Led EVSE on/off
	//gpio_set_level(LED_A, true);

//LED B - Led de carregamento(veiculo conectado ou veiculo carregando) 
		if(DataStruct.vehicleState==9 || DataStruct.vehicleState==6)
		{

			if(DataStruct.vehicleState==9 ||(DataStruct.vehicleState==6&& DataStruct.mcCharging == false))//Veiculo conectado
			{
				gpio_set_level(LED_B, true);
			}
			if(DataStruct.vehicleState == 6 && DataStruct.mcCharging == true)//Veiculo carregando
			{
				ledB = !ledB;
				gpio_set_level(LED_B, ledB); 
  		}
		}else{gpio_set_level(LED_B, false);}

//LED D - Led de erro ou falha
	if(DataStruct.vehicleState == 0 || DataStruct.vehicleState == -12 ){
		gpio_set_level(LED_D, true);
	}else{gpio_set_level(LED_D, false);}

}

//Funcao para controle do incio/fim de recarga pelo botao físico
void leBotao(){ 
		static int cont = 0;
		DataStruct.Contador_BT = cont;
    int bt_estado = gpio_get_level(START_RECHARGER_BT);
     
    if (bt_estado == 1) {
        cont++;
    } else {cont = 0;}

    if (cont >= 1 && cont <= 5) {
        DataStruct.startChargingByUser = 1;
    }

    if (cont >= 30) {
        DataStruct.startChargingByUser = 0;
    }
}

//Funcao para controle do dispositivo de manobra(relés)
void dispositivoDeManobra(int acao){
	if(acao == 1){
		gpio_set_level(RELE_L1, true); // Liga Dispositivo de manobra
		gpio_set_level(RELE_L2, true); // Liga Dispositivo de manobra
		gpio_set_level(RELE_L3, true); // Liga Dispositivo de manobra
		gpio_set_level(RELE_N, true); // Liga Dispositivo de manobra
	}else{
		gpio_set_level(RELE_L1, false); // Desliga Dispositivo de manobra
		gpio_set_level(RELE_L2, false); // Desliga Dispositivo de manobra
		gpio_set_level(RELE_L3, false); // Desliga Dispositivo de manobra
		gpio_set_level(RELE_N, false); // Desliga Dispositivo de manobra
	}
}

//Funcao auxiliar só para printar na tela (Temporária)
void printTela(){
	// Serial.print(">Estado: ");
	// Serial.println(DataStruct.vehicleState);
	
	printf("Estado: %d\n", DataStruct.vehicleState);
	printf("AD CP: %d\n\n", DataStruct.Media_Piloto);
	
	// Serial.print(">Cabo: ");
	// Serial.println(DataStruct.cableCurrent);
	printf("Cabo: %d\n", DataStruct.cableCurrent);
	printf("AD PP: %d\n\n", DataStruct.Ad_Proximidade);
	// printf("Corrente_usuario: %d\n", Dados.Corrente_Usuario);
	// printf("Corrente_max: %d\n", Dados.Corrente_Maxima);
	
	// Serial.print(">Iniciar_Recarga: ");
	// Serial.println(DataStruct.startChargingByUser);
	printf("Iniciar_Recarga: %d\n", DataStruct.startChargingByUser);
	
	// Serial.print(">Razao: ");
	// Serial.println(DataStruct.dutyCycle);

	// Serial.print(">statePinDC: ");
	// Serial.println(DataStruct.statePinDC);

	// Serial.print(">statePinAC: ");
	// Serial.println(DataStruct.statePinAC);
	
	printf("Razao: %0.2f %%\n\n", static_cast<float>((DataStruct.dutyCycle*100)/1023.0f));
	// printf("%");
	//printf("Contador C: %d\n", DataStruct.Contador_C);
	//printf("Contador BT: %d\n", DataStruct.Contador_BT);

	printf("Tensão L1: %0.3f   Corrente L1: %0.3f", myWattmeter.getFilteredVolts(1), myWattmeter.getFilteredCurrents(1));
	printf("  Pot L1: %0.3f", myWattmeter.getPowerApparent());
	
	printf("\nTensão L2: %0.3f   Corrente L2: %0.3f", myWattmeter.getFilteredVolts(2), myWattmeter.getFilteredCurrents(2));
	printf("\nTensão L3: %0.3f   Corrente L3: %0.3f", myWattmeter.getFilteredVolts(3), myWattmeter.getFilteredCurrents(3));

	printf("\n\nEnergia: %0.3f", myWattmeter.getEnergy());

	printf("\n\nAvailable: %d\n", DataStruct.mcAvailable);
	printf("Preparing: %d\n", DataStruct.mcPreparing);
	printf("Charging: %d\n", DataStruct.mcCharging);
	printf("Finishing: %d\n", DataStruct.mcFinishing);
	printf("Faulted: %d\n", DataStruct.mcFaulted);
	printf("Tipo de falha: %s\n", DataStruct.typeError.c_str());

	printf("statePinDC: %d\n", DataStruct.statePinDC);
	printf("statePinAC: %d", DataStruct.statePinAC);

	printf("\n-----------------------------------------------------\n");
}

void stateMachineControl(int state, int dutyCycle){
	if((state == -12 || state == 0) && DataStruct.mcFaulted == false){ 
		DataStruct.mcFaulted=true;
		DataStruct.mcAvailable=false;
		DataStruct.mcPreparing=false;
		DataStruct.mcFinishing=false;
		DataStruct.mcCharging = false;
	}

	if(state == 12 && dutyCycle == 1023 && DataStruct.mcAvailable == false){  
		DataStruct.mcAvailable = true;
		DataStruct.mcCharging = false;
		DataStruct.mcPreparing=false;
		DataStruct.mcFinishing=false;
		DataStruct.mcFaulted = false;
		DataStruct.historyState = 12;
#ifdef COMPILE_OCPP
		setConnectorPluggedInput([](){return false;}, connectorId);
#endif
	}
	// # Todo colocar o motivo da parada se foi local ou pelo veiculo para poder enviar no endtransaction do ocpp

	if(state == 9 && DataStruct.mcPreparing==false && DataStruct.historyState == 12){ //12 -> 9 preparing
		DataStruct.mcPreparing=true;
		DataStruct.mcAvailable = false;
		DataStruct.mcCharging = false;		
		DataStruct.mcFinishing=false;
		DataStruct.mcFaulted = false;
#ifdef COMPILE_OCPP
		setConnectorPluggedInput([](){return true;}, connectorId);
#endif
	}

	if(state == 9 && DataStruct.mcFinishing==false && DataStruct.historyState == 6){ //6 -> 9 finishing 
		DataStruct.mcFinishing=true;
		DataStruct.mcPreparing=false;
		DataStruct.mcAvailable = false;
		DataStruct.mcCharging = false;		
		DataStruct.mcFaulted = false;
#ifdef COMPILE_OCPP
		stopTransaction();
#endif
	}



	if(state == 6 && dutyCycle!=1023 && DataStruct.mcCharging == false){
		printf("mcCharging = if\n");
		DataStruct.mcCharging = true;
		DataStruct.mcPreparing=false;
		DataStruct.mcFinishing=false;
		DataStruct.mcFaulted = false;
		DataStruct.mcAvailable = false;
		DataStruct.historyState = 6;
#ifdef COMPILE_OCPP
		startTransaction();
#endif

	}
}

void monitorFaultStatus(){
	bool stateFault = false;

	if(DataStruct.vehicleState==12||DataStruct.vehicleState==9||DataStruct.vehicleState==6||DataStruct.vehicleState==3){
		stateFault = false;
		DataStruct.typeError = "----";
	}

	//Falha no condicionamento do sinal PWM (melhorar)
	if(DataStruct.dutyCycle == 1023 && DataStruct.vehicleState == -12){
		stateFault = true;   //Alterar 
		DataStruct.typeError = "Falha no condicionamento do PWM";
	}else{
		stateFault = false;
	}

	// Curto entre CP e PE
	if(DataStruct.dutyCycle == 1023 && DataStruct.vehicleState == 0){
		DataStruct.typeError = "Estado E - Curto entre CP e PE ou sem alimentação(+12 e -12)";
	}
	
	// //Fuga CC detectada
	// if(DataStruct.statePinDC == 0){
	// 	stateFault = true;
	// 	DataStruct.state_F = 1;
	// 	DataStruct.typeError = "Fuga CC de 6 mA";
	// }else{
	// 	stateFault = false;
	// 	DataStruct.state_F = 0;
	// 	DataStruct.typeError = "----";
	// }

	// //Fuga CA detectada
	// if(DataStruct.statePinAC == 0){
	// 	stateFault = true;
	// 	DataStruct.state_F = 1;
	// 	DataStruct.typeError = "Fuga CA de 30 mA";
	// }else{
	// 	stateFault = false;
	// 	DataStruct.state_F = 0;
	// 	DataStruct.typeError = "----";
	// }
	// Sobretensão

	// Subtensão

	//
	
}