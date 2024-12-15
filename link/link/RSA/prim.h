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

#ifndef	_prim_h_
#define	_prim_h_

#ifndef _arith_h_
#include	"arith.h"
#endif

int	p_prim		P(( NUMBER*, int ));
void	inv		P(( NUMBER*, NUMBER*, NUMBER* ));

#endif
