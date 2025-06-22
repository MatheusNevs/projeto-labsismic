#include <msp430f5529.h>
#include <stdio.h>

#include "intrinsics.h"
#include "lib/button.h"
#include "lib/buzzer.h"
#include "lib/clock.h"
#include "lib/display.h"
#include "lib/interrupt_timer.h"
#include "lib/sensor.h"

void main(void) {
  WDTCTL = WDTPW | WDTHOLD;

  init_clock();

  init_sensor();
  init_display();
  init_buzzer();
  init_interrupt_timer();
  init_button();

  __enable_interrupt();
  display_clear();
  while (1)
    bip_by_category(get_category());
}
