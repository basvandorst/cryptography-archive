//----------------------------------------------------------------------------
// Filename: e.h
// Application: Header file for e.cpp
// Codeword OTP V1.1 Vernam Cipher Utility
// R.James 02-16-1994
//----------------------------------------------------------------------------

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <dos.h>
#include <errno.h>
#include <dir.h>
#include <new.h>
#include <io.h>

const int MAXBUF=32765;                    // maximum file buffer allocation

void wscroll(int,int,int,int,int,int,int); // scroll new window
void border(int,int,int,int,int,int);      // draw border around new window
void initfs(void);                         // supports findnext() function
void allocError(void);                     // trap memory allocation error

struct ffblk ffblk;                        // findfirst()/findnext() structure

FILE *fp;                                  // target file file-pointer
char fspec[64];                            // target file file-specification
char tfspec[64];                           // temporary fspec used in initfs()
unsigned long Pr[256];                     // unnormalized probability space vector
unsigned long bcnt;                        // byte count returned by fread()
unsigned int fsize;                        // target file file-size       
int handle;                                // DOS file handle used to get file size
unsigned long sum;                         // sum of elements in Pr vector
int savex,savey;                           // save screen coordinates
double S;                                  // floating point version of sum
double H;                                  // entropy of file content
double Hmax;                               // max entropy defined as log256
double Hrel;                               // relative entropy = H/Hmax
double k;                                  // reciprocal of sum S
double P[256];                             // normalized finite probablity space

// Window class
class Window {
private:
	int bg;                 // background color
	int fg;                 // foreground color
	int sx;                 // start of x
	int sy;                 // start of y
	int ex;                 // end of x
	int ey;                 // end of y
	int msize;              // screen buffer size
public:
	Window(int,int,int,int,int,int);
	void help(void);       // help screen
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