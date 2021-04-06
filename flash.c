/* 
 * Copyright 2020 Raphael Assenat <raph@raphnet.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/* How this works
 *
 * It is possible to make a cartridge with a single flash chip used
 * both for holding the game "ROM" and saving wihout requiring a
 * battery backed SRAM.
 *
 * Such flash chips have a write pin and are wired like you would an SRAM,
 * but to enable operations such as programming and erasing, a specific
 * sequence of writes is required. For instance, one must do things such as
 * writing AA to 555, followed by 55 to 2AA, and so on. (See the assembly code
 * below or the flash datasheet for details).
 * 
 * Now, the actual code the console is running is stored on the flash. This
 * means that read accesses are performed as each instruction is fetched which
 * makes it impossible to perform the required sequence of write to issue a
 * various flash commands, as extraneous read cycles would appear between the
 * required writes. Not to mention that during programming and erasing, operations
 * that require time, one cannot read from the flash as the data bits then indicate
 * the operation status rather than data at the requested address.
 *
 * The solution is to run the code from elsewhere, and the only available place,
 * unless the cartridge had two flash chips or an additional ROM, is the console
 * built-in memory. This is acheived by copying machine code to memory, setting
 * up a function pointer and and calling it!
 *
 * Slot 2 is used as a 16k window for all flash acceses. The machine code takes care
 * of bank switching automatically and restores the original bank that was in slot 2
 * before returning. This means that the C code in this file may reside anywhere, even
 * in slot 2.
 *
 * During flash operations, interrupts are disabled. Otherwise, if a vblank interrupt
 * were to occur, the CPU would try to read from the flash and the console would crash.
 * If one always performs flash operations after calling SMS_waitForVBlank() this should
 * not be necessary, but it's good to be safe.
 *
 * The only problem I don't know how to solve happens if the user presses the PAUSE button
 * right in the middle of a flash operation. The pause button is, as far as I know, wired
 * to the NMI and cannot be disabled. But depending on the amount of data written,
 * the flash operation may very well complete in less that 1 ms. The odds of running into
 * this seem low enough. Also, unless the score is saved continuously (which is bad, see below),
 * the flash IO probably won't take place at a moment where a player is likely to pause the
 * game. (for instance, who pauses a game at the Game over screen?)
 *
 * Attention Attention Attention!
 *
 * FLASH is not SRAM. It will wear down quickly if written too often!
 *
 * The MX29F040 is good for *at least* 100000 erase/program cycles,
 * but the point is that there is a limit, and if good practises
 * are not obeyed, there will be problems after a while...
 *
 * Recommendations / Good practises:
 * 
 * - Do not write data to the flash continuously. For instance, do not write
 *   the score each time it increases. Do it once at game over. Or if it is
 *   a save function, do not autosave. Have the user do it from a menu or in-game
 *   save point.
 *
 * - Do not write if nothing has changed.
 *
 * - If you really want the flash to last as long as possible, implement
 *   a wear levelling algorithm. There are MANY ways to do that, but here is a
 *   simple example:
 *
 *     - Use a fixed size structure for all your save data. Have this structure
 *       end by a fixed byte != 0xFF.
 *     - When saving, scan the flash sector backwards until the first != 0xFF
 *       byte is found. This will be where you will write your data.
 *     - Your startup code will have to scan the flash sector backwards to find the
 *       starting point, subtract the size of the fixed size structure and read
 *       the last saved data from there.
 *
 *     - The above can even be made to spread accross several sectors if you are
 *       really motivated.
 *
 * Other things that could be good to know:
 *
 * - Erased sectors will read 0xFF. 
 * - Unprogrammed byte (0xFF) can be programmed at any time.
 * - I'm not sure, but I think this even works at the bit level. (i.e. you can turn
 *   1s into 0s, but not the opposite without doing a sector erase)
 */

#include "SMSlib.h"
#include <string.h>
#include <stdint.h>
#include "flash.h"

