
static char rcsid[] = "@(#)$Id: signals.c,v 5.13 1994/09/01 19:42:39 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.13 $   $State: Exp $
 *
 *			Copyright (c) 1988-1992 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 *******************************************************************************
 * $Log: signals.c,v $
 * Revision 5.13  1994/09/01  19:42:39  syd
 * Moved #defines for SETJMP & LONGJMP from src/editmsg.c to hdrs/defs.h and
 * used them instead of (set|long)jmp directly. This makes the usage consistent
 * and Solaris 2.3 behaves correctly w.r.t stop/continue.
 * From: Jan.Djarv@sa.erisoft.se (Jan Djarv)
 *
 * Revision 5.12  1994/08/30  18:59:38  syd
 * add sigcont to release list
 *
 * Revision 5.11  1994/08/30  18:27:47  syd
 * fix typo
 *
 * Revision 5.10  1994/08/30  18:16:05  syd
 * Fix up Solaris and SVR4 signal handling for timeout
 * From: Syd
 *
 * Revision 5.9  1993/08/03  19:10:50  syd
 * Patch for Elm 2.4 PL22 to correct handling of SIGWINCH signals on
 * DecStations with Ultrix 4.2.
 * The problem was that elm running in an xterm exits silently when the
 * window is resize. This was caused by incorrect signal handling for BSD.
 * From: vogt@isa.de
 *
 * Revision 5.8  1993/04/12  03:04:58  syd
 * The USR2 signal lost messages on some OS:es and did an unnecessary resync
 * on others.
 * From: Jan Djarv <Jan.Djarv@sa.erisoft.se>
 *
 * Revision 5.7  1993/01/20  03:43:37  syd
 * Some systems dont have SIGBUS, make it optional
 *
 * Revision 5.6  1992/12/11  02:39:53  syd
 * A try at making USR? not loose mailbox
 *
 * Revision 5.5  1992/12/07  02:41:21  syd
 * This implements the use of SIGUSR1 and SIGUSR2 as discussed on the
 * mailing list recently, and adds them to the documentation.
 * From: scs@lokkur.dexter.mi.us (Steve Simmons)
 *
 * Revision 5.4  1992/11/26  00:46:13  syd
 * changes to first change screen back (Raw off) and then issue final
 * error message.
 * From: Syd
 *
 * Revision 5.3  1992/10/27  01:43:40  syd
 * Move posix_signal to lib directory
 * From: tom@osf.org
 *
 * Revision 5.2  1992/10/25  02:01:58  syd
 * Here are the patches to support POSIX sigaction().
 * From: tom@osf.org
 *
 * Revision 5.1  1992/10/03  22:58:40  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/** This set of routines traps various signals and informs the
    user of the error, leaving the program in a nice, graceful
    manner.

**/

#include "headers.h"
#include "s_elm.h"

extern int pipe_abort;		/* set to TRUE if receive SIGPIPE */

SIGHAND_TYPE
quit_signal(sig)
{
	dprint(1, (debugfile, "\n\n** Received SIGQUIT **\n\n\n\n"));
	leave(0);
}

SIGHAND_TYPE
hup_signal(sig)
{
	dprint(1, (debugfile, "\n\n** Received SIGHUP **\n\n\n\n"));
	leave(0);
}

SIGHAND_TYPE
term_signal(sig) 
{
	dprint(1, (debugfile, "\n\n** Received SIGTERM **\n\n\n\n"));
	leave(0);
}

SIGHAND_TYPE
ill_signal(sig)
{
	MoveCursor(LINES,0);
	Raw(OFF);
	dprint(1, (debugfile, "\n\n** Received SIGILL **\n\n\n\n"));
	printf(catgets(elm_msg_cat, ElmSet, ElmIllegalInstructionSignal,
		"\n\nIllegal Instruction signal!\n\n"));
	emergency_exit();
}

SIGHAND_TYPE
fpe_signal(sig)  
{
	MoveCursor(LINES,0);
	Raw(OFF);
	dprint(1, (debugfile, "\n\n** Received SIGFPE **\n\n\n\n"));
	printf(catgets(elm_msg_cat, ElmSet, ElmFloatingPointSignal,
		"\n\nFloating Point Exception signal!\n\n"));
	emergency_exit();
}

