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

/*
 *	TRACE	functions
 *	reimplementations, checking for validity
 */

#define	TRACE
#include	<stdio.h>
#include	"arithmetic.h"

#define		ERRMSG(msg)	{ fprintf(stderr,msg); }


/***************************************************************
 *
 * Procedure ddiv
 *
 ***************************************************************/
#ifdef __STDC__

void ddiv(
	L_NUMBER	  high,
	L_NUMBER	  low,
	L_NUMBER	  divide,
	L_NUMBER	 *qp,
	L_NUMBER	 *rp
)

#else

void ddiv(
	high,
	low,
	divide,
	qp,
	rp
)
L_NUMBER	  high;
L_NUMBER	  low;
L_NUMBER	  divide;
L_NUMBER	 *qp;
L_NUMBER	 *rp;

#endif

{
	L_NUMBER	prodH, prodL, behigh, below;

	_ddiv(high,low,divide,qp,rp);	/* original call */
	/* now check */
	_dmult(*qp,divide,&prodH,&prodL);
	_cadd(0,prodH,&behigh,
		_cadd(*rp,prodL,&below,0));
		
	if( (behigh==high) && (below==low) )/*OK*/;
	else  ERRMSG("double word division error.\n");
	return;	
}


/***************************************************************
 *
 * Procedure div
 *
 ***************************************************************/
#ifdef __STDC__

void div(
	L_NUMBER	  A[],
	L_NUMBER	  B[],
	L_NUMBER	  Q[],
	L_NUMBER	  R[]
)

#else

void div(
	A,
	B,
	Q,
	R
)
L_NUMBER	  A[];
L_NUMBER	  B[];
L_NUMBER	  Q[];
L_NUMBER	  R[];

#endif

{
	L_NUMBER	BQ[MAXGENL];
	L_NUMBER	saveR[MAXGENL]; /* cover reflexiv use */
	L_NUMBER	saveQ[MAXGENL]; /* cover reflexiv use */
	
	_div(A,B,saveQ,saveR);	/* execute request */
	_mult(B,saveQ,BQ);
	_add(BQ,saveR,BQ);
	if ( _comp(BQ,A) )
		ERRMSG("long division error.\n");
	_trans(saveQ,Q);
	_trans(saveR,R);
	return;
}

/***************************************************************
 *
 * Procedure trans
 *
 ***************************************************************/
#ifdef __STDC__

void trans(
	L_NUMBER	  From[],
	L_NUMBER	  To[]
)

#else

void trans(
	From,
	To
)
L_NUMBER	  From[];
L_NUMBER	  To[];

#endif

{
	if (*From >= MAXGENL) ALU_exception(*From);
	/*if (*From >= MAXLGTH)
		ERRMSG("warning: transfer exceeds MAXLGTH.\n");
	*/
	_trans(From,To);
}
