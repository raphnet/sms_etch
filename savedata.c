#include <string.h>
#include "flash.h"
#include "savedata.h"
#include "SMSlib.h"

#define SAVEDATA_FLASH_SECTOR	1	// The first 64k (sector 0) is of the game
#define SAVEDATA_FLASH_OFFSET	0x0000

uint8_t g_savedata[SAVEDATA_BLOCK_SIZE];
uint8_t savedata_is_sram_based;

void savedata_init()
{
	if (flash_isKnownId()) {
		flash_readBytes(g_savedata,
							SAVEDATA_BLOCK_SIZE,
							SAVEDATA_FLASH_SECTOR,
							SAVEDATA_FLASH_OFFSET);
		savedata_is_sram_based = 0;
	} else {
		SMS_enableSRAM();
		memcpy(g_savedata, SMS_SRAM, SAVEDATA_BLOCK_SIZE);
		SMS_disableSRAM();
		savedata_is_sram_based = 1;
	}
}

char savedata_commit()
{
	if (savedata_is_sram_based) {
		SMS_enableSRAM();
		memcpy(SMS_SRAM, g_savedata, SAVEDATA_BLOCK_SIZE);
		SMS_disableSRAM();
		return 1;
	}

	/* Only write to flash if the data has changed */
	if (flash_compareBytes(g_savedata, SAVEDATA_BLOCK_SIZE,
										SAVEDATA_FLASH_SECTOR,
										SAVEDATA_FLASH_OFFSET))
	{
		flash_erase64kSector(SAVEDATA_FLASH_SECTOR);
		flash_programBytes(g_savedata, SAVEDATA_BLOCK_SIZE,
										SAVEDATA_FLASH_SECTOR,
										SAVEDATA_FLASH_OFFSET);
		return 1;
	}
	return 0;
}

