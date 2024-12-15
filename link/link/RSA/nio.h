/*******************************************************************************
*									       *
*	Copyright (c) Martin Nicolay,  22. Nov. 1988			       *
*									       *
*	Wenn diese (oder sinngemaess uebersetzte) Copyright-Angabe enthalten   *
*	bleibt, darf diese Source fuer jeden nichtkomerziellen Zweck weiter    *
*	verwendet werden.						       *
*									       *
*	martin@trillian.megalon.de					       *
*									       *
*******************************************************************************/

#ifndef	_nio_h_
#define	_nio_h_

#ifndef _arith_h_
#include	"arith.h"
#endif

/*
 * Prototypes
 */

int	num_sput	P(( NUMBER*, char*, int ));
int	num_fput	P(( NUMBER*, FILE* ));
int	num_sget	P(( NUMBER*, char* ));
int	num_fget	P(( NUMBER*, FILE* ));

#endif
