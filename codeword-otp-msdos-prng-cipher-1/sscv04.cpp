//----------------------------------------------------------------------------
//  Filename: sscv04.cpp
//  Application: symmetric stream cipher version 04
//  Codeword OTP V1.0 Vernam Cipher Utility
//  R.James 02-16-1994
//----------------------------------------------------------------------------
//  Vernam cipher implementation.  This program must be used in conjunction
//  with the keylist file generator g.exe.  Command line format:
//
//              C:\K>sscv04 fspec keyfile keypair {iter}
//
//            - fspec names the file to be translated
//            - keyfile names the keylist file to be used.
//            - keypair identifies one of the 55 keypairs in keyfile.
//            - {iter} is an optional iterations count.  If present,
//              iter must be an integer in the range of 2 to 12.
//
//              Note:  The size of the file to be encrypted or decrypted
//              determines the size of the cipher key produced by the OTP
//              nonlinear random sequence generator (keysize = filesize).
//              The iterations count determines the number of iterations
//              through the inner loop of the generator required to emit
//              one key character in the sequence.
//
//  EXAMPLE 1.  Apply the Vernam cipher utility sscv04.exe to encrypt the file
//              text.doc using the 20th keypair in keylist file KL12345:
//
//              C:\K>sscv04 text.doc KL123456 20         <-- OTP directive
//
//              To decrypt file text.doc, simply resubmit the above directive.
//
//  The keylist file KL123456 must be created prior to running sscv04.  To do
//  this, run the keyset generation utility g.exe:
//
//  EXAMPLE 2.  C:\K>g KL123456 1234567890  <-- see g.cpp listing for details.
//
//  sscv04 may be invoked using the supplied batch file v.bat which returns the
//  relative entropy of file content after translation.  See the e.exe listing
//  for details concerning relative entropy.
//
//  The following example uses v.bat, keypair #47 in keylist file KL930871.491,
//  and iterations count 11 to translate the file text.doc.  If this file is
//  stored as plaintext, it will be stored as ciphertext after the translation
//  run (and vice versa):
//
//  EXAMPLE 3.  C:\K>v text.doc KL930871.491 47 11
//              Hr=0.xxxxxxxxx TEXT.DOC 4208 bytes
//
//  sscv04 uses a variable iteration nonlinear random number generator (NLRNG)
//  to create keys that match the size of the file to be translated.  Key
//  characters are produced on demand (i.e., there is no key buffer).  The
//  NLRNG is a variant of the generator described by Press, et al in the 2nd
//  edition of Numerical Recipes in C, Cambridge University Press, 1992 (see
//  Section 7.5, Random Sequences Based on Data Encryption (300-304)).
//
//  If an iteration count is not specified as the last command line parameter,
//  the default count (=4) is used.  The iteration count may be specified on the
//  command line as an integer in the range of 02 to 12.  EXAMPLE 2 above used
//  the default iteration count.  In EXAMPLE 3 the default was replaced by an
//  iterations count of eleven.
//
//  Note:  Files must be decrypted using the same iterations count used in the
//  encryption run.  Different iterations counts result in different cipher keys.
//  Encrypting a file with one interations count and then attempting to decrypt
//  the file with a different count will further encrypt the file (i.e., the
//  existing ciphertext will itself be encrypted).
//
//  The user must provide confirmation [ Proceed (y/n)? ] if a previously used
//  keypair is specified on the command line.  Confirmation is important because:
//
//      (1) Files encrypted for transmission out of the system must not
//          employ a previously used key.
//      (2) Files encrypted for storage-only should use the same key
//          repeatedly, but not indefinitely.
//
//  Statements (1) and (2) above should be regarded as absolute constraints.
//  Violation of either constraint is sufficient to negate the integrity of
//  the trasmitted or stored ciphertext.
//
//  File handling.  Files are translated in place (i.e., over themselves)
//  using the disk space allocated at the time of file creation.  Temporary
//  files are neither needed nor created during the translation process.
//----------------------------------------------------------------------------

