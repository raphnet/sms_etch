#include "util.h"
#include "SMSlib.h"

#ifdef TARGET_GG
#define SMS_loadBGPalette	GG_loadBGPalette
#define SMS_loadSpritePalette	GG_loadSpritePalette

#define SMS_loadBGPaletteHalfBrightness	GG_loadBGPalette
#define SMS_loadSpritePaletteHalfBrightness	GG_loadSpritePalette
#define SMS_resetPauseRequest()	do { } while(0)
#endif

void util_smsClear(void *bgpal, uint8_t bgpal_half_brightness, void *tiles, int tiles_size)
{
	SMS_displayOff();
	SMS_waitForVBlank();
	SMS_mapROMBank(2);
	if (bgpal_half_brightness) {
		SMS_loadBGPaletteHalfBrightness(bgpal);
	} else {
		SMS_loadBGPalette(bgpal);
	}
	SMS_VRAMmemset(0, 0, 16*1024);	// Clear VRAM
	SMS_loadTiles(tiles, 0, tiles_size);
	SMS_setBGScrollX(0);
	SMS_setBGScrollY(0);
	SMS_initSprites();
	SMS_finalizeSprites();
	SMS_resetPauseRequest();
	SMS_waitForVBlank();
	SMS_copySpritestoSAT();
	SMS_waitForVBlank();
}

