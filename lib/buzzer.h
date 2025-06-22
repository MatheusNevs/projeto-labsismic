#ifndef BUZZER_H
#define BUZZER_H

void init_buzzer(void);
void bip_by_category(unsigned int category);
void lower_volume(void);
void upper_volume(void);
void lock_buzzer_off();

#endif
