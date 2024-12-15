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

/*------------------------------------------------------------+-----*/
/*                                                            ! GMD */
/*   ARITHMETIC for LONG INTEGER    V2.0                      +-----*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/*    INCLUDE  <arithmetic.h>                                       */
/*                                        DATE 01.06.90             */
/*                                                                  */
/*    Note:                                                         */
/*      arithmetic.h uses option                                    */
/*      -DWLNGxx (xx = 16 bzw 32) (word length of the processor)    */
/*                                                                  */
/*------------------------------------------------------------------*/
#ifndef _ARITHMETIC_
#define _ARITHMETIC_

/*------------------------------------------------------------------*/
/* L_NUMBER definition                                              */
/*------------------------------------------------------------------*/

#ifndef _SECURE_
#ifndef MAC
typedef unsigned  L_NUMBER;
#else
typedef unsigned long L_NUMBER;
#endif /* MAC */
#endif

/*------------------------------------------------------------------*
 *      A multi-precision integer number is stored and processed in *
 *      an array of unsigned long integers. The maximum length of   *
 *      the array is MAXLGTH.                                       *
 *                                                                  *
 *      MP Integer ::=  L_NUMBER [MAXLGTH]                          *
 *                                                                  *
 *      The first element (L_NUMBER[0]) indicates the number of     *
 *      following words which contain the MP integer, starting with *
 *      with least significant integer left (first).                *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 * Definitions of constants depending on the worg length            *
 *------------------------------------------------------------------*/

#define  BYTEL          8       /* bits per byte                    */

#ifdef WLNG32                   /* word length 32 bits              */
 
#define  MAXGENL        132     /* Maximum L_NUMBER used during 
                                   key generation (in words)        */
#define  MAXLGTH        66      /* Maximum L_NUMBER else            */
#define  WLNG           32      /* word length in bits              */
#define  WBYTES         4       /* bytes per word                   */
#define  SWBYTES        2       /* << SWBYTES = WBYTES  times 4     */
#define  SWBITS         5       /* << SWBITS = WLNG                 */
#define  HSBIT  0x80000000
#define  HSBYTE 0xFF000000

#endif

#ifdef WLNG16                   /* word length 16 bits              */

#define  MAXGENL        132     /* Maximum L_NUMBER used during 
                                   key generation (in words)        */
#define  MAXLGTH        264     /* Maximum L_NUMBER else            */
#define  WLNG           16      /* word length in bits              */
#define  WBYTES         2       /* bytes per word                   */
#define  SWBYTES        1       /* << SWBYTES = WBYTES  times 2     */
#define  SWBITS         4       /* << SWBITS = WLNG                 */
#define  HSBIT  0x8000
#define  HSBYTE 0xFF00

#endif


#ifndef WLNG
#include "-DWLNGxx Option fehlt!" /* Test ob die -DWLNGxx          */
                                  /* Option  gesetzt wurde         */
#endif                            /* (siehe Header)                */



/*-----------------------------------------------------------------*
 * Definition of shift factors                                     *
 *-----------------------------------------------------------------*/

#define    R1   - 1
#define    R4   - 4
#define    R8   - 8
#define    R16  -16
#define    L1     1
#define    L4     4
#define    L8     8
#define    L16   16



#define   LNUMBER



/*---------------------------------------------------------------*
 *  Global L_NUMBERs                                             *
 *---------------------------------------------------------------*/

#define   LZ_NULL     { 0 }
#define   LZ_EINS     { 1, 1 }
#define   LZ_ZWEI     { 1, 2 }
extern    L_NUMBER    lz_null[];
extern    L_NUMBER    lz_eins[];
extern    L_NUMBER    lz_zwei[];

#ifdef ARITHMETIC_TEST
#include <stdio.h>
extern    FILE *arithmetic_trace;
extern    int arithmetic_trace_counter;
#endif

#ifdef WLNG32
#define   LZ_FERMAT5  { 1, 0x10001 }
#endif

#ifdef WLNG16
#define   LZ_FERMAT5  { 2, 1, 1 }
#endif


/*-------------------------------------------------------------*
 *  Function declarations                                      *
 *-------------------------------------------------------------*/

int             lngtouse();

/*
 *      L_NUMBER addition/subtraction/division
 *
 */

#define ALU_exception(x)        (x%0)   /* TRAP */

#define trans(From,To)  _trans(From,To)
#define comp(A,B)               _comp(A,B)






#ifdef ARITHMETIC_TEST_NOT_STD

#ifndef MS_DOS
#define add(A,B,Sum)    { \
				if(arithmetic_trace_counter) {\
					fprintf(arithmetic_trace, "add:");\
					arithmetic_trace_print(A); \
			 		fprintf(arithmetic_trace, "add: + \nadd:");\
					arithmetic_trace_print(B);\
				} \
				_add(A,B,Sum); \
				if(arithmetic_trace_counter) {\
 					arithmetic_trace_counter--;\
					fprintf(arithmetic_trace, "add: = \nadd:");\
 					arithmetic_trace_print(Sum);\
				}\
			}

