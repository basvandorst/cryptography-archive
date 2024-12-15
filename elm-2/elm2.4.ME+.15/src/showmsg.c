
static char rcsid[] = "@(#)$Id: showmsg.c,v 5.16 1994/08/30 15:09:43 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.16 $   $State: Exp $
 *
 * 			Copyright (c) 1988-1992 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 *******************************************************************************
 * $Log: showmsg.c,v $
 * Revision 5.16  1994/08/30  15:09:43  syd
 * Block sigwinch when in the external pager.
 * From: jwa@yog-sothoth.dcrt.nih.gov (James W. Adams)
 *
 * Revision 5.15  1993/08/23  02:46:07  syd
 * Don't declare _exit() if <unistd.h> already did it.
 * From: decwrl!uunet.UU.NET!fin!chip (Chip Salzenberg)
 *
 * Revision 5.14  1993/08/03  19:28:39  syd
 * Elm tries to replace the system toupper() and tolower() on current
 * BSD systems, which is unnecessary.  Even worse, the replacements
 * collide during linking with routines in isctype.o.  This patch adds
 * a Configure test to determine whether replacements are really needed
 * (BROKE_CTYPE definition).  The <ctype.h> header file is now included
 * globally through hdrs/defs.h and the BROKE_CTYPE patchup is handled
 * there.  Inclusion of <ctype.h> was removed from *all* the individual
 * files, and the toupper() and tolower() routines in lib/opt_utils.c
 * were dropped.
 * From: chip@chinacat.unicom.com (Chip Rosenthal)
 *
 * Revision 5.13  1993/05/08  20:25:33  syd
 * Add sleepmsg to control transient message delays
 * From: Syd
 *
 * Revision 5.12  1993/02/03  19:06:31  syd
 * Remove extra strchr/strcat/strcpy et al declarations
 * From: Syd
 *
 * Revision 5.11  1993/01/20  04:01:07  syd
 * Adds a new integer parameter builtinlines.
 * if (builtinlines < 0) and (the length of the message < LINES on
 *       screen + builtinlines) use internal.
 * if (builtinlines > 0) and (length of message < builtinlines)
 * 	use internal pager.
 * if (builtinlines = 0) or none of the above conditions hold, use the
 * external pager if defined.
 * From: "John P. Rouillard" <rouilj@ra.cs.umb.edu>
 *
 * Revision 5.10  1993/01/19  05:11:45  syd
 * Elm switches screens prematurely when calling metamail. It switches
 * before writing the "Press any key..." message, thus losing metamail output.
 * From: Jan Djarv <Jan.Djarv@sa.erisoft.se>
 *
 * Revision 5.9  1992/12/07  04:29:12  syd
 * add missing err declare
 * From: Syd
 *
 * Revision 5.8  1992/11/26  01:46:26  syd
 * add Decode option to copy_message, convert copy_message to
 * use bit or for options.
 * From: Syd and bjoerns@stud.cs.uit.no (Bjoern Stabell)
 *
 * Revision 5.7  1992/11/26  00:46:50  syd
 * Fix how errno is used so err is inited and used instead
 * as errno gets overwritten by print system call
 * From: Syd
 *
 * Revision 5.6  1992/11/15  01:29:37  syd
 * Clear the screen before displaying MIME:
 * From: marius@rhi.hi.is (Marius Olafsson)
 *
 * Revision 5.5  1992/11/07  16:23:48  syd
 * fix null dereferences from patch 5
 * From: Jukka Ukkonen <ukkonen@csc.fi>
 *
 * Revision 5.4  1992/10/30  21:47:30  syd
 * Use copy_message in mime shows to get encode processing
 * From: bjoerns@stud.cs.uit.no (Bjoern Stabell)
 *
 * Revision 5.3  1992/10/25  01:47:45  syd
 * fixed a bug were elm didn't call metamail on messages with a characterset,
 * which could be displayed by elm itself, but message is encoded with QP
 * or BASE64
 * From: Klaus Steinberger <Klaus.Steinberger@Physik.Uni-Muenchen.DE>
 *
 * Revision 5.2  1992/10/24  13:35:39  syd
 * changes found by using codecenter on Elm 2.4.3
 * From: Graham Hudspith <gwh@inmos.co.uk>
 *
 * Revision 5.1  1992/10/03  22:58:40  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/** This file contains all the routines needed to display the specified
    message.
**/

