#include <stdint.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)	(sizeof(arr) / sizeof(arr[0]))
#endif


void util_smsClear(void *bgpal, uint8_t bgpal_half_brightness, void *tiles, int tiles_size);
