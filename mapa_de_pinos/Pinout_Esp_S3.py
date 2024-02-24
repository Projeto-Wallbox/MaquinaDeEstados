# Definições dos pinos utilizados
pinos_utilizados = {
    "3V3": "3V3",
    "G": "Ground",
    "42": "PIN_SDA",
    "39": "PIN_SCL",
    
    "6": "PWM_PIN",
    "8": "PILOT_PIN",
    "7": "PINO_PROXIMIDADE",
    
    "11": "RELE_L1",
    "12": "RELE_L2",
    "13": "RELE_L3",
    "14": "RELE_N",

    "1": "LED_A",
    "10": "LED_B",
    "2": "LED_C",
    "18": "LED_D",

    "9": "START_RECHARGER_BT",
    "I4": "PIN_FAULT",
    "36": "PIN_TRIG_DC",
    "37": "PIN_TRIG_AC"
}

# Verifica se há pinos duplicados na definição dos pinos utilizados
pinos_definidos = {}
pinos_duplicados = set()
for pin_name, pin_use in pinos_utilizados.items():
    if pin_name in pinos_definidos:
        if pinos_definidos[pin_name] != pin_use:
            pinos_duplicados.add(pin_name)
    else:
        pinos_definidos[pin_name] = pin_use

# Substitui os valores duplicados por "Erro"
for pin_name in pinos_duplicados:
    pinos_utilizados[pin_name] = "Erro"

# Tabela dos pinos GPIO do ESP32 S3
tabela_esp32_s3 = [
    (1, "3V3", "P", "3.3 V power supply"),
    (2, "3V3", "P", "3.3 V power supply"),
    (3, "RST", "I", "EN"),
    (4, "4", "I/O/T", "RTC_GPIO4, GPIO4, TOUCH4, ADC1_CH3"),
    (5, "5", "I/O/T", "RTC_GPIO5, GPIO5, TOUCH5, ADC1_CH4"),
    (6, "6", "I/O/T", "RTC_GPIO6, GPIO6, TOUCH6, ADC1_CH5"),
    (7, "7", "I/O/T", "RTC_GPIO7, GPIO7, TOUCH7, ADC1_CH6"),
    (8, "15", "I/O/T", "RTC_GPIO15, GPIO15, U0RTS, ADC2_CH4, XTAL_32K_P"),
    (9, "16", "I/O/T", "RTC_GPIO16, GPIO16, U0CTS, ADC2_CH5, XTAL_32K_N"),
    (10, "17", "I/O/T", "RTC_GPIO17, GPIO17, U1TXD, ADC2_CH6"),
    (11, "18", "I/O/T", "RTC_GPIO18, GPIO18, U1RXD, ADC2_CH7, CLK_OUT3"),
    (12, "8", "I/O/T", "RTC_GPIO8, GPIO8, TOUCH8, ADC1_CH7, SUBSPICS1"),
    (13, "3", "I/O/T", "RTC_GPIO3, GPIO3, TOUCH3, ADC1_CH2"),
    (14, "46", "I/O/T", "GPIO46"),
    (15, "9", "I/O/T", "RTC_GPIO9, GPIO9, TOUCH9, ADC1_CH8, FSPIHD, SUBSPIHD"),
    (16, "10", "I/O/T", "RTC_GPIO10, GPIO10, TOUCH10, ADC1_CH9, FSPICS0, FSPIIO4, SUBSPICS0"),
    (17, "11", "I/O/T", "RTC_GPIO11, GPIO11, TOUCH11, ADC2_CH0, FSPID, FSPIIO5, SUBSPID"),
    (18, "12", "I/O/T", "RTC_GPIO12, GPIO12, TOUCH12, ADC2_CH1, FSPICLK, FSPIIO6, SUBSPICLK"),
    (19, "13", "I/O/T", "RTC_GPIO13, GPIO13, TOUCH13, ADC2_CH2, FSPIQ, FSPIIO7, SUBSPIQ"),
    (20, "14", "I/O/T", "RTC_GPIO14, GPIO14, TOUCH14, ADC2_CH3, FSPIWP, FSPIDQS, SUBSPIWP"),
    (21, "5V", "P", "5 V power supply"),
    (22, "G", "G", "Ground"),
    (23, "G", "G", "Ground"),
    (24, "TX", "I/O/T", "U0TXD, GPIO43, CLK_OUT1"),
    (25, "RX", "I/O/T", "U0RXD, GPIO44, CLK_OUT2"),
    (26, "1", "I/O/T", "RTC_GPIO1, GPIO1, TOUCH1, ADC1_CH0"),
    (27, "2", "I/O/T", "RTC_GPIO2, GPIO2, TOUCH2, ADC1_CH1"),
    (28, "42", "I/O/T", "MTMS, GPIO42"),
    (29, "41", "I/O/T", "MTDI, GPIO41, CLK_OUT1"),
    (30, "40", "I/O/T", "MTDO, GPIO40, CLK_OUT2"),
    (31, "39", "I/O/T", "MTCK, GPIO39, CLK_OUT3, SUBSPICS1"),
    (32, "38", "I/O/T", "GPIO38, FSPIWP, SUBSPIWP, RGB LED"),
    (33, "37", "I/O/T", "SPIDQS, GPIO37, FSPIQ, SUBSPIQ"),
    (34, "36", "I/O/T", "SPIIO7, GPIO36, FSPICLK, SUBSPICLK"),
    (35, "35", "I/O/T", "SPIIO6, GPIO35, FSPID, SUBSPID"),
    (36, "0", "I/O/T", "RTC_GPIO0, GPIO0"),
    (37, "45", "I/O/T", "GPIO45"),
    (38, "48", "I/O/T", "GPIO48, SPICLK_N, SUBSPICLK_N_DIFF"),
    (39, "47", "I/O/T", "GPIO47, SPICLK_P, SUBSPICLK_P_DIFF"),
    (40, "21", "I/O/T", "RTC_GPIO21, GPIO21"),
    (41, "20", "I/O/T", "RTC_GPIO20, GPIO20, U1CTS, ADC2_CH9, CLK_OUT1, USB_D+"),
    (42, "19", "I/O/T", "RTC_GPIO19, GPIO19, U1RTS, ADC2_CH8, CLK_OUT2, USB_D-"),
    (43, "G", "G", "Ground"),
    (44, "G", "G", "Ground")
]

# Calcula o comprimento máximo do nome do pino utilizado
max_len_pino_utilizado = max(len(nome) for nome in pinos_utilizados.values())

# Calcula o comprimento máximo do texto da coluna "Function"
max_len_function = max(len(func) for _, _, _, func in tabela_esp32_s3)

# Criação da tabela mostrando os pinos utilizados e disponíveis
print("+-----+--------------------+-----------+--------------------------------------------------------------------+")
print(f"| No. | {'Pin Utilizado':<{max_len_pino_utilizado}} | Name      | Function{' ' * (max_len_function - len('Function'))} |")
print("+-----+--------------------+-----------+--------------------------------------------------------------------+")
for pino, nome, _, funcao in tabela_esp32_s3:
    uso = pinos_utilizados.get(nome, '---')
    if uso == 'Erro':
        print(f"|{pino:<4} | {uso:<{max_len_pino_utilizado}} | {nome:<9} | {'':<{max_len_function}} |")
    else:
        print(f"|{pino:<4} | {uso:<{max_len_pino_utilizado}} | {nome:<9} | {funcao:<{max_len_function}} |")
print("+-----+--------------------+-----------+--------------------------------------------------------------------+")
