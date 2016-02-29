#include "log.h"
#include "draw.h"

#include "fatfs/ff.h"
#include "i2c.h"
#include "hid.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#define LOGNAME "/arm9loaderhax/arm9bootloader.log"
#define LOGNAME_BACKUP "/arm9bootloader.log"

static FIL logFile;
bool logFileOpened=false;

int initLog()
{
	if (f_open(&logFile, LOGNAME, FA_READ | FA_WRITE | FA_CREATE_ALWAYS ) != FR_OK)
    {
    	if (f_open(&logFile, LOGNAME_BACKUP, FA_READ | FA_WRITE | FA_CREATE_ALWAYS ) != FR_OK)
    		shutdown();
    }
    logFileOpened=true;
    f_sync (&logFile);            
	f_puts ("opened logfile\n", &logFile);
    f_sync (&logFile);  
    
    return 0;
}

void closeLogFile()
{	
	if(logFileOpened==true)
	{
		f_close(&logFile);
		logFileOpened=false;
	}
}

void debug(const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

	if(logFileOpened)
	{
	    f_puts (str, &logFile);
	    f_putc ('\n', &logFile);
	    f_sync (&logFile);
	}
	drawDebug(str);
}

void panic(const char *format, ...)
{
	char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

	if(logFileOpened)
	{
		debug("ERROR:");
    	debug(str);
    }
	drawDebug(str);
	drawDebug("Press any key to shutdown");
	InputWait();
    shutdown();
}

void shutdown() {
	debug("shutdown");
	closeLogFile();
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
	while(1);
}