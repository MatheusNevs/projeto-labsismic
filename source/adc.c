#include <msp430f5529.h>
#include "../lib/adc.h"

void setup_adc(void) {
    P6SEL |= BIT0;
    ADC12CTL0 = ADC12SHT02 + ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12MCTL0 = ADC12INCH_0;
    ADC12CTL0 |= ADC12ENC;
}

unsigned int ler_adc(void) {
    ADC12CTL0 |= ADC12SC;
    while (ADC12CTL1 & ADC12BUSY);
    return ADC12MEM0;
}
