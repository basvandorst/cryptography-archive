/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*							   RSA Library Interface						*
*							  RSA.H  Updated 23/08/91						*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*			Copyright 1991  Phil Zimmermann.  All rights reserved			*
*						Adapted 1991  Peter C.Gutmann						*
*																			*
****************************************************************************/

#include "defs.h"

/* Define the following to enable fast assembly-language RSA primitives */

#ifdef __MSDOS__
  #define ASM_RSA
#endif /* __MSDOS__ */

/* Basic type for the multiprecision registers.  The internal representation
   for these extended precision integer registers is an array of "units",
   where a unit is a machine word, in this case a WORD.  To perform arithmetic
   on these huge precision integers, we pass pointers to these unit arrays to
   various subroutines */

#define MP_REG			WORD

/* Macros to convert a bitcount to various other values */

#define UNITSIZE		16			/* 16 bits per machine word */

#define bits2units(n)	( ( ( n ) + 15 ) >> 4 )
#define units2bits(n)	( ( n ) << 4 )
#define bytes2units(n)	( ( ( n ) + 1 ) >> 1 )
#define units2bytes(n)	( ( n ) << 1 )
#define bits2bytes(n)	( ( ( n ) + 7 ) >> 3 )

/* MAX_BIT_PRECISION is the expected upper limit on key size.  Note that
   MAX_BIT_PRECISION >= max.key length + SLOP_BITS.  In our case we have
   a maximum key length of 2048 bits + 17 slop bits = 2080 bits (rounded) */

#define	MAX_BIT_PRECISION	2080
#define	MAX_BYTE_PRECISION	( MAX_BIT_PRECISION / 8 )
#define	MAX_UNIT_PRECISION	( MAX_BIT_PRECISION / UNITSIZE )

extern int globalPrecision;			/* Level of precision for all routines */

/* Defines for the RSA routines themselves */

#define mp_burn				mp_clear		/* For destroying sensitive data */
#define mp_modsquare(r1,r2)	mp_modmult( r1, r2, r2 )
#define mp_shiftleft(r1)	mp_rotateleft( r1, 0 )

#ifndef ASM_RSA
  /* The following primitives should be coded in assembly */
  void mp_setp( const int precision );
  void mp_add( MP_REG *reg1, MP_REG *reg2 );
  BOOLEAN mp_sub( MP_REG *reg1, MP_REG *reg2 );
  void mp_rotateleft( MP_REG *reg1, const BOOLEAN carry );

  /* Equivalent functions for some of the primitives */
  #define setPrecision(prec)	mp_setp( units2bits( globalPrecision = ( prec ) ) )
  #define mp_move(dest,src)		memcpy( dest, src, globalPrecision * sizeof( MP_REG ) )
  #define mp_clear(reg,count)	memset( reg, 0, ( count ) * sizeof( MP_REG ) )
#else
  /* The following primitives should be coded in assembly */
  void MP_SETP( const int precision );
  void MP_MOVE( MP_REG *dest, MP_REG *src );
  void MP_CLR( MP_REG *mpReg, const int precision );
  void MP_ADD( MP_REG *reg1, MP_REG *reg2 );
  BOOLEAN MP_SUB( MP_REG *reg1, MP_REG *reg2 );
  void MP_ROL( MP_REG *reg1, const BOOLEAN carry );

  /* Define C primitive names as the equivalent calls to assembly primitives */
  #define setPrecision(precision)	MP_SETP( units2bits( globalPrecision = ( precision ) ) )
  #define mp_add					MP_ADD
  #define mp_sub					MP_SUB
  #define mp_rotateleft				MP_ROL
  #define mp_move					MP_MOVE
  #define mp_clear					MP_CLR
#endif	/* !ASM_RSA */

/* SLOP_BITS is how many "carry bits" to allow for intermediate calculation
   results to exceed the size of the modulus.  It is used by modexp to give
   some overflow elbow room for modmult to use to perform modulo operations
   with the modulus.  The number of slop bits required is determined by the
   modmult algorithm */

#define SLOP_BITS		( UNITSIZE + 1 )

/* Defines to perform various tests on MPI's */

#define testEq(mpi,val)	( ( *( mpi ) == ( val ) ) && ( significance( mpi ) <= 1 ) )
#define countbits(mpi)	initBitSniffer( mpi )
#define countbytes(mpi)	( ( countbits( mpi ) + 7 ) >> 3 )
#define normalize(mpi)	mpi += significance( mpi ) - 1

/* Prototypes for functions in RSA.C */

int initBitSniffer( MP_REG *mpReg );
int significance( MP_REG *mpReg );
void mp_init( MP_REG *mpReg, WORD value );
void mp_modexp( MP_REG *expOut, MP_REG *expIn, MP_REG *exponent, MP_REG *modulus );
void rsaDecrypt( MP_REG *M, MP_REG *C, MP_REG *d, MP_REG *p, MP_REG *q, MP_REG *u );
