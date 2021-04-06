#include <stdint.h>

extern uint8_t start_pressed;
void updateDisplay(int new_x, int new_y, int old_x, int old_y, uint8_t s);
void vsync_keycheck();

