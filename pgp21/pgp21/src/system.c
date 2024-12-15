/*
 * system.c
 *
 * Routines specific for non-MSDOS implementations of pgp.
 * 
 *	Modified 24-Jun-92 HAJK
 *	Adapt for VAX/VMS.
 *
 *	Modified: 11-Nov-92 HAJK
 *	Add FDL Support Routines. 
 */
#include <stdio.h>
#ifdef VMS
#include <file.h>
#include "pgp.h"
#include "mpilib.h"
#include "mpiio.h"
#include "fileio.h"
extern byte textbuf[DISKBUFSIZE];   /*	Defined in FILEIO.C */
#endif /* VMS */
/*===========================================================================*/
/*
 * UNIX
 */

#ifdef UNIX
/*
 * Define USE_SELECT to use the select() system call to check if
 * keyboard input is available. Define USE_NBIO to use non-blocking
 * read(). If you don't define anything the FIONREAD ioctl() command
 * will be used.
 *
 * Define NOTERMIO if you don't have the termios stuff
 */
#include <sys/types.h>
#include <fcntl.h>
#if !defined(MACH) && !defined(BSD)
#include <unistd.h>
#endif

#ifndef	NOTERMIO
#ifndef SVR2
#include <termios.h>
#else
#include <termio.h>
#endif /* not SVR2 */
#else
#include <sgtty.h>
#endif

#ifdef	USE_SELECT
#include <sys/time.h>
#else
#ifndef USE_NBIO
#ifndef sun
#include <sys/ioctl.h>		/* for FIONREAD */
#else /* including both ioctl.h and termios.h gives a lot of warnings on sun */
#include <sys/filio.h>
#endif /* sun */
#ifndef FIONREAD
#define	FIONREAD	TIOCINQ
#endif
#endif
#endif
#include <signal.h>

static void setsigs(void);
static void rmsigs(void);
static void sig1(int);
static void sig2(int);
void breakHandler(int);
static int ttyfd= -1;
#ifndef SVR2
static void (*savesig)(int);
#else
static int (*savesig)(int);
#endif

void ttycbreak(void);
void ttynorm(void);


#ifndef NOTERMIO
#ifndef SVR2
static struct termios itio, tio;
#else
static struct termio itio, tio;
#endif /* not SVR2 */
#else
static struct sgttyb isg, sg;
#endif

#ifdef USE_NBIO
static int kbuf= -1;	/* buffer to store char read by kbhit() */
static int fflags;
#endif

static int gottio = 0;

void ttycbreak(void)
{
	if (ttyfd == -1) {
		if ((ttyfd = open("/dev/tty", O_RDWR)) < 0) {
		    fprintf(stderr, "cannot open tty, using stdin\n");
			ttyfd = 0;
		}
	}
#ifndef NOTERMIO
#ifndef SVR2
	if (tcgetattr(ttyfd, &tio) < 0)
#else
	if (ioctl(ttyfd, TCGETA, &tio) < 0)
#endif  /* not SVR2 */
	{	fprintf (stderr, "\nUnable to get terminal characteristics: ");
		perror("ioctl");
		exitPGP(1);
	}
	itio = tio;
	setsigs();
	gottio = 1;
#ifdef USE_NBIO
	tio.c_cc[VMIN] = 0;
#else
	tio.c_cc[VMIN] = 1;
#endif
	tio.c_cc[VTIME] = 0;
	tio.c_lflag &= ~(ECHO|ICANON);
#ifndef SVR2
	tcsetattr (ttyfd, TCSAFLUSH, &tio);
#else
	ioctl(ttyfd, TCSETAW, &tio);
#endif /* not SVR2 */
#else
    if (ioctl(ttyfd, TIOCGETP, &sg) < 0)
	{	fprintf (stderr, "\nUnable to get terminal characteristics: ");
		perror("ioctl");
		exitPGP(1);
	}
	isg = sg;
	setsigs();
	gottio = 1;
#ifdef CBREAK
    sg.sg_flags |= CBREAK;
#else
    sg.sg_flags |= RAW;
#endif
	sg.sg_flags &= ~ECHO;
    ioctl(ttyfd, TIOCSETP, &sg);
#endif	/* !NOTERMIO */
#ifdef USE_NBIO
#ifndef O_NDELAY
#define	O_NDELAY	O_NONBLOCK
#endif
	if ((fflags = fcntl(ttyfd, F_GETFL, 0)) != -1)
		fcntl(ttyfd, F_SETFL, fflags|O_NDELAY);
#endif
}


