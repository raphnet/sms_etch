#include <string.h>
#include "savestruct.h"

#define MAGIC 'S','K','0','1'

void savestruct_reset(struct savestruct *s)
{
	const uint8_t magic[] = { MAGIC };
	memset(s, 0, sizeof(struct savestruct));
	memcpy(s->magic, magic, sizeof(magic));
}

uint8_t savestruct_valid(const struct savestruct *s)
{
	const uint8_t magic[] = { MAGIC };
	return !memcmp(s->magic, magic, sizeof(magic));
}

