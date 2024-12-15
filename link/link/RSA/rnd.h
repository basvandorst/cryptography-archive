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

#ifndef	_rnd_h_
#define	_rnd_h_

#ifndef _arith_h_
#include	"arith.h"
#endif

void	gen_number	P(( int, NUMBER* ));
void	init_rnd	P(( void ));

#endif
