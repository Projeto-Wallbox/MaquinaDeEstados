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
	static int funcao_principal = 0;
	static int measure_two = 0;
	static int printar = 0;

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
	funcao_principal++;
	measure_two++;
	printar++;

	// a cada 166 us (6kHz)
	medida_piloto = adc1_get_raw(ADC1_CHANNEL_4);	 // Leitura do piloto (1).																			
	media_piloto = positivaPiloto(medida_piloto); // Calcula a média dos sinais (2)

	Dados.Estado_Veiculo = estado_veiculo;  //atualiza estado na struct

	if (funcao_principal >= 6) // a cada 1 ms (1kHz)
	{
		//TESTE DE LEITURA DO ESTADO E0
	  if(Dados.Estado_Veiculo == 0){
			printf("ACIONAR BATERIA");   /// em teste
		}

		funcao_principal = 0;
		medida_proximidade = adc1_get_raw(ADC1_CHANNEL_5);     // Faz a leitura analogica do proximidade (3)
		cabo_conectado = correnteCabo(medida_proximidade);		 // Identificacao do Cabo (4)
		estado_veiculo = defineEstado(media_piloto);           // Determina o Estado (5)
	
		if(Dados.Corrente_Usuario < cabo_conectado){ //logica de maior corrente suportada 
			corrente_maxima = Dados.Corrente_Usuario;
		}else{corrente_maxima = cabo_conectado;}
	
		razao_ciclica = chargingStationMain(estado_veiculo, corrente_maxima); // MAQUINAS DE ESTADOS, CONTATORA E CALCULO DA RAZAO CICLICA    (6 e 7)
	}
	else
	{
		if (measure_two >= 600) // a cada 100 ms ( 10 Hz)
		{
			//Atuzaliza os dados da estrura "maquinaDeEstados"
			leBotao();
			Dados.Corrente_Do_Cabo = cabo_conectado;
			Dados.Corrente_Maxima = corrente_maxima;
			Dados.Razao_Ciclica = razao_ciclica;
			measure_two = 0;
		}
		else
		{
			if (printar >= 6000) // a cada 1000 ms (1 Hz)
			{ 

				acendeLed();
				printTela();
				printar = 0;
			}
		}
	}

	return razao_ciclica;
}

