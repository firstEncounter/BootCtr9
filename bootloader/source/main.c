#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sdmmc.h"
#include "i2c.h"
#include "fatfs/ff.h"
#include "config.h"

#include "hid.h"
#include "draw.h"

#define DEFAULT_PATH NULL
#define DEFAULT_DELAY 2000 /* ms */
#define DEFAULT_PAYLOAD -1 /* <0 - auto, 0 - disable, >0 - enabled */
#define DEFAULT_OFFSET 0x12000
#define DEFAULT_SECTION "GLOBAL"
#define INI_FILE "boot_config.ini"

#define NUM_SERVICESTHATMATTER 5

#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_SIZE		0x00100000


typedef struct
{
    bool scanned;
    u32 sectionSizes[3];
    u8 servicesThatMatter[NUM_SERVICESTHATMATTER];
} executableMetadata_s;

typedef struct {
    configuration config;
    executableMetadata_s em;
} application;

void ClearTop() {
	ClearScreen(TOP_SCREEN0, RGB(255, 255, 255));
	ClearScreen(TOP_SCREEN1, RGB(255, 255, 255));
	current_y = 0;
}

void shutdown() {
	//shutdown if it failed
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
}

void wait_key() {
	/*Debug("");
	Debug("Press key to continue");*/
	InputWait();
}

bool file_exists(const char* path) { 
    FIL fd;
    if(f_open(&fd, path, FA_READ | FA_OPEN_EXISTING) == FR_OK) { 
       f_close(&fd); 
       return true; 
    } 
 	return false; 
} 

char buff[0x1000];

int iniparse(const char* filename, ini_handler handler, void* user)
{
    FIL file;
    int error;
    if (f_open(&file, filename, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return -1; 
    error = ini_parse_stream((ini_reader)f_gets, &file, handler, user);
    f_close(&file);
    return error;
}

int main() {
	FATFS fs;
	if(f_mount(&fs, "0:", 0) == FR_OK)
	{
		
		application app = {
	        .config = {
	            .section = DEFAULT_SECTION,
	            .path = DEFAULT_PATH,
	            .delay = DEFAULT_DELAY,
	            .payload = DEFAULT_PAYLOAD,
	            .offset = DEFAULT_OFFSET,
	        }
    	};
    	iniparse(INI_FILE, handler, &app.config);

    	u32 key = HID_STATE;
    	switch (key) {
	        // using X-macros to generate each switch-case rules
	        // https://en.wikibooks.org/wiki/C_Programming/Preprocessor#X-Macros
	        #define KEY(k) \
	        case KEY_##k: \
	            app.config.section = "KEY_"#k; \
	            break;
	        #include "keys.def"
	        default:
	            app.config.section = "DEFAULT";
	            break;
	    }


	    int config_err = iniparse(INI_FILE, handler, &app.config);
	    switch (config_err) {
	        case 0:
	            // section not found, try to load [DEFAULT] section
	            if (!app.config.path) {
	                app.config.section = "DEFAULT";
	                // don't need to check error again
	                iniparse(INI_FILE, handler, &app.config);
	                if (!app.config.path)
	                    shutdown();//panic("Section [DEFAULT] not found or \"path\" not set.");
	            } else if (!file_exists(app.config.path)) {
	                shutdown();//panic("In section [%s], file %s not found.",
	                        //app.config.section, app.config.path);
	            }
	            break;
	        case -1:
	            shutdown();//panic("Config file %s not found.", INI_FILE);
	            break;
	        case -2:
	            // should not happen, however better be safe than sorry
	            shutdown();//panic("Config file %s too big.", INI_FILE);
	            break;
	        default:
	            shutdown();//panic("Error found in config file %s on line %d.",
	                    //INI_FILE, config_err);
	            break;
	    }

		FIL payload;
		u32 br;
		
		//wait_key();
		if(app.config.payload!=0)
		{	
			if(f_open(&payload, app.config.path, FA_READ | FA_OPEN_EXISTING) == FR_OK)
			{
				if(app.config.offset>0)
					f_lseek (&payload, app.config.offset);
				f_read(&payload, PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
				((void (*)())PAYLOAD_ADDRESS)();
			}
		}
	}

	shutdown();
    
	return 0;
}