typedef uint16_t (*u16funcptr)(void);
typedef uint8_t (*u8funcptr16_8)(uint16_t,uint8_t);
typedef void (*voidfuncptr)(void);
typedef void (*voidfuncptr8)(uint8_t);
typedef void (*voidfuncptr8_16_8)(uint8_t,uint16_t,uint8_t);

static const uint8_t ll_readSiliconID[] = {
	0xF3,           // di

	// Step 1: Write AA to address 555
	0x21,0x55,0x05, // ld  hl, #0x0555
	0x36,0xAA,      // ld  (hl), #0xAA

	// Step 2: Write 55 to address 2AA
	0x21,0xAA,0x02, // ld  hl, #0x02AA 
	0x36,0x55,      // ld  (hl), #0x55

	// Step 3: Write 90 to address 555
	0x21,0x55,0x05, // ld  hl, #0x0555
	0x36,0x90,      // ld  (hl), #0x90

	// Step 4: Read with A1=0,A0=0 for manufacture code.
	0x2A,0x00,0x00, // ld  hl, (#0000)
	
	// hl contains the function return value. preserve it.
	0xE5,           // push    hl

	// Reset / Return to read mode? Write 0xF0 anywhere.
	0x21,0x00,0x00, // ld  hl, #0000
	0x36,0xF0,      // ld  (hl), #0xF0

	0xE1,           // pop hl (return value)

	0xFB,           // ei
	0xC9            // ret
};

	// Argument: uint8 bank number for slot 2
static const uint8_t ll_sector_erase[] = {
	0xF3,			// di

	// Save current bank in slot 2
	0x21,0xFF,0xFF, // ld hl, #0xFFFF
	0x4E,           // ld c, (hl)

	// Retreive the bank to use from function argument
	0xFD,0x21,0x02,0x00, // ld iy, #2
	0xFD,0x39,           // add iy, sp
	0xFD,0x46,0x00,      // ld b, 0(iy)
	0x70,                // ld (hl), b  ; switch bank

	// Step 1: Write AA to address 555
	0x21, 0x55, 0x05,// ld  hl, #0x0555
	0x36, 0xAA,		// ld  (hl), #0xAA
	// Step 2: Write 55 to address 2AA
	0x21, 0xAA, 0x02,// ld  hl, #0x02AA
	0x36, 0x55,		// ld  (hl), #0x55
	// Step 3: Write 80 to address 555
	0x21, 0x55, 0x05,// ld  hl, #0x0555
	0x36, 0x80,		// ld  (hl), #0x80
	// Step 4: Write AA to address 555
	0x21, 0x55, 0x05,// ld  hl, #0x0555
	0x36, 0xAA,		// ld  (hl), #0xAA
	// Step 5: Write 55 to address 2AA
	0x21, 0xAA, 0x02,// ld  hl, #0x02AA
	0x36, 0x55,		// ld  (hl), #0x55
	// Step 6: Write 30 to sector address
	0x21, 0x00, 0x80,// ld  hl, #0x8000 ; Slot 2
	0x36, 0x30,		// ld  (hl), #0x30

	// Now poll Q7 for completion. Q7 will read 0 until the operation completes
// poll:
	0x7E,			// ld  a, (hl)
	0xE6, 0x80,		// and a, #0x80
	0x28, 0xFB,		// jr  Z, poll

	// Done. Reset / Return to read mode (not sure if necessary)
	0x21, 0x00, 0x00,//ld  hl, #0000
	0x36, 0xF0,		//ld  (hl), #0xF0

	// Restore original bank in slot 2
	0x21,0xFF,0xFF,  // ld hl, #0xFFFF
	0x71,            // ld (hl), c

	0xFB,			// ei
	0xC9			// ret
};

	// function arguments
	//   uint8_t data byte
	//   uint16_t address (within slot 2)
	//   uint8_t bank for slot 2
