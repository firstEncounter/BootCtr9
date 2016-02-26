#include "common.h"
#include "fatfs/sdmmc/sdmmc.h"
#include "i2c.h"
#include "fatfs/ff.h"

#define BOOTLOADER_PAYLOAD_ADDRESS	0x24F00000
#define BOOTLOADER_PAYLOAD_SIZE		0x00100000


int main()
{
	FATFS fs;
	FIL payload;
	u32 br;
	
	if(f_mount(&fs, "0:", 1) == FR_OK)
	{
		if(f_open(&payload, "arm9bootloader.bin", FA_READ | FA_OPEN_EXISTING) == FR_OK)
		{
			f_read(&payload, BOOTLOADER_PAYLOAD_ADDRESS, BOOTLOADER_PAYLOAD_SIZE, &br);
			f_close(&payload);
			f_mount(&fs, "0:", 1);
			((void (*)())BOOTLOADER_PAYLOAD_ADDRESS)();
		}
	}
	
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
    return 0;
}
