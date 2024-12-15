/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

 
#include	"arithmetic.h"

/*	sign(A-B)	*/
/***************************************************************
 *
 * Procedure _comp
 *
 ***************************************************************/
#ifdef __STDC__

int _comp(
	register L_NUMBER	 *Ap,
	register L_NUMBER	 *Bp
)

#else

int _comp(
	Ap,
	Bp
)
register L_NUMBER	 *Ap;
register L_NUMBER	 *Bp;

#endif

{
	register int	s = *Ap - *Bp;
	
	if (s>0)	return 1;
	if (s<0)	return -1;
	
	{	register L_NUMBER	*stop = Ap;
		
		Ap += lngofln(Ap); Bp += lngofln(Bp);
		for ( ; Ap>stop; Ap--, Bp-- ) {
			if (*Ap > *Bp)	return 1;
			if (*Ap < *Bp)	return -1;
		}
	}
	return 0;
}
