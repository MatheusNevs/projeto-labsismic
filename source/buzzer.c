#include <msp430f5529.h>
#include "../lib/buzzer.h"

#define BUZZER_FREQ 3000      // 3 kHz
#define SMCLK_FREQ 1000000    // 1 MHz

void buzzer_init(void) {
    P1DIR |= BIT2;     // P1.2 como saída
    P1SEL |= BIT2;     // TA0.1 função PWM

    TA0CCTL1 = OUTMOD_0; // Reset/Set
    TA0CCR0 = (SMCLK_FREQ / BUZZER_FREQ) - 1;
    TA0CCR1 = TA0CCR0 / 2; // 50% duty cycle
    TA0CTL = TASSEL_2 + MC_1; // SMCLK, modo up
}

void buzzer_on(void) {
    TA0CCTL1 = OUTMOD_7; // PWM ativo
}

void buzzer_off(void) {
    TA0CCTL1 = OUTMOD_0; // Força LOW
}
