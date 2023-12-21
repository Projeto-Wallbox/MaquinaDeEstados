//***** Bibliotecas *****//	
#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "MaquinaDeEstados.h"
GlobalStruct Dados; //Inicializa estrutura de dados (VER MELHOR FORMA)

//Funcao que deve ser chamada na Interrupcao
int funcaoInterrupcao()
{ // Esta funcao e chamada a cada 16,6 us
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
	medida_piloto = adc1_get_raw(ADC1_CHANNEL_4);	 // Leitura do piloto (1).																			
	media_piloto = positivaPiloto(medida_piloto); // Calcula a média dos sinais (2)

	Dados.Estado_Veiculo = estado_veiculo;  //atualiza estado na struct

	if (cont_principal >= 6) // a cada 1 ms (1kHz)
	{
		//TESTE DE LEITURA DO ESTADO E0
	  if(Dados.Estado_Veiculo == 0){
			//printf("ACIONAR BATERIA");   /// em teste
		}

		cont_principal = 0;
		medida_proximidade = adc1_get_raw(ADC1_CHANNEL_5);     // Faz a leitura analogica do proximidade (3)
		cabo_conectado = correnteCabo(medida_proximidade);		 // Identificacao do Cabo (4)
		estado_veiculo = defineEstado(media_piloto);           // Determina o Estado (5)
	
		if(Dados.Corrente_Usuario < cabo_conectado && Dados.Corrente_Usuario<=32){ //logica de maior corrente suportada 
			corrente_maxima = Dados.Corrente_Usuario;
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
			Dados.Corrente_Do_Cabo = cabo_conectado;
			Dados.Corrente_Maxima = corrente_maxima;
			Dados.Razao_Ciclica = razao_ciclica;
			Dados.Media_Piloto = media_piloto;

			cont_atualiza = 0;
		}
		else
		{
			if (cont_interfaceUsuario >= 6000) // a cada 1000 ms (1 Hz)
			{ 
				acendeLed();
				printTela();
				cont_interfaceUsuario = 0;
			}
		}
	}

	return razao_ciclica;
}

//Funcao que calcula media dos sinais lidos no AD Piloto
int positivaPiloto(int piloto)
{
	// printf("P_Piloto\n");
	static int j = 0;											 // L�gica de determina��o do valor m�ximo de um per�odo
	static int media[5] = {0, 0, 0, 0, 0}; // 5 maiores valores medidos em um per�odo
	static int media_piloto = 4000;

	if (j >= 1023) // if(j>=1023)
	{
		media[0] = 0;
		media[1] = 0;
		media[2] = 0;
		media[3] = 0;
		media[4] = 0;
		j = 0;
		media_piloto = media_piloto; // @suppress("Assignment to itself")
	}
	else
	{
		j++;
		if ((piloto > media[0]) || (piloto > media[1]) || (piloto > media[2]) || (piloto > media[3]) || (piloto > media[4]))
		{
			if (piloto > media[0])
			{
				media[0] = piloto + 1;
			}
			else
			{
				if (piloto > media[1])
				{
					media[1] = piloto + 1;
				}
				else
				{
					if (piloto > media[2])
					{
						media[2] = piloto + 1;
					}
					else
					{
						if (piloto > media[3])
						{
							media[3] = piloto + 1;
						}
						else
						{
							if (piloto > media[4])
							{
								media[4] = piloto + 1;
							}
							else
							{
								media[0] = media[0];
								media[1] = media[1];
								media[2] = media[2];
								media[3] = media[3];
								media[4] = media[4];
							}
						}
					}
				}
			}
		}
		else
		{
			media_piloto = media_piloto;
		}

		if (j < 923) // if(j<923)
		{
			media_piloto = media_piloto;
		}
		else
		{
			media_piloto = (media[0] + media[1] + media[2] + media[3] + media[4]) / 5;
		}
	}

	return media_piloto;
}

