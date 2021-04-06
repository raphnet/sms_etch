#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "SMSlib.h"
#include "PSGlib.h"
#include "data.h"
#include "util.h"
#include "defs.h"
#include "savedata.h"
#include "inlib.h"
#include "vfont.h"
#include "knob.h"
#include "main.h"
#include "sinlut.h"

#define MAX_PY (DRAWAREA_TILES_H * 8 - 1)
#define MAX_PX (DRAWAREA_TILES_W * 8 - 1)

// Uses 11k out of 8k. Oups.
//static uint8_t drawbuf[DRAWAREA_TILES_W * DRAWAREA_TILES_H * 32];

// 1bpp tiles use only 2.8k. Good!
//static uint8_t drawbuf[DRAWAREA_TILES_W * DRAWAREA_TILES_H * 8];
#define POINTER_SID	0
#define LK_SID	1
#define RK_SID	2
#define UL_SID	3
#define UR_SID	4
#define LL_SID	5
#define LR_SID	6
#define N_SPRITES	7
static uint8_t spriteIds[N_SPRITES];
static uint8_t spriteX[N_SPRITES];
static uint8_t spriteY[N_SPRITES];
uint8_t *drawbuf;

static uint8_t drawing = 0;
static uint8_t paddle_mode = 0;
static uint8_t fill_mode = 0;

void setupSprite(uint8_t sidx, uint8_t x, uint8_t y, uint8_t tile_id)
{
	spriteX[sidx] = x;
	spriteY[sidx] = y;
	spriteIds[sidx] = SMS_addSprite(x, y, tile_id);
}

// Move a sprite relative to is current stored position. The
// new position is not stored (temporary)
//
// sids Sprite index
// x relative x (added to current x)
// y relative y (added to current y)
void tempMoveSprite(uint8_t sidx, int8_t x, int8_t y)
{
	SMS_updateSpritePosition(spriteIds[sidx], spriteX[sidx]+x, spriteY[sidx]+y);
}

uint8_t start_pressed;

void vsync_keycheck()
{
	SMS_waitForVBlank();
	SMS_copySpritestoSAT();
	PSGFrame();
	PSGSFXFrame();
	inlib_poll();

	// Very lazy check exploiting the fact that the buttons
	// members aligns in all structures...
	if (inlib_port1.sms.buttons) {
		start_pressed = 1;
	}
}


__sfr __at 0xBE VDPDataPort;


// 7 ok
// 5 ok << chosen for safety
// 4 ok
// 3 not ok
#define WAIT_VRAM __asm nop \
						nop \
						nop \
						nop \
						nop __endasm


static void blitTile(uint8_t *data, uint16_t tilefrom, unsigned char n_tiles)
{
	uint8_t i, v;

	__asm
	di
	__endasm;

	SMS_setAddr(0x4000 | (tilefrom*32));

	for (i=0; i<n_tiles*8; i++) {
		v = ~(*data++);
		VDPDataPort=v;
		WAIT_VRAM;
		VDPDataPort=v;
		WAIT_VRAM;
		VDPDataPort=v;
		WAIT_VRAM;
		VDPDataPort=v;
		WAIT_VRAM;
	}

	__asm
	ei
	__endasm;


}

static void syncDisplay(void)
{
	int y, x;

	for (y=0; y < DRAWAREA_TILES_H; y++) {

		blitTile(drawbuf + (y * DRAWAREA_TILES_W) * 8,
					DRAWAREA_FIRST_TID + y * DRAWAREA_TILES_W,
					DRAWAREA_TILES_W);
	}
}

