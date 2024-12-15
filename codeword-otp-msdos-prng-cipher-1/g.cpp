//----------------------------------------------------------------------------
//  Filename: g.cpp
//  Application: sscv04 key generation utility
//  Codeword OTP V1.0 Vernam Cipher Utility
//  R.James 02-16-1994
//----------------------------------------------------------------------------
//  This program generates sscv04 (one time pad) keylist files.  Each keylist
//  file contains the keylist filename, a sitecode, 55 keypairs, and the
//  iterations count.  Command line format:
//
//                    C:\K>g KLx.y sitecode {xx}
//
//    -  KLx names the keylist file.  Keylist filenames must begin with KL,
//       x must be an integer on the interval [000000,999999], and y (if present)
//       must be an integer in the range of 0 to 999.
//    -  sitecode is a 10-digit integer on the interval [0000000000,4294967295].
//    -  {xx} is an optional iterations count. If present, the iterations count
//       must an integer in the range of 2 to 12. Default=4.
//
//  Example:          C:\K>g KL123456 1234567890
//
//  Output:            Keylist:   KL123456
//                     Sitecode:  1234567890
//
//                @01  0203242106 0072487451
//                @02  1195091972 0542464683
//                @03  1369693658 1413424491
//                -04  0564980222 1335493578
//                  .
//                  .
//                  .
//                @53  1845811898 1335571289
//                @54  1798512222 0925415219
//                @55  1122506687 0564392013 [04]   <-- [iterations count]
//
//  @ signs indentify unused keypairs.  Keypairs that have been used at least
//  once in file translation have keypair indices with hyphen prefixes
//  (e.g., keypair 04).
//-----------------------------------------------------------------------------
#include "g.h"

