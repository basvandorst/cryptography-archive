//----------------------------------------------------------------------------
// Filename: k.h
// Application: Header file for k.cpp (sscv03 keyset generator)
// Codeword V3.0 Stream Cipher
// R.James 02-16-1994
//----------------------------------------------------------------------------

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <dos.h>
#include <new.h>
#include <ctype.h>

int const KEYCOUNT=2;                      // generate 2-key keysets
int const KEYSIZE=11;                      // with 11 8-bit chars per key
int const CONTINUE=0;
int const ABORT=1;
int const TRUE = 1;
int const FALSE = 0;

char nextch(void);                         // get next key character
int  filechk(char *);                      // determine if target file exists
int  getans(void);                         // get user response to (y/n)? prompt
void wscroll(int,int,int,int,int,int,int); // scroll to create new window
void border(int,int,int,int,int,int);      // draw border around scrolled window
void allocError(void);                     // trap memory allocation error

FILE *kp;                                  // target file file-pointer  
time_t t;                                  // system time access
char dtbuf[64];                            // date-time stamp buffer
int savex,savey;                           // save cursor coordinates
unsigned int seed;                         // linear generator seed value
char fspec[64];                            // target file specification
char keybuf[KEYSIZE+1];                    // ASCII key character buffer
int V[87];                                 // shuffle vector

// Window class
class Window {
	int bg;              // background color
	int fg;              // foreground color
	int sx;              // start of x
	int sy;              // start of y
	int ex;              // end of x
	int ey;              // end of y
	char *p;             // screen buffer ptr
	int msize;           // screen buffer size
public:
	Window(int,int,int,int,int,int);
	int confirm();       // require user to confirm file overwrite
	void help(void);     // display summary screen
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