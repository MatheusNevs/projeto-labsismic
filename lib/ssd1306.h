#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_set_cursor(uint8_t col, uint8_t page);
void ssd1306_print(const char* str);
void ssd1306_draw_char(char c);

#endif
