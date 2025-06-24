# Sistema de Sensor de Ré com MSP430F5529

Projeto final da disciplina Laborátio de Sistemas Microprocessados semestre 2025.1.

## Introdução

### Contextualização

A segurança viária é uma preocupação crescente, especialmente em ambientes urbanos com grande fluxo de veículos e pedestres. Manobras em marcha ré, principalmente em veículos sem sensores de estacionamento, representam um risco significativo de pequenos acidentes.

### Definição do Problema

Muitos veículos antigos não possuem sensores de ré, aumentando o risco de colisões com obstáculos, outros veículos ou pessoas. A ausência desse recurso pode causar danos materiais e colocar em risco a integridade física de pedestres.

### Objetivos do Trabalho

Desenvolver um sistema de sensor de ré baseado na plataforma MSP430F5529, capaz de detectar obstáculos traseiros e emitir alertas sonoros proporcionais à distância, além de informar a distância (cm) do objeto mais próximo ao veículo, prevenindo acidentes durante manobras de ré.

## Proposta de Solução

A solução consiste em um sistema embarcado que utilizada um sensor infravermelho para medir a distância de obstáculos, um buzzer para emitir sinais sonoros (bips) com intervalos variando conforme a distância e a velocidade de proximidade a obstáculos, um display para informar a distância (cm), e botões da própria launchpad para controlar o volume ou desligar/ligar o buzzer.

## Lista de Materiais e Esquemático

### Recursos do MSP430F5529 Utilizados

- **ADC:** Leitura do sensor Sharp GP2Y0A21YK0F (0 a 3,3V), conversão de tensão em distância (10 a 80 cm).
- **PWM:** Geração de sinais de frequências e duty cycles diferentes para o Buzzer.
- **I2C:** Comunicação com o display OLED SSD1306.
- **GPIO:** Entrada analógica do sensor de distância, controle do buzzer via saída digital/PWM, comunicação I2C com display.
- **Timers:** Geração de interrupções periódicas para amostragem, contador para cálculo da velocidade e controle de funcionamento do Buzzer, geração de sinais PWM.
- **Interrupções:** Usadas para amostragem periódica da tensão so sensor de distância e, em sequência, mostragem no display, assim como para acionamento dos botões .
- **Botões:** Controle de parâmetros utilizados na geração de sinais PWM, mudando volume/intensidade do Buzzer.

### Periféricos Utilizados

| Componente                       | Função                             | Custo Aproximado |
| -------------------------------- | ---------------------------------- | ---------------- |
| Sensor Sharp GP2Y0A21YK0F        | Medição de distância (10-80 cm)    | R$50,00          |
| Buzzer 3,3V                      | Alerta sonoro (controlado por PWM) | R$17,00          |
| Display OLED I2C 0.96” (SSD1306) | Exibição de distância e velocidade | R$20,00          |
| Protoboard, resistores, jumpers  | Montagem do circuito               | R$50,00          |

### Esquemático (Descrição das conexões)

- **Protoboard:** Conectada à alimentação 5v da MSP430f5529 e ao GND para facilitação de alimentação.
- **Sensor Sharp GP2Y0A21YK0F:** Conectado no 5v (cabo vermelho) e no GND (cabo preto) da protoboard, a saída analógica (cabo amarelo) conectado ao pino 6.0 da MSP (canal ADC).
- **Buzzer:** Conectado ao pino de saída PWM (P1.2) do MSP430 e ao GND da protoboard.
- **Display OLED SSD1306:** Comunicação I2C via P3.0 (SDA) e P3.1 (SCL) do MSP430, alimentado com 5v e aterrado no GND da protoboard.

## Implementação

### Sensor de Distância

O sensor Sharp GP2Y0A21YK0F fornece uma saída analógica inversamente proporcional à distância do obstáculo. O canal ADC do MSP430 (P6.0) é configurado para ler essa tensão (0,3V a 3,3V), convertendo-a em valores digitais (0 a 4095). A distância é calculada por uma equação empírica baseada na curva característica do sensor. Um timer (TA1) gera interrupções periódicas para amostrar o sensor e calcular a velocidade de aproximação, utilizando a diferença de leituras e o tempo entre elas (medido com o TA2).

**Cálculo do ADC:**

- Resolução: 12 bits (0-4095)
- Tensão de referência: 3,3V
- Conversão: `tensão = (adc_val / 4095.0) * 3.3`
- Distância: `distância = 4800 / (tensão * 200 - 20)`
- Velocidade: `velocidade = (distância_anterior - distância_atual) / tempo entre medições`

### Display OLED (SSD1306)

O display SSD1306 é controlado via I2C (endereçamento 0x3C). O MSP430 inicializa o barramento I2C e envia comandos para configurar o display (modo gráfico, contraste, etc). A renderização dos números é feita por bitmaps (fontes 5x7 e 16x16), desenhados coluna a coluna. O display exibe a distância em tempo real, atualizados a cada ciclo de amostragem, nas interrupções periódicas geradas pelo TA1.

**Configurações:**

- BRW (divisor de clock I2C): 10 (para ~100kHz)
- Comandos de inicialização: sequência padrão SSD1306 (ex: 0xAE, 0xA8, 0x3F, ...)
- Impressão: uso de bitmaps para renderizar caracteres grandes (16x16) no display

### Buzzer

O buzzer é acionado por PWM gerado pelo TimerA0 do MSP430. O pino P1.2 é configurado como saída PWM. A frequência do PWM é fixada em 3khz, mas o PWM é desligado e ligado periodicamente para criar a sensação de bips espaçados no tempo de acordo com a distância. O duty cycle é ajustável para controle de volume por botões da placa, que também oferecem a funcionalidade de desligar/ligar o som.

**Configurações:**

- Clock: SMCLK = 1 MHz
- Frequência PWM: 3 kHz (TA0CCR0 = 333)
- Duty cycle: ajustado via TA0CCR1 (ex: 70% do período)
- Volume: 5 níveis de duty cycle, controlados pelos botões da placa

### Timer para Contagem (TimerA2)

O TimerA2 é configurado em modo contínuo para fornecer base de tempo para cálculos de velocidade e temporização de eventos (ex: espaçamento entre bips do buzzer). O valor do registrador TA2R é utilizado para medir intervalos de tempo entre amostragens.

**Configurações:**

- Clock: ACLK (32.768 kHz)
- Divisor: ID_3 (divisão por 8, aumentar range de contagem)
- Modo: contínuo (MC_2)
- Uso: cálculo de delta de ciclos para estimar tempo decorrido

## Resultados e Conclusão

O sistema desenvolvido detecta obstáculos traseiros com precisão na faixa de 10 a 80 cm, emitindo alertas sonoros cuja frequência (dos bips) aumentam conforme o risco de colisão, e intensidade podem ser ajustáveis. O display OLED fornece feedback visual claro da distância de aproximação. E, finalmente, botões podem ser usados para interagir com o sistema, ajustando parâmetros de acordo com a preferência do usuário. Assim, é proporcionado um sistema confiável para auxiliar na segurança viária, especialmente em manobras de ré.
