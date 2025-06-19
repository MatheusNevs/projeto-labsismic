#include "../lib/display.h"
#include "../lib/font5x7.h"
#include <msp430.h>
#include <stdio.h>

static void ssd1306_command(uint8_t cmd) {
  while (UCB0CTL1 & UCTXSTP)
    ;
  UCB0CTL1 |= UCTR + UCTXSTT;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0TXBUF = 0x00;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0TXBUF = cmd;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0CTL1 |= UCTXSTP;
  while (UCB0CTL1 & UCTXSTP)
    ;
}

static void ssd1306_data(uint8_t data) {
  while (UCB0CTL1 & UCTXSTP)
    ;
  UCB0CTL1 |= UCTR | UCTXSTT;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0TXBUF = 0x40;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0TXBUF = data;
  while (!(UCB0IFG & UCTXIFG))
    ;
  UCB0CTL1 |= UCTXSTP;
}

void ssd1306_clear(void) {
  ssd1306_command(0x21);
  ssd1306_command(0);
  ssd1306_command(127);
  ssd1306_command(0x22);
  ssd1306_command(0);
  ssd1306_command(7);
  int i;
  for (i = 0; i < 128 * 8; i++) {
    ssd1306_data(0x00);
  }
}

void ssd1306_set_cursor(uint8_t col, uint8_t page) {
  ssd1306_command(0x21);
  ssd1306_command(col);
  ssd1306_command(127);
  ssd1306_command(0x22);
  ssd1306_command(page);
  ssd1306_command(7);
}

void ssd1306_draw_char(char c) {
  if (c < '0' || c > '9')
    return;
  const uint8_t *bitmap = font5x7[c - '0'];
  uint8_t i;
  for (i = 0; i < 5; i++) {
    ssd1306_data(bitmap[i]);
  }
  ssd1306_data(0x00);
}

void ssd1306_print(const char *str) {
  while (*str) {
    ssd1306_draw_char(*str++);
  }
}

void init_display() {
  // Inicializacao do I2C
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

  // Inicializacao do ssd1306
  ssd1306_command(0xAE);
  ssd1306_command(0xA8);
  ssd1306_command(0x3F);
  ssd1306_command(0xD3);
  ssd1306_command(0x00);
  ssd1306_command(0x40);
  ssd1306_command(0xA1);
  ssd1306_command(0xC8);
  ssd1306_command(0xDA);
  ssd1306_command(0x12);
  ssd1306_command(0x81);
  ssd1306_command(0x7F);
  ssd1306_command(0xA4);
  ssd1306_command(0xA6);
  ssd1306_command(0xD5);
  ssd1306_command(0x80);
  ssd1306_command(0x8D);
  ssd1306_command(0x14);
  ssd1306_command(0x20);
  ssd1306_command(0x00);
  ssd1306_command(0xAF);
}

void display_distance(float distancia) {
  char buffer[3];
  sprintf(buffer, "%d", (int)distancia);
  ssd1306_set_cursor(0, 0);
  ssd1306_print(buffer);
}

void display_clear() { ssd1306_clear(); }