static void clearDisplay(uint8_t full)
{
	int y, x, i, xe, ye;
	uint8_t *d;
	static uint8_t msk = 0xAA;
	static uint8_t off = 0;

	if (full) {
		memset(drawbuf, 0, DRAWAREA_TILES * 8);
	} else {
		for (i=0; i<DRAWAREA_TILES * 8; i+=4) {
			drawbuf[i+(off&3)] &= msk;

			// Avoid music slowdown 
			if ((i&0x1ff) == 0) {
				SMS_waitForVBlank();
				PSGFrame();
				PSGSFXFrame();
			}
		}
		msk ^= 0xff;

		for (i=0; i<DRAWAREA_TILES * 8; i+=4) {
			drawbuf[i+((2+off)&3)] &= msk;

			// Avoid music slowdown 
			if ((i&0x1ff) == 0) {
				SMS_waitForVBlank();
				PSGFrame();
				PSGSFXFrame();
			}
		}
		off++;
	}


	for (y=0; y < DRAWAREA_TILES_H; y++) {
		SMS_waitForVBlank();
		SMS_copySpritestoSAT();
		PSGFrame();
		PSGSFXFrame();

		blitTile(drawbuf + (y * DRAWAREA_TILES_W) * 8,
					DRAWAREA_FIRST_TID + y * DRAWAREA_TILES_W,
					DRAWAREA_TILES_W / 2);

		SMS_waitForVBlank();
		SMS_copySpritestoSAT();
		PSGFrame();
		PSGSFXFrame();

		blitTile(drawbuf + (y * DRAWAREA_TILES_W + DRAWAREA_TILES_W / 2) * 8,
					DRAWAREA_FIRST_TID + y * DRAWAREA_TILES_W + DRAWAREA_TILES_W / 2,
					DRAWAREA_TILES_W / 2);


		ye = (y & 3)<<2;
		SMS_setBGScrollY(ye);
		xe = (y & 1)<<2;
		SMS_setBGScrollX(xe);

		for (i=LK_SID; i<N_SPRITES; i++) {
			tempMoveSprite(i, xe, -ye);
		}


	}

	for (i=LK_SID; i<N_SPRITES; i++) {
		tempMoveSprite(i, 0, 0);
	}

	SMS_setBGScrollX(0);
	SMS_setBGScrollY(0);
}

static void setupTilemap()
{
	int y, x;
	int tid = DRAWAREA_FIRST_TID;

	SMS_loadTileMap(0,0, main_tilemap, main_tilemap_size);

	for (y=0; y<DRAWAREA_TILES_H; y++) {
		SMS_setNextTileatXY(DRAWAREA_X, y + DRAWAREA_Y);
		for (x=0; x<DRAWAREA_TILES_W; x++) {
			SMS_setTile(tid++);
		}
	}

	// Add one row of black tiles below the screen, to avoid visible
	// artefacts when shaking
	for (y=24; y<26; y++) {
		SMS_setNextTileatXY (0, y);
		for (x=0; x<32; x++) {
			SMS_setTile(BGBLANK_TID);
		}
	}
}

static void putPixel(int x, int y)
{
	int modified_tile;
	uint8_t bit;

	// Draw!
	modified_tile = (x>>3) + (y>>3) * DRAWAREA_TILES_W;

	bit = 0x80 >> (x & 7);
	drawbuf[(modified_tile << 3) + (y & 0x7)] |= bit;

	blitTile(drawbuf + (modified_tile << 3), DRAWAREA_FIRST_TID + modified_tile, 1);
}

static void drawLine(int x0, int y0, int x1, int y1)
{
	int16_t dx = abs(x1-x0);
	int16_t dy = -abs(y1-y0);
	int8_t sx = x0<x1 ? 1 : -1;
	int8_t sy = y0<y1 ? 1 : -1;
	int err = dx+dy;
	int e2;
	uint16_t modified_tile, next_tile;
	uint8_t bit;

	modified_tile = (x0>>3) + (y0>>3) * DRAWAREA_TILES_W;

	while(1) {

		next_tile = (x0>>3) + (y0>>3) * DRAWAREA_TILES_W;
		if (modified_tile != next_tile) {
			blitTile(drawbuf + (modified_tile << 3), DRAWAREA_FIRST_TID + modified_tile, 1);
		}
		modified_tile = next_tile;

		bit = 0x80 >> (x0 & 7);
		drawbuf[(modified_tile << 3) + (y0 & 0x7)] |= bit;


		if ((x0 == x1) && (y0 == y1))
			break;
		e2 = err * 2;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}

	blitTile(drawbuf + (modified_tile << 3), DRAWAREA_FIRST_TID + modified_tile, 1);
}