//Funcao que define o cabo conectado, de acordo com a Leitura AD Proximidade
int correnteCabo(int proximidade) {
	//printf("Coorente_Cabo\n");
	static int corrente_cabo=0;

	if(proximidade<(392))															//if(proximidade<(((60*3.3)/(60+470))*(4095/3.9)))
	{
		corrente_cabo=0;
	}
	else
	{
		if(proximidade>(546)&&(proximidade<(861))) 				//if(proximidade>(((80*3.3)/(80+470))*(4095/3.6))&&(proximidade<(((140*3.3)/(140+470))*(4095/3.6))))
		{
			corrente_cabo=63;
		}
		else
		{
			if(proximidade>(1013)&&(proximidade<(1550)))			//if(proximidade>(((164*3.3)/(164+470))*(4095/3.45))&&(proximidade<(((308*3.3)/(308+470))*(4095/3.45))))
			{
				corrente_cabo=32;
			}
			else
			{
				if(proximidade>(1854)&&(proximidade<(2685)))			//if(proximidade>(((400*3.3)/(400+470))*(4095/3.35))&&(proximidade<(((936*3.3)/(936+470))*(4095/3.35))))
				{
					corrente_cabo=20;
				}
				else
				{
					if(proximidade>(2750)&&(proximidade<(3386))) 		//if(proximidade>(((1100*3.3)/(1100+470))*(4095/3.35))&&(proximidade<(((2460*3.3)/(2460+470))*(4095/3.35))))
					{
						corrente_cabo=13;
					}
					else
					{
						if(proximidade>(3600))													//if(proximidade>(((4499*3.3)/(4500+470))*(4095/3.35)))
						{
							corrente_cabo = 0;
						}
						else
						{
							corrente_cabo = 0;
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
{
	//printf("Estado\n");
	static int estado=12;

	if(media_x1>3900)				
	{
		estado=12;
	}
	else
	{
		if((media_x1>2500)&&(media_x1<3900))
		{
		estado=9;
		}
		else
		{
			if((media_x1>1900)&&(media_x1<2400))
			{
				estado=6;
			}
			else
			{
				if((media_x1>700)&&(media_x1<1900))
				{
					estado=3;
				}
				else
				{
					if((media_x1>500)&&(media_x1<700))
					{
						estado=0;
					}
					else
					{
						if(media_x1<200)
						{
							estado=-12;
						}
						else
						{
							estado = estado;
						}
					}
				}
			}
		}
	}
	return estado;
}

//Funcao que contém a logica de comutacao do dispositivo de manobra
int chargingStationMain(int estado, int corrente_cabo)
{	//Entra nessa lógica 1 vez a cada ciclo do PWM (1 kHz)
	
	static int razao=1000;											//Razão cíclica do PWM
	static int k=0;															//Contador do bloqueio da contatora
	static int m=0;												      //Contador do bloqueio da razão cíclica
	static int corrente_da_estacao=0;			      //Corrente máxima que a estaçao irá fornecer
	static int cont = 0;
	static bool estado_F=false;									//Variével que altera o valor da razão cíclica para 0
	static bool estadoDispositivoManobra=false;	//Estado do dispositivo de manobra
	static bool bloqueio_contatora=false;				//Variável que bloqueia a contatora por 6 segundo caso entre no modo ventilação
	static bool bloqueio_razao_ciclica=false;		//Variável que bloqueia a alteração da razão cíclica por 5 segundos
	static bool iniciar_recarga=false;					//Variável que bloqueia a alteração da razão cíclica por 5 segundos

	estado_F = gpio_get_level(GPIO_NUM_23);
	iniciar_recarga = Dados.Iniciar_Recarga;
	Dados.Contador = cont;
//******Lógica que decide qual será a corrente máxima da estação e o estado da contatora*********************
	//Lógica para os Estados: A, B, E e F ou cabo desconectado
	if((estado==12)||(estado==9)||(estado==0)||(estado==-12)||(corrente_cabo==0))			
	{
		estadoDispositivoManobra=false;
		dispositivoDeManobra(estadoDispositivoManobra);
		bloqueio_contatora=false;
		k=0;
		cont = 0;
		if(estado==9)		//Se Estado B ative o PWM
		{
			corrente_da_estacao=corrente_cabo;
		}
		else
		{
			corrente_da_estacao=0;
		}
	}
	//Lógica para os Estados C e D com cabo conectado
	else
	{	
		//Lógica para o Estado C
		if(estado==6)
		{
			k=0;
			corrente_da_estacao=corrente_cabo;
			if(razao!=1000 && iniciar_recarga == 1)			//A contatora não pode fechar antes de o PWM indicar a estação como pronta
			{
				cont = 0;
				bloqueio_contatora=true;
				estadoDispositivoManobra=true;
				dispositivoDeManobra(estadoDispositivoManobra);
			}else{                // Lógica de abertura da contatora, não responta ao término da recarga (Transição 10.2)
				if(cont >= 6000){   //Minimo 6 segundos
							estadoDispositivoManobra = 0;
							dispositivoDeManobra(estadoDispositivoManobra);
					}else{
							cont++;
					}
			}
		}
		//Lógica para o Estado D
		else
		{
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
				corrente_da_estacao=0;
				estadoDispositivoManobra=false;
				dispositivoDeManobra(estadoDispositivoManobra);
			}
		}
	}

	//************Codificação da corrente máxima da estação através da razão cíclica do PWM***********************
	//Lógica caso a estação não esteja pronta para fornecer energia
	if(corrente_da_estacao==0 || iniciar_recarga == 0)
	{
		razao=1000;
	}
	
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
				razao = (corrente_da_estacao/0.6)*(1023/100);
		}else{razao = 1000;}
		
		bloqueio_razao_ciclica=true;
	}

	//Mantém a razão cíclica em 0%, enquando o erro persistir
	if(estado_F == true){
		razao = 0;
	}

	return razao;
}

//Funcao para controle dos led indicadores
void acendeLed(){
	static bool ledEstadoC = false;
	static bool ledEstadoErro = false;

	//Veiculo conectado
	if(Dados.Estado_Veiculo == 9 || Dados.Estado_Veiculo == 6){
		gpio_set_level(LED_ESTADO_B, true);
	}else{gpio_set_level(LED_ESTADO_B, false);}

	//Se está carregando, pisca o led 
	if(Dados.Estacao_Carregando == true && Dados.Razao_Ciclica<1000){
		ledEstadoC = !ledEstadoC;
		gpio_set_level(LED_ESTADO_C, ledEstadoC); 
  }else{gpio_set_level(LED_ESTADO_C, false);}
  
	//Estado de Erro
	if(Dados.Estado_Veiculo == 0 || Dados.Estado_Veiculo == -12 ){
		//ledEstadoErro = !ledEstadoErro;
		gpio_set_level(LED_ESTADO_ERRO, true);
	}else{gpio_set_level(LED_ESTADO_ERRO, false);}

}

//Funcao para controle do incio/fim de recarga pelo botao 
void leBotao(){ 
		static int cont = 0;
    int bt_estado = gpio_get_level(BT_INICIAR_RECARGA);
     
    if (bt_estado == 1) {
        cont++;
    } else {cont = 0;}

    if (cont >= 1 && cont <= 5) {
        Dados.Iniciar_Recarga = 1;
    }

    if (cont >= 30) {
        Dados.Iniciar_Recarga = 0;
    }
}

//Funcao para controle do dispositivo de manobra(relés)
void dispositivoDeManobra(int acao){
	if(acao == 1){
		gpio_set_level(RELE_N, true); // Desliga Dispositivo de manobra
	}else{
		gpio_set_level(RELE_N, false); // Desliga Dispositivo de manobra
	}
}

//Funcao auxiliar só para printar na tela
void printTela(){
	printf("Estado: %d\n", Dados.Estado_Veiculo);
	printf("AD CP: %d\n", Dados.Media_Piloto);
	printf("Cabo: %d\n", Dados.Corrente_Do_Cabo);
	printf("Corrente_usuario: %d\n", Dados.Corrente_Usuario);
	printf("Corrente_max: %d\n", Dados.Corrente_Maxima);
	printf("Razao: %d\n\n", Dados.Razao_Ciclica);

	printf("Iniciar_Recarga: %d\n", Dados.Iniciar_Recarga);
	printf("Carregando: %d\n", Dados.Estacao_Carregando);
	printf("Contador: %d\n\n", Dados.Contador);
}
