#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sdmmc.h"
#include "i2c.h"
#include "fatfs/ff.h"

#include "hid.h"
#include "draw.h"


#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_SIZE		0x00100000

void ClearTop() {
	ClearScreen(TOP_SCREEN0, RGB(255, 255, 255));
	ClearScreen(TOP_SCREEN1, RGB(255, 255, 255));
	current_y = 0;
}

void wait_key() {
	/*Debug("");
	Debug("Press key to continue");*/
	InputWait();
}

int main() {

	FATFS fs;
	FIL payload;
	u32 br;
	
	//wait_key();
	if(f_mount(&fs, "0:", 0) == FR_OK)
	{
		if(f_open(&payload, "arm9loaderhax.bin", FA_READ | FA_OPEN_EXISTING) == FR_OK)
		{
			f_read(&payload, PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
			((void (*)())PAYLOAD_ADDRESS)();
		}
	}
	//shutdown if it failed
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
    
	return 0;
}