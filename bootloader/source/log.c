#include "log.h"

#include "fatfs/ff.h"
#include "i2c.h"

static FIL logFile;
bool logFileOpened=false;

int initLog()
{
	const char logname[]="/arm9bootloader.log";
	
	if (f_open(&logFile, logname, FA_READ | FA_WRITE | FA_CREATE_ALWAYS ) != FR_OK)
    	shutdown();
    logFileOpened=true;
    f_sync (&logFile);            
	f_puts ("opened logfile\n", &logFile);
    f_sync (&logFile);  
    
    return 0;
}

void closeLogFile()
{
	f_close(&logFile);
	logFileOpened=false;
}

void debug(const char *msg)
{
	if(logFileOpened)
	{
	    f_puts (msg, &logFile);
	    f_putc ('\n', &logFile);
	    f_sync (&logFile);
	}
}

void panic(const char *msg)
{
	if(logFileOpened)
	{
		debug("ERROR:");
    	debug(msg);
    }

    shutdown();
}

void shutdown() {
	debug("shutdown");
	closeLogFile();
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
	while(1);
}