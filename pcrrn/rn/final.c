/* $Header: final.c,v 4.3 85/05/01 11:38:08 lwall Exp $
 *
 * $Log:	final.c,v $
 * Revision 4.3  85/05/01  11:38:08  lwall
 * Baseline for release with 4.3bsd.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "term.h"
#include "ng.h"
#include "init.h"
#include "bits.h"
#include "last.h"
#include "rcstuff.h"
#include "ngdata.h"
#include "artio.h"
#include "INTERN.h"
#include "final.h"

void
final_init()
{
#ifdef SIGTSTP
    sigset(SIGTSTP, stop_catcher);	/* job control signals */
    sigset(SIGCONT, cont_catcher);	/* job control signals */
#endif

    sigset(SIGINT, int_catcher);	/* always catch interrupts */
    sigset(SIGFPE, sig_catcher);
#ifdef SIGXCPU
    sigset(SIGXCPU, sig_catcher);
#endif
#ifdef SIGXFSZ
    sigset(SIGXFSZ, sig_catcher);
#endif
}

void					/* very much void */
finalize(status)
int status;
{
#ifdef SERVER
    char artname[32];
#endif SERVER

    UNLINK(lockname);
#ifdef SERVER
    if (openart > 0) {
	sprintf(artname, "i:/tmp/rrn%ld.art", openart);
        UNLINK(artname);
    }
    UNLINK(active_name);
    close_server();
#endif SERVER
    exit(status);
}

/* come here on interrupt */

int
int_catcher()
{
    sigset(SIGINT,int_catcher);
#ifdef DEBUGGING
    if (debug)
	write(2,"int_catcher\n",12);
#endif
    if (!waiting) {
	if (int_count) {		/* was there already an interrupt? */
	    write(2,"\nBye-bye.\n",10);
	    sig_catcher(0);		/* emulate the other signals */
	}
	int_count++;
    }
}

/* come here on signal other than interrupt, stop, or cont */

int
sig_catcher(signo)
{
#ifdef VERBOSE
    static char *signame[] = {
	"",
	"HUP",
	"INT",
	"QUIT",
	"ILL",
	"TRAP",
	"IOT",
	"EMT",
	"FPE",
	"KILL",
	"BUS",
	"SEGV",
	"SYS",
	"PIPE",
	"ALRM",
	"TERM",
	"???"
#ifdef SIGTSTP
	,"STOP",
	"TSTP",
	"CONT",
	"CHLD",
	"TTIN",
	"TTOU",
	"TINT",
	"XCPU",
	"XFSZ"
#ifdef SIGPROF
	,"VTALARM",
	"PROF"
#endif
#endif
	};
#endif

#ifdef SIGTTOU
#ifndef lint
    sigignore(SIGTTOU);
#endif lint
#endif
#ifdef DEBUGGING
    if (debug) {
	printf("\nSIG%s--.newsrc not restored in debug\n",signame[signo]);
	finalize(-1);
    }
#endif
    if (panic)
	abort();
    panic = TRUE;			/* disable terminal I/O */
    if (doing_ng) {			/* need we reconstitute rc line? */
	yankback();
	restore_ng();			/* then do so (hope this works) */
    }
    doing_ng = FALSE;
    if (rc_changed)			/* need we write .newsrc out? */
	write_rc();			/* then do so */
    rc_changed = FALSE;
#ifdef VERBOSE
	IF(verbose)
	    printf("\nCaught %s%s--.newsrc restored\n",
		signo ? "a SIG" : "an internal error", signame[signo]);
	ELSE
#endif
#ifdef TERSE
	    printf("\nSignal %d--bye bye\n",signo);
#endif
    finalize(1);				/* and blow up */
}

#ifdef SIGTSTP
/* come here on stop signal */

int
stop_catcher()
{
    if (!waiting) {
	checkpoint_rc();		/* good chance of crash while stopped */
#ifdef DEBUGGING
	if (debug)
	    write(2,"stop_catcher\n",13);
#endif
	sigset(SIGTSTP,SIG_DFL);	/* enable stop */
#ifdef BSD42
	sigsetmask(sigblock(0) & ~(1 << (SIGTSTP-1)));
#endif
	kill(0,SIGTSTP);		/* and do the stop */
    }
    sigset(SIGTSTP,stop_catcher);	/* unenable the stop */
}

/* come here on cont signal */

int
cont_catcher()
{
    sigset(SIGCONT,cont_catcher);
    savetty();
#ifdef MAILCALL;
    mailcount = 0;			/* force recheck */
#endif
    if (!panic) {
	if (!waiting) {
#ifdef DEBUGGING
	    if (debug)
		write(2,"cont_catcher\n",13);
#endif
	    forceme("\f");		/* cause a refresh */
					/* (defined only if TIOCSTI defined) */
	}
    }
}
#endif

