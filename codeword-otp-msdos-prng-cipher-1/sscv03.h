//----------------------------------------------------------------------------
//    Filename: sscv03.h
//    Application:  Symmetric stream cipher version 03 header file
//    Codeword Version 3.0 Stream Cipher
//    R.James 02-16-1994
//----------------------------------------------------------------------------

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <ctype.h>
#include <dir.h>
#include <new.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <bios.h>

const int BUFSIZE=32767;                   // maximum fille buffer allocation
const int ROTSIZE=256;                     // length of path matrix
const int KEYLENGTH=12;                    // V3.0 key length for ASSIIZ strings
const int KEYSIZE=11;                      // V3.0 key length
const int SELECT=4;                        // initial translation key index
const int N=4;                             // length of constant vectors

unsigned char shuffle(void);               // Bays-Durham shuffle algorithm
void initfs(char *);                       // supports findnext() function
void generator(int);                       // OTP random number generator
void translate(long int,unsigned char *);  // V3.0 stream cipher implementation
void wscroll(int,int,int,int,int,int,int); // popup window and help screen utility
void border(int,int,int,int,int,int);      // popup window and help screen utility
void help(void);                           // display usage summary
void allocError(void);                     // trap memory allocation error

struct ffblk ffblk;                        // findfirst()/findnext() structure
static int kndx;                           // 11-character key index
union tag {                                // address path matrix elements
	unsigned char lbuf[256*KEYSIZE-4];
	unsigned char path[256][KEYSIZE-4];
} B;

union rng {                                // seed nonlinear generator
	unsigned char TCkey[5];
	unsigned long TBkey;
} S;

FILE *fp;                                  // target file file-pointer
int  handle;                               // DOS handle used to get file size
long int fsize;                            // target file file-size
char tfspec[64];                           // temporary file spec used in initfs()
int skip;                                  // advance into linear generator sequence
int savex,savey;                           // save screen coordinates
unsigned int seed;                         // linear generator seed
unsigned long Seed[2];                     // nonlinear generator seeds
unsigned char key[KEYLENGTH];              // V3.0 translation key
unsigned char V[ROTSIZE];                  // Bays-Durham shuffle vector

// left constant vector used in the NLRNG
static unsigned long lcvect[]={
	0xC3D8067BL, 0x5B54ED61L, 0x672A9746L, 0xD147396CL};

// right constant vector used in the NLRNG
static unsigned long rcvect[]={
	0x706DF456L, 0x24CF78E2L, 0xEC349A81L, 0xF06214F5L};

// Window class
class Window {
	int bg;        // background color
	int fg;        // foreground color
	int sx;        // start of x
	int sy;        // start of y
	int ex;        // end of x
	int ey;        // end of y
	int msize;     // screen buffer size;
public:
	Window (int,int,int,int,int,int);
	void help();
};
// constructor function
Window::Window(int bgc,int fgc,int startx,int starty,int endx,int endy)
{
	bg=bgc;
	fg=fgc;
	sx=startx;
	sy=starty;
	ex=endx;
	ey=endy;
}
