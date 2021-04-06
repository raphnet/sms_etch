#ifndef _flash_h__
#define _flash_h__

#include <stdint.h>

#define FLASH_ID_MX29F040	0xA4C2

/* Basic flash functions - Implementing real low flash commands. */
void flash_erase64kSector(uint8_t sector_id);
void flash_programByte(uint8_t value, uint8_t sector, uint16_t offset);
uint16_t flash_readSiliconID(void);

/* Helper functions, built using the above. */
char flash_isKnownId(void);
void flash_readBytes(uint8_t *buffer, int size, uint8_t sector, uint16_t offset);
void flash_programBytes(const uint8_t *buffer, int size, uint8_t sector, uint16_t offset);
uint8_t flash_readByte(uint8_t sector, uint16_t offset);
char flash_compareBytes(const uint8_t *buffer, int size, uint8_t sector, uint16_t offset);

#endif // _flash_h__

