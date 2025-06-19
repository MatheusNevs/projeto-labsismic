#include <msp430f5529.h>
#include "../lib/i2c.h"

void i2c_init(void) {
    UCB0CTL1 |= UCSWRST;
    UCB0CTL1 &= ~UCSWRST;

    P3SEL |= BIT0 + BIT1;
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
    UCB0CTL1 = UCSSEL_2 + UCSWRST;
    UCB0BR0 = 10;
    UCB0BR1 = 0;
    UCB0I2CSA = 0x3C;
    UCB0CTL1 &= ~UCSWRST;
}
