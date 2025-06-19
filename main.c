#include <msp430f5529.h>
#include <stdio.h>

#include "intrinsics.h"
#include "lib/i2c.h"
#include "lib/ssd1306.h"
#include "lib/adc.h"
#include "lib/buzzer.h"

volatile unsigned int adc_val;
volatile float distancia_atual = 0, distancia_anterior = 0;
volatile float velocidade = 0;
volatile unsigned int categoria = 0;

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
        return 0;
    float delta_tempo = delta_ciclos / 32768.0;
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
        return;
    }

    if (categoria != categoria_anterior)
    {
        estado = 0;
        instante_referencia = agora;
        categoria_anterior = categoria;
    }

    switch (estado)
    {
    case 0:
        buzzer_on();
        instante_referencia = agora;
        estado = 1;
        break;
    case 1:
        if (tempo_decorrido >= 9828)
        {
            if (categoria == 4)
                estado = 0;
            else
            {
                buzzer_off();
                instante_referencia = agora;
                estado = 2;
            }
        }
        break;
    case 2:
        if ((categoria == 3 && tempo_decorrido >= 6553) ||
            (categoria == 2 && tempo_decorrido >= 19660) ||
            (categoria == 1 && tempo_decorrido >= 32768))
            estado = 0;
        break;
    }
}

// --- Interrupção do Timer ---

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A_ISR(void)
{
    distancia_anterior = distancia_atual;

    adc_val = ler_adc();
    distancia_atual = calcular_distancia(adc_val);

    unsigned int tempo_atual = TA2R;

    velocidade = calcular_velocidade(distancia_anterior, distancia_atual, tempo_atual, tempo_anterior);
    tempo_anterior = tempo_atual;

    categoria = definir_categoria(distancia_atual, velocidade);

    if (categoria > 0)
        exibir_distancia(distancia_atual);
    else
        ssd1306_clear();
}

// --- Inicialização ---

void setup_timer_interrupt()
{
    TA1CCTL0 = CCIE;
    TA1CCR0 = 0xFFFF / 4;
    TA1CTL = TASSEL_1 + MC_1;
}

void setup_timer2_clock()
{
    TA2CTL = TASSEL_1 | MC_2 | TACLR;
}

void clock_init(void)
{
    UCSCTL3 = SELREF_2;
    UCSCTL4 = SELA_2 + SELS_3 + SELM_3;
    UCSCTL1 = DCORSEL_0;
    UCSCTL2 = FLLD_0 + 30;
    __bis_SR_register(SCG0);
    UCSCTL5 = 0;
    __bic_SR_register(SCG0);
    __delay_cycles(50000);
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    clock_init();
    i2c_init();
    ssd1306_init();
    setup_adc();
    buzzer_init();
    setup_timer_interrupt();
    setup_timer2_clock();

    __enable_interrupt();
    ssd1306_clear();

    while (1)
        emitir_som_por_categoria_responsivo();
}