void ttynorm(void)
{	gottio = 0;
#ifdef USE_NBIO
	if (fcntl(ttyfd, F_SETFL, fflags) == -1)
		perror("fcntl");
#endif
#ifndef NOTERMIO
#ifndef SVR2
	tcsetattr (ttyfd, TCSAFLUSH, &itio);
#else
	ioctl(ttyfd, TCSETAW, &itio);
#endif /* not SVR2 */
#else
    ioctl(ttyfd, TIOCSETP, &isg);
#endif
	rmsigs();
}

static void sig1 (int sig)
{
#ifndef NOTERMIO
#ifndef SVR2
	tcsetattr (ttyfd, TCSANOW, &itio);
#else
	ioctl(ttyfd, TCSETAW, &itio);
#endif /* not SVR2 */
#else
    ioctl(ttyfd, TIOCSETP, &isg);
#endif
	signal (sig, SIG_DFL);
	if (sig == SIGINT)
		breakHandler(SIGINT);
	kill (getpid(), sig);
}

static void sig2 (int sig)
{	if (gottio)
		ttycbreak();
	else
		setsigs();
}

static void setsigs(void)
{	savesig = signal (SIGINT, sig1);
#ifdef	SIGTSTP
	signal (SIGCONT, sig2);
	signal (SIGTSTP, sig1);
#endif
}

static void rmsigs(void)
{	signal (SIGINT, savesig);
#ifdef	SIGTSTP
	signal (SIGCONT, SIG_DFL);
	signal (SIGTSTP, SIG_DFL);
#endif
}

#ifndef CRUDE
int kbhit(void)
/* Return TRUE if there is a key to be read */
{
#ifdef USE_SELECT		/* use select() system call */
	struct timeval t;
	fd_set n;
	int r;

	timerclear(&t);
	FD_ZERO(&n);
	FD_SET(ttyfd, &n);
	r = select(32, &n, NULL, NULL, &t);
	if (r == -1) {
		perror("select");
		exitPGP(1);
	}
	return r > 0;
#else
#ifdef	USE_NBIO		/* use non-blocking read() */
	unsigned char ch;
	if (kbuf >= 0) 
		return(1);
	if (read(ttyfd, &ch, 1) == 1) {
		kbuf = ch;
		return(1);
	}
	return(0);
#else
	long lf;
	if (ioctl(ttyfd, FIONREAD, &lf) == -1) {
		perror("ioctl: FIONREAD");
		exitPGP(1);
	}
	return(lf);
#endif
#endif
}
#endif	/* !CRUDE */

int getch(void)
{
	char c;
#ifdef USE_NBIO
	while (!kbhit());	/* kbhit() does the reading */
	c = kbuf;
	kbuf = -1;
#else
	read(ttyfd, &c, 1);
#endif
	return(c);
}

#ifdef BSD
VOID *memset(s, c, n)
VOID *s;
register int c, n;
{
	register char *p = s;
	++n;
	while (--n)
		*p++ = c;
	return(s);
}
int memcmp(s1, s2, n)
register unsigned char *s1, *s2;
register int n;
{
	if (!n)
		return(0);
	while (--n && *s1 == *s2) {
		++s1;
		++s2;
	}
	return(*s1 - *s2);
}
VOID *memcpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
	char *p = s1;
	++n;
	while (--n)
		*s1++ = *s2++;
	return(p);
}
#endif /* BSD */