#include "sscv04.h"

void main(int argc,char *argv[])
{
	long int i;                 // loop control over file size
	int j,err;                  // loop control over iterations count
	int temp;                   // stores temporary copy of keypair index
	int xorsw;                  // toggle selects next key character to
                                // be emitted by the generator

	// variables internal to the nonlinear random number generator
	unsigned long temp1,temp2;
	unsigned long exchange;
	unsigned long temph=0,templ=0;

	char *p;                    // scratch pointer

	// define memory allocation error handler
	set_new_handler(allocError);

	// get current working directory
	getcwd(cwdbuf,MAXPATH);

	// process command line arguments
	if(argc==1) {
		Window summary(BLUE,WHITE,1,1,77,24);
		summary.help();
		exit(0);
	}
	if(argc<4) {
		cerr << "Incomplete command line." << "\n";
		exit(1);
	}
	if(argc>5) {
		cerr << "Extra characters on command line.\n";
		exit(1);
	}
	// if file specification is preceeded by a hyphen, strip off the hyphen
	// and set the global flag to force output of the random number generator
	// seeds selected by the specified keypair index.  This is a diagnostic
	// feature not normally required by the user.
	p=argv[1];
	if(*argv[1]=='-') {
		p++;
		globalflag=1;
	}
	else globalflag=0;

	// derive absolute path
	strcpy(fspec,p);
	if(!getpath(fspec)) exit(1);	// error exit if path cannot be derived

	// bring in the keyfile specification and convert it to upper case.  This
	// file is opened and closed in function getseeds();
	strcpy(kspec,argv[2]);
	for(i=0; i<strlen(kspec); i++) kspec[i]=tolower(kspec[i]);

	// check for malformed keylist filename
	err = 0;
	if(strncmp(kspec,"kl",2)) err++;
	if(*(kspec+8)=='.' && (strlen(kspec)<10 || strlen(kspec)>12)) err++;
	if(*(kspec+8)!='.' && strlen(kspec)!=8) err++;
	if(err) {
		cerr << "Invalid keyfile specification.\n";
		exit(1);
	}
	// verify that the keypair index is in range
	strcpy(str,argv[3]);
	temp=atoi(str);
	if(temp<1 || temp>MAXKEYS) {
		cerr << "Keypair index is not in the interval 01 to 55.\n";
		exit(1);
	}
	// prefix @ to keypair index in preparation for initial keypair search
	// in function getseeds()
	sprintf(str,"@%02d",temp);

	// set the default iterations count
	iter=4;

	// check for redefinition of iterations count
	if(argc==5) {
		temp=atoi(argv[4]);
		if(temp<2 || temp>12) {
			cerr << "Iterations count is not in the interval 02 to 12.\n";
			exit(1);
		}
		iter=temp;
	}
	// attempt to open target file
	if((fp=fopen(fspec,"r+b"))==NULL) {
		cerr << "Input file "<< fspec << " not found." << "\n";
		exit(1);
	}
	// attempt open transaction log.  If TRLOG is protected by file
	// attribute, warn user and require confirmation to proceed
	if((tr=fopen("TRLOG","a"))==NULL) {
		Window inactive(LIGHTGRAY,DARKGRAY,24,10,55,13);
		if(inactive.warning()) {
		fclose(fp); // close the target file
		exit(1);	// user quits...
		}
	}
	// determine file size
	handle=fileno(fp);
	if((fsize=filelength(handle))==NULL) {
		cerr << fspec << " is a zero length file." << "\n";
		fclose(fp);			// close target file
		if(tr) fclose(tr);	// close transaction log if open
		exit(1);
	}
	// allocate file buffer
	filesize=fsize;
	if(fsize>MAXSIZE) fsize=MAXSIZE;
	unsigned char *buf = new unsigned char[fsize];	// allocation
	
	// extract keypair from keylist
	if((p=getseeds(kspec,str))==NULL) {
		delete [] buf;						// conditional deallocation
		exit(1);
	}
	strcpy(str,p);

	// clear file location pointer and key character selection toggle
	loc=xorsw=0;

	// translation loop -- stay in loop until bcnt = 0 is returned from
	// the fread(...) function
	for(;;) {
		bcnt=fread(buf,sizeof(char),fsize,fp);
		if(!bcnt) break;

		// Nonlinear random sequence generator
		for(i=0; i<bcnt; i++) {
			for(j=0; j<iter; j++) {
				exchange = Rseed;
				temp1 = (exchange=(Rseed)) ^ *(lcvect+j);
				templ = temp1 & 0xFFFF;
				temph = temp1 >> 16;
				temp2 = templ*templ + ~(temph*temph);
				Rseed = Lseed ^ (((temp1=(temp2>>16) |
				((temp2 & 0xFFFF)<<16)) ^ *(rcvect+j)) + templ*temph);
				Lseed = exchange;
			}
            // translate selected byte value
			xorsw^=1;
			if(xorsw) *(buf+i) ^= (unsigned char) (Lseed >> 8);
			else      *(buf+i) ^= (unsigned char) (Rseed >> 8);
		}
		// reset the file pointer
		fseek(fp,loc,SEEK_SET);

		// write translated block over last input block
		fwrite(buf,sizeof(unsigned char),bcnt,fp);
		loc=ftell(fp);
	}
	// exit procedures
	Lseed=Rseed=0L;
	fclose(fp);
	for(i=0; i<fsize; i++) *(buf+i) ^= *(buf+i);
	delete [] buf;				// deallocation

	// compute relative entropy of translated file
	entropy();

	// make transaction log entry if TRLOG file is open
	if(tr) {
		datetime();
		for(i=0; i<strlen(fspec); i++) fspec[i] = tolower(fspec[i]);
		fprintf(tr," translated %s %ld bytes\n         %s %s %s %.9f [%02d]\n",
				fspec,filesize,datebuf,kspec,str,Hrel,iter);
		fclose(tr);
	}
	// Done
	exit(0);
}
char *getseeds(char *kspec,char *str)
{
	// extract keypair referenced by user specified keypair index -- this is
	// where previously used keypairs are detected.  Search strategy:
	//
	//  1.  Keylist file headers contain absolute paths of possible variable
	//      lengths.  Variable length headers negate the possiblity of computing
	//      keypair offsets from the top of the file.  Therefore, it is required
	//      to search for a match on the specified keypair index.
	//
	//  2.  Search for keypair @xx (xx in the range of 01 to 55).  If found,
	//      seed the nonlinear random sequence generator with the corresponding
	//      keypair and exit.  If a match is not found...
	//
	//  3.  Replace the @ symbol with a hyphen and search again.  If a match is
	//      found, popup the "previously used keypair" confirmation window:
	//
	//          a.  If the user elects to proceed, seed the sequence generator
	//              using the indexed keypair and exit.  Otherwise,
	//
	//          b.  abort the translation run.
	//
	//  Note:  The last entry in a keylist file is the value of the iterations
	//  count in square brackets, e.g., [04].  File search is terminated upon
	//  encountering a left bracket.  If a match is not found during the second
	//  search, the keylist file is declared to be corrupted (the keypair index
	//  was verified to be in range but could not be found in the file).
	//-------------------------------------------------------------------------

	FILE *fp;						// keylist file-pointer
	int kp;							// binary key pair index
	int handle;						// keylist file handle
	unsigned int fsize;				// keylist filesize
	unsigned char seed1[11];		// 1st generator ASCII seed
	unsigned char seed2[11];		// 2nd generator ASCII seed
	char *p,*k;						// scratch pointers

	seed1[11]='\0';
	seed2[11]='\0';

	// skip over the @ symbol and convert the keypair index to internal binary
	kp=atoi(str+1);

	// verify that the keypair index is in range
	if(kp<1 || kp >55) {
		cout << "Invalid str index.\n";
		return(NULL);
	}
	// attempt to open the specified keypair file
	if((fp=fopen(kspec,"r+b"))==NULL) {
		cerr << "Keylist file " << kspec << " not found.\n";
		return(NULL);
	}
	// determine the file size
	handle=fileno(fp);
	fsize=filelength(handle);

	// allocate a buffer to hold the keyfile
	char *kbuf = new char[fsize];				// allocation

	// get the keyfile
	if((fread(kbuf,sizeof(char),fsize,fp))==NULL) {
		cerr << "Error reading keyfile";
		delete [] kbuf;				// conditional deallocation
		return(NULL);	
	}
	// search for "@xx"
	p=kbuf;
	while(*p!='[' && strncmp(str,p,3)) p++;
	if(*p=='[') {

		// search again, this time for "-xx"
		*str='-';
		p=kbuf;
		while(*p!='[' && strncmp(str,p,3)) p++;

		// if no match here then file is corrupt
		if(*p=='[') {
			cout << "Keylist has been corrupted.\n";
			delete [] kbuf;		// conditional deallocation
			return(NULL);
		}
	}
	// matched on previously used keypair -- popup confirmation window
	if(*str=='-') {
		Window besure(LIGHTGRAY,DARKGRAY,22,10,58,13);
		if(besure.confirm()) {
			delete [] kbuf;			// conditional deallocation
			return(NULL);			// user quits
		}
	}
	// ensure that this keypair is marked as "previously used"
	if(*p=='@') *p='-';

	// skip over to beginning of first seed
	p+=5;

	// extract seeds
	k=seed1;
	while(*p!=' ') *(k++)=*(p++);
	p++;
	k=seed2;
	while(*p!=' ') *(k++)=*(p++);

	// seed sequence generator
	Lseed=atol(seed1);
	Rseed=atol(seed2);

	// update the keypair file and shut it down
	rewind(fp);
	fwrite(kbuf,sizeof(char),fsize,fp);
	fclose(fp);
	delete [] kbuf;		// deallocation

	// check for diagnostic output of the extracted keypair
	if(globalflag) {
		cout << "Lseed " << Lseed << "\n";
		cout << "Rseed " << Rseed << "\n";
	}
	// return an ASCII copy of the keypair index for output
	// to transaction log
	return(str);
}
void datetime()
{
	_dos_getdate(&dd);
	gettime(&tt);
	sprintf(datebuf,"%02d-%02d-%d ",dd.month,dd.day,dd.year);
	sprintf(timebuf,"%02d:%02d:%02d",tt.ti_hour,tt.ti_min,tt.ti_sec);
	strcat(datebuf,timebuf);
}
void entropy()
{
	// Compute the relative entropy of finite schemes derived from disk file
	// contents for inclusion in transaction log entries.  Relative entropy is
	// used to report on the textual state of the translated file:
	//
	//                 Textual State              Hrel
	//              --------------------      -----------
	//              plaintext TEXT file       0.60 - 0.65
	//              plaintext BINARY file     0.80 - 0.85
	//              ciphertext ALL files      0.99...
	//
	// Values of relative entropy shown in the above table are accurate for files
	// of approximately 1.2K bytes in length and larger.
	//
	// Relative entropy is an example of a one-way hash function.  The file
	// content (of any type or size of file) is hashed to its relative entropy
	// and displayed to nine places after the decimal point.  Reporting relative
	// entropy to nine places after the decimal point ensures that a change in
	// polarity of 1-bit in eight million will be detected.  In addition to
	// providing the user with a means of determining the textual state of any
	// given file, relative entropy also (1) provides a file signature that can
	// be used as the basis of authentication schemes and (2) places theoretical
	// limits on file compression ratios when files are compressed by lossless
	// methods.
	//----------------------------------------------------------------------------

	FILE *fp;
	int i,j,index;
	unsigned int Pr[256];
	long int bcnt,fsize;
	unsigned long sum;
	int handle;
	double Hmax, H, S, k, P[256];

	// open target file
	if((fp=fopen(fspec,"rb"))==NULL) {
		cerr << fspec << " not found." << "\n";
		exit(1);
	}
	// get file size
	handle = fileno(fp);
	if((fsize=filelength(handle))==NULL) {
		cerr << fspec << " is a zero length file." << "\n";
		fclose(fp);
		exit(1);
		}
	// allocate file buffer
	if(fsize>MAXSIZE) fsize = MAXSIZE;
	unsigned char *buf = new unsigned char[fsize];	// allocation

	// initialize accumulation vectors
	for(i=0; i<256; i++) Pr[i]=0;
	for(i=0; i<256; i++) P[i]=0.0;

	// main processing loop
	for(;;) {

		// read next file segment and count the occurrence of each ASCII
		// character in the buffer
		bcnt = fread(buf,sizeof(char),fsize,fp);
		if(!bcnt) break;
			for(i=0; i<bcnt; i++) {
			index = (int) buf[i];
			Pr[index]++;
		}
		index = 0;

		// skip over zero entry (ASCII character not present in buffer),
		// otherwise count this character
		for(i=0; i<256; i++) {
			if(!Pr[i]) continue;
			P[index++] = (double) Pr[i];
		}
	}
	// create finite scheme -- this procedure normalizes an incomplete finite
	// scheme to produce a representation of a discrete probability space
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
	fclose(fp);
	delete [] buf;					// deallocation
}
int Window::warning()
{
	// Warn user that the transaction log file, TRLOG, is inactive.
	// TRLOG is inactive if it is assigned the READ_ONLY file attribute.

	char ans;

	// save cursor position and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create popup window
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p = new char[msize];
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	textbackground(LIGHTGRAY);
	textcolor(DARKGRAY);

	// display warning and prompt
	gotoxy(sx+2,sy+1);
	cprintf("Transaction log is inactive.");
	gotoxy(sx+9,sy+2);
	cprintf("Proceed (y/n)?");
	ans=getans();

	// restore screen and return user response
	puttext(sx,sy,ex,ey,p);
	delete [] p;					// deallocation
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
	return(ans);
}
int Window::confirm()
{
	// Inform user that a previously used keypair has been specified on the
	// command line.  Require confirmation to proceed.

	char ans;

	// save cursor position and remove cursor
	savex=wherex();
	savey=wherey();
	_setcursortype(_NOCURSOR);

	// create popup window
	msize=2*(ex-sx+1)*(ey-sy+1);
	char *p=new char[msize];				// allocation
	gettext(sx,sy,ex,ey,p);
	wscroll(6,LIGHTGRAY,0,sx,sy,ex,ey);
	border(LIGHTGRAY,DARKGRAY,sx,sy,ex,ey);
	textbackground(LIGHTGRAY);
	textcolor(DARKGRAY);

	// display confirmation prompt
	gotoxy(sx+2,sy+1);
	cprintf("Previously used keypair detected.");
	gotoxy(sx+10,sy+2);
	cprintf("Proceed (y/n)?");
	ans=getans();

	// restore screen and return user response
	puttext(sx,sy,ex,ey,p);
	delete []  p;								// deallocation
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
	return(ans);
}
int getans()
{
	// get user response to (y/n)? prompts

	char ans;

	for(;;) {
		ans=tolower(getch());
		if(ans=='y') return(0);
		if(ans=='n') return(1);
	}
}
int getpath(char *fspec)
{
	// Derive the absolute path of the user supplied file specification.  The
	// absolute path is required to avoid ambiguities in transaction log entries.

	int i;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char file[_MAX_FNAME];
	char ext[_MAX_EXT];
	char temp[_MAX_DIR]="\\";
	char *p;

	// ensure all upper case
	for(i=0; i<strlen(fspec); i++)
		fspec[i] = toupper(fspec[i]);

	// disallow wildcards
	if(strchr(fspec,'*') || strchr(fspec,'?')) {
		cout << "Wildcards not permitted in OTP translation\n";
		return(FALSE);
	}
	// split the path to gain access to path components
	_splitpath(fspec,drive,dir,file,ext);
	if(strlen(drive)) {
		if(!strlen(dir)) strcpy(dir,"\\");
		if(strlen(dir) && *dir!='\\') {
			strcat(temp,dir);
			strcpy(dir,temp);
		}
	}
	// put it back together
	_makepath(fspec,drive,dir,file,ext);

	//allocate a temporary buffer and derive the absolute path
	char *buf = new char[80];			// allocation
	_fullpath(buf,fspec,_MAX_PATH);
	strcpy(fspec,buf);

	//----------------------------------------------------------------------
	// use DOS to validate path -- if it's possible to change to the derived
	// path then it must be valid, otherwise it isn't
	//----------------------------------------------------------------------

	// get pointer to end of temporary path buffer
	p=buf+strlen(buf);

	// scan backward for back slash
	while(*p!='\\' && p>buf) p--;

	// since p points to a derived absolute path, a back slash must be
	// present -- null terminate the path string (i.e., strip off the
	// filename) to allow subsequent paths comparison
	*p='\0';

	// check for absolute path = current working directory path -- if equal
	// then nothing to do -- if not equal then validate...
	if(strcmp(buf,cwdbuf)) {

		// attempt to change to the derived absolute path
		if(chdir(buf)) {

			// attempt failed -- inform user and bail out
			cout << "Invalid path or file(s) not found\n";
			delete [] buf;			// conditional deallocation
			return(FALSE);
		}
		// attempt succeeded so derived path must be valid...
		// restore current working directory
		chdir(cwdbuf);
	}
	// Done
	delete [] buf;					// deallocation
	return(TRUE);
}
void wscroll(int dir,int attr,int count,int sx,int sy,int ex,int ey)
{
	// BIOS scroll

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
	// Draw border.  bg and fg specify foreground and background colors,
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
	// Display utility summary screen.

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

	// display summary usage text
	x=4; y=2;
	gotoxy(x,y++);
	cprintf("                 CODEWORD OTP V1.1 VERNAM CIPHER UTILITY");
	y++;
	textcolor(CYAN);
	gotoxy(x,y++);
	cprintf("                       Vernam Cipher (sscv04.exe)");
	y++;
	gotoxy(x,y++);
	cprintf("Usage:   C:\\K>v fspec kspec key# {iter}");
	y++;
	gotoxy(x,y++);
	cprintf("         - v.bat is used to submit sscv04.exe directives");
	gotoxy(x,y++);
	cprintf("         - fspec names the file to be encrypted or decrypted");
	gotoxy(x,y++);
	cprintf("         - kspec names the keylist file to be used");
	gotoxy(x,y++);
	cprintf("         - key# specifies the keypair index in the named keylist");
	gotoxy(x,y++);
	cprintf("         - {iter} is an optional NLRNG iteration count: 2 to 12");
	gotoxy(x,y++);
	cprintf("           (4 is default).");
	y++;
	gotoxy(x,y++);
	cprintf("Example:   C:\\K>v text.doc KL309275 37");
	y++;
	gotoxy(x,y++);
	cprintf("Example:   C:\\K>v d:\\mydir\\myfile.ext KL284619 42 10");
	y++;
	gotoxy(x,y++);
	cprintf("           Run g.exe without command line parameters for details");
	gotoxy(x,y++);
	cprintf("           concerning creating OTP V1.1 keylist files.");
	y++;
	textcolor(WHITE);
	gotoxy(x,y++);
	cprintf("                        Press any key to exit...");
	getch();

	// restore screen and return
	textbackground(BLACK);
	textcolor(LIGHTGRAY);
	puttext(sx,sy,ex,ey,p);
	delete [] p;					// deallocation
	gotoxy(savex,savey);
	_setcursortype(_NORMALCURSOR);
}
void allocError()
{
	// inform user that a memory allocation error has occurred

	cout << "Memory allocation error.\n";
	exit(1);
}
