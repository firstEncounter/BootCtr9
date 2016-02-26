#include <stdio.h>
#include <stdlib.h>
#include "fatfs/sdmmc/sdmmc.h"
#include "fatfs/ff.h"
#include "config.h"
#include "hid.h"
#include "log.h"
#include "i2c.h"


#define DEFAULT_PATH {0}
#define DEFAULT_DELAY 2000 /* ms */
#define DEFAULT_PAYLOAD -1 /* <0 - auto, 0 - disable, >0 - enabled */
#define DEFAULT_OFFSET 0x12000
#define DEFAULT_SECTION "GLOBAL"
#define INI_FILE "/arm9loaderhax/boot_config.ini"
#define INI_FILE_BOOTCTR "/boot_config.ini"

#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_SIZE		0x00100000


bool file_exists(const char* path) { 
    FIL fd;
    if(f_open(&fd, path, FA_READ | FA_OPEN_EXISTING) == FR_OK) { 
       f_close(&fd); 
       return true; 
    } 
 	return false; 
} 

int iniparse(const char* filename, ini_handler handler, void* user)
{
    FIL file;
    int error;
    if(file_exists(INI_FILE))
    {
    	if (f_open(&file, INI_FILE, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        	return -1; 
    }
    else if(file_exists(INI_FILE_BOOTCTR))
    {
    	if (f_open(&file, INI_FILE_BOOTCTR, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        	return -1;
    }
    f_lseek(&file,0);
    error = ini_parse_stream((ini_reader)f_gets, &file, handler, user);
    f_close(&file);
    return error;
}

int main() {
	unsigned int br;
	configuration app =  {
	        .section = DEFAULT_SECTION,
	        .path = DEFAULT_PATH,
	        .delay = DEFAULT_DELAY,
	        .payload = DEFAULT_PAYLOAD,
	        .offset = DEFAULT_OFFSET,
    };
	FATFS fs;
	FIL payload;

	if(f_mount(&fs, "0:", 1) == FR_OK)
	{
		initLog();

    	debug("Reading GLOBAL section");
    	iniparse(INI_FILE, handler, &app);
    	
    	debug("Checking input");
		u32 key = GetInput();
        // using X-macros to generate each switch-case rules
        // https://en.wikibooks.org/wiki/C_Programming/Preprocessor#X-Macros
        #define KEY(k) \
        if(key & KEY_##k) \
            app.section = "KEY_"#k; \
        else
        #include "keys.def"
            app.section = "DEFAULT";

	    debug("Key checked - selected section:");
	    debug(app.section);

	    int config_err = iniparse(INI_FILE, handler, &app);

	    debug("Reading selected section");
	    switch (config_err) {
	        case 0:
	            // section not found, try to load [DEFAULT] section
	            if (strlen(app.path)==0) {
	            	debug("Section not found, trying to load the default section");
	                app.section = "DEFAULT";
	                // don't need to check error again
	                iniparse(INI_FILE, handler, &app);
	                if (!app.path)
	                    panic("Section [DEFAULT] not found or \"path\" not set.");
	            } else if (!file_exists(app.path)) {
	                debug("[ERROR] Target payload not found:");
	                panic(app.path);
	            }
	            break;
	        case -1:
	            panic("Config file not found.");//, INI_FILE);
	            break;
	        case -2:
	            // should not happen, however better be safe than sorry
	            panic("Config file is too big.");//, INI_FILE);
	            break;
	        default:
	            panic("Error found in config file");//,
	                    //INI_FILE, config_err);
	            break;
	    }
		
		debug("Checking payload");
		if(app.payload==0)
		{	
			panic("Trying to load a 3dsx - this is not supported by this version");
		}
		if(!file_exists(app.path))
		{
            debug("[ERROR] Target payload not found:");
            panic(app.path);
		}

		debug("Loading Payload:");
		debug(app.path);
		if(f_open(&payload, app.path, FA_READ | FA_OPEN_EXISTING) == FR_OK)
		{
			if(app.offset>0)
			{ 	
				char buffer [33];
				itoa(app.offset,buffer,16);
				debug("Jump to offset:");
				debug(buffer);
				f_lseek (&payload, app.offset);
			}
			debug("Reading payload");
			f_read(&payload, (void*)PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
			debug("Finished reading the payload");

			debug("closing files and unmount sd");
			f_close(&payload);
			closeLogFile();
			f_mount(&fs, "0:", 1);
			debug("Jumping to the payload");
			((void (*)())PAYLOAD_ADDRESS)();
		}
	}
	debug("Failed to mount the sd-card or jump to the payload");
	shutdown();
    
	return 0;
}