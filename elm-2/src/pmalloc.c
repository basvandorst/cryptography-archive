
static char rcsid[] = "@(#)$Id: pmalloc.c,v 5.4 1994/03/11 20:59:02 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.4 $   $State: Exp $
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
 * $Log: pmalloc.c,v $
 * Revision 5.4  1994/03/11  20:59:02  syd
 * I made the following changes to elm 2.4pl23 module pmalloc.c after testing on
 * a DEC Alpha running OSF1 v1.3.  The difficulty is that the Alpha hardware
 * requires certain alignment for some operations which was not supplied by
 * pmalloc (8 byte for addresses instead of 4 byte).  I made a simple minded
 * change to pmalloc.c to force alignment of the allocated data block to be
 * adequate on DEC Alpha OSF1.
 * From: jim@lime.rsmas.miami.edu (Jim Brown)
 *
 * Revision 5.3  1992/12/07  04:28:03  syd
 * change include from defs to headers as now needs LINES
 * From: Syd
 *
 * Revision 5.2  1992/11/26  00:46:13  syd
 * changes to first change screen back (Raw off) and then issue final
 * error message.
 * From: Syd
 *
 * Revision 5.1  1992/10/03  22:58:40  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/** This routine contains a cheap permanent version of the malloc call to 
    speed up the initial allocation of the weedout headers and the uuname 
    data.  

      This routine is originally from Jim Davis of HP Labs, with some 
    mods by me.
**/

#include <stdio.h>
#include "headers.h"
#include "s_elm.h"

extern nl_catd elm_msg_cat;	/* message catalog	    */
/*VARARGS0*/

#define MIN_BOUNDARY 8 /* power to 2 */

char *pmalloc(size)
int size; 
{
	/** return the address of a specified block **/

	static char *our_block = NULL;
	static int   free_mem  = 0;

	char   *return_value;

	/** if bigger than our threshold, just do the real thing! **/

	if (size > PMALLOC_THRESHOLD) 
	   return((char *) malloc(size));

	/** if bigger than available space, get more, tossing what's left **/

	size = ((size+3+4)/4)*4;	/* Go to quad byte boundary, fill */

	if (size > free_mem) {
	  if ((our_block = (char *) malloc(PMALLOC_BUFFER_SIZE)) == NULL) {
	    MoveCursor(LINES,0);
	    Raw(OFF);
	    fprintf(stderr, catgets(elm_msg_cat, ElmSet, ElmCouldntMallocBytes,
		    "\n\nCouldn't malloc %d bytes!!\n\n"),
		    PMALLOC_BUFFER_SIZE);
	    leave(0);	
          }
	  our_block += 4;  /* just for safety, don't give back true address */
	  free_mem = PMALLOC_BUFFER_SIZE-4;
	}

	if (((long)our_block&(MIN_BOUNDARY-1)) != 0) {
	  int		drek;

	  drek = ((long)our_block&(MIN_BOUNDARY-1)); /* round to minimum */
	  our_block += (MIN_BOUNDARY-drek);
	  free_mem -= (MIN_BOUNDARY-drek);
	}

	return_value  = our_block;	/* get the memory */
	our_block += size;		/* use it up      */
	free_mem  -= size;		/*  and decrement */

	return( (char *) return_value);
}
