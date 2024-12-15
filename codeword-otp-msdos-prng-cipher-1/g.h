//----------------------------------------------------------------------------
//  Filename: g.h
//  Application: Header file for g.cpp (sscv04 key generation utility)
//  Codeword OTP V1.0 Vernam Cipher Utility
//  R.James 02-16-1994
//----------------------------------------------------------------------------

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <dos.h>
#include <new.h>
#include <dir.h>
#include <io.h>
#include "rng.h"

int const MAXSEED=65535;                    // max linear generator seed value
int const MAXKEYS=55;                       // # of keypairs produced per run
int const TRUE = 1;                         // success
int const FALSE = 0;                        // failure

void genkey(void);                          // OTP nonlinear generator
void datetime(void);                        // system date-time    
void wscroll(int,int,int,int,int,int,int);  // popup window utility
void border(int,int,int,int,int,int);       // popup window utility
int getans();                               // get user (y/n)? responses
int check_kspec(char *);                    // validate keylist filename
int check_scode(char *);                    // validate sitecode
void allocError(void);                      // trap memory allocation error

FILE *fp;                                   // target file file-pointer
FILE *tl;                                   // transaction log file-pointer
union REGS r;                               // used to access BIOS int 10h
int savex,savey;                            // save cursor coordinates
char kbuf[9];                               // key buffer
char datebuf[24];                           // system date buffer
char timebuf[10];                           // system time buffer
char cwdbuf[MAXPATH];                       // current working directory buffer
struct dosdate_t dd;                        // system date structure
struct time tt;                             // system time structure

// Window class
class Window {
private:
	int bg;                 // background color 
	int fg;                 // foreground color
	int sx;                 // start of x
	int sy;                 // start of y
	int ex;                 // end of x
	int ey;                 // end of y
	char *p;                // screen buffer ptr
	int msize;              // screen buffer size
public:
	Window(int,int,int,int,int,int);
	int warning(void);      // warn user that TRLOG is inactive
	int confirm(char *);    // require user to confirm file overwrite
	void help(void);        // display summary screen
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