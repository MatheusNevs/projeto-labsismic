#include "../lib/interrupt_timer.h"
#include "../lib/display.h"
#include "../lib/sensor.h"
#include <msp430f5529.h>
#include <stdio.h>

void init_interrupt_timer() {
  TA1CCTL0 = CCIE;
  TA1CCR0 = 0xFFFF / 4;
  TA1CTL = TASSEL_1 + MC_1;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A_ISR(void) {
  sample();

  if (get_category() > 0)
    display_distance(get_current_distance());
  else
    display_clear();
}