//Funcao que calcula media dos sinais lidos no AD Piloto
int positivaPiloto(int Piloto)
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
		if ((Piloto > media[0]) || (Piloto > media[1]) || (Piloto > media[2]) || (Piloto > media[3]) || (Piloto > media[4]))
		{
			if (Piloto > media[0])
			{
				media[0] = Piloto + 1;
			}
			else
			{
				if (Piloto > media[1])
				{
					media[1] = Piloto + 1;
				}
				else
				{
					if (Piloto > media[2])
					{
						media[2] = Piloto + 1;
					}
					else
					{
						if (Piloto > media[3])
						{
							media[3] = Piloto + 1;
						}
						else
						{
							if (Piloto > media[4])
							{
								media[4] = Piloto + 1;
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
int chargingStationMain(int estado, int corrente_maxima)
{
  static int razao = 1000;                // Razao ciclica do PWM
  static int k = 0;                       // Contador do bloqueio da contatora
  static int l = 0;                       // Gerador da interrupcaoo de 1 kHz
  static int m = 0;                       // Contador do bloqueio da razao ciclica
  static int corrente_da_estacao = 0;     // Corrente maxima que a estacaoo ira fornecer
  static int contatora = 0;               // Estado da contatora
  static int esta_carregando = 0;         // detecta se_ a estacaoo de recarga esta carregando
  static int bloqueio_contatora = 0;      // bloqueia a contatora por 6 segundos caso entre no modo ventilacaoo
  static int bloqueio_razao_ciclica = 0;  // Variavel que bloqueia a alteracao da razao ciclica por 5 segundos
  static int abrir_dispositvo_erroS2 = 0; // finalizar_recarga Abrir dispositivo de manobra (Transicao 10.2)
	static int cont = 0;
  
  Dados.Estacao_Carregando = esta_carregando; 								//Atualiza esta informacao na estrutura;
  
  //***************Codigo da Corrente na estacao e da contatora*************************
  //ETAPA DA LOGICA DO ACIONAMENTO DA CONTATORA - **fazer uma logica de acionamento para os reles
  if ((estado == 0) || (estado == -12) || (corrente_maxima == 0))//mantem dispositivo de manobra desligado
  {
    corrente_da_estacao = 0; 
    contatora = 0;
    //gpio_set_level(RELE_N, false); // Desliga Dispositivo de manobra
		dispositivoDeManobra(contatora);
    esta_carregando = 0;
    if ((estado == 0) || (estado == -12))  //mantem dispositivo de manobra desligado
    {
      bloqueio_contatora = 0;
      k = 0;
    }
  }
  else
  {
    if ((estado == 6) && (esta_carregando == 1)) //Fecha dispositivo de manobra
    {
      bloqueio_contatora = 1;
      contatora = 1;
    }
    if ((bloqueio_contatora == 1) && (estado == 3))
    {
      corrente_da_estacao = 0;
      contatora = 1;
      dispositivoDeManobra(contatora);
			//gpio_set_level(RELE_N, true); // Liga Dispositivo de manobra
      if (l == 1)
      {
        k = k + l;
      }
      esta_carregando = 0;
      bloqueio_contatora = 1;
      if (k >= 60)
      { // <---------Deveria ser 6000
        bloqueio_contatora = 0;
        k = 0;
      }
    }

    // Logica da corrente da estacaoo e da contatora
    else
    {
      k = 0;
      if (((estado == 12) || (estado == 9) || (estado == 6)) && ((corrente_maxima == 13) || (corrente_maxima == 20) || (corrente_maxima == 32) || (corrente_maxima == 63)))
      {
        if (estado==6)
        {
					if(abrir_dispositvo_erroS2==0){
						contatora = 1;
         		dispositivoDeManobra(contatora);
						//gpio_set_level(RELE_N, true); // Liga Dispositivo de manobra
         		esta_carregando = 1;
					}else{
						Dados.Iniciar_Recarga = 0;
						contatora = 0;
         		dispositivoDeManobra(contatora);
					} 

					if(Dados.Iniciar_Recarga == 0){  //Razao de 100% quando ir para o estado A ou quando iniciar_recarga esta "desligado"
            razao = 1002;
          }
				}
        else
        {//ALTERADO
          if((estado == 12)||(Dados.Iniciar_Recarga == 0)){  //Razao de 100% quando ir para o estado A ou quando iniciar_recarga esta "desligado"
            razao = 1002;
          }
          contatora = 0;
					dispositivoDeManobra(contatora);
          //gpio_set_level(RELE_N, false); // Desliga Dispositivo de manobra
          esta_carregando = 0;
					abrir_dispositvo_erroS2 = 0;  //variável de controle relacionado a transicao 10.2
					cont = 0;                     //contador relacionado a transicao 10.2
        }
        corrente_da_estacao = corrente_maxima;  //AQUI ERA ALTERADO O VALOR DA CORRENTE
      }
			else
      {
        //corrente_da_estacao = 0;
        contatora = 0;
				dispositivoDeManobra(contatora);
        esta_carregando = 0;
      }
    }
  }

  //LOGICA DA ABERTURA DA CONTATORA CASO O VEICULO NAO ABRA S2 (SEQUENCIA 10.2)
	Dados.Finalizar_Recarga = abrir_dispositvo_erroS2;
	Dados.Contador = cont;
	if(razao>=1000 && estado==6){ //ver da corrente
		if(cont >= 6000){  //min 6S
			abrir_dispositvo_erroS2 = 1;
		}else{
			cont++;
		}
	}

  // *************************** Codigo do Gerado PWM *************************************
  // LOGICA DO BLOQUEIO DA RAZAO CICLICA (SEQUENCIA 6)
  if ((bloqueio_razao_ciclica == 1) && (l == 1)) //para nao alterar a razao ciclica em menos de 5s apos ter feito uma atualizacao 
  {
    m = m + l;
    if (m >= 50)
    { 
      bloqueio_razao_ciclica = 0;
      m = 0;
    }
  }

  // Logica de determinacao da largura de pulso
  if ((l == 1) && (bloqueio_razao_ciclica == 0))
  { 
    bloqueio_razao_ciclica = 1;
    //razao = (corrente_da_estacao/0.6)*(1023/100);
    switch (corrente_da_estacao)
    {
      //case 63:
       // razao = 533;
        //break;
      case 32:
        razao = 533;
        break;
      case 20:
        razao = 333;
        break;
      case 13:
        razao = 216; // razao=(0.13/0.6)*1000;
        break;
      case 6:
        razao = 100; 
        break;
      default:
        razao = 1000;//1000;
        break;
    }
  }

  if (l >= 100)
  {
    l = 0;
  }

  // Logica da interrupcao do PWM
  l = l + 1;

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
		ledEstadoErro = !ledEstadoErro;
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

    if (cont >= 50) {
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
	printf("Cabo: %d\n", Dados.Corrente_Do_Cabo);
	printf("Corrente_usuario: %d\n", Dados.Corrente_Usuario);
	printf("Corrente_max: %d\n\n", Dados.Corrente_Maxima);
				
	printf("Iniciar_Recarga: %d\n", Dados.Iniciar_Recarga);
	printf("Razao: %d\n", Dados.Razao_Ciclica);
	printf("Carregando: %d\n", Dados.Estacao_Carregando);
	printf("Finalizar: %d\n", Dados.Finalizar_Recarga);
	printf("Contador: %d\n\n", Dados.Contador);
}
