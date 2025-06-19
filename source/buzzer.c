#include "../lib/buzzer.h"
#include "lib/utils.h"
#include <msp430f5529.h>

#define BUZZER_FREQ 3000   // 3 kHz
#define SMCLK_FREQ 1000000 // 1 MHz

void init_buzzer(void) {
  P1DIR |= BIT2; // P1.2 como saída
  P1SEL |= BIT2; // TA0.1 função PWM

  TA0CCTL1 = OUTMOD_0; // Reset/Set
  TA0CCR0 = (SMCLK_FREQ / BUZZER_FREQ) - 1;
  TA0CCR1 = TA0CCR0 / 5;    // 50% duty cycle
  TA0CTL = TASSEL_2 + MC_1; // SMCLK, modo up
}

void buzzer_on(void) {
  TA0CCTL1 = OUTMOD_7; // PWM ativo
}

void buzzer_off(void) {
  TA0CCTL1 = OUTMOD_0; // Força LOW
}

void bip_by_category(unsigned int category) {
  static unsigned int state = 0;
  static unsigned int previous_category = 0;
  static unsigned int reference_instant = 0;

  unsigned int now_instant = TA2R;
  unsigned int past_time =
      calculate_delta_cycles(now_instant, reference_instant);

  if (category == 0) {
    buzzer_off();
    state = 0;
    return;
  }

  if (category != previous_category) {
    state = 0;
    reference_instant = now_instant;
    previous_category = category;
  }

  switch (state) {
  case 0:
    buzzer_on();
    reference_instant = now_instant;
    state = 1;
    break;
  case 1:
    if (past_time >= 9828) {
      if (category == 4)
        state = 0;
      else {
        buzzer_off();
        reference_instant = now_instant;
        state = 2;
      }
    }
    break;
  case 2:
    if ((category == 3 && past_time >= 6553) ||
        (category == 2 && past_time >= 19660) ||
        (category == 1 && past_time >= 32768))
      state = 0;
    break;
  }
}
