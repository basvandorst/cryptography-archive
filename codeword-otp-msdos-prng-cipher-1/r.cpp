//----------------------------------------------------------------------------
// Filename: r.cpp
// Application: File removal utility (trasher)
// Codeword OTP V1.1 Vernam Cipher Utility
// R.James 02-16-1994
//----------------------------------------------------------------------------
// This utility removes files by first overwritting file content and then
// erasing the file.  Files are overwritten with three predetermined patterns
// and one random pattern.  If necessary (and it usually is), file size is
// extended to occupy an integral number of 512 byte blocks.  As an example,
// file text.doc, supplied with the utilities, occupies 8.22 512 blocks or
// 4208 bytes, so DOS must allocate nine blocks to this file.  400 bytes are
// added to the file size to produce a file of 4608 bytes (9 full blocks)
// prior to overwritting and erasure.
//
//    -------------------------------------------------------------------
//    The file removal procedure implemented in this utility conforms to
//    recommendations found in the National Computer Security Center
//    publication:  A Guide to Understanding Data Remanence in Automated
//    Information Systems, NCSC-TG-025 Version 2, Sep 1991.
//    -------------------------------------------------------------------
//
// Command line format:         C:\K>r fspec
//
//                              fspec names the file(s) to be removed
//
// Example:                     C:\K>r text.doc
//
// Wildcards are permitted.
//
// Example:                     C:\K>r file?.*
//
// Example:                     C:\K>r c:\subdir1\*.*
//
// A positive response to the confirmation prompt
//
//                       Remove fspec (y/n)?
//
// is required when attempting to remove files with this utility.  Run r.exe
// without command line arguments to display the summary usage screen.
//
// If the transaction log is inactive (TRLOG assigned the read only attribute),
// it is necessary to respond to the warning prompt:
//
//                       Transaction log is inactive.
//                             Proceed (y/n)?
// -----------------------------------------------------------------------

#include "r.h"

