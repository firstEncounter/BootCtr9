#include <stdio.h>
#include "sdmmc.h"
#include "fatfs/ff.h"
#include "config.h"
#include "hid.h"
#include "log.h"


#define DEFAULT_PATH {0}
#define DEFAULT_DELAY 2000 /* ms */
#define DEFAULT_PAYLOAD -1 /* <0 - auto, 0 - disable, >0 - enabled */
#define DEFAULT_OFFSET 0x12000
#define DEFAULT_SECTION "GLOBAL"
#define INI_FILE "/boot_config.ini"


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
    if (f_open(&file, "boot_config.ini", FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return -1; 
    f_lseek(&file,0);
    error = ini_parse_stream((ini_reader)f_gets, &file, handler, user);
    f_close(&file);
    return error;
}

int main() {
	FATFS fs;
	if(f_mount(&fs, "0:", 0) == FR_OK)
	{

		FIL payload;
		unsigned int br;
		configuration app =  {
	            .section = DEFAULT_SECTION,
	            .path = DEFAULT_PATH,
	            .delay = DEFAULT_DELAY,
	            .payload = DEFAULT_PAYLOAD,
	            .offset = DEFAULT_OFFSET,
    	};

		initLog();
    	iniparse(INI_FILE, handler, &app);
    	debug("Checking key");

    	u32 key = GetInput();
    	switch (key) {
	        // using X-macros to generate each switch-case rules
	        // https://en.wikibooks.org/wiki/C_Programming/Preprocessor#X-Macros
	        #define KEY(k) \
	        case KEY_##k: \
	            app.section = "KEY_"#k; \
	            break;
	        #include "keys.def"
	        default:
	            app.section = "DEFAULT";
	            break;
	    }

	    debug("Key checked- selected section:");
	    debug(app.section);

	    int config_err = iniparse(INI_FILE, handler, &app);

	    debug("Load selected section");
	    switch (config_err) {
	        case 0:
	            // section not found, try to load [DEFAULT] section
	            if (!app.path) {
	            	debug("section not found, trying to load the default section");
	                app.section = "DEFAULT";
	                // don't need to check error again
	                iniparse(INI_FILE, handler, &app);
	                if (!app.path)
	                    panic("Section [DEFAULT] not found or \"path\" not set.");
	            } else if (!file_exists(app.path)) {
	                panic("In section [%s], file %s not found.");//,
	                        //app.section, app.path);
	            }
	            break;
	        case -1:
	            panic("Config file %s not found.");//, INI_FILE);
	            break;
	        case -2:
	            // should not happen, however better be safe than sorry
	            panic("Config file %s too big.");//, INI_FILE);
	            break;
	        default:
	            panic("Error found in config file %s on line %d.");//,
	                    //INI_FILE, config_err);
	            break;
	    }
		
		debug("Checking payload");
		
		if(app.payload==0)
		{	
			panic("Trying to load a 3dsx");
		}

		debug("Loading Payload:");
		debug(app.path);
		if(f_open(&payload, app.path, FA_READ | FA_OPEN_EXISTING) == FR_OK)
		{
			if(app.offset>0)
				f_lseek (&payload, app.offset);

			f_read(&payload, (void*)PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
			((void (*)())PAYLOAD_ADDRESS)();
		}
	}

	shutdown();
    
	return 0;
}