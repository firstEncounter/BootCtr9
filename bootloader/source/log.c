#include "log.h"

#include "fatfs/ff.h"
#include "i2c.h"

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
	//debug("shutdown");
	//closeLogFile();
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
	//exit(0);
	while(1);
}