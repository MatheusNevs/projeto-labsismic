#include "../lib/clock.h"
#include <msp430f5529.h>

void init_clock(void) {
  // Inicializacao dos clocks principais
  UCSCTL3 = SELREF_2;
  UCSCTL4 = SELA_2 + SELS_3 + SELM_3;
  UCSCTL1 = DCORSEL_0;
  UCSCTL2 = FLLD_0 + 30;
  __bis_SR_register(SCG0);
  UCSCTL5 = 0;
  __bic_SR_register(SCG0);
  __delay_cycles(50000);

  // Timer A2 para contagem
  TA2CTL = TASSEL_1 | MC_2 | TACLR | ID_3;
}