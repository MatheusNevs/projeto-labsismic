#include "../lib/buzzer.h"
#include "lib/utils.h"
#include <msp430f5529.h>
#include <stdint.h>

#define BUZZER_FREQ 3000   // 3 kHz
#define SMCLK_FREQ 1000000 // 1 MHz
#define VOLUME_STEPS 5

volatile uint8_t buzzer_off_lock;
volatile uint8_t buzzer_step;

static void update_duty(void)
{
  TA0CCR1 = (TA0CCR0 * 0.7 * buzzer_step) / VOLUME_STEPS;
}

void init_buzzer(void)
{
  buzzer_off_lock = 0;
  buzzer_step = VOLUME_STEPS / 2;

  P1DIR |= BIT2;
  P1SEL |= BIT2;

  TA0CCTL1 = OUTMOD_0;
  TA0CCR0 = (SMCLK_FREQ / BUZZER_FREQ) - 1;
  update_duty();
  TA0CTL = TASSEL_2 | MC_1;
}

void buzzer_on(void) { TA0CCTL1 = OUTMOD_7; }
void buzzer_off(void) { TA0CCTL1 = OUTMOD_0; }

void lower_volume(void)
{
  if (buzzer_step > 0)
  {
    --buzzer_step;
    update_duty();
  }
}

void upper_volume(void)
{
  if (buzzer_step < VOLUME_STEPS)
  {
    ++buzzer_step;
    update_duty();
  }
}

void lock_buzzer_off(void)
{
  buzzer_off_lock = !buzzer_off_lock;
  if (buzzer_off_lock)
    buzzer_off();
  else
    buzzer_on();
}

void bip_by_category(unsigned int category)
{
  if (buzzer_off_lock)
  {
    buzzer_off();
    return;
  }

  static unsigned int state = 0;
  static unsigned int previous_category = 0;
  static unsigned int reference_instant = 0;

  unsigned int now_instant = TA2R;
  unsigned int past_time =
      calculate_delta_cycles(now_instant, reference_instant);

  if (category == 0)
  {
    buzzer_off();
    state = 0;
    return;
  }
  if (category != previous_category)
  {
    state = 0;
    reference_instant = now_instant;
    previous_category = category;
  }

  switch (state)
  {
  case 0:
    buzzer_on();
    reference_instant = now_instant;
    state = 1;
    break;
  case 1:
    if (past_time >= 9828 / 8)
    {
      if (category == 4)
        state = 0;
      else
      {
        buzzer_off();
        reference_instant = now_instant;
        state = 2;
      }
    }
    break;
  case 2:
    if ((category == 3 && past_time >= 6553 / 8) ||
        (category == 2 && past_time >= 19660 / 8) ||
        (category == 1 && past_time >= 32768 / 8))
      state = 0;
    break;
  }
}
