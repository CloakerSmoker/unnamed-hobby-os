
#include "doomgeneric.h"
#include <stdio.h>
#include <sys/time.h>
#include <linux/fb.h>

int FrameBuffer;
int FB_Width;
int FB_Height;
int FB_Depth;
int FB_Stride;

void DG_Init() {
    FrameBuffer = open("/dev/fb0", "rw");

    struct fb_var_screeninfo VariableInfo = {0};
    ioctl(FrameBuffer, FBIOGET_VSCREENINFO, &VariableInfo);

    FB_Width = VariableInfo.xres;
    FB_Height = VariableInfo.yres;
    FB_Depth = VariableInfo.bits_per_pixel / 8;

    FB_Stride = FB_Width * FB_Depth;
}

#define DG_Width DOOMGENERIC_RESX
#define DG_Height DOOMGENERIC_RESY
#define DG_Depth sizeof(pixel_t)
#define DG_Stride (DG_Width * DG_Depth)

void DG_DrawFrame() {
    lseek(FrameBuffer, 0, SEEK_SET);

    for (int Row = 0; Row < DOOMGENERIC_RESY; Row++) {
        int FB_Offset = Row * FB_Stride;
        int DG_Offset = Row * DG_Stride;

        lseek(FrameBuffer, FB_Offset, SEEK_SET);
        write(FrameBuffer, DG_ScreenBuffer + DG_Offset, DG_Stride);
    }
}

void DG_SleepMs(uint32_t ms) {
    usleep(ms * 1000);
}

uint32_t DG_GetTicksMs() {
    struct timeval  tp;
    struct timezone tzp;

    gettimeofday(&tp, &tzp);

    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

int DG_GetKey(int* pressed, unsigned char* doomKey) {
	return 0;
}

void DG_SetWindowTitle(const char * title) {

}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    while (1) {
        doomgeneric_Tick();
    }
    
    return 0;
}