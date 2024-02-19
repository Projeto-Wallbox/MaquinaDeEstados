# Máquina de estados, wattímetro e OCPP 1.6

## Esse projeto contém: 
## - Máquina de estados
Está contido nos arquivos [*state_machine.cpp*](src/state_machine.cpp) e [*state_machine.h*](src/state_machine.h) as funções implementadas de acordo com digrama de estados da *IEC 61851-1* contendo:
 - Geração do sinal *PWM*;
 - Leitura dos sinais do circuito piloto e circuito proximidade, já condicionados para um *range* de 0 à 3.3V de acordo com o Conversore Analógico-Digital do ESP32S3;
 - Definições dos estados e razão cíclica do sinal *PWM* de acordo com a corrente máxima da estação;
  
## - Wattímetro
Está contido nos arquivos [*wattmeter_sensor.cpp*](src/wattmeter_sensor.cpp) e [*wattmeter_sensor.h*](src/wattmeter_sensor.h) as funções implementadas para a inicialização do sensor [*ACS37800*](https://www.allegromicro.com/en/products/sense/current-sensor-ics/zero-to-fifty-amp-integrated-conductor-sensor-ics/acs37800), leitura de tesão e corrente além do cálculo de poência e energia.

## - OCPP
Juntamente com este projeto está sendo implementado o cliente OCPP, sendo integrado junto a máquina de estados para coleta de dados, utilizando a biblioteca MicroOCPP. 