static void drawLineSync(int x0, int y0, int x1, int y1)
{
	int16_t dx = abs(x1-x0);
	int16_t dy = -abs(y1-y0);
	int8_t sx = x0<x1 ? 1 : -1;
	int8_t sy = y0<y1 ? 1 : -1;
	int err = dx+dy;
	int e2;
	uint16_t modified_tile, next_tile;
	uint8_t bit;

	modified_tile = (x0>>3) + (y0>>3) * DRAWAREA_TILES_W;

	while(1) {

		next_tile = (x0>>3) + (y0>>3) * DRAWAREA_TILES_W;
		if (modified_tile != next_tile) {
			blitTile(drawbuf + (modified_tile << 3), DRAWAREA_FIRST_TID + modified_tile, 1);

			vsync_keycheck();
		}
		modified_tile = next_tile;

		bit = 0x80 >> (x0 & 7);
		drawbuf[(modified_tile << 3) + (y0 & 0x7)] |= bit;


		if ((x0 == x1) && (y0 == y1))
			break;
		e2 = err * 2;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}

	blitTile(drawbuf + (modified_tile << 3), DRAWAREA_FIRST_TID + modified_tile, 1);
	vsync_keycheck();
}


void updateDisplay(int new_x, int new_y, int old_x, int old_y, uint8_t s)
{
	int modified_tile;
	uint8_t bit;

	if (drawing) {
		SMS_hideSprite(spriteIds[POINTER_SID]);
	} else {
		SMS_updateSpritePosition(spriteIds[POINTER_SID], new_x + DRAWAREA_X * 8, new_y + DRAWAREA_Y * 8);
	}

	if (drawing) {
		putPixel(new_x, new_y);

		// points can be non-adjacent, fill the gap with lines
		if (paddle_mode || fill_mode) {
			if (!s) {
				drawLine(new_x, new_y, old_x, old_y);
			} else {
				drawLineSync(new_x, new_y, old_x, old_y);
			}
		}
	}

	// Animate knobs
	spriteX[LK_SID] = LEFT_KNOB_X;
	spriteY[LK_SID] = LEFT_KNOB_Y;
	addRotatedXY(new_x, &spriteX[LK_SID], &spriteY[LK_SID]);
	SMS_updateSpritePosition(spriteIds[LK_SID], spriteX[LK_SID], spriteY[LK_SID]);

	spriteX[RK_SID] = RIGHT_KNOB_X;
	spriteY[RK_SID] = RIGHT_KNOB_Y;
	addRotatedXY(new_y, &spriteX[RK_SID], &spriteY[RK_SID]);
	SMS_updateSpritePosition(spriteIds[RK_SID], spriteX[RK_SID], spriteY[RK_SID]);

}

enum {
	EVTYPE_DRAWSTRING,
	EVTYPE_CLEAR,
	EVTYPE_CLEAR_FULL,
	EVTYPE_FN,
	EVTYPE_END
};

struct scriptEvent {
	uint16_t frame;
	uint8_t evtype;
	const char *str;
	int x, y;
	int scale;
	void (*fn)(struct scriptEvent *ev);
};

void stepIt(int *val, int *inc, int max)
{
	*val += *inc;
	if (*val < 0) {
		*inc = -*inc;
		*val = -*val;
	}
	if (*val >= max) {
		*inc = -*inc;
		*val = max - (*val - max) - 1;
	}
}

void effect2(struct scriptEvent *ev)
{
	int prev_x, prev_y;
	int x, y;
	uint16_t i;
	int idx, idx2;
	int x1=0, y1=30, x2=MAX_PX, y2=MAX_PY-30;

	x = prev_x = 40;
	y = prev_y = 50;

	for (i=0; i<512 && !start_pressed; i++) {

		x = MAX_PX / 2 + (cheapSin(i/2) >> 10) + (cheapCos(i*2+i) >> 11) ;
		y = MAX_PY / 2 + (-cheapCos(i/2+i) >> 11) + (cheapSin(i*5) >> 12) ;

		if (i) {
			updateDisplay(prev_x, prev_y, x, y, 1);
		}

		prev_x = x;
		prev_y = y;
	}

}

void effect3(struct scriptEvent *ev)
{
	int prev_x, prev_y;
	int x, y;
	uint16_t i;
	int idx, idx2;
	int x1=0, y1=30, x2=MAX_PX, y2=MAX_PY-30;

	x = prev_x = 40;
	y = prev_y = 50;

	for (i=0; i<512 && !start_pressed; i++) {

		x = MAX_PX / 2 + ( (cheapSin(i) >> 11) + (cheapCos(i*3+i/2) >> 10) ) ;
		y = MAX_PY / 2 + ( (cheapCos(i) >> 11) + (cheapSin(i*3+i/2) >> 10) ) ;

		if (i) {
			updateDisplay(prev_x, prev_y, x, y, 1);
		}

		prev_x = x;
		prev_y = y;
	}

}

