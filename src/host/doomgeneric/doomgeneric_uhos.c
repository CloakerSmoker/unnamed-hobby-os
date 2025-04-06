
#include "doomgeneric.h"
#include <stdio.h>
#include <sys/time.h>
//#include <linux/fb.h>

#define FBIOGET_VSCREENINFO	0x4600
#define FBIOPUT_VSCREENINFO	0x4601
#define FBIOGET_FSCREENINFO	0x4602

typedef unsigned int __u32;
typedef unsigned short __u16;
typedef unsigned char __u8;

struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/ 
	__u16 xpanstep;			/* zero if no hardware panning  */
	__u16 ypanstep;			/* zero if no hardware panning  */
	__u16 ywrapstep;		/* zero if no hardware ywrap    */
	__u32 line_length;		/* length of a line in bytes    */
	unsigned long mmio_start;	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O  */
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 capabilities;		/* see FB_CAP_*			*/
	__u16 reserved[2];		/* Reserved for future compatibility */
};

struct fb_bitfield {
	__u32 offset;			/* beginning of bitfield	*/
	__u32 length;			/* length of bitfield		*/
	__u32 msb_right;		/* != 0 : Most significant bit is */ 
					/* right */ 
};

struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what			*/
	__u32 grayscale;		/* 0 = color, 1 = grayscale,	*/
					/* >1 = FOURCC			*/
	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/	

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 colorspace;		/* colorspace for FOURCC-based modes */
	__u32 reserved[4];		/* Reserved for future compatibility */
};

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