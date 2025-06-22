#include "../lib/sensor.h"
#include "../lib/utils.h"
#include <msp430f5529.h>
#include <stdio.h>

static float current_distance = 0, previous_distance = 0;
static float speed = 0;
static unsigned int category = 0;
static unsigned int previous_time = 0;

void init_sensor() {
  P6SEL |= BIT0;
  ADC12CTL0 = ADC12SHT02 + ADC12ON;
  ADC12CTL1 = ADC12SHP;
  ADC12MCTL0 = ADC12INCH_0;
  ADC12CTL0 |= ADC12ENC;
}

static float calculate_distance() {
  unsigned int adc_val;
  ADC12CTL0 |= ADC12SC;
  while (ADC12CTL1 & ADC12BUSY)
    ;
  adc_val = ADC12MEM0;

  float tension = (adc_val / 4095.0) * 3.3;
  return 4800 / (tension * 200 - 20);
}

static float calculate_speed(float anterior, float atual,
                             unsigned int current_time,
                             unsigned int previous_time) {
  unsigned int delta_cycles =
      calculate_delta_cycles(current_time, previous_time);
  if (delta_cycles == 0)
    return 0;
  float delta_time = delta_cycles / 4096;
  return (anterior - atual) / delta_time;
}

static unsigned int define_category(float distance, float speed) {
  if (distance < 10 || distance > 80)
    return 0;
  if (distance <= 20)
    return 4;
  if (distance < 30)
    return (speed > 10) ? 4 : 3;
  if (distance < 50)
    return (speed > 10) ? 3 : 2;
  return (speed > 10) ? 2 : 1;
}

void sample(void) {
  current_distance = calculate_distance();

  unsigned int current_time = TA2R;
  speed = calculate_speed(previous_distance, current_distance, current_time,
                          previous_time);
  previous_time = current_time;

  category = define_category(current_distance, speed);
}

float get_current_distance(void) { return current_distance; }
unsigned int get_category(void) { return category; }