void effect4(struct scriptEvent *ev)
{
	int prev_x, prev_y;
	int x, y;
	uint16_t i;
	int idx, idx2;
	int x1=0, y1=30, x2=MAX_PX, y2=MAX_PY-30;

	x = prev_x = 40;
	y = prev_y = 50;

	for (i=0; i<2048; i++) {

		x = MAX_PX / 2 + ( (cheapSin(i*8+i) >> 10) );
		y = MAX_PY / 2 + ( (cheapSin(i*7) >> 10) );

		if (i) {
			drawLineSync(prev_x, prev_y, x, y);
		}

		prev_x = x;
		prev_y = y;
	}

}

void effect1(struct scriptEvent *ev)
{
	int x1=0, y1=30, x2=MAX_PX, y2=MAX_PY-30;
	int x1i=0, y1i=5, x2i=-0, y2i=-5;
	int i;

	for (i=0; i<10; i++) {

		stepIt(&x1, &x1i, MAX_PX);
		stepIt(&x2, &x2i, MAX_PX);
		stepIt(&y1, &y1i, MAX_PY);
		stepIt(&y2, &y2i, MAX_PY);

		drawLineSync(x1, y1, x2, y2);

	}

}

struct scriptEvent script[] = {

	{ 0, EVTYPE_DRAWSTRING, "raphnet presents", 40, 25, 2 },

	{ 60 * 1, EVTYPE_DRAWSTRING, "sms a sketch", 35, 45, 3 },

	{ 60 * 2, EVTYPE_DRAWSTRING, "for the smspower.org", 20, 80, 2 },
	{ 60 * 2, EVTYPE_DRAWSTRING, "2021 coding competition", 15, 100, 2 },

	{ 60 * 3, EVTYPE_CLEAR_FULL },

	{ 60 * 3, EVTYPE_DRAWSTRING, "Music arranged from:", 5, 40, 2 },
	{ 60 * 3, EVTYPE_DRAWSTRING, "<Pictures at an exhibition>", 5, 65, 2 },
	{ 60 * 3, EVTYPE_DRAWSTRING, "by Mussorgsky", 5, 80, 2 },

//	{ 60 * 3, EVTYPE_DRAWSTRING, "(1st <Promenade> movement)", 5, 85, 2 },

	{ 60 * 5, EVTYPE_CLEAR_FULL },

	{ 0, EVTYPE_DRAWSTRING, "Tools used:", 5, 30, 2 },
	{ 0, EVTYPE_DRAWSTRING, "devkitSMS (SMSlib, PSGlib),", 5, 55, 2 },
	{ 0, EVTYPE_DRAWSTRING, "sdcc, DefleMask, gimp...", 5, 70, 2 },

	{ 60 * 6, EVTYPE_DRAWSTRING, "Thanks!", 5, 100, 2 },

	{ 60 * 7, EVTYPE_CLEAR_FULL },


	{ 0, EVTYPE_DRAWSTRING, "What do you see?", 5, 15, 2 },
	{ 0, EVTYPE_FN, NULL, 0, 0, 0, effect2 },

	{ 0, EVTYPE_DRAWSTRING, "I cannot draw very well so I cheated with", 5, 100, 1 },
	{ 0, EVTYPE_DRAWSTRING, "a mix of trigonometric functions...", 5, 110, 1 },
	{ 60 * 10 , EVTYPE_CLEAR_FULL },

	{ 0, EVTYPE_FN, NULL, 0, 0, 0, effect3 },

	{ 60 * 11 , EVTYPE_CLEAR },
	{ 0 , EVTYPE_CLEAR },
	{ 0 , EVTYPE_CLEAR },
	{ 0, EVTYPE_DRAWSTRING, "Your turn!", 100, 90, 2 },

	{ 60 * 12 , EVTYPE_CLEAR },
	{ 0, EVTYPE_CLEAR },
	{ 0, EVTYPE_CLEAR },

	{ 0 , EVTYPE_END }

};