#define sub(A,B,Diff)   { \
				if(arithmetic_trace_counter) {\
					fprintf(arithmetic_trace, "sub:");\
					arithmetic_trace_print(A);\
					fprintf(arithmetic_trace, "sub: - \nsub:");\
					arithmetic_trace_print(B);\
				}\
			 	_sub(A,B,Diff);\
			 	if(arithmetic_trace_counter) {\
 					arithmetic_trace_counter--;\
					fprintf(arithmetic_trace, "sub: = \nsub:");\
					arithmetic_trace_print(Diff);\
				}\
			}

#define mult(A,B,AB)    { \
				if(arithmetic_trace_counter) {\
					fprintf(arithmetic_trace, "mult:");\
					arithmetic_trace_print(A);\
					fprintf(arithmetic_trace, "mult: * \nmult:");\
					arithmetic_trace_print(B);\
				}\
			 	_mult(A,B,AB);\
			 	if(arithmetic_trace_counter) {\
 					arithmetic_trace_counter--;\
					fprintf(arithmetic_trace, "mult: = \nmult:");\
			 		arithmetic_trace_print(AB);\
				}\
			}

#define shift(A,I,B)    { \
				if(arithmetic_trace_counter) {\
					fprintf(arithmetic_trace, "shift:");\
					arithmetic_trace_print(A);\
					if(I > 0) fprintf(arithmetic_trace, "shift: << %d\n", I);\
					else fprintf(arithmetic_trace, "shift: >> %d\n", -(I));\
				}\
			 	_shift(A,I,B);\
			 	if(arithmetic_trace_counter) {\
 					arithmetic_trace_counter--;\
					fprintf(arithmetic_trace, "shift: = \nshift:");\
			 		arithmetic_trace_print(B);\
				}\
			}

#define div(A,B,Q,R)    { \
				if(arithmetic_trace_counter) {\
					fprintf(arithmetic_trace, "div:");\
			 		arithmetic_trace_print(A);\
					fprintf(arithmetic_trace, "div: / \ndiv:");\
					arithmetic_trace_print(B);\
				}\
			 	_div(A,B,Q,R);\
				if(arithmetic_trace_counter) {\
 					arithmetic_trace_counter--;\
					fprintf(arithmetic_trace, "div: = \ndiv:");\
					arithmetic_trace_print(Q);\
					fprintf(arithmetic_trace, "div: Re \ndiv:");\
					arithmetic_trace_print(R);\
				}\
			}

#define normalize(N)    { L_NUMBER zero = 0; _sub(N,&zero,N); }
#else
/* skip defines for asm routines **
define add             ADD
define sub             SUB
define mult            MULT
define div             DIV
define shift           SHIFT
*/
#define normalize(N)    { L_NUMBER zero = 0; SUB(N,&zero,N); }
#endif

#else /* ARITHMETIC_TEST */


#ifndef MS_DOS
#define add(A,B,Sum)    _add(A,B,Sum)
#define sub(A,B,Diff)   _sub(A,B,Diff)
#define mult(A,B,AB)    _mult(A,B,AB)
#define div(A,B,Q,R)    _div(A,B,Q,R)
#define shift(A,I,B)    _shift(A,I,B)
#define normalize(N)    { L_NUMBER zero = 0; _sub(N,&zero,N); }
#else
/* skip defines for asm routines **
define add             ADD
define sub             SUB
define mult            MULT
define div             DIV
define shift           SHIFT
*/
#define normalize(N)    { L_NUMBER zero = 0; SUB(N,&zero,N); }
#endif

#endif /* ARITHMETIC_TEST */



#define cadd(A,B,A_B,c) _cadd(A,B,A_B,c)
#define dmult(a,b,ab0,ab1)      _dmult(a,b,ab0,ab1)
#define ddiv(a1,a0,b,q,r)       _ddiv(a1,a0,b,q,r)
#define xor(a,b,x)      _xor(a,b,x)


/*      define _trans macro inline expanded */
#define _trans(src,dst) { register L_NUMBER *stop,*dp = (dst), *sp = (src); \
                          if( sp != dp ) {stop = sp;    \
                          for( dp = dp+*sp, sp = sp+*sp; sp >= stop; ) *dp-- = *sp--; \
                        } }
#define inttoln(I,LI)   ( (I)? ( *(LI) = 1, *((LI)+1) = (I) ) : (*(LI) = 0) )
#define lntoint(L)      (*(L)? ( *((L)+1) ) : 0)
#define checkln(L)      (*(L)? (-(*((L)+*(L))==0) : 0)
#define lngofln(L)      (*(L))
#define lntoINTEGER(L,I)        lntoctets(L,I,0)
#define INTEGERtoln(I,L)        octetstoln(I,L,0,(*(I)).noctets)




#ifdef __STDC__

