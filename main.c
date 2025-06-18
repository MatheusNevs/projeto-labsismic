#include <msp430f5529.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/i2c.h"
#include "lib/ssd1306.h"
#include "lib/adc.h"
#include "lib/buzzer.h"

// --- Funções auxiliares ---

float calcular_distancia(unsigned int adc_val)
{
    float tensao = (adc_val / 4095.0) * 3.3;
    return 4800 / (tensao * 200 - 20);
}

unsigned int calcular_delta_ciclos(unsigned int tempo_atual, unsigned int tempo_anterior)
{
}

float calcular_velocidade(float anterior, float atual, unsigned int tempo_atual, unsigned int tempo_anterior)
{
    unsigned int delta_ciclos = (atual >= anterior) ? (atual - anterior) : (0xFFFF - anterior + atual + 1);
    return (anterior - atual) / delta_ciclos;
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

void emitir_som_por_categoria(unsigned int categoria)
{
    switch (categoria)
    {
    case 4:
        buzzer_on();
        __delay_cycles(300000); // som contínuo
        break;
    case 3:
        buzzer_on();
        __delay_cycles(300000);
        buzzer_off();
        __delay_cycles(200000);
        break;
    case 2:
        buzzer_on();
        __delay_cycles(300000);
        buzzer_off();
        __delay_cycles(600000);
        break;
    case 1:
        buzzer_on();
        __delay_cycles(300000);
        buzzer_off();
        __delay_cycles(1000000);
        break;
    }
}

// --- Função principal ---

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;         // Desativa Watchdog
    TA1CTL = TASSEL_2 + MC_2 + TACLR; // SMCLK, modo contínuo, reset

    i2c_init();
    ssd1306_init();
    setup_adc();
    buzzer_init();

    unsigned int adc_val;
    unsigned int tempo_anterior = 0, tempo_atual = 0, delta_ciclos = 0;
    float tensao = 0, distancia_atual = 0, distancia_anterior = 0, velocidade = 0;
    unsigned int categoria = 0;

    ssd1306_clear();

    while (1)
    {
        adc_val = ler_adc();

        distancia_anterior = distancia_atual;
        distancia_atual = calcular_distancia(adc_val);

        tempo_atual = TA1R;
        velocidade = calcular_velocidade(distancia_anterior, distancia_atual, tempo_atual, tempo_anterior);
        tempo_anterior = tempo_atual;

        categoria = definir_categoria(distancia_atual, velocidade);

        if (categoria > 0)
        {
            exibir_distancia(distancia_atual);
            emitir_som_por_categoria(categoria);
        }
        else
        {
            ssd1306_clear();
            buzzer_off();
        }

        __delay_cycles(100000);
    }
}
