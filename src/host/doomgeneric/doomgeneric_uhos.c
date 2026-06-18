
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

#include <sys/mman.h>

void* FB;

FILE* InputStream;

void DG_Init() {
    FrameBuffer = open("/dev/fb0", "rw");

	FB = mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_SHARED, FrameBuffer, 0);

    struct fb_var_screeninfo VariableInfo = {0};
    //ioctl(FrameBuffer, FBIOGET_VSCREENINFO, &VariableInfo);

    //FB_Width = VariableInfo.xres;
    //FB_Height = VariableInfo.yres;
    //FB_Depth = VariableInfo.bits_per_pixel / 8;

	FB_Width = 1024;
	FB_Height = 768;
	FB_Depth = 4;

    FB_Stride = FB_Width * FB_Depth;

	InputStream = fopen("/dev/hid/keyboard1", "r");
}

#include "i_video.h"

#define DG_Width DOOMGENERIC_RESX
#define DG_Height SCREENHEIGHT
#define DG_Depth sizeof(pixel_t)
#define DG_Stride (DG_Width * DG_Depth)

extern int fb_scaling;

#include <poll.h>
#include <unistd.h>

void DG_DrawFrame() {
    //lseek(FrameBuffer, 0, SEEK_SET);

    for (int Row = 0; Row < DOOMGENERIC_RESY; Row++) {
        int FB_Offset = Row * FB_Stride;
        int DG_Offset = (Row * DG_Stride) / 4;

		memcpy(FB + FB_Offset, DG_ScreenBuffer + DG_Offset, DG_Stride);

        //lseek(FrameBuffer, FB_Offset, SEEK_SET);
        //write(FrameBuffer, DG_ScreenBuffer + DG_Offset, DG_Stride);
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

#include "doomkeys.h"

typedef struct {
	union {
		struct {
			int KeyCode;
			char Modifiers;
			char ASCII;
		};

		int64_t Padding;
	};
} KernelKeyInput;

#define KERNEL_KEY_MODIFIER_RELEASED 16

#define KERNEL_KEY_ASCII_BASE 0x20
#define KERNEL_KEY_ASCII_LAST 0x7e

#define KERNEL_KEY_SPECIAL_BASE 1

#define KERNEL_KEY_UP KERNEL_KEY_SPECIAL_BASE + 22
#define KERNEL_KEY_DOWN KERNEL_KEY_SPECIAL_BASE + 23
#define KERNEL_KEY_LEFT KERNEL_KEY_SPECIAL_BASE + 24
#define KERNEL_KEY_RIGHT KERNEL_KEY_SPECIAL_BASE + 25
#define KERNEL_KEY_ESCAPE KERNEL_KEY_SPECIAL_BASE + 26
#define KERNEL_KEY_ENTER KERNEL_KEY_SPECIAL_BASE + 27
#define KERNEL_KEY_SHIFT KERNEL_KEY_SPECIAL_BASE + 28
#define KERNEL_KEY_CONTROL KERNEL_KEY_SPECIAL_BASE + 29
#define KERNEL_KEY_ALT KERNEL_KEY_SPECIAL_BASE + 30

typedef struct {
	int KernelKeyCode;
	int DoomKeyCode;
} KernelKeyMapping;

KernelKeyMapping KernelKeyMappings[] = {
	{ KERNEL_KEY_UP, KEY_UPARROW },
	{ KERNEL_KEY_DOWN, KEY_DOWNARROW },
	{ KERNEL_KEY_LEFT, KEY_RIGHTARROW },
	{ KERNEL_KEY_RIGHT, KEY_LEFTARROW },
	{ KERNEL_KEY_ESCAPE, KEY_ESCAPE },
	{ KERNEL_KEY_ENTER, KEY_ENTER },
	{ KERNEL_KEY_ALT, KEY_LALT },
	{ 'e', KEY_USE },
	{ ' ', KEY_FIRE },
	{ '\t', KEY_TAB},
	{ 0, 0 } // Sentinel
};

int DG_GetKey(int* pressed, unsigned char* doomKey) {
	
	struct pollfd PollFD = {0};
	PollFD.fd = fileno(InputStream);
	PollFD.events = POLLIN;

	poll(&PollFD, 1, 0);

	if (PollFD.revents & POLLIN) {
		KernelKeyInput Input = {0};
		int BytesRead = read(fileno(InputStream), &Input, sizeof(Input));

		//printf("Read %d bytes from input stream\n", BytesRead);

		for (int i = 0; KernelKeyMappings[i].KernelKeyCode != 0; i++) {
			if (KernelKeyMappings[i].KernelKeyCode == Input.KeyCode) {
				//printf("Kernel key code %d maps to Doom key code %d\n", Input.KeyCode, KernelKeyMappings[i].DoomKeyCode);
				//printf("Modifiers: %d\n", Input.Modifiers);
				//printf("Pressed? %s\n", (Input.Modifiers & KERNEL_KEY_MODIFIER_RELEASED) ? "No" : "Yes");

				*doomKey = KernelKeyMappings[i].DoomKeyCode;
				*pressed = Input.Modifiers & KERNEL_KEY_MODIFIER_RELEASED ? 0 : 1;
				return 1; // Special key
			}
		}

		if ('a' <= Input.KeyCode && Input.KeyCode <= 'z') {
			*doomKey = Input.KeyCode;
			*pressed = Input.Modifiers & KERNEL_KEY_MODIFIER_RELEASED ? 0 : 1;
			return 1; // ASCII key
		}
	}

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