#include "headers.h"
#include <errno.h>
#include "s_elm.h"
#include "me.h"

extern int errno;

int pipe_abort = FALSE; /* not used anymore, but won't compile without it */

#ifdef MIME
static int
need_meta (hdr)
struct header_rec *hdr;
{
	/* Determine whether or not we need to call metamail to display the
	 * message contents.
	 */
#ifdef USE_PGP
	if (hdr->pgp & (PGP_MESSAGE | PGP_SIGNED_MESSAGE))
		return 0;
#endif
	if ((hdr->status & (MIME_MESSAGE)) && (hdr->status & MIME_UNSUPPORTED))
		return TRUE;
	  	if (hdr->status & (MIME_MESSAGE | PRE_MIME_CONTENT))
		return hdr->mime_rec.notplain;
	return 0;
}
#endif

int
show_msg(number)
int number;
{
	/*** Display number'th message.  Get starting and ending lines
	     of message from headers data structure, then fly through
	     the file, displaying only those lines that are between the
	     two!

	     Return 0 to return to the index screen or a character entered
	     by the user to initiate a command without returning to
	     the index screen (to be processed via process_showmsg_cmd()).
	***/

	struct header_rec *current_header = headers[number-1];

	dprint(4, (debugfile,"displaying %d lines from message %d using %s\n", 
		current_header->lines, number, pager));

	if (number > message_count || number < 1)
	  return(0);

	if(ison(current_header->status, NEW)) {
	  clearit(current_header->status, NEW);   /* it's been read now! */
	  current_header->status_chgd = TRUE;
	}
	if(ison(current_header->status, UNREAD)) {
	  clearit(current_header->status, UNREAD);   /* it's been read now! */
	  current_header->status_chgd = TRUE;
	}

#ifdef MIME
	/* May need recheck: (Because now defination is recursive) */
	if ((current_header->status & MIME_MESSAGE) && 
	    current_header->mime_rec.notplain)
	  attach_parse(current_header,mailfile);

        if (need_meta(current_header) && have_metamail()) {
		char fname[STRING], Cmd[SLEN], line[VERY_LONG_STRING];
		int code, err;
		long lines = current_header->lines;
		FILE *fpout;

		if (fseek(mailfile, current_header->offset, 0) == -1) {
		    err = errno;
		    dprint(1, (debugfile,
		      "Error: seek %d bytes into file, errno %s (show_message)\n",
		      current_header->offset, error_description(err)));
		    error2(catgets(elm_msg_cat, ElmSet, ElmSeekFailedFile,
		      "ELM [seek] couldn't read %d bytes into file (%s)."),
		      current_header->offset, error_description(err));	
		    return(0);
		}
		sprintf(fname, "%semm.%d.%d", temp_dir, getpid(), getuid());
		if ((fpout = safeopen_rdwr(fname)) == NULL) {
		  err = errno;
		  dprint(1, (debugfile,
			     "Error: open of temporary file %s, errno %s (show_message)\n",
			     fname, error_description(err)));
		  error2(catgets(elm_msg_cat, ElmSet, ElmCantOpenAsOutputFile,
				 "Can't open \"%s\" as output file! (%s)."),
			 fname, error_description(err));
		  return(0);
		}
		/* Let metamail decode it! 
		 * (Now CM_DECODE also decodes MIME and PGP) -KEH
		 */
		copy_message(mailfile,current_header,
			     "", fpout, 0);
		(void) fclose (fpout);
		sprintf(Cmd, "%s -p -z -m Elm %s", metamail_path, fname);
		Raw(OFF);
		ClearScreen();
		printf ("Executing metamail...\n");
		code = system_call(Cmd, SY_ENAB_SIGINT|SY_ENV_METAMAIL);
		Raw(ON | NO_TITE);	/* Raw on but don't switch screen */
		(void) unlink (fname);
		PutLine0(LINES,0, catgets(elm_msg_cat, ElmSet, ElmPressAnyKeyIndex,
			     "Press any key to return to index."));
		(void) ReadCh();
		printf("\r\n");
		Raw(OFF | NO_TITE); /* Raw off so raw on takes effect */
		Raw(ON); /* Finally raw on and switch screen */
		return(0);
	}
#endif
        return (metapager (mailfile, current_header, TRUE));
}
