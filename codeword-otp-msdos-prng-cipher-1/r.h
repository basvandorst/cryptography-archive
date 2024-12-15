//-----------------------------------------------------------------------------
// Filename: r.h
// Application: File removal utility r.cpp header file
// Codeword OTP V1.1 Vernam Cipher Utility
// R.James 02-16-1994
//-----------------------------------------------------------------------------

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>
#include <dos.h>
#include <dir.h>
#include <new.h>
#include <io.h>

const int WRITECOUNT=4;                     // predetermined overwrite count

// function prototypes
void wscroll(int,int,int,int,int,int,int);  // scroll new window
void border(int,int,int,int,int,int);       // draw border around scrolled window
void initfs(void);                          // used with findnext() function
void datetime(void);                        // access system date-time
void getpath(char *);                       // derive absolute path
int  confirm(char *);                       // confirm file removal request
int  warning(void);                         // warn user that TRLOG is inactive
int  getans(void);                          // get response to (y/n)? prompt
void allocError(void);                      // trap memory allocation error

// program variables
FILE *fp;                   // file pointer for input file
FILE *tr;                   // file pointer for TRLOG file
int  cwdpath;               // prefix cwd to fspec if this flag is set
int  savex,savey;           // save current screen coordinates
char cwdbuf[MAXPATH];       // current working directory buffer
char savcwd[MAXPATH];       // backup copy of cwd
char tempcwd[MAXPATH];      // temporary cwd used with remove()
char fspec[64];             // target file file_specification
char tfspec[64];            // temp. fspec used with initfs()
char pspec[64];             // fspec with cwd prefix
struct ffblk ffblk;         // structure used with findfirst/findnext
struct dosdate_t dd;        // system date
char datebuf[24];           // system date buffer
struct time tt;             // system time
char timebuf[10];           // system time buffer

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
	int confirm(char *);    // require user to confirm file removal
	int warning(void);      // warn user that TRLOG is inactive
	void help(void);        // display summary (or usage) screen
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