void runScript(void)
{
	static uint16_t frame = 0;
	struct scriptEvent *nextEvent;
	unsigned int keys, now;

	vsync_keycheck();

	start_pressed = 0;
	nextEvent = &script[0];
	for(;!start_pressed;frame++)
	{
		vsync_keycheck();

		if (nextEvent && (nextEvent->frame < frame)) {
			if (nextEvent->evtype != EVTYPE_END) {
				// Execute the event
				switch (nextEvent->evtype)
				{
					case EVTYPE_DRAWSTRING:
						pen_x = nextEvent->x;
						pen_y = nextEvent->y;

						drawing = 1;
						fill_mode = 1;
						drawString(nextEvent->str, nextEvent->scale);
						drawing = 0;
						fill_mode = 0;
						break;

					case EVTYPE_CLEAR:
						clearDisplay(0);
						break;

					case EVTYPE_CLEAR_FULL:
						clearDisplay(1);
						break;

					case EVTYPE_FN:
						drawing = 1;
						fill_mode = 1;
						if (nextEvent->fn) {
							nextEvent->fn(nextEvent);
						}
						drawing = 0;
						fill_mode = 0;
						break;

				}
				nextEvent++;
			} else {
				clearDisplay(1);
				nextEvent = NULL;
				return;
			}
		}

	}

	clearDisplay(1);
	nextEvent = NULL;
}

static void inputDiag_printDevice(const char *title, uint8_t startcol, const inlib_data *dat)
{
	uint8_t i;

	SMS_setNextTileatXY(startcol,8);
	printf(title);

	SMS_setNextTileatXY(startcol,10);
	switch (dat->type)
	{
		default: printf("Unknown"); break;
		case INLIB_TYPE_SMS:
			printf("SMS Controller");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->sms.buttons);
			break;
		case INLIB_TYPE_PADDLE:
			printf("SMS Paddle    ");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->paddle.buttons);
			SMS_setNextTileatXY(startcol,12);
			printf("Val: %02x", dat->paddle.value);
			break;
		case INLIB_TYPE_MD_MOUSE:
			printf("MD mouse      ");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->mdmouse.buttons);
			SMS_setNextTileatXY(startcol,12);
			printf("X: %d  ", dat->mdmouse.x);
			SMS_setNextTileatXY(startcol,13);
			printf("Y: %d  ", dat->mdmouse.y);
			break;
		case INLIB_TYPE_GFX_V2:
			printf("GFX. BRD. V2  ");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->gfx2.buttons);
			SMS_setNextTileatXY(startcol,12);
			printf("X: %d  ", dat->gfx2.x);
			SMS_setNextTileatXY(startcol,13);
			printf("Y: %d  ", dat->gfx2.y);
			SMS_setNextTileatXY(startcol,14);
			printf("P: %d  ", dat->gfx2.pressure);
			break;
		case INLIB_TYPE_SPORTSPAD:
			printf("SPORTSPAD     ");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->spad.buttons);
			SMS_setNextTileatXY(startcol,12);
			printf("X: %d  ", dat->spad.x);
			SMS_setNextTileatXY(startcol,13);
			printf("Y: %d  ", dat->spad.y);
			break;
		case INLIB_TYPE_LIGHT_PHASER:
			printf("LIGHT PHASER  ");
			SMS_setNextTileatXY(startcol,11);
			printf("Btn: %02x", dat->phaser.buttons);
			SMS_setNextTileatXY(startcol,12);
			printf("X: %d  ", dat->phaser.x);
			SMS_setNextTileatXY(startcol,13);
			printf("Y: %d  ", dat->phaser.y);
			break;
	}

}



