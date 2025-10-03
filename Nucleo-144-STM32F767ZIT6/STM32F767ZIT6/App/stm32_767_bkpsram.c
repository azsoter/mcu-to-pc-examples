/*
* stm32_767_bkpsram.c
*
*  Created on: December 09, 2024
*      Author: Andras Zsoter
*
* Copyright (c) 2024 Andras Zsoter
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include <forth_config.h>

// Since we don't have any "real" backup media, and flash sectors on these microcontrollers are way too large to use for something small
// such as as Forth blocks we just use the 4K backup SRAM as non-volatile storage to store a couple of Forth blocks (4) on the device.
// These should be enough to e.g. have some customization / config stored on the device. It is also supposed to live through reboots
// and should not loose it contents even if the device is powered down, if there is an actual battery powering the backup SRAM.
//
// This code is somewhat experimental (more like quick and dirty -- just to test a concept).
// I don't actually have a battery connected and I am not sure if the board I am experimenting on is even wired
// properly to test if the backup SRAM would retain its contents over a power cycle, so I am just following documentation and examples.
// Even if it may misbehave it should not be too far off from the real deal and it should be fixable.
//
#if defined(FORTH_INCLUDE_BLOCKS)
#include <main.h>
#include <stm32f7xx_ll_pwr.h>
#include <string.h>
#include <forth.h>

#include <printing.h>

// Forth blocks are supposed to be 1024 bytes long.
// Just hard code it for this quick and dirty implementation.

forth_scell_t forth_READ_BLOCK(forth_cell_t block_number, uint8_t *buffer)
{
	if ((0 == block_number) || (4 < block_number))
	{
		return -33; // Block read exception.
	}

	memcpy(buffer, (void *)(BKPSRAM_BASE + (1024 * (block_number - 1))) ,1024);
	return 0;
}

forth_scell_t forth_WRITE_BLOCK(forth_cell_t block_number, uint8_t *buffer)
{
	if ((0 == block_number) || (4 < block_number))
	{
		return -34; // Block write exception.
	}

	HAL_PWR_EnableBkUpAccess();
	memcpy((void *)(BKPSRAM_BASE + (1024 * (block_number - 1))),buffer, 1024);
	HAL_PWR_DisableBkUpAccess();
	return 0;
}

void bkpsram_init(void)
{
	uint32_t backed_up;

	// PRINTF("%s() RCC_APB1ENR_PWREN = 0x%08x\r\n", __FUNCTION__, READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN));
	 __HAL_RCC_PWR_CLK_ENABLE();
	 HAL_PWR_EnableBkUpAccess();
	 // PRINTF("%s() RCC_AHB1ENR_BKPSRAMEN = 0x%08x\r\n", __FUNCTION__, READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_BKPSRAMEN));
	 __HAL_RCC_BKPSRAM_CLK_ENABLE();
	 // PRINTF("%s() RCC_AHB1ENR_BKPSRAMEN = 0x%08x\r\n", __FUNCTION__, READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_BKPSRAMEN));
	 // PRINTF("%s() LL_PWR_IsActiveFlag_BRR() = 0x%08x\r\n", __FUNCTION__, LL_PWR_IsActiveFlag_BRR());

	 backed_up = LL_PWR_IsActiveFlag_BRR(); // Remember if the backup regulator was already on during initialization.
	 LL_PWR_EnableBkUpRegulator();
	 while (!LL_PWR_IsActiveFlag_BRR()); 	// Wait for the backup regulator to come up.


	 if (!backed_up) // There was no backup, so wipe all blocks clean (fill them with spaces).
	 {
		 PUTS("\r\n** Backup SRAM was turned off, wiping blocks clean. **\r\n");
		 memset((void *)BKPSRAM_BASE, 32, 4096);
	 }

	 HAL_PWR_DisableBkUpAccess();
}

int forth_init_blocks(void)
{
	bkpsram_init();
	return 0;
}
#endif // FORTH_INCLUDE_BLOCKS