#if defined(MACH) || defined(SVR2) || defined(BSD)
int remove(name)
char *name;
{
	return unlink(name);
}
#endif

#ifdef SVR2
int rename(old, new)
register char *old, *new;
{
	unlink(new);
	if (link(old, new) < 0)
		return -1;
	if (unlink(old) < 0) {
		unlink(new);
		return -1;
	}
	return 0;
}
#endif /* SVR2 */

/* not all unices have clock() */
long
Clock()	/* not a replacement for clock(), just for random number generation */
{
#if defined(BSD) || defined(sun) || defined(MACH) || defined(linux)
#include <sys/time.h>
#include <sys/resource.h>
	struct rusage ru;

	getrusage(RUSAGE_SELF, &ru);
	return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec +
		ru.ru_stime.tv_sec + ru.ru_stime.tv_usec +
		ru.ru_minflt + ru.ru_majflt +
		ru.ru_inblock + ru.ru_oublock +
		ru.ru_maxrss + ru.ru_nvcsw + ru.ru_nivcsw;

#else	/* no getrusage() */
#include <sys/times.h>
	struct tms tms;

	times(&tms);
	return(tms.tms_utime + tms.tms_stime);
#endif
}
#endif /* UNIX */


/*===========================================================================*/
/*
 * VMS
 */

#ifdef VMS			/* kbhit()/getch() equivalent */

/*
 * This code defines an equivalent version of kbhit() and getch() for
 * use under VAX/VMS, together with an exit handler to reset terminal
 * characteristics.
 *
 * This code assumes that kbhit() has been invoked to test that there
 * are characters in the typeahead buffer before getch() is invoked to
 * get the answer.
 */

#include <descrip.h>
#include <devdef>
#include <iodef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <dcdef.h>
#include <rms.h>

#define FDL$M_FDL_STRING    2		/* Use string for fdl text */
#define FDLSIZE		    4096	/* Maximum possible file size */

static volatile short	_kbhitChan_ = 0;

static volatile struct IOSB {
	unsigned short sts;
	unsigned short byteCount;
	unsigned short terminator;
	unsigned short terminatorSize;
	} iosb;

static $DESCRIPTOR (kbdev_desc, "SYS$COMMAND:");

static volatile struct {
	char Class;
	char Type;
	unsigned short BufferSize;
	unsigned int Mode;
	int ExtChar;
  } CharBuf, OldCharBuf;

static $DESCRIPTOR (out_file_descr, "SYS$DISK:[]"); /* Default Output File Descr */

static int flags = FDL$M_FDL_STRING;


/*
 * **-kbhit_handler-This exit handler restores the terminal characteristics
 *
 * Description:
 *
 * This procedure is invoked to return the the terminal to normality (depends
 * on what you think is normal!). Anyway, it gets called to restore
 * characteristics either through ttynorm or via an exit handler.
 */
void kbhit_handler(int *sts)
{
  int mysts;

  CharBuf.Mode = OldCharBuf.Mode;
  CharBuf.ExtChar = OldCharBuf.ExtChar;
  CharBuf.Mode &= ~TT$M_NOECHO;
  CharBuf.ExtChar &= ~TT2$M_PASTHRU;
  if ((mysts = sys$qiow (
			   0,
			   _kbhitChan_,
			   IO$_SETMODE,
			   &iosb,
			   0,
			   0,
			   &CharBuf,
			   12,
			   0,
			   0,
			   0,
			   0)) & 01) mysts = iosb.sts;
  (void) sys$dassgn (
	  _kbhitChan_);
  _kbhitChan_ = 0;
  if (!(mysts & 01)) {
	fprintf(stderr,"\nFailed to reset terminal characteristics!");
	(void) lib$signal(mysts);
  }
}


