/* MRR's modified version to try to get it to work with
 *  Windows 3.0.
 * This is the second time--Oh, how I hate PWB!!!
 *
 *  Mark Riordan	 23-SEP-1990
 */
/*
 * SRCLIB\NETLIB\VEC_SRCH.C
 *
 * Copyright (C) 1986,1987,1988 by FTP Software, Inc.
 * 
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 * 
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by FTP Software, Inc.
 * 
 * EDIT HISTORY:
 * 05-Feb-88	jog	basically lifted from 'old' versions of _find_vec
 * 			 routine
 * 		jbvb	Clean up leftover code.
 * 23-Feb-88	jbvb	Use new pctcp_cleanup() for onexit() shutdown.
 * 24-Feb-88	jbvb	Add logic to turn off DOS BREAK ON, and save old
 *			 state in _dos_break so pctcp_cleanup() can restore
 *			 it when we exit.
 * 27-Feb-88	jbvb	Don't modify syscall interrupt directly - call the
 *			 library to do it.
 * 02-Mar-88	jbvb	Use correct argument to DOS call
 */

#include <dos.h>

#define	FIRST_VEC	0x20
#define	LAST_VEC	0xE0

/* find the packet driver vector - returns the vector number, or 0 on
	failure.
*/

int	_pctcp_int = 0;		/* global which is our syscall interrupt */
int	_dos_break = 0;		/* DL returned from DOS function 33h */

static char match[] = "TCPTSR";

extern void _net_set_vector();
extern void pctcp_cleanup();
static int check_match(char far *t);

int
vec_search()
{
	int i = 0x61;

	_pctcp_int = i;		/* set the global variable */
#if 0
	_net_set_vector(i);     /* Modify library call */
#endif
	return(i);
}

old_vec_search()
{
	char far * (far *vec);		/* pointer into the interrupt table */
	register int i;
	char far *handler;	/* pointer to an interrupt handler */
	union REGS reg;

	FP_SEG(vec) = 0;

	for(i = FIRST_VEC; i < LAST_VEC; i++) {
		/* fixup the pointer into the table
		*/
		FP_OFF(vec) = i << 2;

		/* get the handler and bump it (hopefully) over the jmp
		*/
		handler = *vec + 3;
		
		/* check if it points at PC/TCP kernel's service routine.
		*/
		if(check_match(handler)) {
			_pctcp_int = i;		/* set the global variable */

			_net_set_vector(i);	/* Modify library call */

			reg.x.ax = 0x3300;	/* Get BREAK state */
			intdos(&reg, &reg);
			_dos_break = reg.h.dl;	/* Save old BREAK state */

			reg.x.ax = 0x3301;	/* Set BREAK state */
			reg.h.dl = 0;		/*  to OFF for PC/TCP */
			intdos(&reg, &reg);
			
			onexit(pctcp_cleanup);	/* Hook shutdown function */
			return i;
			}
		}

        return (0);
	}

static int
check_match(t)
	char far *t; {
	register char *s;
	
	for(s = match; *s; s++)
		if(*t++ != *s)
			return 0;

	return 1;
	}