void main(int argc,char *argv[])
{
	int handle;               // target file handle
	unsigned int seed;        // linear generator seed
	unsigned long i,j;        // loop control over filesize
	unsigned long fsize;      // target file size (bytes)
	unsigned long bps;        // bytes per sector (bps = 512 for DOS)
	unsigned long residue;    // byte count of short block
	unsigned long blocks;     // number of blocks after short block extended

	//                        Overwrite patterns.....
	unsigned char pattern[]={ 0xAA,       // 10101010
                              0x55,       // 01010101
                              0xCC };     // 11001100

	unsigned char ranchar;    // random overwrite byte
	struct diskfree_t free;   // used to determine bps

	// define memory allocation error handler
	set_new_handler(allocError);

	// verify command line
	if(argc==1) {
		Window summary(LIGHTGRAY,DARKGRAY,1,2,77,21);
		summary.help();             // display usage screen
		exit(0);                    // and terminate normally
	}
	// get bytes per sector (probably 512, but be sure)
	if(_dos_getdiskfree(0,&free) != 0) {
		cerr << "Error in _dos_getdiskfree" << "\n";
		exit(1);
	}
	bps=free.bytes_per_sector;

	// get absolute path
	strcpy(fspec,argv[1]);
	getpath(fspec);
	for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);

	// find first file
	strcpy(tfspec,fspec);
	if((findfirst(fspec,&ffblk,0))!=NULL) {
		if(strchr(fspec,'*') || strchr(fspec,'?'))
			cerr << "No files found." << "\n";
		else {
			cerr << fspec << " not found.\n";
		}
		fclose(tr);
		exit(1);
	}
	// open transaction log
	if((tr=fopen("TRLOG","a"))==NULL) {
		Window warn(LIGHTGRAY,DARKGRAY,24,10,55,13);
		if(!warn.warning()) exit(1);
	}
	// get user confirmation
	Window besure(LIGHTGRAY,DARKGRAY,30,10,0,0);
	if(!besure.confirm(fspec)) exit(1);;

	// main processing loop
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	for(;;) {
		initfs();
		if((fp=fopen(fspec,"r+b"))==NULL) {
			fclose(tr);
			goto Label;
		}
		// get file size
		handle=fileno(fp);
		if((fsize=filelength(handle))==NULL) {
			fclose(fp);
			goto Label;
		}
		// extend last block to bps boundary
		residue=fsize % bps;
		if(residue) {
			blocks=fsize/bps;
			fsize=bps*(blocks+1);
		}
		// overwrite extended file with three predetermined patterns

		for(i=0; i<WRITECOUNT; i++) {
			for(j=0; j<fsize; j++) fputc(*(pattern+i),fp);
			rewind(fp);
		}
		// overwrite extended file with one random pattern
		srand((unsigned int) time(NULL));
		for(i=0; i<fsize; i++) {
			ranchar=(unsigned char) rand();
			fputc(ranchar,fp);
		}
		rewind(fp);
		fclose(fp);

		// erase file in the DOS sense
		Label:
		if(remove(fspec)) {
			textbackground(BLACK);
			textcolor(LIGHTBLUE);
			cprintf("Error removing %s",fspec);
			textbackground(BLACK);
			textcolor(LIGHTGRAY);
		}
		else cprintf("Removed %s",fspec);
		cerr << "\n";

		// make transaction log entry if trlog is file open
		if(tr && strcmp(fspec,"TRLOG")) {
			for(i=0; i<strlen(fspec); i++) fspec[i]=tolower(fspec[i]);
			datetime();
			fprintf(tr," removed %s\n",fspec);
			fprintf(tr,"         %s\n",datebuf);
		}
		// try to find next file (this may be a wildcard run)
		if((findnext(&ffblk))!=NULL) break;
	}
	// done -- if transaction log file is opne, close it
	if(tr) fclose(tr);
}
void initfs()
{
	// This function is used to preserve the drive designator and path in
	// wildcard runs.  The findfirst and findnext functions store the found
	// file name in ffblk.ff_name.  initfs() ensures that, for each fopen()
	// and findnext() operation, fspec contains an absolute path.

	int i;
	char *p;

	strcpy(fspec,tfspec);
	p=fspec+strlen(fspec);
	while(*p!='\\' && *p!=':' && p>fspec) p--;
	if(*p=='\\' || *p==':') {
		p++;
		*p='\0';
		strcat(fspec,ffblk.ff_name);
	}
	else strcpy(fspec,ffblk.ff_name);
	for(i=0; i<strlen(fspec); i++) fspec[i]=toupper(fspec[i]);
}
void datetime()
{
	// Acquire system date and time for inclusion in
	// transaction log entries.  The date-time stamp
	// is concatenated in the "datebuf" buffer.

	_dos_getdate(&dd);
	gettime(&tt);
	sprintf(datebuf,"%02d-%02d-%d ",dd.month,dd.day,dd.year);
	sprintf(timebuf,"%02d:%02d:%02d",tt.ti_hour,tt.ti_min,tt.ti_sec);
	strcat(datebuf,timebuf);
}
int Window::warning(void)
{
	// popup "inactive TRLOG" warning

	char ans;

	// save current screen coordinates and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create window
	p=new char[msize];							// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	textbackground(LIGHTGRAY);
	textcolor(DARKGRAY);
	gotoxy(sx+2,sy+1);

	// display warning text
	cprintf("Transaction log is inactive.");
	gotoxy(sx+9,sy+2);
	cprintf("Proceed (y/n)?");
	ans=getans();

	// restore screen and return user response
	puttext(sx,sy,ex,ey,p);
	delete [] p;									// deallocation
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	_setcursortype(_NORMALCURSOR);
	gotoxy(savex,savey);
	return(ans);
}
int Window::confirm(char *fn)
{
	// Require user to confirm file removal request

	int i;
	char rem[74]="Remove ";
	char que[]=" (y/n)?";
	char ans;

	// save current cursor position and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// scale window coordinates to contain variable width text string
	strcat(rem,fn);
	strcat(rem,que);
	sx=(76-(strlen(rem)))/2;
	ex=sx+strlen(rem)+3;
	sy=10;
	ey=12;

	// compute screen buffer size requirement
	msize=2*(ex-sx+1)*(ey-sy+1);
	
	// create window
	p = new char[msize];						// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	gotoxy(sx+2,sy+1);
	cprintf("%s",rem);
	i=getans();

	//restore screen and exit
	puttext(sx,sy,ex,ey,p);
	delete [] p;									// deallocation
	_setcursortype(_NORMALCURSOR);
	gotoxy(savex,savey);
	return(i);
}
int getans()
{
	// get user response from popup window

	char ans;

	while(1) {
		ans=tolower(getch());
		if(ans=='y') return(1);
		if(ans=='n') return(0);
	}
}
void getpath(char *fspec)
{
	// derive absolute path of file specification for transaction log entry

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	char temp[_MAX_DIR]="\\";

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
void wscroll(int dir,int attr,int count,int sx,int sy,int ex,int ey)
{
	// Scroll in window using function function 6 or 7 of BIOS int 10h.
	// conio.h routines use base screen coordinates (1,1) while int 10h
	// uses (0,0).  Screen coordinates passed to this function must be
	// decremented to maintain compatibility.

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
	// Draw border around scrolled screen area.  Call border(...) after
	// calling wscroll(...) using the same screen coordinates (sx,sy,ex,ey).
	// background and foreground screen colors are specified by bg and fg,
	// respectively.

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
	// display r.exe summary screen.

	int x,y;

	// save cursor position and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create window
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p = new char[msize];			// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,BLUE,0,sx,sy,ex,ey);
	border(BLUE,WHITE,sx,sy,ex,ey);
	textbackground(BLUE);
	textcolor(WHITE);

	// display usage summary text
	x=4; y=3;
	gotoxy(x,y++);
	cprintf("                CODEWORD OTP V1.1 VERNAM CIPHER UTILITY");
	y++;
	textcolor(CYAN);
	gotoxy(x,y++);
	cprintf("                     File Removal Utility (r.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("  Usage:    C:\\K>r fspec");
	y++;
	gotoxy(x,y++);
	cprintf("            fspec names the file to be removed (wildcards permitted).");
	y++;
	gotoxy(x,y++);
	cprintf("  Example:  C:\\K>r text.doc");
	y++;
	gotoxy(x,y++);
	cprintf("  Example:  C:\\K>r d:\\mydir1\\subdir1\\*.*");
	y++;
	gotoxy(x,y++);
	cprintf("  r.exe removes files in accordance with guidelines established in");
	gotoxy(x,y++);
	cprintf("  A Guide to Understanding Data Remanence in Automated Information");
	gotoxy(x,y++);
	cprintf("  Systems, NCSC-TG-025 Version 2, Sep 1991.");
	textcolor(WHITE);
	y++;
	gotoxy(x,y++);
	cprintf("                        Press any key to exit...");
	getch();

	// restore screen and return;
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	puttext(sx,sy,ex,ey,p);
	delete [] p;						// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	// issue memory allocation failure error message and exit

	cerr << "Memory allocation error.\n";
}