#ifdef _SECURE_
       void	octetstoln	(OctetString *octs, L_NUMBER lnum[], int offset, int size);
       void	lntoctets	(L_NUMBER lnum[], OctetString *octs, int size);
       void	bitstoln	(BitString *bits, L_NUMBER lnum[], int offset, int size);
       void	lntobits	(L_NUMBER lnum[], BitString *bits, int size);
       RC	aux_LN2BitString2	(BitString *bitstring, L_NUMBER lnum[], int size);
       RC	aux_LN2OctetString2	(OctetString *octetstring, L_NUMBER lnum[], int size);
       RC	aux_OctetString2LN2	(L_NUMBER *lnum, OctetString *ostr);
       RC	aux_BitString2LN2	(L_NUMBER *lnum, BitString *bstr);
       BitString *aux_LN2BitString	(L_NUMBER lnum[], int size);
       OctetString *aux_LN2OctetString	(L_NUMBER lnum[], int size);
       L_NUMBER *aux_OctetString2LN	(OctetString *ostr);
       L_NUMBER *aux_BitString2LN	(BitString *bstr);

#endif
#ifdef ARITHMETIC_TEST
       void arithmetic_trace_print(L_NUMBER	  ln[]);
#endif
       int	intlog2	(L_NUMBER v);
       int	lngtouse	(L_NUMBER modul[]);

       int	madd	(L_NUMBER op1[], L_NUMBER op2[], L_NUMBER erg[], L_NUMBER modul[]);
       int	msub	(L_NUMBER op1[], L_NUMBER op2[], L_NUMBER erg[], L_NUMBER modul[]);
       int	mmult	(L_NUMBER op1[], L_NUMBER op2[], L_NUMBER erg[], L_NUMBER modul[]);
       int	mdiv	(L_NUMBER op1[], L_NUMBER op2[], L_NUMBER erg[], L_NUMBER modul[]);
       int	mexp	(L_NUMBER bas[], L_NUMBER exp[], L_NUMBER erg[], L_NUMBER modul[]);
       void	mexp2	(L_NUMBER exp[], L_NUMBER erg[], L_NUMBER modul[]);
       void	_xor	(register L_NUMBER op1[], register L_NUMBER op2[], register L_NUMBER erg[]);
       int	_comp	(register L_NUMBER *Ap, register L_NUMBER *Bp);
       void	_add	(register L_NUMBER *Ap, register L_NUMBER *Bp, register L_NUMBER *Sum);
       int	_cadd	(L_NUMBER opa, L_NUMBER opb, register L_NUMBER *opa_b, register L_NUMBER carry);
       void	_div	(L_NUMBER A[], L_NUMBER B[], L_NUMBER Q[], L_NUMBER R[]);
       void	_dmult	(L_NUMBER a, L_NUMBER b, L_NUMBER *high, L_NUMBER *low);
       void	_mult	(L_NUMBER A[], L_NUMBER B[], L_NUMBER erg[]);
       void	_shift	(L_NUMBER A[], int exp2b, L_NUMBER S[]);
       void	_sub	(register L_NUMBER *Ap, register L_NUMBER *Bp, register L_NUMBER *Diff);
       void	_ddiv	(L_NUMBER high, L_NUMBER low, L_NUMBER divide, L_NUMBER *qp, L_NUMBER *rp);
       int	primzahl	(L_NUMBER primzahl[], L_NUMBER fakt[], int length);
       int	nextprime	(L_NUMBER start[], L_NUMBER factor[], int version);
       int	optimize	(L_NUMBER zahl[], int version);
       int	rabinstest	(L_NUMBER zahl[]);
       int	start	(L_NUMBER wert[], L_NUMBER per[], L_NUMBER mod[]);
       int	rndm	(int lgth, L_NUMBER zahl[]);
#else
#ifdef _SECURE_
       void	octetstoln	();
       void	lntoctets	();
       void	bitstoln	();
       void	lntobits	();
       RC	aux_LN2BitString2	();
       RC	aux_LN2OctetString2	();
       RC	aux_OctetString2LN2	();
       RC	aux_BitString2LN2	();
       BitString *aux_LN2BitString	();
       OctetString *aux_LN2OctetString	();
       L_NUMBER *aux_OctetString2LN	();
       L_NUMBER *aux_BitString2LN	();
#endif
#ifdef ARITHMETIC_TEST
       void arithmetic_trace_print();
#endif
       int	intlog2	();
       int	lngtouse	();

       int	madd	();
       int	msub	();
       int	mmult	();
       int	mdiv	();
       int	mexp	();
       void	mexp2	();
       void	_xor	();
       int	_comp	();
       void	_add	();
       int	_cadd	();
       void	_div	();
       void	_dmult	();
       void	_mult	();
       void	_shift	();
       void	_sub	();
       void	_ddiv	();
       int	primzahl	();
       int	nextprime	();
       int	optimize	();
       int	rabinstest	();
       int	start	();
       int	rndm	();

#endif
#endif
