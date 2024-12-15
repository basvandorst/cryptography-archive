//---------------------------------------------------------------------
//	Filename: sscv04.h
//	Application: Symmetric stream cipher version 04 header file
//	Codeword OTP V1.0 Vernam Cipher Utility
//	R.James 02-16-1994
//---------------------------------------------------------------------

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include <dos.h>
#include <new.h>
#include <dir.h>
#include <io.h>

const int MAXSIZE=32765;                    // max allocated buf size
const int MAXKEYS=55;                       // maximum generated keys
const int TRUE = 1;                         // indicates success
const int FALSE = 0;                        // indicates error
const int MAXPOINTERS = 4;                  // max concurrent memory allocations
void allocError(void);                      // trap memory allocation error
char *getseeds(char *,char *);              // extract seeds
void wscroll(int,int,int,int,int,int,int);  // BIOS scroll
void border(int,int,int,int,int,int);       // draw border
void datetime(void);                        // sys. date-time
void entropy(void);                         // compute relative entropy
int  getans(void);                          // get prompt response
int  getpath(char *);                       // derive absolute path

FILE *fp;                                   // input file ptr
FILE *sp;                                   // keylist file ptr
FILE *tr;                                   // transaction log file ptr

struct ffblk ffblk;                         // findfirst(...) function structure
struct dosdate_t dd;                        // DOS date return structure
struct time tt;                             // DOS time return structure
int  savex,savey;                           // save screen coordinates
int  globalflag;                            // general purpose global flag
int  handle;                                // target file file-handle
char datebuf[24];                           // sys. date
char timebuf[10];                           // sys. time
char cwdbuf[MAXPATH];                       // current working directory buff
char fspec[64];                             // input file specification
char tfspec[64];                            // temporary fspec
char kspec[64];                             // key file specification
char str[32];                               // ASCII keypair index
unsigned long filesize;                     // input full fsize
unsigned long fsize;                        // incremental input file size
double Hrel;                                // relative entropy
long int loc;                               // current file ptr location
unsigned long bcnt;                         // read count (bytes)
unsigned long wcnt;                         // write count (bytes)
char *kbuf;                                 // keyfile buffer
unsigned char *buf;                         // input buff allocated to input file

int iter;                                   // iterations count
unsigned long Lseed,Rseed;                  // NLRNG seeds

// left constant vector used in the NLRNG
static unsigned long lcvect[]={ // left constant vector
	0x5B54ED61L, 0x672A9746L, 0xD147396CL, 0xC3D8067BL,
	0x6C9A143FL, 0x64B3EC49L, 0x44BE3BD0L, 0x34F4CC69L,
	0xF0E27F08L, 0xC92B01BFL, 0x426A57E6L, 0xF0207BDCL };

// right constant vector used in the NLRNG
static unsigned long rcvect[]={ // right constant vector
	0x706DF456L, 0xEC349A81L, 0x24CF78E2L, 0xF06214F5L,
	0x4C1294FFL, 0x6B404E6FL, 0x04133F7DL, 0x97A496A5L,
	0x186B693EL, 0x02C64FBBL, 0xA5355C27L, 0x780ED9D1L };

// Window class
class Window {
private:
	int bg;             // background color
	int fg;             // foreground color
	int sx;             // start of x
	int sy;             // start of y
	int ex;             // end of x
	int ey;             // end of y
	int msize;          // memory buffer size
public:
	Window(int,int,int,int,int,int);
	int warning(void);
	int confirm(void);
	void help(void);
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