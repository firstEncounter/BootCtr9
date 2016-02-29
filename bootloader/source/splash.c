#include "splash.h"
#include "draw.h"
#include "log.h"
#include "fatfs/sdmmc/sdmmc.h"
#include "fatfs/ff.h"

int splash_ascii(void)
{
    // print BootCtr logo
    // http://patorjk.com/software/taag/#p=display&f=Bigfig&t=BootCtr
    ClearScreen(TOP_SCREEN0,0);
    DrawStringF(5,5,ASCII_ART_TEMPLATE, VERSION_STRING);
    return 0;
}

int splash_image(char *splash_path)
{
    // load image in memory, doing proper error checking
    FIL splash_file;
    unsigned int br;
    if(strlen(splash_path)==0)
    {
        debug("Splash image not set, use default screen");
        // force ASCII splash art in case of error
        splash_ascii();
        return -1;
    }
    if(f_open(&splash_file, splash_path, FA_READ | FA_OPEN_EXISTING) != FR_OK)
    {
        debug("Couldn't open splash image %s.", splash_path);
        // force ASCII splash art in case of error
        splash_ascii();
        return -1;
    }

    // copy splash image to framebuffer
    f_read(&splash_file, (void*)(*(u32*)0x23FFFE00), 0x00600000, &br);

    return 0;    
}