unsigned int exsts;

static struct {
	int link;
	void *rtn;
	int argcnt;
	int *stsaddr;
   } exhblk = { 0, &(kbhit_handler), 1, &(exsts)};

/*
 * **-kbhit-Find out if a key has been pressed
 *
 * Description:
 *
 * Make the terminal noecho and sense the characters coming in by looking at
 * the typeahead count.
 */
int kbhit()
{
  int sts = 1;

  struct {
	unsigned short TypAhdCnt;
	char FirstChar;
	char Reserved[5];
  } TypCharBuf;

  if (_kbhitChan_ == 0) {
	if ((sts = sys$assign (
			   &kbdev_desc,
			   &_kbhitChan_,
			   0,
			   0)) & 1 == 0) lib$stop(sts);
	if ((sts = sys$qiow (
			   0,
			   _kbhitChan_,
			   IO$_SENSEMODE,
			   &iosb,
			   0,
			   0,
			   &CharBuf,
			   12,
			   0,
			   0,
			   0,
			   0)) & 01) sts = iosb.sts;
	if (sts & 01) {
	  if (!(CharBuf.Class & DC$_TERM)) {
	    fprintf(stderr,"\nNot running on a terminal");
	    exitPGP(1);
	  } else {
	    OldCharBuf.Mode = CharBuf.Mode;
	    OldCharBuf.ExtChar = CharBuf.ExtChar;
	    CharBuf.Mode |= TT$M_NOECHO;
	    CharBuf.ExtChar |= TT2$M_PASTHRU;
	    if ((sts = sys$qiow (
			   0,
			   _kbhitChan_,
			   IO$_SETMODE,
			   &iosb,
			   0,
			   0,
			   &CharBuf,
			   12,
			   0,
			   0,
			   0,
			   0)) & 01) sts = iosb.sts;
	    if (sts & 01) {
	  /*
	  **  Declare Exit Handler
	  */
	       (void) sys$dclexh (&exhblk);
	    } else {
              fprintf(stderr,"\nFailed to set terminal characteristics!");
	      (void) lib$signal(sts);
              exitPGP(1);
            }
	  }
	}
  }
  /*
  **  Get typeahead count
  */
  if ((sts = sys$qiow (
			   0,
			   _kbhitChan_,
			   IO$_SENSEMODE | IO$M_TYPEAHDCNT,
			   &iosb,
			   0,
			   0,
			   &TypCharBuf,
			   8,
			   0,
			   0,
			   0,
			   0)) & 01) sts = iosb.sts;
  if (sts & 01) return(TypCharBuf.TypAhdCnt>0);
  (void) lib$signal(sts);
  exitPGP(1);
}

/*
 * **-getch-Get a character and return it
 *
 * Description:
 *
 * Get a character from the keyboard and return it.
 */
int getch()
{
  unsigned int sts;
  volatile char CharBuf;

  if ((sts = sys$qiow (
			  0,
			  _kbhitChan_,
			  IO$_READVBLK,
			  &iosb,
			  0,
			  0,
			  &CharBuf,
			  1,
			  0,
			  0,
			  0,
			  0)) & 01) sts = iosb.sts;
  if (sts & 01) return ((int) CharBuf);
  fprintf(stderr,"\nFailed to get character");
  (void) lib$signal(sts);
}

ttynorm()
{
  int sts;

  if (_kbhitChan_ != 0) {
    (void) SYS$CANEXH(&exhblk);
    kbhit_handler(&sts);
  }
}

void ttycbreak ()
{
   ttynorm();
}

unsigned long	vms_clock_bits[2];	/* VMS Hardware Clock */
const long	vms_ticks_per_update = 100000L; /* Clock update int. */

/*
 * FDL Stuff For Getting & Setting File Characteristics
 * This code was derived (loosely!) from the module LZVIO.C in the public 
 * domain LZW compress routine as found on the DECUS VAX SIG tapes (no author
 * given, so no credits!) 
 */

