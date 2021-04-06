#include <stdint.h>

extern uint8_t pen_x, pen_y;

void drawLetter(char c, uint8_t scale);
void drawString(const char *str, int8_t scale);