static const uint8_t ll_program_byte[] = {
	0xF3,                // di

	// Save current bank in slot 2
	0x21,0xFF,0xFF,      // ld hl, #0xFFFF
	0x4E,                // ld c, (hl)

	// ; Retreive the bank to use from function argument
	0xFD,0x21,0x02,0x00, // ld iy, #2
	0xFD,0x39,           // add iy, sp
	0xFD,0x46,0x03,      // ld b, 3(iy)
	0x70,                // ld (hl), b	; switch bank

	// Step 1: Write AA to address 555
	0x21,0x55,0x05,      // ld  hl, #0x0555
	0x36,0xAA,           // ld  (hl), #0xAA
	// Step 2: Write 55 to address 2AA
	0x21,0xAA,0x02,      // ld  hl, #0x02AA
	0x36,0x55,           // ld  (hl), #0x55
	// Step 3: Write A0 to address 555
	0x21,0x55,0x05,      // ld  hl, #0x0555
	0x36,0xA0,           // ld  (hl), #0xA0
	// Step 4: Write the byte
	0xFD,0x7E,0x00,      // ld  a, (iy) ; Data byte
	0xFD,0x6E,0x01,      // ld  l, 1(iy) ; Get the address argument (lower bits)
	0xFD,0x66,0x02,      // ld  h, 2(iy) ; Get the address argument (upper bits)
	0x77,                // ld  (hl), a ; Perform the write

	// Now poll Q7 for completion. Q7 is the complement of what was written until completion.
	0xE6,0x80,           // and a, #0x80
	0x57,                // ld  d, a
// pollprog:
	0x7E,                // ld  a, (hl)
	0xE6,0x80,           // and a, #0x80
	0xBA,                // cp  a, d
	0x20,0xFA,           // jr  NZ, pollprog

	// Reset / Return to read mode by wiring F0 anywhere. (necessary?)
	0x21,0x00, 0x00,     // ld  hl, #0000
	0x36,0xF0,           // ld  (hl), #0xF0

	// Restore original bank in slot 2
	0x21,0xFF,0xFF,      // ld hl, #0xFFFF
	0x71,                // ld (hl), c

	0xFB,                // ei
	0xC9,                // ret
};

	// function arguments
	//   uint16_t address (within slot 2)
	//   uint8_t bank for slot 2
	//
	// return one uint8_t value
static const uint8_t ll_read_byte[] = {
	0xF3,
	// Save current bank in slot 2
	0x21,0xFF,0xFF,     // ld hl, #0xFFFF
	0x4E,               // ld c, (hl)
	// Retreive the bank to use from function argument
	0xFD,0x21,0x02,0x00,// ld iy, #2
	0xFD,0x39,          // add iy, sp
	0xFD,0x46,0x02,     // ld b, 2(iy)
	0x70,               // ld (hl), b	; bank switch

	0xFD,0x6E,0x00,     // ld l, 0(iy) ; Get the address argument (lower bits)
	0xFD,0x66,0x01,     // ld h, 1(iy) ; Get the address argument (upper bits)
	0x6E,               // ld l, (hl) ; Perform the read, store the value in L (return value)
	// Restore original bank in slot 2
	0xFD,0x21,0xFF,0xFF,// ld iy, #0xFFFF
	0xFD,0x71,0x00,     // ld (iy), c
	0xFB,               // ei
	0xC9                // ret
};

/** Erase a 64k flash sector 
 *
 * Sector 0: Banks 0-3    << Do not erase this, it's the game!
 * Sector 1: Banks 4-7
 * Sector 2: Banks 8-11
 * ...
 * This eventually wraps around depending on the flash size, so be careful!
 *
 * This function is aware that the flash is accessed through a 16k window
 * at address 0x8000 (slot 2) and automatically sets the correct bank
 * for the sector erase operation.
 *
 * The original bank in slot 2 is remaped at the end.
 *
 */
void flash_erase64kSector(uint8_t id)
{
	uint8_t fn[sizeof(ll_sector_erase)];
	voidfuncptr8 fn_inmem;

	/* Copy the low level function to memory, and setup the function pointer */
	memcpy(fn, ll_sector_erase, sizeof(ll_sector_erase));
	fn_inmem = (voidfuncptr8)fn;

	/* Do it! */
	fn_inmem(id << 2);
}