/*
 * **-fdl_generate-Generate An FDL
 *
 * Description:
 *
 * This procedure takes the name of an existing file as input and creates
 * an fdl. The FDL is retuned by pointer and length. The FDL space should be
 * released after use with a call to free();
 */
int fdl_generate(char *in_file, char **fdl, short *len)
/*
 * Arguments:
 *
 *	in_file	    char*   Filename of file to examine (Zero terminated).
 *
 *	fdl	    char*   Pointer to FDL that was created.
 *
 *	len	    short   Length of FDL created.
 *
 * Status Returns:
 *
 * VMS style. lower bit set means success.
 */
{

    struct dsc$descriptor fdl_descr = { 0,
				DSC$K_DTYPE_T,
				DSC$K_CLASS_D,
				0};
    struct FAB fab, *fab_addr;
    struct RAB rab, *rab_addr;
    struct NAM nam;
    struct XABFHC xab;
    int sts;
    int badblk;

/*
 * Build FDL Descriptor
 */
    if (!(sts = str$get1_dx(&FDLSIZE,&fdl_descr)) & 01) return(0);
/*
 * Build RMS Data Structures
 */
    fab = cc$rms_fab;
    fab_addr = &fab;
    nam = cc$rms_nam;
    rab = cc$rms_rab;
    rab_addr = &rab;
    xab = cc$rms_xabfhc;
    fab.fab$l_nam = &nam;
    fab.fab$l_xab = &xab;
    fab.fab$l_fna = in_file;
    fab.fab$b_fns = strlen(in_file);
    rab.rab$l_fab = &fab;
    fab.fab$b_fac = FAB$M_GET | FAB$M_BIO; /* This open block mode only */
/*
 * Attempt to Open File
 */
    if (!((sts = sys$open(&fab)) & 01)) {
	if (verbose) {
	    fprintf(stderr,"\n(SYSTEM) Failed to $OPEN %s\n",in_file);
	    (void) lib$signal(fab.fab$l_sts,fab.fab$l_stv);
	}
	return(sts);
    }
    if (fab.fab$l_dev & DEV$M_REC) {
	fprintf(stderr,"\n(SYSTEM) Attempt to read from output only device\n");
	sts = 0;
    } else {
	rab.rab$l_rop = RAB$M_BIO;
	if (!((sts = sys$connect(&rab)) & 01)) {
	    if (verbose) {
		fprintf(stderr,"\n(SYSTEM) Failed to $CONNECT %s\n",in_file);
		(void) lib$signal(fab.fab$l_sts,fab.fab$l_stv);
	    }
	} else {
	    if (!((sts = fdl$generate(
			&flags,
			&fab_addr,
			&rab_addr,
			NULL,NULL,
			&fdl_descr,
			&badblk,
			len)) & 01)) {
		if (verbose)
		    fprintf(stderr,"\n(SYSTEM) Failed to generate FDL\n",in_file);
		free(fdl);
	    } else {
		if (!(*fdl = malloc(*len))) return(0);
		memcpy(*fdl,fdl_descr.dsc$a_pointer,*len);
	    }
	    (void) str$free1_dx(&fdl_descr);
	}
        sys$close(&fab);
    }
    return(sts);	    
}

/*	  
 * **-fdl_close-Closes files created by fdl_generate
 *  
 * Description:
 *
 * This procedure is invoked to close the file and release the data structures
 * allocated by fdl$parse.
 */
void fdl_close(void* rab)
/*
 * Arguments:
 *
 *	rab	void *	Pointer to RAB (voided to avoid problems for caller).
 *
 * Returns:
 *
 *	None.
 */
{
    struct FAB *fab;

    fab = ((struct RAB *) rab)->rab$l_fab;
    if (fab) {  /*  Close file if not already closed */
	if (fab->fab$w_ifi) sys$close(fab);
    }
    fdl$release( NULL, &rab);	  
}

