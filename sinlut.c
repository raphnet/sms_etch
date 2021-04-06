#include "sinlut.h"

static const int sinlut[256] = {
// Sinus lookup table
32000, 31990, 31961, 31913, 31845, 31759, 31653, 31528, 31385, 31222, 31041, 30840, 30622, 30384, 30129, 29855, 
29564, 29254, 28927, 28583, 28221, 27842, 27447, 27035, 26607, 26162, 25702, 25227, 24736, 24230, 23710, 23175, 
22627, 22065, 21489, 20901, 20300, 19687, 19062, 18425, 17778, 17119, 16451, 15772, 15084, 14387, 13681, 12967, 
12245, 11516, 10780, 10037,  9289,  8534,  7775,  7011,  6242,  5470,  4695,  3917,  3136,  2354,  1570,   785, 
    0,  -785, -1570, -2354, -3136, -3917, -4695, -5470, -6242, -7011, -7775, -8534, -9289, -10037, -10780, -11516, 
-12245, -12967, -13681, -14387, -15084, -15772, -16451, -17119, -17778, -18425, -19062, -19687, -20300, -20901, -21489, -22065, 
-22627, -23175, -23710, -24230, -24736, -25227, -25702, -26162, -26607, -27035, -27447, -27842, -28221, -28583, -28927, -29254, 
-29564, -29855, -30129, -30384, -30622, -30840, -31041, -31222, -31385, -31528, -31653, -31759, -31845, -31913, -31961, -31990, 
-32000, -31990, -31961, -31913, -31845, -31759, -31653, -31528, -31385, -31222, -31041, -30840, -30622, -30384, -30129, -29855, 
-29564, -29254, -28927, -28583, -28221, -27842, -27447, -27035, -26607, -26162, -25702, -25227, -24736, -24230, -23710, -23175, 
-22627, -22065, -21489, -20901, -20300, -19687, -19062, -18425, -17778, -17119, -16451, -15772, -15084, -14387, -13681, -12967, 
-12245, -11516, -10780, -10037, -9289, -8534, -7775, -7011, -6242, -5470, -4695, -3917, -3136, -2354, -1570,  -785, 
    0,   785,  1570,  2354,  3136,  3917,  4695,  5470,  6242,  7011,  7775,  8534,  9289, 10037, 10780, 11516, 
12245, 12967, 13681, 14387, 15084, 15772, 16451, 17119, 17778, 18425, 19062, 19687, 20300, 20901, 21489, 22065, 
22627, 23175, 23710, 24230, 24736, 25227, 25702, 26162, 26607, 27035, 27447, 27842, 28221, 28583, 28927, 29254, 
29564, 29855, 30129, 30384, 30622, 30840, 31041, 31222, 31385, 31528, 31653, 31759, 31845, 31913, 31961, 31990,
};

int16_t cheapSin(uint8_t angle256)
{
	return sinlut[angle256];
}

int16_t cheapCos(uint8_t angle256)
{
	return cheapSin(angle256 + 64);
}

