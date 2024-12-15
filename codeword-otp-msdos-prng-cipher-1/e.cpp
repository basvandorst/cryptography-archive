//----------------------------------------------------------------------------
// Filename: e.cpp
// Application: Compute relative entropy (Hr)
// Codeword OTP V1.1 Vernam Cipher Utility
// R.James 02-16-1994
//----------------------------------------------------------------------------
// This utility computes the relative entropy of finite schemes derived from
// disk file contents.  Command line format:
//
//                          C:\K>e fspec
//
//                        - fspec names the input file
//
// Example:                 C:\K>e text.doc
//
// Example:                 C:\K>e file?.*
//
// Example:                 C:\K>e c:\subdir1\*.*
//
// Algorithm:   [1]  Count the occurrence of each ASCII character in the file.
//                   The character count representation is called an incomplete
//                   finite scheme.
//              [2]  Sum the character counts.
//              [3]  Compute the reciprocal of the sum, k = 1 / S.
//              [4]  Form the product of k and each character count to
//                   normalize the scheme.
//              [5]  Sum the products of character counts with their
//                   logarithms.  This sum gives the entropy H of the finite
//                   scheme (i.e., entropy of file content).
//              [6]  Divide the file entropy by max entropy, Hmax=log256,
//                   to produce the relative entropy, 0.0 <= Hrel <= 1.0.
//
// The logarithmic base is arbitrary.  e.exe uses logarithms to base 10.  To
// avoid domain errors, p*log(p) is set to be zero (not computed) if p = 0.
//
// e.exe may be used to determine the textual state of a file (i.e., plaintext
// or ciphertext).  Typical values of relative entropy by file type are:
//
//						 File type             Hr
//						 --------------    -----------
//						 plaintext text    0.60 - 0.65
//						 plaintext binary  0.80 - 0.85
//						 all ciphertext    0.99...
//
// Relative entropy is displayed to nine places after the decimal point which
// is sufficient to detect a polarity change of 1 bit in 8 million, or better.
//
// Relative entropy as computed by e.exe is a function of the occurrence of
// ASCII character codes in a file (but not their relative positions within
// the file) and the file size.  Ciphertext files of approximately 1.2K bytes
// or more exhibit Hr > 0.99.  Values of relative entropy computed from the
// contents of smaller files are less.  Regardless of file size, relative
// entropy of plaintext is less than that of its associated ciphertext.
//-----------------------------------------------------------------------------

#include "e.h"