/*
 * **-fdl_create-Create A File Using the recorded FDL (hope we get it right!)
 *
 * Description:
 *
 * This procedure accepts an FDL and uses it create a file. Unfortunately
 * there is no way we can easily patch into the back of the VAX C I/O
 * subsystem.
 */
void * fdl_create( char *fdl, short len, char *outfile, char *preserved_name)
/*
 * Arguments:
 *
 *	fdl	char*	FDL string descriptor.
 *
 *	len	short	Returned string length.
 *
 *	outfile	char*	Output filename.
 *
 *	preserved_name char*	Name from FDL.
 *
 * Returns:
 *
 *     0 in case of error, or otherwise the RAB pointer.
 */
{
    void *sts;
    int sts2;
    struct FAB *fab;
    struct RAB *rab;
    struct NAM nam;
    int badblk;
    char *resnam;

    struct dsc$descriptor fdl_descr = {
			    len,
			    DSC$K_DTYPE_T,
			    DSC$K_CLASS_S,
			    fdl
			    };

    sts = NULL;
/*
 * Initialize RMS NAM Block
 */
    nam = cc$rms_nam;
    nam.nam$b_rss = NAM$C_MAXRSS;
    nam.nam$b_ess = NAM$C_MAXRSS;
    if (!(resnam = nam.nam$l_esa = malloc(NAM$C_MAXRSS+1))) {
	fprintf(stderr,"\n(FDL_CREATE) Out of memory!\n");
	return(NULL);
    }
/*
 * Parse FDL
 */
    if (!((sts2 = fdl$parse( &fdl_descr,
				&fab,
				&rab,
				&flags)) & 01)) {
	fprintf(stderr,"\nCreating (fdl$parse)\n");
	(void) lib$signal(sts2);
    } else {
/*
 * Extract & Return Name of FDL Supplied Filename
 */
	memcpy (preserved_name,fab->fab$l_fna,fab->fab$b_fns);
	preserved_name[fab->fab$b_fns] = '\0';
/*
 * Set Name Of Temporary File
 */
	fab->fab$l_fna = outfile;
	fab->fab$b_fns = strlen(outfile);
/*
 * Connect NAM Block
 */
	fab->fab$l_nam = &nam;
	fab->fab$l_fop |= FAB$M_NAM | FAB$M_CIF;
	fab->fab$b_fac |= FAB$M_BIO | FAB$M_PUT;
/*
 * Create File
 */
	if (!(sys$create(fab) & 01)) {
	    fprintf(stderr,"\nCreating (RMS)\n");
	    (void) lib$signal(fab->fab$l_sts,fab->fab$l_stv);
	    fdl_close(rab);
	} else {
	    if (verbose) {
		resnam[nam.nam$b_esl+1] = '\0';
		fprintf(stderr,"\nCreated %s successfully\n",resnam);
	    }
	    rab->rab$l_rop = RAB$M_BIO;
	    if (!(sys$connect(rab) & 01)) {
		fprintf(stderr,"\nConnecting (RMS)\n");
		(void) lib$signal(rab->rab$l_sts,rab->rab$l_stv);
		fdl_close(rab);
	    } else sts = rab;
	}
	fab->fab$l_nam = 0; /* I allocated NAM block, so I must deallocate it! */
    }
    free(resnam);
    return(sts);		
}

/*
 * **-fdl_copyfile2bin-Copies the input file to a 'binary' output file
 *
 * Description:
 *
 * This procedure is invoked to copy from an opened file f to a file opened
 * directly through RMS. This allows us to make a block copy into one of the
 * many esoteric RMS file types thus preserving characteristics without blowing
 * up the C RTL. This code is based directly on copyfile from FILEIO.C.
 *
 * Calling Sequence:
 */
