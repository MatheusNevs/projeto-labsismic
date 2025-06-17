#include <msp430f5529.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/i2c.h"
#include "lib/ssd1306.h"
#include "lib/adc.h"
#include "lib/buzzer.h"


void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    TA1CTL = TASSEL_2 + MC_2 + TACLR;

    i2c_init();
    ssd1306_init();
    setup_adc();
    buzzer_init();

    char buffer[3];
    unsigned int adc_val, tempo_anterior = 0, tempo_atual = 0, delta_ciclos = 0, categoria = 0;
    float tensao, distancia_atual = 0, distancia_anterior = 0, velocidade = 0;

    ssd1306_clear();
    ssd1306_set_cursor(0, 0);



    while (1) {
        // leitura da tensao
        adc_val = ler_adc();
        tensao = (adc_val / 4095.0) * 3.3;

        // medicao da distancia
        distancia_anterior = distancia_atual;
        distancia_atual = 4800 / (tensao * 200 - 20);

        // tempo de medicao
        tempo_atual = TA1R;
        delta_ciclos = (tempo_atual >= tempo_anterior)? (tempo_atual - tempo_anterior) : (0xFFFF - tempo_anterior + tempo_atual + 1);
        tempo_anterior = tempo_atual;

        // velocidade em cm/s
        velocidade = (distancia_anterior - distancia_atual)/(delta_ciclos);

        // categorizacao
        if (distancia_atual >= 10 && distancia_atual <= 80) {
            if (distancia_atual <= 20)
                categoria = 4;
            else if (distancia_atual > 20 && distancia_atual < 30)
                categoria = 3;
            else if (distancia_atual > 30 && distancia_atual < 50)
                categoria = 2;
            else if (distancia_atual > 50 && distancia_atual < 80)
                categoria = 1;

            // velocidade > 10cm/s
            if (velocidade > 10)
                categoria = categoria == 4? 4: categoria + 1;
        } else {
            categoria = 0;
        }

        if (categoria > 0 && categoria <5) {
            int d = (int)(distancia_atual);
            sprintf(buffer, "%d", d);
            ssd1306_set_cursor(0, 0);
            ssd1306_print(buffer);


        if (categoria == 4) { // categoria 4
            // Som contínuo
            buzzer_on();
            __delay_cycles(300000); // breve delay para exibição
        } else {
            // Bip intermitente
            buzzer_on();
            __delay_cycles(300000); // som por 300ms
            buzzer_off();

            if (categoria == 3) { // categoria 3
                __delay_cycles(200000); // intervalo de 200ms
            } else if (categoria == 2) { // categoria 2
                __delay_cycles(600000); // 600ms
            } else if (categoria == 1) { // categoria 1
                __delay_cycles(1000000); // 1s
            }
        }
        } else { // categoria 0
            ssd1306_clear();
            buzzer_off();
        }

        __delay_cycles(100000);
    }
}