void inputDiag_inlib(void)
{
	uint8_t polling = 1;
	uint8_t a = 0;

	SMS_displayOff();
	SMS_autoSetUpTextRenderer();
	SMS_displayOn();
	SMS_waitForVBlank();
	SMS_waitForVBlank();
	SMS_waitForVBlank();
	SMS_setNextTileatXY(1,1);
	printf("Input device diagnostic ");
	SMS_setNextTileatXY(1,3);
	printf("SMS-A-SKETCH V" VERSION_STR);
	SMS_waitForVBlank();

	inlib_init();



	SMS_resetPauseRequest();


	do {
		SMS_waitForVBlank();
		inlib_poll();

		inputDiag_printDevice("Port 1", 1, &inlib_port1);
		inputDiag_printDevice("Port 2", 16, &inlib_port2);

		SMS_setNextTileatXY(1,5);
		printf("%02x", a);
		a++;

		if (inlib_port1.type == INLIB_TYPE_PADDLE) {
			if (inlib_port2.type != INLIB_TYPE_PADDLE) {
				SMS_setNextTileatXY(1,20);
				printf("TWO PADDLES REQUIRED.  ");
				inlib_init();
				continue;
			}

			SMS_setNextTileatXY(1,17);
			printf("PADDLE MODE ENABLED    ");
		} else if (inlib_port1.type == INLIB_TYPE_MD_MOUSE) {
			SMS_setNextTileatXY(1,17);
			printf("MOUSE MODE ENABLED     ");
		} else if (inlib_port1.type == INLIB_TYPE_GFX_V2) {
			SMS_setNextTileatXY(1,17);
			printf("GFX. BRD. V2 ENABLED   ");
		}
		else if (inlib_port1.type == INLIB_TYPE_SPORTSPAD) {
			SMS_setNextTileatXY(1,17);
			printf("SPORTSPAD MODE ENABLED ");
		}
		else if (inlib_port1.type == INLIB_TYPE_LIGHT_PHASER) {
			SMS_setNextTileatXY(1,17);
			printf("LIGHT PHASER MODE      ");
		}
		else {
			SMS_setNextTileatXY(1,17);
			printf("CONTROLLER MODE ENABLED");
		}

		SMS_setNextTileatXY(1,23);
		printf("PRESS TO CONTINUE");

		switch (inlib_port1.type)
		{
			case INLIB_TYPE_MD_MOUSE:
				if (inlib_port1.mdmouse.buttons) {
					polling = 0;
				}
				break;
			case INLIB_TYPE_SMS:
				if (inlib_port1.sms.buttons) {
					if (inlib_port1.sms.buttons != SMS_BTN_1) {
						break;
					}
					polling = 0;
				}
				break;
			case INLIB_TYPE_PADDLE:
				if (inlib_port1.paddle.buttons) {
					polling = 0;
				}
				break;
			case INLIB_TYPE_GFX_V2:
				if (inlib_port1.gfx2.buttons) {
					polling = 0;
				}
				break;
			case INLIB_TYPE_SPORTSPAD:
				if (inlib_port1.spad.buttons) {
					polling = 0;
				}
				break;
			case INLIB_TYPE_LIGHT_PHASER:
				if (inlib_port1.phaser.buttons) {
					polling = 0;
				}
				break;
		}

		if (inlib_port1.type == INLIB_TYPE_SMS) {
			SMS_setNextTileatXY(1,19);
			printf("Press PAUSE for SPORTSPAD");
			SMS_setNextTileatXY(1,20);
			printf("And Graphics Board v2");
			SMS_setNextTileatXY(1,21);
			printf("Or light phaser");
		}

		if (SMS_queryPauseRequested()) {
			SMS_resetPauseRequest();

			if (inlib_port1.type == INLIB_TYPE_SMS) {
				inlib_port1.type = INLIB_TYPE_GFX_V2;
				inlib_poll();
				inlib_poll();
			} else if (inlib_port1.type == INLIB_TYPE_GFX_V2) {
				inlib_port1.type = INLIB_TYPE_SPORTSPAD;
				inlib_poll();
				inlib_poll();
			} else if (inlib_port1.type == INLIB_TYPE_SPORTSPAD) {
				inlib_port1.type = INLIB_TYPE_LIGHT_PHASER;
				inlib_poll();
				inlib_poll();
			} else if (inlib_port1.type == INLIB_TYPE_LIGHT_PHASER) {
				inlib_port1.type = INLIB_TYPE_SMS;
				inlib_poll();
				inlib_poll();
			}

		}


	} while(polling);

}

void interruptibleFrameDelay(uint16_t frames)
{
	while (frames--) {
		SMS_waitForVBlank();

		if (inlib_port1.sms.buttons) {
			start_pressed = 1;
		}

	}
}

