#include "../lib/utils.h"

unsigned int calculate_delta_cycles(unsigned int current,
                                    unsigned int previous) {
  return (current >= previous) ? (current - previous)
                               : (0xFFFF - previous + current + 1);
}
