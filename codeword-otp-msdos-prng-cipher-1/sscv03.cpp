//-----------------------------------------------------------------------------
//  Filename: sscv03.cpp
//  Application: symmetric stream cipher
//  Codeword V3.0 private key cipher utility
//  R.James 02-02-1993
//-----------------------------------------------------------------------------
//  The V3.0 stream cipher uses an 11 character (88-bit) key to translate
//  (i.e., encrypt or decrypt) any file that can be created or maintained
//  in the DOS environment.
//
//  sscv03 uses the sscv04 nonlinear random number generator (NLRNG) and the
//  multiplicative congruential generator supplied with the Borland C++ 4.0
//  compiler.  The NLRNG is used to initialize a 1792 byte data structure
//  called the path matrix.  The path matrix is referenced as a 256 x 7 two
//  dimensional array.  Shuffled returns from the linear generator are used
//  during the translation process to step the 11 key character values in a
//  manner that ensures aperiodicity.
//
//  Each byte of the input stream must pass through each of the seven columns
//  of the path matrix.  Entry into the first column is determined by the
//  value of the currently indexed key character. Row indices into subsequent
//  path matrix columns are determined by the previously selected path matrix
//  character.  Paths taken through the path matrix are independent of the
//  byte values in the input stream.  Each input byte is summed modulo 2 seven
//  times as it passes through the path matrix.
//
//  Command line format:    C:\K>sscv03 filename key
//
//  EXAMPLE 1.              C:\K>sscv03 text.doc Vuz#8]rTM@e
//
//  Translating files by direct submission to sscv03 is NOT recommended.  Use
//  the sscv03 keyset generator k.exe to create translation batch files that
//  contain two sscv03 command line directives.  To create an sscv03 keyset:
//
//  EXAMPLE 2.              C:\K>k tr    <-- see k.cpp listing for details.
//
//  In this example, k.exe will produce a translation batch file named tr.bat
//  that may be used to encrypt any file in the directory hierarchy:
//
//  EXAMPLE 3.              C:\K>tr d:\mydir\subdir1\myfile.ext
//
//  Unlike sscv04, sscv03 accepts wildcards in the filename and filename
//  extension:
//
//  EXAMPLE 4.              C:\K>tr file?.*
//
//  EXAMPLE 5.              C:\K>tr c:\whatever\*.*
//
//  sscv03 is included as part of the OTP V1.1 suite of cryptographic utilities
//  for the purpose of protecting installed cryptographic resources.  Storing
//  cipher programs and keylist files in one dedicated subdirectory simplifies
//  the process of encrypting these files using sscv03.  Assuming that copies
//  of sscv03.exe and tr.bat are stored on the diskette currently in the A
//  drive:
//
//  EXAMPLE 6.                A:\>tr c:\k\*.*
//
//  This submission of tr.bat results in each file in C:\K being translated
//  twice.  If these files were stored as plaintext, they will now be stored as
//  ciphertext.  To gain access to plaintext, resubmit the sscv03 directives
//  from the A drive via tr.bat.
//
//  It has been demonstrated that sscv03 ciphertext is vulnerable to the method
//  of attack-in-depth if the user persists in encrypting multiple files using
//  the same keyset.  Double wildcard encryption runs obviate this attack.
//  During wildcard encryption runs only the first file to pass through the
//  wildcard filter will be translated using the specified key.  All subsequent
//  files are translated with residual keys (i.e., key values left over from
//  previous translations).  Thus, no two files are encrypted using one and the
//  same key.
//
//  File handling.  Files are encrypted in place (i.e., over themselves).
//  Temporary files are neither needed nor created during the translation
//  process.
//-----------------------------------------------------------------------------

#include "sscv03.h"