void main(int argc, char *argv[])
{
	int i,j;				// program scratch variables
	int index;				// index into Pr vector used to count characters
	char *p;				// scratch pointer

	// define memory allocation error handler
	set_new_handler(allocError);

	// check for help request
	if(argc==1) {
		Window summary(BLUE,CYAN,1,1,77,25);
		summary.help();
		exit(0);
	}
	// check for command line argument
	if(argc!=2) {
		cerr << "Filename required." << "\n";
		exit(2);
	}
	// SSCV04 compatibility code to allow e.exe to be called from v.bat
	// and SSCV03 translation batch files (SSCV04 allows a hyphen prefix
	// on file specifications to force output of keypair values referenced
	// by the keypair index -- the hyphen prefix has no meaning in SSCV03.
	// If present, it is simply removed.
	p=argv[1];
	if(*argv[1]=='-') {
		p++;
	}
	// get absolute path of filespec
	strcpy(fspec,p);
	getpath(fspec);

	// find first file
	strcpy(tfspec,fspec);
	if((findfirst(fspec,&ffblk,0))!=NULL) {
		for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);
		if(strchr(fspec,'*') || strchr(fspec,'?'))
			cerr << "No files found." << "\n";
		else
			cerr << fspec << " not found." << "\n";
		exit(1);
	}
	// main processing loop
	for(;;) {
		initfs();	// required for wildcard processing
		if((fp=fopen(fspec,"r+b"))==NULL) {
			cerr << "Unable to open " << fspec << "\n";
			exit(1);
		}
		// get file size
		handle = fileno(fp);
		if((fsize=filelength(handle))==NULL) {
			cerr << fspec << " is a zero length file." << "\n";
			fclose(fp);
			exit(2);
			}
		// allocate target file buffer
		if(fsize>MAXBUF) fsize = MAXBUF;
		unsigned char *buf = new unsigned char[fsize];	// allocation

		// accumulate character counts
		sum=0L;
		for(i=0; i<256; i++) Pr[i]=0L;
		for(i=0; i<256; i++) P[i]=0.0;
		for(;;) {
			bcnt=fread(buf,sizeof(char),fsize,fp);
			if(!bcnt) break;
			sum+=bcnt;
			for(i=0; i<bcnt; i++) {
				index = (int) buf[i];
				Pr[index]++;
			}
			index = 0;
			for(i=0; i<256; i++) {
				if(!Pr[i]) continue;
				P[index++] += (double) Pr[i];
			}
		}
		// create finite scheme
		S = 0.0;
		for(i=0; i<256; i++) S += P[i];
		k = 1.0 / S;
		for(i=0; i<256; i++) P[i] *= k;
		S = 0.0;
		for(i=0; i<256; i++) S += P[i];
		Hmax = log10(256.0);
		H = 0.0;

		// compute relative entropy of finite scheme
		for(i=0; i<256; i++) {
			if(P[i]==0.0) continue;
			H += P[i]*log10(P[i]);
		}
		H = fabs(H);
		Hrel = fabs(H/Hmax);

		// display run result
		printf("Hr %.9f %s %ld bytes\n",Hrel,fspec,sum);
		fclose(fp);
		delete [] buf;									// deallocation
		Label:if((findnext(&ffblk))!=NULL) exit(0);
	}
}
void getpath(char *fspec)
{
	// derive absolute path

	int i;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	char temp[_MAX_DIR]="\\";
	char *p;

	// split the path to gain access to path components
	_splitpath(fspec,drive,dir,file,ext);
	if(!strlen(file)) strcpy(file,"*.*");

	// adjust for back slash if necessary
	if(strlen(drive)) {
		if(!strlen(dir)) strcpy(dir,"\\");
		if(strlen(dir) && *dir!='\\') {
			strcat(temp,dir);
			strcpy(dir,temp);
		} 
	}
	_makepath(fspec,drive,dir,file,ext);

	// derive absolute path
	char *buf = new char[80];				// allocation
	_fullpath(buf,fspec,_MAX_PATH);
	strcpy(fspec,buf);
	delete [] buf;								// deallocation
}
void initfs()
{
	// restore file specification for findnext(...) function
	// if filename.ext contains wildcards

	int i;
	char *p;

	strcpy(fspec,tfspec);
	p=fspec+strlen(fspec);
	while(*p!='\\' && p>fspec) p--;
	if(*p=='\\') {
		p++;
		*p='\0';
		strcat(fspec,ffblk.ff_name);
	}
	else strcpy(fspec,ffblk.ff_name);
	for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);
}
void wscroll(int dir,int attr,int count,int sx,int sy,int ex,int ey)
{
	// window utility used to scroll new window.  Borland C++ 4.0
	// conio.h functions address the screen using coordinates
	// (1,1,80,25); BIOS int 10h uses (0,0,79,24).  Therefore,
	// incoming screen coordinates must be decremented to maintain
	// compatability

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
	// window utility used to draw border in scrolled window

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

	// save current screen coordinates and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create window
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p = new char[msize];					// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,BLUE,0,sx,sy,ex,ey);
	border(BLUE,WHITE,sx,sy,ex,ey);
	textbackground(BLUE); textcolor(WHITE);

	// display usage summary text
	x=8; y=2;
	gotoxy(x,y++);
	cprintf("             CODEWORD OTP V1.1 VERNAM CIPHER UTILITY");
	y++;
	textcolor(CYAN);
	gotoxy(x,y++);
	cprintf("                Relative Entropy Utility (e.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("Usage:   C:\\K>e fspec");
	y++;
	gotoxy(x,y++);
	cprintf("         fspec names the input file (wildcards permitted).");
	y++;
	gotoxy(x,y++);
	cprintf("         Relative entropy (Hr) is a decimal number on the");
	gotoxy(x,y++);
	cprintf("         interval [0.0, 1.0].  Typical values by file type");
	gotoxy(x,y++);
	cprintf("         are:");
	y++;
	gotoxy(x,y++);
	cprintf("                  File type              Hr");
	gotoxy(x,y++);
	cprintf("                  ----------------  -----------");
	gotoxy(x,y++);
	cprintf("                  plaintext text    0.60 - 0.66");
	gotoxy(x,y++);
	cprintf("                  plaintext binary  0.80 - 0.86");
	gotoxy(x,y++);
	cprintf("                  ciphertext        0.99...");
	y++;
	gotoxy(x,y++);
	cprintf("Example: C:\\K>e text.doc");
	gotoxy(x,y++);
	cprintf("         Hr 0.620035927 TEXT.DOC 4208 Bytes");
	y++;
	textcolor(WHITE);
	gotoxy(x,y++);
	cprintf("                     Press any key to exit...");
	getch();

	// restore screen and return
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	puttext(sx,sy,ex,ey,p);
	delete [] p;						// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	cerr << "Memory allocation error.";
}
