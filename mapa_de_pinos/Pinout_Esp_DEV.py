# Definições dos pinos utilizados
pinos_utilizados = {
    "3V3": "3V3",
    "GND": "Groud",
    "IO21": "PIN_SDA",
    "IO22": "PIN_SCL",

    "IO2": "PWM_PIN",
    "IO32": "PILOT_PIN",
    "IO33": "PINO_PROXIMIDADE",
    
    "IO4": "RELE_N",
    "IO5": "RELE_L3",
    "IO18": "RELE_L2",
    "IO19": "RELE_L1",

    "IO26": "LED_A",
    "IO15": "LED_B",
    "IO14": "LED_C",
    "IO27": "LED_D",

    "IO23": "START_RECHARGER_BT",
    "IO34": "PIN_FAULT",
    "IO12": "PIN_TRIG_DC",
    "IO13": "PIN_TRIG_AC"
}

# Tabela dos pinos GPIO do ESP32
tabela_esp32 = [
    (1, "3V3", "3.3 V power supply"),
    (2, "EN", "CHIP_PU, Reset"),
    (3, "VP", "GPIO36, ADC1_CH0, S_VP"),
    (4, "VN", "GPIO39, ADC1_CH3, S_VN"),
    (5, "IO34", "GPIO34, ADC1_CH6, VDET_1"),
    (6, "IO35", "GPIO35, ADC1_CH7, VDET_2"),
    (7, "IO32", "GPIO32, ADC1_CH4, TOUCH_CH9, XTAL_32K_P"),
    (8, "IO33", "GPIO33, ADC1_CH5, TOUCH_CH8, XTAL_32K_N"),
    (9, "IO25", "GPIO25, ADC1_CH8, DAC_1"),
    (10, "IO26", "GPIO26, ADC2_CH9, DAC_2"),
    (11, "IO27", "GPIO27, ADC2_CH7, TOUCH_CH7"),
    (12, "IO14", "GPIO14, ADC2_CH6, TOUCH_CH6, MTMS"),
    (13, "IO12", "GPIO12, ADC2_CH5, TOUCH_CH5, MTDI"),
    (14, "GND", "Ground"),
    (15, "IO13", "GPIO13, ADC2_CH4, TOUCH_CH4, MTCK"),
    (16, "D2", "GPIO9, D2"),
    (17, "D3", "GPIO10, D3"),
    (18, "CMD", "GPIO11, CMD"),
    (19, "5V", "5 V power supply"),
    (20, "GND", "Ground"),
    (21, "IO23", "GPIO23"),
    (22, "IO22", "GPIO22"),
    (23, "TX", "GPIO1, U0TXD"),
    (24, "RX", "GPIO3, U0RXD"),
    (25, "IO21", "GPIO21"),
    (26, "GND", "Ground"),
    (27, "IO19", "GPIO19"),
    (28, "IO18", "GPIO18"),
    (29, "IO5", "GPIO5"),
    (30, "IO17", "GPIO17"),
    (31, "IO16", "GPIO16"),
    (32, "IO4", "GPIO4, ADC2_CH0, TOUCH_CH0"),
    (33, "IO0", "GPIO0, ADC2_CH1, TOUCH_CH1, Boot"),
    (34, "IO2", "GPIO2, ADC2_CH2, TOUCH_CH2"),
    (35, "IO15", "GPIO15, ADC2_CH3, TOUCH_CH3, MTDO"),
    (36, "D1", "GPIO8, D1"),
    (37, "D0", "GPIO7, D0"),
    (38, "CLK", "GPIO6, CLK")
]

# Calcula o comprimento máximo do nome do pino utilizado
max_len_pino_utilizado = max(len(nome) for nome in pinos_utilizados.values())

# Calcula o comprimento máximo do texto da coluna "Function"
max_len_function = max(len(func) for _, _, func in tabela_esp32)

# Criação da tabela mostrando os pinos utilizados e disponíveis
print("+------+--------------------+-----------+-----------------------------------------+")
print(f"|No.   | {'Pin Utilizado':<{max_len_pino_utilizado}} |  Name     | Function{' ' * (max_len_function - len('Function'))} |")
print("+------+--------------------+-----------+-----------------------------------------+")
for pino, nome, funcao in tabela_esp32:
    uso = pinos_utilizados.get(nome, '---')
    print(f"| {pino:<4} | {uso:<{max_len_pino_utilizado}} | {nome:<9} | {funcao:<{max_len_function}} |")
print("+--------------+------+-----------+-------------------------+")
