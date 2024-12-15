//-----------------------------------------------------------------------------
//	Filename: k.cpp
//	Application: sscv03 keyset generator
//	Codeword V3.0 Stream Cipher
//	R.James 02-16-1994
//-----------------------------------------------------------------------------
//  This program generates sscv03 (V3.0 stream cipher) keyset files.  Each
//  keyset file contains two sscv03 command line directives.  Each directive
//  contains one 11 character (88-bit) key.  Command line format:
//
//								C:\K>k fspec
//
//		- fspec names the translation batch file to be created.
//
//  Example:				C:\K>k tr
//
//  Output:	      @echo off                      <-- surpress display
//                rem TR.BAT                     <-- filename          
//                rem Tue Jan 11 21:21:52 1994   <-- date-time
//                if (%1)==() goto help          <-- skip translation
//                sscv03 %1 mU86mFcO@:h          <-- 1st sscv03 directive
//                if errorlevel 1 goto end       <-- exit here if error
//                sscv03 %1 q}vf`A]txqB          <-- 2nd sscv03 directive
//                e %1                           <-- display rel. entropy
//                goto end                       <-- skip usage screen
//                :help                          <-- display usage screen
//                sscv03                             for sscv03.exe
//                :end                           <-- done
//
//  k.exe uses the shuffled returns from the compiler supplied linear random
//	sequence generator, reduced modulo 87, as indices into an 87 (0 to 86)
//	element key character array.  Thus, more than 2.16exp(21) 11-character
//	keys are possible.  Two keys produce an "induced" keyspace magnitude given
//	by:
//
//			n!/[2!(n-2)!] = n(n-1)(n-2)!/[2!(n-2)!] = 0.5[n(n-1)]
//			n = 2.16exp(21) ==> 2.34 x 10exp(42) (induced keyspace)
//
//  The average time required to recover a keyset (in a surreptitious effort
//  to recover plaintext by trial) is 0.5[2.34 x 10exp(42)] = 1.17 x 10exp(42)
//  seconds.  Assuming a trial rate of 20 million keysets per second, it would
//  require, on average, 1.86 x 10exp(27) years to succeed.
//
//  Ciphertext produced by sscv03.exe using 2-key keysets consistently exhibits
//  relative entropy Hrel > 0.99.  Hence frequencies of occurrence and relative
//  positions of characters in plaintext do not correlate with the frequencies
//  and relative positions of characters in the corresponding ciphertext (i.e.,
//  there are no statistical hooks in sscv03 produced ciphertext).
//
//  Seed values are derived from the system clock.  Therefore, k.exe cannot
//  be used to generate a specific 2-key keyset on demand.
//
//  k.exe produces batch files only.  Any and all filename exetnsions are
//  replaced by the extension .bat.  Examples of using sscv03 in conjunction
//  with translation batch files produced by k.exe are:
//
//  EXAMPLE 1.  Use tr.bat to encrypt plaintext file text.doc:
//
//                   C:\K>tr text.doc    <-- sscv03 directive
//
//              To decrypt file text.doc simply resubmit the above directive.
//
//  EXAMPLE 2.  Wildcard translation:  C:\K>tr d:\mydir\subdir1\*.*
//
//			    This run will translate each file in mydir\subdir1 on
//              the D: drive.
//----------------------------------------------------------------------------

#include "k.h"

