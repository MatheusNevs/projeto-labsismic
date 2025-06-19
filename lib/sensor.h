#ifndef SENSOR_H
#define SENSOR_H

void init_sensor(void);
void sample(void);
float get_current_distance(void);
unsigned int get_category(void);

#endif