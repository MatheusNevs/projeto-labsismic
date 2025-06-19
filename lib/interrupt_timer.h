#ifndef INTERRUPT_TIMER_H
#define INTERRUPT_TIMER_H

void init_interrupt_timer(void);
__interrupt void Timer1_A_ISR(void);

#endif
