/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Adapted from MetroWerks SetupA4.h

	$Id: SetupA5.h,v 1.4 1997/09/18 01:34:58 lloyd Exp $
____________________________________________________________________________*/

#ifndef __SETUPA5__
#define __SETUPA5__

PGP_BEGIN_C_DECLARATIONS

#if !defined(__POWERPC__) && !defined(__CFM68K__)

	#define PrepareCallbackA5()	RememberA5() 
	#define EnterCallbackA5()	long oldA5 = SetUpA5()
	#define ExitCallbackA5()	RestoreA5(oldA5)

	static void RememberA5(void);
	
	static asm long SetUpA5(void)
	{
		move.l	a5,d0
		lea		__storage,a5
		move.l	(a5),a5
		rts
		
	/* this storage is only referenced thru data cache */
	__storage:	dc.l	0
	
		entry	static RememberA5
		lea		__storage,a0
		move.l	a5,(a0)
		rts
	}
	
	static long RestoreA5(long:__D0):__D0 = 0xC18D;

#else

	#define PrepareCallbackA5() 
	#define EnterCallbackA5()
	#define ExitCallbackA5()
	#define RememberA5() 	0
	#define SetUpA5() 		0L
	#define RestoreA5(x)	0L
	
#endif

PGP_END_C_DECLARATIONS

#endif
