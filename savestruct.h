#include <stdint.h>
#include "defs.h"

struct savestruct {
	uint8_t magic[4];
	uint8_t flags;
	uint8_t drawbuf[DRAWAREA_TILES_W * DRAWAREA_TILES_H * 8];
};

void savestruct_reset(struct savestruct *s);
uint8_t savestruct_valid(const struct savestruct *s);

#define SAVESTRUCT ((struct savestruct *)g_savedata)
