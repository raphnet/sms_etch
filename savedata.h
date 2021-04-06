#ifndef _savedata_h__
#define _savedata_h__

#include "savestruct.h"

#define SAVEDATA_BLOCK_SIZE		sizeof(struct savestruct)

extern uint8_t g_savedata[];
extern uint8_t savedata_is_sram_based;

/* Load saved data from Flash or SRAM into g_savedata. */
void savedata_init();

/* Commit g_savedata to Flash or SRAM */
char savedata_commit();


#endif // _savedata_h__