#ifdef SIGBUS
SIGHAND_TYPE
bus_signal(sig)
{
	MoveCursor(LINES,0);
	Raw(OFF);
	dprint(1, (debugfile, "\n\n** Received SIGBUS **\n\n\n\n"));
	printf(catgets(elm_msg_cat, ElmSet, ElmBusErrorSignal,
		"\n\nBus Error signal!\n\n"));
	emergency_exit();
}
#endif

SIGHAND_TYPE
segv_signal(sig)
{
	MoveCursor(LINES,0);
	Raw(OFF);
	dprint(1, (debugfile,"\n\n** Received SIGSEGV **\n\n\n\n"));
	printf(catgets(elm_msg_cat, ElmSet, ElmSegmentViolationSignal,
		"\n\nSegment Violation signal!\n\n"));
	emergency_exit();
}

SIGHAND_TYPE
alarm_signal(sig)
{	
	signal(SIGALRM, alarm_signal);

	/** silently process alarm signal for timeouts... **/
	if (InGetPrompt)
		{
#ifdef HASSIGHOLD
		sigrelse(SIGALRM);
#endif
		LONGJMP(GetPromptBuf, 1);
		}
}

SIGHAND_TYPE
pipe_signal(sig)
{
	/** silently process pipe signal... **/
	dprint(2, (debugfile, "*** received SIGPIPE ***\n\n"));
	
	pipe_abort = TRUE;	/* internal signal ... wheeee!  */

	signal(SIGPIPE, pipe_signal);
}

#ifdef SIGTSTP
int was_in_raw_state;

SIGHAND_TYPE
sig_user_stop(sig)
{
	dprint(1, (debugfile,"\n\n** Received SIGTSTP **\n\n\n\n", sig));
	/* This is called when the user presses a ^Z to stop the
	   process within BSD 
	*/
#ifdef	SIGTSTP
	signal(SIGTSTP, SIG_DFL);
#endif

	was_in_raw_state = RawState();
	Raw(OFF);	/* turn it off regardless */

	printf(catgets(elm_msg_cat, ElmSet, ElmStoppedUseFGToReturn,
		"\n\nStopped.  Use \"fg\" to return to ELM\n\n"));

	kill(getpid(), SIGSTOP);
}

SIGHAND_TYPE
sig_return_from_user_stop(sig)
{
	/** this is called when returning from a ^Z stop **/
	dprint(1, (debugfile,"\n\n** Received SIGCONT **\n\n\n\n", sig));

	signal(SIGCONT, sig_return_from_user_stop);
	signal(SIGTSTP, sig_user_stop);

	printf(catgets(elm_msg_cat, ElmSet, ElmBackInElmRedraw,
	 "\nBack in ELM. (You might need to explicitly request a redraw.)\n\n"));

	if (was_in_raw_state)
	  Raw(ON);

	if (InGetPrompt)
		{
#ifdef HASSIGHOLD
		sigrelse(SIGTSTP);
		sigrelse(SIGCONT);
#endif
		LONGJMP(GetPromptBuf, 1);
		}
}
#endif

#ifdef SIGWINCH
SIGHAND_TYPE
winch_signal(sig)
{
	resize_screen = 1;
	signal(SIGWINCH, winch_signal);

	if (InGetPrompt)
		{
#ifdef HASSIGHOLD
		sigrelse(SIGWINCH);
#endif
		LONGJMP(GetPromptBuf, 1);
		}
}
#endif

SIGHAND_TYPE
usr1_signal(sig)
{
	dprint(1, (debugfile, "\n\n** Received SIGUSR1 **\n\n\n\n"));
	question_me = FALSE;
	while ( leave_mbox(TRUE, FALSE, TRUE) == -1)
		newmbox(cur_folder, TRUE);

	leave(0);
}

SIGHAND_TYPE
usr2_signal(sig)
{
	dprint(1, (debugfile, "\n\n** Received SIGUSR2 **\n\n\n\n"));
	while ( leave_mbox(FALSE, TRUE, FALSE) == -1)
		newmbox(cur_folder, TRUE);

	leave(0);
}