int fdl_copyfile2bin( FILE *f, void *rab, word32 longcount)
/*
 * Arguments:
 *
 *	f	    FILE*	Pointer to input file
 *
 *	rab	    RAB*	Pointer to output file RAB
 * 
 *	longcount   word32	Size of file
 *
 * Returns:
 *
 *	0   If we were successful.
 *	-1  We had an error on the input file (VAXCRTL).
 *	+1  We had an error on the output file (direct RMS).
 */
{
    int status = 0;
    word32 count;
    ((struct RAB *) rab)->rab$l_rbf = &textbuf;
    ((struct RAB *) rab)->rab$l_bkt = 0;
    do { /*  Read and write longcount bytes */
	if (longcount < (word32) DISKBUFSIZE)
	    count = longcount;
	else
	    count = DISKBUFSIZE;
	count = fread(textbuf,1,count,f);
	if (count > 0) {
/*	  
 *  No byte order conversion required, source and target system are both VMS so have
 *  the same byte ordering.
 */	  
	    ((struct RAB *) rab)->rab$w_rsz = (unsigned short) count;
	    if (!(sys$write (
		       rab, 
		       NULL, 
		       NULL) & 01)) {
		  lib$signal(((struct RAB *) rab)->rab$l_sts,((struct RAB *) rab)->rab$l_stv);
		  status = 1;
		  break;
	    }
	    longcount -= count;
	}
    } while (count==DISKBUFSIZE);
    burn(textbuf);
    return(status);
}
#endif /* VMS */


/*========================================================================*/
/*
 * AMIGA
 */

#ifdef AMIGA	/* Amiga-specific stuff */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <libraries/dosextens.h>
#ifdef LATTICE
#include <proto/exec.h> 
#include <proto/dos.h> 
#endif /* LATTICE */
extern FILE *pgpout;
extern int aecho;


/* amiga version of getch() 
   Cor Bosman , jul-22-92 
*/


sendpacket(struct MsgPort *rec,LONG action,LONG arg1) 
{
  struct StandardPacket *pkt;
  struct msgPort *rp;
  LONG res1 = 0L;

  if (rp = (struct MsgPort *)CreatePort(NULL,0L)) {
    if (pkt = (struct StandardPacket *)\
	 AllocMem(sizeof(struct StandardPacket),MEMF_PUBLIC|MEMF_CLEAR)) {
	   pkt->sp_Msg.mn_Node.ln_Name = (BYTE *)&pkt->sp_Pkt;
	   pkt->sp_Pkt.dp_Link = &pkt->sp_Msg;
	   pkt->sp_Pkt.dp_Port = rp;
	   pkt->sp_Pkt.dp_Type = action;
	   pkt->sp_Pkt.dp_Arg1 = arg1;
	   PutMsg(rec,&pkt->sp_Msg);
	   WaitPort(rp);
	   GetMsg(rp);
	   res1 = pkt->sp_Pkt.dp_Res1;
	   FreeMem((UBYTE*)pkt,sizeof(struct StandardPacket));
	 }
	 DeletePort(rp);
	}
	return(res1);

}

/* ttycbreak for amiga.
 * Cor Bosman , jul-30-92
*/

void ttycbreak()
{
  BPTR in,out;
  char buf[128];
  struct MsgPort *ch;

  in=Input();
  out=Output();
  ch = ((struct FileHandle *)BADDR(in))->fh_Type;
  sendpacket(ch,ACTION_SCREEN_MODE,-1L);
}

/* ttynorm for amiga
 * Cor Bosman , jul-30-92
*/

void ttynorm()
{

  BPTR in,out;
  char buf[128];
  struct MsgPort *ch;

  in=Input();
  out=Output();
  ch = ((struct FileHandle *)BADDR(in))->fh_Type;
  sendpacket(ch,ACTION_SCREEN_MODE,0L);
}

char getch(void)
{
  char buf[128];
  BPTR in,out;

  in = Input();
  out = Output();
  Read(in,buf,1);
  if (aecho) 
    Write(out, buf, 1);
  return(buf[0]);
}

