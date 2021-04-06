#include "knob.h"

static const int8_t knobsteps[128] =
{
	  9,   0, // step 0 - 0 deg.
	  8,   0, // step 1 - 5 deg.
	  8,   1, // step 2 - 11 deg.
	  8,   2, // step 3 - 16 deg.
	  8,   3, // step 4 - 22 deg.
	  7,   4, // step 5 - 28 deg.
	  7,   5, // step 6 - 33 deg.
	  6,   5, // step 7 - 39 deg.
	  6,   6, // step 8 - 45 deg.
	  5,   6, // step 9 - 50 deg.
	  5,   7, // step 10 - 56 deg.
	  4,   7, // step 11 - 61 deg.
	  3,   8, // step 12 - 67 deg.
	  2,   8, // step 13 - 73 deg.
	  1,   8, // step 14 - 78 deg.
	  0,   8, // step 15 - 84 deg.
	  0,   9, // step 16 - 90 deg.
	  0,   8, // step 17 - 95 deg.
	 -1,   8, // step 18 - 101 deg.
	 -2,   8, // step 19 - 106 deg.
	 -3,   8, // step 20 - 112 deg.
	 -4,   7, // step 21 - 118 deg.
	 -5,   7, // step 22 - 123 deg.
	 -5,   6, // step 23 - 129 deg.
	 -6,   6, // step 24 - 135 deg.
	 -6,   5, // step 25 - 140 deg.
	 -7,   5, // step 26 - 146 deg.
	 -7,   4, // step 27 - 151 deg.
	 -8,   3, // step 28 - 157 deg.
	 -8,   2, // step 29 - 163 deg.
	 -8,   1, // step 30 - 168 deg.
	 -8,   0, // step 31 - 174 deg.
	 -9,   0, // step 32 - 180 deg.
	 -8,   0, // step 33 - 185 deg.
	 -8,  -1, // step 34 - 191 deg.
	 -8,  -2, // step 35 - 196 deg.
	 -8,  -3, // step 36 - 202 deg.
	 -7,  -4, // step 37 - 208 deg.
	 -7,  -5, // step 38 - 213 deg.
	 -6,  -5, // step 39 - 219 deg.
	 -6,  -6, // step 40 - 225 deg.
	 -5,  -6, // step 41 - 230 deg.
	 -5,  -7, // step 42 - 236 deg.
	 -4,  -7, // step 43 - 241 deg.
	 -3,  -8, // step 44 - 247 deg.
	 -2,  -8, // step 45 - 253 deg.
	 -1,  -8, // step 46 - 258 deg.
	  0,  -8, // step 47 - 264 deg.
	  0,  -9, // step 48 - 270 deg.
	  0,  -8, // step 49 - 275 deg.
	  1,  -8, // step 50 - 281 deg.
	  2,  -8, // step 51 - 286 deg.
	  3,  -8, // step 52 - 292 deg.
	  4,  -7, // step 53 - 298 deg.
	  5,  -7, // step 54 - 303 deg.
	  5,  -6, // step 55 - 309 deg.
	  6,  -6, // step 56 - 314 deg.
	  6,  -5, // step 57 - 320 deg.
	  7,  -5, // step 58 - 326 deg.
	  7,  -4, // step 59 - 331 deg.
	  8,  -3, // step 60 - 337 deg.
	  8,  -2, // step 61 - 343 deg.
	  8,  -1, // step 62 - 348 deg.
	  8,   0, // step 63 - 354 deg.
};

void addRotatedXY(uint8_t angle, uint8_t *x, uint8_t *y)
{
	uint8_t offset;

	offset = (angle & 0x3F) << 1;
	*x += knobsteps[offset];
	*y += knobsteps[offset+1];
}