void main( int argc, char *argv[] )
{
	int i;                          // scratch variable
	int skip;                       // advance into linear generator sequence
	int wildcard;                   // wildcard flag
	unsigned int seed;              // linear generator seed value
	long  loc;                      // file location pointer
	long  bcnt;                     // byte count used with fread function
	char *fspec;                    // target file specification

	// define memory allocation error handler
	set_new_handler(allocError);

	// check for help request
	if(argc==1) {
		Window summary(BLUE,WHITE,2,2,78,24);
		summary.help();
		exit(1);
	}
	// get command line arguments
	if(argc==3) {
		fspec=argv[1];
		if(*fspec=='-') fspec++;
		for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);
		strcpy(key,argv[2]);
	}
	else {
		cerr << "Invalid directive" << "\n";
		exit(1);
	}
	// check for wildcard run
	if(strchr(fspec,'*') || strchr(fspec,'?')) wildcard = 1;
	else wildcard = 0;

	// validate key length
	if(strlen(key)!=KEYSIZE) {
		cerr << "Invalid key" << "\n";;
		exit(1);
	}
	// seed nonlinear generator
	for(i=0; i<4; i++)  *(S.TCkey+i)=*(key+i);
	*(Seed+1)=S.TBkey;
	for(i=0; i<4; i++) *(S.TCkey+i)=*(key+i+6);
	*(Seed+0)=S.TBkey;

	// seed linear generator
	generator(0);

	seed=(unsigned int) (*(Seed+0)^*(Seed+1));
	srand(seed);

	// advance into linear sequence
	skip=(int) abs(seed>>8);
	for(i=0; i<skip; i++) rand();

	// create shuffle table
	for(i=0; i<ROTSIZE; i++) *(V+i) = rand() >> 8;

	// find first file
	strcpy(tfspec,fspec);
	if((findfirst(fspec,&ffblk,0))!=NULL) {
		cerr << fspec << " not found" << "\n";
		exit(1);
	}
	// translation processing loop
	for(;;) {
		generator(1);
		initfs(fspec);
		if((fp=fopen(fspec,"r+b"))==NULL) {
			cerr << "Unable to open " << fspec << "\n";
			exit(1);
		}
		// get file size
		handle = fileno(fp);
		if((fsize=filelength(handle))==NULL) {
			cerr << fspec << " is a zero length file" << "\n";
			fclose(fp);
			exit(1);
		}
		// allocate memory
		if(fsize>BUFSIZE) fsize = BUFSIZE;
		unsigned char *buf = new unsigned char[fsize];	// allocation
		kndx=SELECT;
		loc=0;
		for(;;) {
			bcnt = fread(buf,sizeof(char),fsize,fp);
			if(bcnt==0L) break;
			translate(bcnt,buf);
			fseek(fp, loc, SEEK_SET);
			fwrite(buf, sizeof(char), bcnt, fp);
			loc = ftell(fp);
		}
		if((fclose(fp))==EOF) {
			cerr << "Error closing " << fspec << "\n";
			delete [] buf;	// conditional deallocation
			exit(1);
		}
		for(i=0; i<fsize; i++) *(buf+i) ^= *(buf+i);
		delete [] buf;										// deallocation
		*(Seed+0) = *(Seed+1) = 0L;
		if(wildcard) cout << "tr: " << fspec << "\n";
		if((findnext(&ffblk))!=NULL) exit(0);
	}
}
void generator(int mode)
{
	// nonlinear random number generator.  call with mode=0 to seed
	// generator or mode=1 to initialize path matrix

	int i,j;
	unsigned int left,right;
	unsigned long temp1,temp2,exchange,temph=0,templ=0;

	for(i=0; i<1792; i+=N) {
		for(j=0; j<N; j++) {
			exchange = *(Seed+1);
			temp1 = (exchange=(*(Seed+1))) ^ *(lcvect+j);
			templ = temp1 & 0xFFFF;
			temph = temp1 >> 16;
			temp2 = templ*templ + ~(temph*temph);
			*(Seed+1) = *(Seed+0) ^ (((temp1=(temp2>>16) |
				((temp2 & 0xFFFF)<<16)) ^
				*(rcvect+j)) + templ*temph);
			*(Seed+0) = exchange;
		}
		// if mode = 0, the generator is being called for keyset production
		// if mode = 1, the generator is being called to initialize the path
		// matrix
		if(!mode) return;
		left = (unsigned int) *(Seed+0);
		right= (unsigned int) *(Seed+1);

		// store output in path matrix
		*(B.lbuf+i+0) = (unsigned char) (left >> 8);
		*(B.lbuf+i+1) = (unsigned char) left;
		*(B.lbuf+i+2) = (unsigned char) (right >> 8);
		*(B.lbuf+i+3) = (unsigned char) right;
	}
}
void initfs(char *fspec)
{
	// reset filespec for findnext function

	int i, j, k;

	j = 0;
	strcpy(fspec,tfspec);
	for(i=strlen(fspec); i>0; i--) {
		if(fspec[i]=='\\' || fspec[i]==':') {
			j = i + 1;
			break;
		}
	}
	for(i=0; i<13; i++) fspec[j++] = ffblk.ff_name[i];
}
void translate(long int bcnt,unsigned char *buf)
{
	// V3.0 symmetric stream cipher

	int i,j;
	unsigned char cchar;

	for(i=0; i<bcnt; i++) {
		cchar=*(key+kndx);
		for(j=0; j<KEYSIZE-4; j++) {
			cchar=*(*(B.path+cchar)+j);
			*(buf+i)^=cchar;
		}
		*(key+kndx)=shuffle();
		if((++kndx)>KEYSIZE) kndx = 0;
	}
}
unsigned char shuffle()
{
	// Bays-Durham shuffle to destroy sequential correlations in sequences
	// produced by the linear generator

	int index;
	unsigned char ch;

	index = (int) (rand() >> 8);
	ch = *(V+index);
	*(V+index) = (rand() >> 8);
	return(ch);
}
void wscroll(int dir,int attr,int count,int sx,int sy,int ex,int ey)
{
	// window utility

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
	// window utility

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

	// create window
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p = new char[msize];			// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,BLUE,0,sx,sy,ex,ey);
	border(BLUE,WHITE,sx,sy,ex,ey);
	textcolor(WHITE);

	// display usage summary text
	x=6; y=3;
	gotoxy(x,y++);
	cprintf("                     CODEWORD V3.0 STREAM CIPHER");
	y++;
	textcolor(CYAN);
	gotoxy(x,y++);
	cprintf("                            (sscv03.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("Usage:      Run k.exe to create a translation batch file");
	gotoxy(x,y++);
	cprintf("            containing a V3.0 stream cipher 2-key keyset.");
	y++;
	gotoxy(x,y++);
	cprintf("Example 1.  C:\\K>k tr");
	y++;
	gotoxy(x,y++);
	cprintf("            Use the translation batch file, tr.bat, produced");
	gotoxy(x,y++);
	cprintf("            by k.exe to encrypt or decrypt any file in the");
	gotoxy(x,y++);
	cprintf("            directory hierarchy.");
	y++;
	gotoxy(x,y++);
	cprintf("Example 2.  C:\\K>tr text.doc");
	y++;
	gotoxy(x,y++);
	cprintf("Example 3.  C:\\K>tr c:\\subdir1\\myfile.ext");
	y++;
	gotoxy(x,y++);
	y++;
	cprintf("Example 4.  C:\\K>tr d:\\subdir2\\*.*");
	y++;
	textcolor(WHITE);
	gotoxy(x,y++);
	cprintf("                        Press any key to exit...");
	gotoxy(x,y++);
	getch();

	// restore screen and exit
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	puttext(sx,sy,ex,ey,p);
	delete [] p;									// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	cerr << "Memory allocation error.\n";
	exit(1);
}
