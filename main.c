#include <msp430f5529.h>
#include <stdio.h>
#include <stdlib.h>

#include "intrinsics.h"
#include "lib/i2c.h"
#include "lib/ssd1306.h"
#include "lib/adc.h"
#include "lib/buzzer.h"

volatile unsigned int adc_val;
volatile float distancia_atual = 0, distancia_anterior = 0;
volatile float velocidade = 0;
volatile unsigned int categoria = 0;
volatile unsigned int nova_medida = 0;

unsigned int tempo_anterior = 0;

// --- Funções auxiliares ---

float calcular_distancia(unsigned int adc_val)
{
    float tensao = (adc_val / 4095.0) * 3.3;
    return 4800 / (tensao * 200 - 20);
}

unsigned int calcular_delta_ciclos(unsigned int atual, unsigned int anterior)
{
    return (atual >= anterior) ? (atual - anterior) : (0xFFFF - anterior + atual + 1);
}

float calcular_velocidade(float anterior, float atual, unsigned int tempo_atual, unsigned int tempo_anterior)
{
    unsigned int delta_ciclos = calcular_delta_ciclos(tempo_atual, tempo_anterior);

    if (delta_ciclos == 0)
        return 0; // evita divisão por zero

    float delta_tempo = delta_ciclos / 32768.0; // tempo em segundos
    return (anterior - atual) / delta_tempo;
}

unsigned int definir_categoria(float distancia, float velocidade)
{
    if (distancia < 10 || distancia > 80)
        return 0;
    if (distancia <= 20)
        return 4;
    if (distancia < 30)
        return (velocidade > 10) ? 4 : 3;
    if (distancia < 50)
        return (velocidade > 10) ? 3 : 2;
    return (velocidade > 10) ? 2 : 1;
}

void exibir_distancia(float distancia)
{
    char buffer[3];
    sprintf(buffer, "%d", (int)distancia);
    ssd1306_set_cursor(0, 0);
    ssd1306_print(buffer);
}

void emitir_som_por_categoria_responsivo()
{
    static unsigned int estado = 0;
    static unsigned int categoria_anterior = 0;
    static unsigned int instante_referencia = 0;

    unsigned int agora = TA2R;
    unsigned int tempo_decorrido = calcular_delta_ciclos(agora, instante_referencia);

    if (categoria == 0)
    {
        buzzer_off();
        estado = 0;
        categoria_anterior = 0;
        return;
    }

    // Se a categoria mudou, reinicia
    if (categoria != categoria_anterior)
    {
        estado = 0;
        instante_referencia = TA2R;
        categoria_anterior = categoria;
    }

    switch (estado)
    {
    case 0: // Liga o buzzer
        buzzer_on();
        instante_referencia = TA2R;
        estado = 1;
        break;

    case 1:                          // Espera buzzer ligado por 300 ms (3276*3 ciclos)
        if (tempo_decorrido >= 9828) // 300 ms
        {
            if (categoria == 4)
            {
                estado = 0; // Som contínuo
            }
            else
            {
                buzzer_off();
                instante_referencia = TA2R;
                estado = 2;
            }
        }
        break;

    case 2: // Espera silenciosa conforme a categoria
        if (
            (categoria == 3 && tempo_decorrido >= 6553) ||  // 200 ms
            (categoria == 2 && tempo_decorrido >= 19660) || // 600 ms
            (categoria == 1 && tempo_decorrido >= 32768)    // 1000 ms
        )
        {
            estado = 0;
        }
        break;
    }
}

// --- Interrupção do Timer (medição periódica) ---

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A_ISR(void)
{
    char buffer[32];

    distancia_anterior = distancia_atual;

    adc_val = ler_adc();
    distancia_atual = calcular_distancia(adc_val);

    unsigned int tempo_atual = TA2R;
    sprintf(buffer, "Ciclo atual e anterior: %d.%d\n", tempo_atual, tempo_anterior);
    printf(buffer);

    velocidade = calcular_velocidade(distancia_anterior, distancia_atual, tempo_atual, tempo_anterior);
    tempo_anterior = tempo_atual;

    int parte_inteira = (int)velocidade;
    int parte_decimal = (int)(velocidade * 100) % 100;

    sprintf(buffer, "Velocidade: %d.%d\n", parte_inteira, parte_decimal);
    printf(buffer);

    sprintf(buffer, "Distancia: %dcm\n", (int)distancia_atual);
    printf(buffer);

    categoria = definir_categoria(distancia_atual, velocidade);

    if (categoria > 0)
        exibir_distancia(distancia_atual);
    else
        ssd1306_clear();
}

// --- Configura Timer para interrupções periódicas ---

void setup_timer_interrupt()
{
    TA1CCTL0 = CCIE;
    TA1CCR0 = 0xFFFF / 4;
    TA1CTL = TASSEL_1 + MC_1;
}

void setup_timer2_clock(void)
{
    TA2CTL = TASSEL_1 | MC_2 | TACLR;
    // TASSEL_1: Usa ACLK (32.768 Hz)
    // MC_2: Modo contínuo (conta até 0xFFFF e reinicia)
    // TACLR: Limpa o contador no início
}

// --- Função principal ---

void uart_init(void)
{
    // Configura os pinos P4.4 = TXD e P4.5 = RXD para função UART
    P4SEL |= BIT4 + BIT5;
    P4DIR |= BIT4; // TXD como saída

    UCA1CTL1 |= UCSWRST;           // Reset UART para configurar
    UCA1CTL1 = UCSSEL_2 | UCSWRST; // SMCLK, manter em reset
    UCA1BR0 = 104;                 // Divisor para 9600 baud com 1 MHz clock
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_1;   // Modulação
    UCA1CTL1 &= ~UCSWRST; // Libera UART
}

int fputc(int c, FILE *stream)
{
    while (!(UCA1IFG & UCTXIFG))
        ;          // espera buffer vazio
    UCA1TXBUF = c; // envia caractere
    return c;
}

void clock_init(void)
{
    UCSCTL3 = SELREF_2;                 // Fator de referência = REFOCLK (32768 Hz)
    UCSCTL4 = SELA_2 + SELS_3 + SELM_3; // ACLK = REFOCLK, SMCLK e MCLK = DCO
    UCSCTL1 = DCORSEL_0;                // DCO range = 1 MHz
    UCSCTL2 = FLLD_0 + 30;              // Fator de multiplicação DCO (30*32768 ~ 1 MHz)
    __bis_SR_register(SCG0);            // Desabilita o FLL enquanto configurando
    UCSCTL5 = 0;                        // Desabilita divisores
    __bic_SR_register(SCG0);            // Reabilita FLL
    __delay_cycles(50000);              // Aguarda estabilidade do clock
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    clock_init();
    uart_init();

    i2c_init();
    ssd1306_init();
    setup_adc();
    buzzer_init();
    setup_timer_interrupt();
    setup_timer2_clock();

    __enable_interrupt();

    ssd1306_clear();

    while (1)
    {
        emitir_som_por_categoria_responsivo();
    }
}