void main(void)
{
	int lev_id = 0;
	int px = (DRAWAREA_TILES_W / 2) * 8;
	int py = (DRAWAREA_TILES_H / 2) * 8;
	int last_px = px;
	int last_py = py;

	SMS_init();
	SMS_displayOff();

	util_smsClear(main_pal, 0, main_tiles, main_tiles_size);

	inputDiag_inlib();
	SMS_displayOff();

	util_smsClear(main_pal, 0, main_tiles, main_tiles_size);

	SMS_useFirstHalfTilesforSprites(1);
	SMS_loadSpritePalette(main_pal);

	SMS_loadTiles(sprites_tiles, SPRITETILES_START, sprites_tiles_size);

	// This is a hack for mapper auto-detection in meka. See
	// the source code for Mapper_AutoDetect.
	//
	// Basically, I was very unlucky, the byte sequence
	// for the LD (8000),A instruction appeared once
	// somewhere in my ROM, and meka assumed a CodeMasters
	// mapper due to this.
	//
	// If at least on LD (FFFF),A instruction is found, this
	// does not happen.
	//
	// sdcc does not compile SMS_mapROMBank(2) to
	// LD (FFFF), A so this was necessary.
	//
	// If the ROM is too small, Meka also assumes there is no mapper. But this is
	// wrong in this case as SRAM is used... So data_bank2.c and data_bank3.c are there
	// to bump the size up to 64kB...
	__asm
		push af
		ld a, #2
		ld (0xFFFF), a
		pop af
	__endasm;
	SMS_mapROMBank(2);


	setupSprite(POINTER_SID, 0, 0, POINTER_TID);

	// Add 4 fixed sprites around the drawarea for rounding corners
	setupSprite(UL_SID, DRAWAREA_X * 8, DRAWAREA_Y * 8, UL_MASK_SPRITE_ID);
	setupSprite(UR_SID, (DRAWAREA_X + DRAWAREA_TILES_W - 1) * 8, DRAWAREA_Y * 8, UR_MASK_SPRITE_ID);
	setupSprite(LL_SID, DRAWAREA_X * 8, (DRAWAREA_Y + DRAWAREA_TILES_H - 1) * 8, LL_MASK_SPRITE_ID);
	setupSprite(LR_SID, (DRAWAREA_X + DRAWAREA_TILES_W - 1) * 8,
						(DRAWAREA_Y + DRAWAREA_TILES_H - 1) * 8,
						LR_MASK_SPRITE_ID);

	setupSprite(LK_SID, LEFT_KNOB_X, LEFT_KNOB_Y, KNOBPOINT_SPRITE_ID);
	setupSprite(RK_SID, RIGHT_KNOB_X, RIGHT_KNOB_Y, KNOBPOINT_SPRITE_ID);

	SMS_waitForVBlank();
	SMS_copySpritestoSAT();

	drawbuf = &(SAVESTRUCT->drawbuf[0]);

	setupTilemap();

	updateDisplay(px, py, last_px, last_py, 0);
	syncDisplay();




	SMS_displayOn();

	PSGPlayNoRepeat(promenade_psgc);

	runScript();

	savedata_init();
	if (!savestruct_valid(SAVESTRUCT)) {
		savestruct_reset(SAVESTRUCT);
		savedata_commit();
	}

	PSGStop();
	syncDisplay();

	for(;;)
	{
		unsigned int keys, now;

		SMS_waitForVBlank();
		SMS_copySpritestoSAT();
		PSGFrame();
		PSGSFXFrame();
		inlib_poll();

		switch (inlib_port1.type)
		{
			case INLIB_TYPE_SMS:
				keys = SMS_getKeysPressed();
				now = SMS_getKeysStatus();

				if (now & PORT_A_KEY_UP) { if (py > 0) { py--; } }
				if (now & PORT_A_KEY_DOWN) { if (py < MAX_PY) { py++; } }
				if (now & PORT_A_KEY_LEFT) { if (px > 0) { px--; } }
				if (now & PORT_A_KEY_RIGHT) { if (px < MAX_PX) { px++; } }

				if ((last_px != px) || ( last_py != py)) {
					updateDisplay(px, py, last_px, last_py, 0);

					last_px = px;
					last_py = py;
				}

				if (keys & PORT_A_KEY_1) {
					drawing = !drawing;
					updateDisplay(px, py, last_px, last_py, 0);
				}

				if (keys & PORT_A_KEY_2) {
					savedata_commit();
					syncDisplay();
				}
				break;

			case INLIB_TYPE_PADDLE:

				px = inlib_port1.paddle.value;
				py = 255-inlib_port2.paddle.value;

				if (px > MAX_PX) { px = MAX_PX; }
				if (py > MAX_PY) { py = MAX_PY; }

				if ((last_px != px) || ( last_py != py)) {
					fill_mode = 1;
					updateDisplay(px, py, last_px, last_py, 0);

					last_px = px;
					last_py = py;
				}

				if (inlib_port1.paddle.pressed & PADDLE_BUTTON) {
					drawing = !drawing;
					updateDisplay(px, py, last_px, last_py, 0);
				}
				break;

			case INLIB_TYPE_MD_MOUSE:

				px += inlib_port1.mdmouse.x;
				py += -inlib_port1.mdmouse.y;

				if (px > MAX_PX) { px = MAX_PX; }
				if (py > MAX_PY) { py = MAX_PY; }
				if (px < 0) { px = 0; }
				if (py < 0) { py = 0; }

				if ((last_px != px) || ( last_py != py)) {
					fill_mode = 1;
					updateDisplay(px, py, last_px, last_py, 0);

					last_px = px;
					last_py = py;
				}

				drawing = inlib_port1.mdmouse.buttons & MDMOUSE_BTN_LEFT;

				if (inlib_port1.mdmouse.buttons & MDMOUSE_BTN_MID) {
					clearDisplay(0);
				}

				if (inlib_port1.mdmouse.buttons & MDMOUSE_BTN_START) {
					savedata_commit();
					syncDisplay();
				}


				break;


			case INLIB_TYPE_SPORTSPAD:

				px += -inlib_port1.spad.x;
				py += -inlib_port1.spad.y;

				if (px > MAX_PX) { px = MAX_PX; }
				if (py > MAX_PY) { py = MAX_PY; }
				if (px < 0) { px = 0; }
				if (py < 0) { py = 0; }

				if ((last_px != px) || ( last_py != py)) {
					fill_mode = 1;
					updateDisplay(px, py, last_px, last_py, 0);

					last_px = px;
					last_py = py;
				}

				drawing = inlib_port1.spad.buttons & SPORTSPAD_BTN_1;

				if (inlib_port1.spad.buttons & SPORTSPAD_BTN_2) {
					savedata_commit();
					syncDisplay();
				}


				break;

			case INLIB_TYPE_LIGHT_PHASER:
				drawing = inlib_port1.phaser.buttons;

				if (drawing) {
					if (inlib_port1.phaser.x < 64) {
						px = 0;
					} else {
						px = inlib_port1.phaser.x - 64;
					}
					if (inlib_port1.phaser.y < 32) {
						py = 0;
					} else {
						py = inlib_port1.phaser.y - 32;
					}

					if (px > MAX_PX) { px = MAX_PX; }
					if (py > MAX_PY) { py = MAX_PY; }
					if (px < 0) { px = 0; }
					if (py < 0) { py = 0; }

					if ((last_px != px) || ( last_py != py)) {
						fill_mode = 1;

						if (last_px && last_py) {
							updateDisplay(px, py, last_px, last_py, 0);
						}

					}

					last_px = px;
					last_py = py;
				} else {
					last_px = last_py = 0;
				}


				break;

			case INLIB_TYPE_GFX_V2:

				// 0 - 255
				px = inlib_port1.gfx2.x;

				// 36 - 255
				py = inlib_port1.gfx2.y - 50;

				if (px > MAX_PX) { px = MAX_PX; }
				if (py > MAX_PY) { py = MAX_PY; }
				if (px < 0) { px = 0; }
				if (py < 0) { py = 0; }

				if ((last_px != px) || ( last_py != py)) {
					fill_mode = 1;
					updateDisplay(px, py, last_px, last_py, 0);

					last_px = px;
					last_py = py;
				}

				drawing = inlib_port1.gfx2.buttons;
				break;


		}




		if (SMS_queryPauseRequested())
		{
			SMS_resetPauseRequest();
			clearDisplay(0);
		}
	}
}


SMS_EMBED_SEGA_ROM_HEADER(9999, 0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(VERSION_MAJ, VERSION_MIN, "raphnet.", "SMS-a-sketch", "\"Etch a sketch\"(tm) style drawing for SMS");