void main(int argc,char *argv[])
{
	time_t t;	// elapsed time in sec since 00:00:00 GMT Jan 1, 1970
	int i,j,k,v;            // program variables
	int temp;               // scratch variable
	char kspec[64];         // key file specification
	char sitecode[11];		// 10-digit sitecode
	char *p,ans;

	// define memory allocation error handler
	set_new_handler(allocError);

	// check for summary screen request
	if(argc==1) {
		Window summary(BLUE,WHITE,1,1,77,24);
		summary.help();
		exit(1);
	}
	// open transaction log
	if((tl=fopen("TRLOG","a"))==NULL) {
		Window inactive(LIGHTGRAY,DARKGRAY,22,10,53,13);
		if(!inactive.warning()) exit(1);
	}
	// initialize iterations counter to default
	ivar=4;

	// verify iterations count, if present (2 thru 12)
	if(argc==4) {
		v=atoi(argv[3]);
		if(v<2 || v>12) {
			cerr << "Invalid interations count." << "\n";
			exit(1);
		}
		ivar=v;
		argc--;
	}
	// tell user this command line will not work
	if(argc!=3) {
		cerr << "Incomplete command line." << "\n";
		exit(1);
	}
	// get keylist filename
	strcpy(kspec,argv[1]);
	for(i=0; i<strlen(kspec); i++) kspec[i]=toupper(kspec[i]);
	if(!check_kspec(kspec)) exit(1);

	// get sitecode
	strcpy(sitecode,argv[2]);
	if(!check_scode(sitecode)) exit(1);

	// Note:  NLRNG is now seeded by Lword and Rword

	// open new keylist file for output
	if((fp=fopen(argv[1],"w"))==NULL) {
		cerr << "Unable to open output file." << "\n";
		exit(1);
	}
	// set path
	getcwd(cwdbuf,MAXPATH);
	strcat(cwdbuf,"\\");
	strcat(cwdbuf,argv[1]);		// cwdbuf stores path\filename
	for(i=0; i<strlen(cwdbuf); i++) cwdbuf[i] = tolower(cwdbuf[i]);

	// generate keylist file
	fprintf(fp,"\n           Keylist:   %s",cwdbuf);
	fprintf(fp,"\n           Sitecode:  %s\n",argv[2]);
	for(i=0; i<MAXKEYS; i++) {
		fprintf(fp,"\n      @%02d  ",i+1);
		genkey();
		fprintf(fp,"%010ld",labs(Lword));
		fprintf(fp," ");
		genkey();
		fprintf(fp,"%010ld",labs(Rword));
		fprintf(fp," ");
	}
	fprintf(fp,"[%02d]\n",ivar);
	fclose(fp);

	// make transaction log entry
	if(tl) {
		datetime();
		fprintf(tl," created %s\n         %s [%02d]\n",cwdbuf,datebuf,ivar);
		fclose(tl);
	}
	exit(FALSE);
}
void genkey()
{
	// NLRNG

	long int i;                     // loop over file size
	int j;                          // iterations count loop control

	// generator temporary variables
	unsigned int left,right;
	unsigned long temp1,temp2;
	unsigned long temph=0,templ=0;
	unsigned long exchg;

	// loop over file size
	for(i=0; i<9; i++) {
		// loop over iterations count
		for(j=0; j<ivar; j++) {
			exchg = Rword;
			temp1 = (exchg=(Rword)) ^ *(c1+j);
			templ = temp1 & 0xFFFF;
			temph = temp1 >> 16;
			temp2 = templ*templ + ~(temph*temph);
			Rword = Lword ^ (((temp1=(temp2>>16) |  // Lword and Rword are
				((temp2 & 0xFFFF)<<16)) ^           // defined in "rng.h"
				*(c2+j)) + templ*temph);
			Lword = exchg;
		}
	}
}
int Window::warning(void)
{
	// popup "Transaction log inactive" window.

	int i,ans;

	// save cursor coordinates and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// allocate screen buffer -- if allocation fails, issue warning
	// at the DOS level
	msize=2*(ex-sx+1)*(ey-sy+1);
	p=new char[msize];
	if(!p) {
		cout << "Transaction log is inactive.  Proceed (y/n)?";
		if(!getans()) exit(1);
	}
	// create window
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	textbackground(LIGHTGRAY);
	textcolor(DARKGRAY);

	// display warning text
	gotoxy(sx+2,sy+1);
	cprintf("Transaction log is inactive.");
	gotoxy(sx+9,sy+2);
	cprintf("Proceed (y/n)?");
	ans=getans();

	// restore screen and return "ans"
	puttext(sx,sy,ex,ey,p);
	delete [] p;
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	_setcursortype(_NORMALCURSOR);
	gotoxy(savex,savey);
	return(ans);
}
int Window::confirm(char *fn)
{
	// This function determines if the keylist file named in fn already
	// exists.  If the file does exist, the user is warned that keypair
	// and sitecode information may be lost if overwrite occurs.  User
	// is given the option to proceed or abort.

	int y,rtnval;
	char ans;

	// Try to open file named in fn.  If the file opens successfully
	// then it must exist.
	if((fp=fopen(fn,"rb"))==NULL) return(1);

	// File opened.  Shut it down and proceed to warning.
	fclose(fp);

	// save cursor coordinates and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create window
	msize=2*(ex-sx+1)*(ey-sy+1);
	p=new char[msize];					// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	textbackground(LIGHTGRAY);
	textcolor(DARKGRAY);

	// display warning text
	y=sy+1;
	gotoxy(sx+2,y++);
	cprintf("%s already exists...",fn);
	gotoxy(sx+2,y++);
	cprintf("Replacing this file may result in the loss");
	gotoxy(sx+2,y++);
	cprintf("of sitecode and keypair usage information.");
	y++;
	gotoxy(sx+14,y++);
	cprintf("Proceed (y/n)? ");
	ans=getans();

	// restore screen and return prompt response
	puttext(sx,sy,ex,ey,p);
	delete [] p;						// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
	return(ans);
}
int getans()
{
	// get user response from popup window

	char ans;

	while(1) {
		ans=tolower(getch());
		if(ans=='y') return(1);
		if(ans=='n') return(FALSE);
	}
}
void datetime()
{
	// get system date-time -- build date-time stamp in datebuf

	_dos_getdate(&dd);
	gettime(&tt);
	sprintf(datebuf,"%02d-%02d-%d ",dd.month,dd.day,dd.year);
	sprintf(timebuf,"%02d:%02d:%02d",tt.ti_hour,tt.ti_min,tt.ti_sec);
	strcat(datebuf,timebuf);
}
int check_kspec(char *kspec)
{
	// validate keyfile name...

	char tbuf[64];                     // temporary buffer
	char *endptr;                      // used with strtoul() function
	char *p;                           // scratch pointer

	// check for path in kspec -- path not allowed to encourage user to
	// create keylist files in current working directory only
	p=kspec + strlen(kspec);
	while(*p!='\\' && *p!=':' && p > kspec) p--;
	if(*p=='\\' || *p==':') {
		cout << "Path is not allowed in keylist filename.\n";
		return(FALSE);
	}
	// verify that keylist filename begins with KL
	if(strncmp(kspec,"KL",2)) {
		cerr << "Keylist filename does not begin with KL.\n";
		return(FALSE);
	}
	// check keylist filename format
	p=strchr(kspec,'.');
	if(p && p!=kspec+8) {
		cerr << "Invalid extension if keylist filename\n";
		return(FALSE);
	}
	if(*(kspec+8)!='.' && strlen(kspec)!=8) {
		cerr << "Invalid keylist filename (length error).\n";
		return(FALSE);
	}
	if(strlen(kspec) > 8) {
		if(*(kspec+8)=='.' && (strlen(kspec)<10 || strlen(kspec)>12)) {
			cerr << "Invalid keylist filename (format error).\n";
			return(FALSE);
		}
	}
	// check for possible overwrite
	Window overwrite(LIGHTGRAY,DARKGRAY,20,9,65,15);
	if(!overwrite.confirm(kspec)) return(FALSE);

	// remove '.' from filename if present
	strcpy(tbuf,kspec);
	p=strchr(kspec,'.');
	if(p!=NULL) {
		*(p++)='\0';
		strcat(kspec,p);
	}
	// check for nondigit in filename string
	p=kspec+2;
	int len = strlen(p);
	while( (*p>='0') && (*p<='9') && len-- ) p++;
	if(len) {
		cerr << "Invalid numeric field in keylist filename.\n";
		return(FALSE);
	}
	// done
	Lword=(unsigned long) strtoul(kspec+2,&endptr,10);	// 1st generator seed
	strcpy(kspec,tbuf);	// restore input
	return(TRUE);
}
int check_scode(char *sitecode)
{
	// validate sitecode

	int i,len;
	char *p;

	len=strlen(sitecode);
	if(len==10) {
		p=sitecode;
		for(i=0; i<len; i++,p++) {
			if((*p < '0') || (*p > '9')) {
				cerr << "Non numeric character in sitecode." << "\n";
				return(FALSE);
			}
		}
	}
	else {
		cerr << "Sitecode is not 10 digits in length.\n";
		return(FALSE);
	}
	Rword=(unsigned long)atol(sitecode);	// 2nd generator seed
	return(TRUE);
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
	// display usage summary text

	int x,y;

	// remove cursor and save current cursor position
	_setcursortype(_NOCURSOR);
	savex=wherex();
	savey=wherey();

	// save screen
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p = new char[msize];		// allocation
	gettext(sx,sy,ex,ey,p);

	// create window
	wscroll(6,bg,0,sx,sy,ex,ey);
	border(bg,fg,sx,sy,ex,ey);
	textbackground(bg);
	textcolor(fg);

	// display usage summary screen
	x=4; y=3;
	gotoxy(x,y++);
	cprintf("               CODEWORD OTP V1.1 VERNAM CIPHER UTILITY");
	y++;
	textcolor(CYAN);
	gotoxy(x,y++);
	cprintf("                  Keylist generation utility (g.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("         This utility creates keylist files used with SSCV04.EXE.");
	gotoxy(x,y++);
	cprintf("         Each keylist file contains 55 keypairs numbered 01 to 55.");
	y++;
	gotoxy(x,y++);
	cprintf("Usage:   C:\\>g KLx.y sitecode {ic}");
	y++;
	gotoxy(x,y++);
	cprintf("         - KLx.y names the keyfile to be created.  KL may be upper");
	gotoxy(x,y++);
	cprintf("           or lower case; x must be a six digit integer on the");
	gotoxy(x,y++);
	cprintf("           interval [000000,999999].  If present, y must be one");
	gotoxy(x,y++);
	cprintf("           to three digits, each digit on the interval [0,9].");
	gotoxy(x,y++);
	cprintf("         - a sitecode is a ten digit integer on the interval");
	gotoxy(x,y++);
	cprintf("           [0000000000,4294967295].");
	gotoxy(x,y++);
	cprintf("         - {ic} is an optional NLRNG iteration count, 2 through 12.");
	gotoxy(x,y++);
	cprintf("           If {ic} is not specified, the value defaults to 4.");
	y++;
	gotoxy(x,y++);
	cprintf("Example: C:\\>g KL298173.392 2009137568 5");
	y++;
	textcolor(WHITE);
	gotoxy(x,y++);
	cprintf("                       Press any key to exit...");
	getch();

	// release window and restore screen
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	puttext(sx,sy,ex,ey,p);
	delete [] p;						// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	// inform user that memory allocation failed

	cerr << "Memory allocation error.\n";
	exit(1);
}