/** Read the Flash Silicon ID.
 *
 * The Silicon ID has two bytes: Manufacturer code and device code.
 *
 * For MX29F040, those are 0xC2 and 0xA4.
 *
 * If this code is not running on a flash chip which recognizes the
 * read sequences, the word stored in ROM at address 0 is returned instead.
 *
 */
uint16_t flash_readSiliconID(void)
{
	uint8_t fn[sizeof(ll_readSiliconID)];
	u16funcptr fn_inmem;

	/* Copy the low level function to memory, and setup the function pointer */
	memcpy(fn, ll_readSiliconID, sizeof(fn));
	fn_inmem = (voidfuncptr)fn;

	return fn_inmem();
}


/** Program a byte in the flash
 *
 * This function is aware that the flash is reprogrammed through a 16k window
 * at 0x8000 (slot 2) and switches bank automatically.
 *
 * The original bank in slot 2 is remaped at the end.
 */
void flash_programByte(uint8_t value, uint8_t sector, uint16_t offset)
{
	uint8_t usebank;
	uint8_t fn[sizeof(ll_program_byte)];
	voidfuncptr8_16_8 fn_inmem;
	
	memcpy(fn, ll_program_byte, sizeof(fn));
	fn_inmem = (voidfuncptr8_16_8)fn;

	/* Determine the bank based on the sector and 16k area to be accessed */
	usebank = sector << 2; // 16k sectors
	usebank += offset >> 14;

	/* Compute the address within slot 2 */
	offset &= 0x3FFF;
	offset |= 0x8000;

	/* Get the correct bank in slot 2 and perform the write! */
	fn_inmem(value, offset, usebank);
}

/** Check if the silicon ID for the installed flash chip is known.
 *
 * This can be used to implement hybrid software which can
 * fall back to "standard" SRAM use (emulators and other cartridges).
 *
 */
char flash_isKnownId(void)
{
	uint16_t id = flash_readSiliconID();

	if (id == FLASH_ID_MX29F040) {
		return 1;
	}

	return 0;
}

/** Program an array of byte to flash. */
void flash_programBytes(const uint8_t *buffer, int size, uint8_t sector, uint16_t offset)
{
	while(size--) {
		flash_programByte(*buffer, sector, offset);
		buffer++;
		offset++;
	}
}


/** Read a byte from the flash.
 *
 * This function takes care of bank switching and resets the bank in slot 2
 * to what it was before the call.
 *
 */
uint8_t flash_readByte(uint8_t sector, uint16_t offset)
{
	uint8_t bank;
	uint8_t fn[sizeof(ll_read_byte)];
	u8funcptr16_8 fn_inmem;

	/* Copy the low level function to memory, and setup the function pointer */
	memcpy(fn, ll_read_byte, sizeof(fn));
	fn_inmem = (u8funcptr16_8)fn;

	/* Determine the bank based on the sector and 16k area to be accessed */
	bank = sector << 2; // 16k sectors
	bank += offset >> 14;

	/* Compute the address within slot 2 */
	offset &= 0x3FFF;
	offset |= 0x8000;

	return fn_inmem(offset, bank);
}

/** Read an array of bytes from the flash.
 *
 * Quite inefficient, but should do the job for now.
 *
 */
void flash_readBytes(uint8_t *buffer, int size, uint8_t sector, uint16_t offset)
{
	while(size--) {
		*buffer = flash_readByte(sector, offset);
		buffer++;
		offset++;
	}
}


/** Compare the array in parameter with what the flash already contains.
 *
 * Returns 0 if equal.
 *
 */
char flash_compareBytes(const uint8_t *buffer, int size, uint8_t sector, uint16_t offset)
{
	uint8_t tmp;

	while (size--) {
		tmp = flash_readByte(sector, offset);
		if (tmp != *buffer) {
			return 1;
		}
		offset++;
		buffer++;
	}

	return 0;
}