/* kbhit() function for amiga.
 * Cor Bosman , jul-30-92
*/

int kbhit() 
{
  if(WaitForChar(Input(), 1)) return 1;
  return 0;
}

#ifdef LATTICE

/*
 *  Lattice-C  ^C-Handler 
*/

int CXBRK()
{
  BPTR in,out;
  struct MsgPort *ch;
  in=Input();
  out=Output();

  /* it might happen we catch a ^C while in cbreak mode.
   * so always set the screen to the normal mode.
  */

  ch = ((struct FileHandle *)BADDR(in))->fh_Type;
  sendpacket(ch, ACTION_SCREEN_MODE, 0L);


  fprintf(pgpout, "\n*** Program Aborted.\n");
  exitPGP(6); /* INTERRUPT */
}
#endif

/*------------------------------------------------------------------------
 * clock.c -- time in microseconds since first call of clock()
 *
 * RP: this function is missing from SAS/C library.
 */

#include <time.h>

long clock()
{
	static unsigned long oldms = -1;
	unsigned long cl[2],ms;

	timer(cl);
	ms = cl[0] * 1000000 + cl[1] % 1000000;
	if(oldms == -1) {
		oldms = ms;
		return 0;
	} else {
		return ((long)(ms-oldms));
	}
}


#endif /* AMIGA */



/*===========================================================================*/
/*
 * other stuff for non-MSDOS systems
 */

#ifdef ATARI
#include <string.h>
#endif

#if !defined(MSDOS) && !defined(ATARI)
#include <ctype.h>
char *strlwr(char *s)
{	/*
	**        Turns string s into lower case.
	*/
	int c;
	char *p = s;
	while (c = *p)
		*p++ = to_lower(c);
	return(s);
}
#endif /* !MSDOS && !ATARI */


#ifdef strstr
#undef strstr
/* Not implemented on some systems - return first instance of s2 in s1 */
char *mystrstr (char *s1, char *s2)
{	int i;
	char *strchr();

	if (!s2 || !*s2)
		return s1;
	for ( ; ; )
	{	if (!(s1 = strchr (s1, *s2)))
			return s1;
		for (i=1; s2[i] && (s1[i]==s2[i]); ++i)
			;
		if (!s2[i])
			return s1;
		++s1;
	}
}
#endif /* strstr */


#ifdef fopen
#undef fopen

#ifdef ATARI
#define F_BUF_SIZE 8192  /* seems to be a good value ... */

FILE *myfopen(const char *filename, const char *mode)
/* Open streams with larger buffer to increase disk I/O speed. */
/* Adjust F_BUF_SIZE to change buffer size.                    */
{
    FILE *f;

    if ( (f = fopen(filename, mode)) != NULL )
        if (setvbuf(f, NULL, _IOFBF, F_BUF_SIZE)) /* no memory? */
        {
            fclose(f);                 /* then close it again */
            f = fopen(filename, mode); /* and try again in normal mode */
        }
    return(f);                         /* return either handle or NULL */
}
	
#else /* ATARI */

/* Remove "b" from 2nd arg */
FILE *myfopen(char *filename, char *type)
{	char buf[10];

	buf[0] = *type++;
	if (*type=='b')
		++type;
	strcpy(buf+1,type);
	return fopen(filename, buf);
}
#endif /* not ATARI */
#endif /* fopen */


#ifndef MSDOS
#ifdef OS2

static int chr = -1;

int kbhit(void)
{
	if (chr == -1)
	  	chr = _read_kbd(0, 0, 0);
	return (chr != -1);
}

int getch(void)
{
	int c;

	if (chr >= 0) {
		c = chr;
		chr = -1;
	} else
	  	c = _read_kbd(0, 1, 0);

	return c;
}

#endif /* OS2 */
#endif /* MSDOS */