void main(int argc, char *argv[])
{
	int i,j;                              // loop control variables
	int count;                            // scratch character counter
	char *p;                              // scratch pointer

	// define memory allocation error handler
	set_new_handler(allocError);

	// process command line arguments
	if(argc==1) {
		Window summary(BLUE,WHITE,1,1,78,25);
		summary.help();
		exit(1);
	}
	if(argc>2) {
		cerr << "Too many arguments on command line." << "\n";
		exit(2);
	}
	// set file extension to .bat
	strcpy(fspec,argv[1]);
	p=fspec+strlen(fspec)-1;
	while(*p!='\\' && p>fspec) p--;
	if(*p=='\\') p++;
	if(*p=='.') {
		cerr << "Malformed filename." << "\n";
		exit(1);
	}
	count=0;
	while(*p!='.' && *p) {
		p++;
		count++;
	}
	*p='\0';
	if(count>8) {
		cerr << "More than 8 characters in file name." << "\n";
		exit(1);
	}
	strcat(p,".bat");
	if(filechk(fspec)) {
		Window warn(LIGHTGRAY,DARKGRAY,16,9,62,16);
		if(warn.confirm()==ABORT) exit(1);
	}
	// open keylist file for output
	for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);
	if((kp=fopen(fspec,"w"))==NULL) {
		cerr << "Unable to open keyset file." << "\n";
		exit(1);
	}
	// seed generator
	t=time(NULL);
	seed=(unsigned int) t;
	srand(seed);

	// initialize shuffle vector
	for(i=0; i<87; i++) *(V+i)=(int) rand() % 87;

	// generate keylist batch file
	fprintf(kp,"@echo off\n");
	fprintf(kp,"rem %s\n",fspec);
	t=time(NULL);
	strcpy(dtbuf,ctime(&t));
	fprintf(kp,"rem %s",dtbuf);
	fprintf(kp,"if (%1)==() goto help\n");
	for(i=0; i<KEYCOUNT; i++) {
		for(j=0; j<KEYSIZE; j++) *(keybuf+j)=nextch();
		*(keybuf+KEYSIZE)='\0';
		fprintf(kp,"sscv03 %1 ");
		fprintf(kp,"%s\n",keybuf);
		if(!i) fprintf(kp,"if errorlevel 1 goto end\n");
	}
	fprintf(kp,"e %1\n");
	fprintf(kp,"goto end\n");
	fprintf(kp,":help\n");
	fprintf(kp,"sscv03\n");
    fprintf(kp,":end\n");

	// exit
	fclose(kp);
}
char nextch()
{
	// This function returns shuffled key characters.  Returns from the
	// linear generator are reduced modulo 87 and used as indices into the
	// arbitrairly arranged key character set.

	int index;
	char ch;

	 char kchars[] = {
		'=',  'n',  'D',  'C',  'T',  'F',  'G',  'H',  'I',  'J',  'K',
		'c',  'M',  'd',  'O',  'P',  'Q',  'V',  'S',  'E',  'U',  'R',
		'W',  'X',  'Y',  'Z',
		'6',  'h',  'L',  'N',  'j',  'f',  ',',  'b',  'i',  'e',  'k',
		'l',  'm',  'B',  'o',  'p',  '2',  'r',  't',  's',  'u',  '*',
		'w',  'x',  '8',  'z',
		'0',  '1',  'q',  '3',  '4',  '5',  'a',  '7',  'y',  '+',
		'`',   '~',  '!',  '@',  '#',  '$',  '^',  '&',  'v',  ':', '.',
		'-',   '_',  '9',  'A',  '{',  '}',  '[',  ']',  '(',  ';',  ')',
		'/',  '?',  'g' };

	// shuffle return from linear generator
	index=(int) rand()%87;
	ch=*(V+index);
	*(V+index)=(unsigned char) rand()%87;
	ch=kchars[index];
	return(ch);  // next key character
}
int filechk(char *fn)
{
	FILE *fp;
	char ans;

	// determine if the file named in fn already exists
	if((fp=fopen(fn,"rb"))==NULL) return(FALSE);
	fclose(fp);
	return(TRUE);
}
int Window::confirm()
{
	// warn user that an existing keyset file will be overwritten if
	// processing is allowed to proceed

	// save current screen coordinates and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// compute screen buffer requirement
	msize=2*(ex-sx+1)*(ey-sy+1);

	// create popup window
	char *p = new char[msize];					// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	gotoxy(sx+2,sy+1);

	// issue warning text
	cprintf("A translation batch file by the same name");
	gotoxy(sx+2,sy+2);
	cprintf("already exists.  Replacing this file will");
	gotoxy(sx+2,sy+3);
	cprintf("destroy a keyset that may have been used to");
	gotoxy(sx+2,sy+4);
	cprintf("encrypt files that are still in your system.");
	gotoxy(sx+14,sy+6);
	cprintf("Proceed (y/n)? ");
	char ans=getans();

	// restore screen and return user prompt response
	_setcursortype(_NORMALCURSOR);
	puttext(sx,sy,ex,ey,p);
	delete [] p;									// deallocation
	gotoxy(savex,savey);
	return ans;
}
int getans()
{
	// get user response to (y/n)? prompt

	char ans;

	for(;;) {
		ans=tolower(getch());
		if(ans=='y') return(CONTINUE);
		if(ans=='n') return(ABORT);
	}
}
void wscroll(int dir,int attr,int count,int sx,int sy,int ex,int ey)
{
	// window utility uses BIOS interrupt 10h function 6 (or 7) to
	// scroll in window

	union REGS r;
	r.h.al=count;
	r.h.ah=dir;
	r.h.bh=attr<<4;
	r.h.cl=sx-1;
	r.h.ch=sy-1;
	r.h.dl=ex-1;
	r.h.dh=ey-1;
	int86(0x10,&r,&r);
}
void border(int bg,int fg,int sx,int sy,int ex,int ey)
{
	// window utility to draw border around scrolled window

	int i;

	textbackground(bg);
	textcolor(fg);

	for(i=sx; i<ex; i++) {
		gotoxy(i,sy);
		putch(196);
		gotoxy(i,ey);
		putch(196);
	}
	for(i=sy; i<ey; i++) {
		gotoxy(sx,i);
		putch(179);
		gotoxy(ex,i);
		putch(179);
	}
	gotoxy(sx,sy);	putch(218);
	gotoxy(sx,ey);	putch(192);
	gotoxy(ex,sy);	putch(191);
	gotoxy(ex,ey);	putch(217);
}
void Window::help()
{
	// display usage summary screen

	int x,y;

	// save current cursor position and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// compute screen buffer size requirement
	msize=2*(ex-sx+1)*(ey-sy+1);

	// create window
	p=new char[msize];
	gettext(sx,sy,ex,ey,p);
	wscroll(6,bg,0,1,1,78,25);
	border(bg,fg,1,1,78,25);
	textbackground(bg);
	textcolor(fg);

	// display usage summary text
	x=4; y=2;
	gotoxy(x,y++);
	cprintf("                  CODEWORD V3.0 STREAM CIPHER UTILITY");
	textcolor(CYAN);
	y++;
	gotoxy(x,y++);
	cprintf("                    SSCV03 Keyset Generator (k.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("  Usage:     C:\\>k fspec");
	y++;
	gotoxy(x,y++);
	cprintf("             fspec<.bat implied> names the output file.");
	y++;
	gotoxy(x,y++);
	cprintf("  Example 1. C:\\>k tr <-- create translation batch file named tr.bat");
	y++;
	gotoxy(x,y++);
	cprintf("             k.exe creates two sscv03.exe command line directives and");
	gotoxy(x,y++);
	cprintf("             writes them to disk in a translation batch file format.");
	gotoxy(x,y++);
	cprintf("             Examples of using tr.bat to encrypt and decrypt files:");
	y++;
	gotoxy(x,y++);
	cprintf("  Example 2. C:\\K>tr text.doc  <-- encrypt file text.doc (step 1).");
	gotoxy(x,y++);
	cprintf("  Example 3. C:\\K>tr text.doc  <-- decrypt file text.doc (step 2).");
	gotoxy(x,y++);
	cprintf("  Example 4. C:\\K>tr *.exe     <-- encrypt all .EXE files in C:\\K");
	gotoxy(x,y++);
	cprintf("  Example 5. C:\\K>tr *.exe     <-- decrypt all .EXE files in C:\\K");
	gotoxy(x,y++);
	cprintf("  Example 6. A:\\>tr c:\\k\\*.*   <-- encrypt all files in C:\\K from A:");
	gotoxy(x,y++);
	cprintf("  Example 7. A:\\>tr c:\\k\\*.*   <-- decrypt all files in C:\\K from A:");
	y++;
	textcolor(WHITE);
	gotoxy(x,y++);
	cprintf("                       Press any key to exit...");
	getch();

	// restore screen and return
	puttext(sx,sy,ex,ey,p);
	delete [] p;
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	// issue memory allocation error message and exit

	cerr << "Memory allocation error.\n";
	exit(1);
}
