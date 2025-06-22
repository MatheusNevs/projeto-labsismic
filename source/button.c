#include "../lib/button.h"
#include "../lib/buzzer.h"
#include "../lib/utils.h"
#include <msp430f5529.h>

volatile unsigned int p1_press_time;
volatile unsigned int p2_press_time;

void init_button(void)
{
  // --- P1.1 (botão DIMINUI) ---
  P1DIR &= ~BIT1;
  P1REN |= BIT1;
  P1OUT |= BIT1;
  P1IES |= BIT1;
  P1IFG &= ~BIT1;
  P1IE |= BIT1;

  // --- P2.1 (botão AUMENTA) ---
  P2DIR &= ~BIT1;
  P2REN |= BIT1;
  P2OUT |= BIT1;
  P2IES |= BIT1;
  P2IFG &= ~BIT1;
  P2IE |= BIT1;

  __bis_SR_register(GIE);
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
  if (!(P1IFG & BIT1))
    return;
  P1IFG &= ~BIT1;

  __delay_cycles(5000);

  if (!(P1IN & BIT1))
  {
    p1_press_time = TA2R;
    P1IES &= ~BIT1;
  }
  else
  {
    unsigned int delta = calculate_delta_cycles(TA2R, p1_press_time);
    if (delta / 4096 >= 2)
      lock_buzzer_off();
    else
      lower_volume();
    P1IES |= BIT1;
  }
}

#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void)
{
  if (!(P2IFG & BIT1))
    return;
  P2IFG &= ~BIT1;

  __delay_cycles(5000);

  if (!(P2IN & BIT1))
  {
    p2_press_time = TA2R;
    P2IES &= ~BIT1;
  }
  else
  {
    unsigned int delta = calculate_delta_cycles(TA2R, p2_press_time);
    if (delta / 4096 >= 2)
      lock_buzzer_off();
    else
      upper_volume();
    P2IES |= BIT1;
  }
}
