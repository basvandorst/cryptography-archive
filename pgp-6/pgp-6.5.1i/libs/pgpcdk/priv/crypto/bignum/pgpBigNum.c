/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpBigNum.c,v 1.11 1999/03/10 02:47:07 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpBigNum.h"
#include "bn.h"




#define IsBNError( i )			( (i) != 0 )
#define IsntBNError( i )		( (i) == 0 )


struct PGPBigNum
{
	BigNum			bn;
};

	static PGPBoolean
pgpBigNumIsValid( PGPBigNumRef bn )
{
	return( IsntNull( bn ) &&
		PGPMemoryMgrIsValid( bn->bn.mgr ) );
}
#define pgpValidateBigNum( bn )	PGPValidateParam( pgpBigNumIsValid( bn ) )

/* Creates a new bignum */
	PGPError
PGPNewBigNum(
	PGPMemoryMgrRef	mgr,
	PGPBoolean		secure,
	PGPBigNumRef *	newBN )
{
	PGPError		err	= kPGPError_NoErr;
	PGPBigNumRef	ref;
	
	PGPValidatePtr( newBN );
	*newBN	= kPGPInvalidBigNumRef;
	
	ref	= (PGPBigNumRef) PGPNewData( mgr,
			sizeof( **newBN ), kPGPMemoryMgrFlags_Clear);
	if ( IsntNull( ref ) )
	{
		bnBegin( &ref->bn, mgr, secure );
		*newBN	= ref;
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* destoys the bignum and all memory it uses */
	PGPError
PGPFreeBigNum( PGPBigNumRef	bn )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	bnEnd( &bn->bn );
	
	err	= PGPFreeData( bn );
	
	return( err );
}



/* Create a new big num with same value as src */
	PGPError
PGPCopyBigNum(
	PGPBigNumRef	src,
	PGPBigNumRef * 	dest )
{
	PGPError		err	= kPGPError_NoErr;
	PGPBigNumRef	temp	= NULL;
	
	PGPValidatePtr( dest );
	*dest	= NULL;
	pgpValidateBigNum( src );
	
	err	= PGPNewBigNum( src->bn.mgr, src->bn.isSecure, &temp );
	if ( IsntPGPError( err ) )
	{
		err	= PGPAssignBigNum( src, temp );
		if ( IsPGPError( err ) )
		{
			PGPFreeBigNum( temp );
			temp	= NULL;
		}
	}
	
	*dest	= temp;
	
	return( err );
}


/* Make existing bignum dest have same value as source */
	PGPError
PGPAssignBigNum(
	PGPBigNumRef	src,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( src );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnCopy( &dest->bn, &src->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}



/* Swap two BigNums.  Very fast. */
	PGPError
PGPSwapBigNum( PGPBigNumRef	a, PGPBigNumRef	b)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( a );
	pgpValidateBigNum( b );
	
	bnSwap( &a->bn, &b->bn );
	
	return( err );
}




/*
 * Move bytes between the given buffer and the given BigNum encoded in
 * base 256.  I.e. after either of these, the buffer will be equal to
 * (bn / 256^lsbyte) % 256^len.  The difference is which is altered to
 * match the other!
 */
	PGPError
PGPBigNumExtractBigEndianBytes(
	PGPBigNumRef	bn,
	PGPByte *			dest,
	PGPUInt32			lsbyte,
	PGPUInt32			len )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	
	bnExtractBigBytes( &bn->bn, dest, lsbyte, len );
	
	return( err );
}

				
	PGPError
PGPBigNumInsertBigEndianBytes(
	PGPBigNumRef	bn,
	PGPByte const *	src,
	PGPUInt32		lsbyte,
	PGPUInt32		len )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	
	if ( IsBNError( bnInsertBigBytes( &bn->bn, src, lsbyte, len ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* The same, but the buffer is little-endian. */
	PGPError
PGPBigNumExtractLittleEndianBytes(
	PGPBigNumRef	bn,
	PGPByte *		dest,
	PGPUInt32		lsbyte,
	PGPUInt32		len )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	
	bnExtractLittleBytes( &bn->bn, dest, lsbyte, len );
	
	return( err );
}

				
	PGPError
PGPBigNumInsertLittleEndianBytes(
	PGPBigNumRef	bn,
	PGPByte const *	src,
	PGPUInt32		lsbyte,
	PGPUInt32		len )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	
	if ( IsBNError( bnInsertLittleBytes( &bn->bn, src, lsbyte, len ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* Return the least-significant bits (at least 16) of the BigNum */
	PGPUInt16
PGPBigNumGetLSWord( PGPBigNumRef	bn )
{
	PGPUInt16	lsWord	= 0;
	
	if ( pgpBigNumIsValid( bn ) )
	{
		lsWord	= bnLSWord( &bn->bn );
	}
	
	return( lsWord );
}


/*
 * Return the number of significant bits in the BigNum.
 * 0 or 1+floor(log2(src))
 */
	PGPUInt32
PGPBigNumGetSignificantBits( PGPBigNumRef	bn )
{
	PGPUInt32	numBits;
	
	pgpValidateBigNum( bn );
	
	numBits	= bnBits( &bn->bn );
	
	return( numBits );
}


/*
 * Adds two bignums into dest.  Faster if dest is same as lhs or rhs.
 */
	PGPError
PGPBigNumAdd(
	PGPBigNumRef	lhs,
	PGPBigNumRef	rhs,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	int			bnError;
	
	pgpValidateBigNum( lhs );
	pgpValidateBigNum( rhs );
	pgpValidateBigNum( dest );
	
	if ( lhs == dest )
	{
		bnError	= bnAdd( &dest->bn, &rhs->bn );
	}
	else if ( rhs == dest )
	{
		bnError	= bnAdd( &dest->bn, &lhs->bn );
	}
	else
	{
		bnError	= bnCopy( &dest->bn, &lhs->bn );
		if ( bnError == 0 )
		{
			bnError	= bnAdd( &dest->bn, &rhs->bn );
		}
	}
	
	if ( IsBNError( bnError ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/*
 * lhs-rhs.  dest and src may be the same, but bnSetQ(dest, 0) is faster.
 * if dest < src, returns error and dest is undefined.
 */
	PGPError
PGPBigNumSubtract(
	PGPBigNumRef	lhs,
	PGPBigNumRef	rhs,
	PGPBigNumRef	dest,
	PGPBoolean *	underflowPtr )
{
	PGPError	err	= kPGPError_NoErr;
	int			bnError;
	PGPBoolean	underflow	= FALSE;
	
	if ( IsntNull( underflowPtr ) )
		*underflowPtr	= FALSE;
		
	pgpValidateBigNum( lhs );
	pgpValidateBigNum( rhs );
	pgpValidateBigNum( dest );
	
	if ( lhs == dest )
	{
		bnError	= bnSub( &dest->bn, &rhs->bn );
		underflow	= (bnError == 1);
		bnError		= 0;
	}
	else if ( rhs == dest )
	{
		BigNum	temp;
		PGPBoolean	secure;
		
		secure	= lhs->bn.isSecure || rhs->bn.isSecure ||
					dest->bn.isSecure;
		bnBegin( &temp, dest->bn.mgr, secure );
			bnError	= bnCopy( &temp, &lhs->bn );
			if ( bnError == 0 )
			{
				bnError	= bnSub( &temp, &rhs->bn );
				underflow	= (bnError == 1);
				bnError		= 0;
				
				bnError	= bnCopy( &dest->bn, &temp );
			}
		bnEnd( &temp );
	}
	else
	{
		bnError	= bnCopy( &dest->bn, &lhs->bn );
		if ( bnError == 0 )
		{
			bnError	= bnSub( &dest->bn, &rhs->bn );
			underflow	= (bnError == 1);
			bnError		= 0;
		}
	}
	
	if ( IsntNull( underflowPtr ) )
		*underflowPtr	= underflow;
		
	if ( bnError == -1 )
		err	= kPGPError_OutOfMemory;
	
	return( err );
}



/* Return sign (-1, 0, +1) of a-b.  a <=> b --> bnCmpQ(a, b) <=> 0 */
	PGPInt32
PGPBigNumCompareQ(
	PGPBigNumRef	bn,
	PGPUInt16		sm )
{
	PGPUInt32	result	= 0;
	
	if ( pgpBigNumIsValid( bn ) )
	{
		result	= bnCmpQ( &bn->bn, sm );
	}
	
	return( result );
}


/* dest = src, where 0 <= src < 2^16. */
	PGPError
PGPBigNumSetQ(
	PGPBigNumRef	dest,
	PGPUInt16		sm)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnSetQ( &dest->bn, sm ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = bn + sm, where 0 <= sm < 2^16 */
	PGPError
PGPBigNumAddQ(
	PGPBigNumRef	bn,
	PGPUInt16		sm,
	PGPBigNumRef	dest)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( bn );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnAddQ( &dest->bn, sm ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = bn + sm, where 0 <= sm < 2^16 */
	PGPError
PGPBigNumSubtractQ(
	PGPBigNumRef	bn,
	PGPUInt16		sm,
	PGPBigNumRef	dest,
	PGPBoolean *	underflowPtr )
{
	PGPError	err	= kPGPError_NoErr;
	int			bnError	= 0;
	PGPBoolean	underflow	= FALSE;
	
	if ( IsntNull( underflowPtr ) )
		*underflowPtr	= FALSE;
		
	pgpValidateBigNum( bn );
	pgpValidateBigNum( dest );

	if ( bn != dest )
	{
		bnError	= bnCopy( &dest->bn, &bn->bn );
		if ( IsBNError( bnError ) )
			err	= kPGPError_OutOfMemory;
	}
	
	if ( IsntBNError( bnError ) )
	{
		bnError	= bnSubQ( &dest->bn, sm );	
		if ( IsBNError( bnError ) )
		{
			underflow	= TRUE;
		}
	}
	
	if ( IsntNull( underflowPtr ) )
		*underflowPtr	= underflow;
	
	return( err );
}


/* Return sign (-1, 0, +1) of a-b.  a <=> b --> bnCmp(a, b) <=> 0 */
	PGPInt32
PGPBigNumCompare(
	PGPBigNumRef	lhs,
	PGPBigNumRef	rhs )
{
	if ( pgpBigNumIsValid( lhs ) && pgpBigNumIsValid( rhs ) )
	{
		return( bnCmp( &lhs->bn, &rhs->bn ) );
	}
	
	return( 0 );
}


/* dest = src * src.  dest may be the same as src, but it costs time. */
	PGPError
PGPBigNumSquare(
	PGPBigNumRef	src,
	PGPBigNumRef	dest)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( src );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnSquare( &dest->bn, &src->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = a * b.  dest may be the same as a or b, but it costs time. */
	PGPError
PGPBigNumMultiply(
	PGPBigNumRef	lhs,
	PGPBigNumRef	rhs,
	PGPBigNumRef	dest)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( lhs );
	pgpValidateBigNum( rhs );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnMul( &dest->bn, &lhs->bn, &rhs->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = a * b, where 0 <= b < 2^16.  dest and a may be the same. */
	PGPError
PGPBigNumMultiplyQ(
	PGPBigNumRef	lhs,
	PGPUInt16		sm,
	PGPBigNumRef	dest)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( lhs );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnMulQ( &dest->bn, &lhs->bn, sm ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/*
 * q = n/d, r = n%d.  r may be the same as n, but not d,
 * and q may not be the same as n or d.
 * re-entrancy issue: this temporarily modifies d, but restores
 * it for return.
 */
	PGPError
PGPBigNumDivide(
	PGPBigNumRef	numerator,
	PGPBigNumRef	denominator,
	PGPBigNumRef	quotient,
	PGPBigNumRef	remainder)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( numerator );
	pgpValidateBigNum( denominator );
	pgpValidateBigNum( quotient );
	pgpValidateBigNum( remainder );
	
	if ( IsBNError( bnDivMod( &quotient->bn,
				&remainder->bn, &numerator->bn, &denominator->bn) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}

/*
 * dest = n % d.  dest and src may be the same, but not dest and d.
 * re-entrancy issue: this temporarily modifies d, but restores
 * it for return.
 */
	PGPError
PGPBigNumMod(
	PGPBigNumRef	numerator,
	PGPBigNumRef	denominator,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( numerator );
	pgpValidateBigNum( denominator );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnMod( &dest->bn, &numerator->bn, &denominator->bn) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* return src % d, where 0 <= d < 2^16.  */
	PGPUInt16
PGPBigNumModQ(
	PGPBigNumRef	numerator,
	PGPUInt16		denominator )
{
	PGPUInt16	result	= 0;
	
	if ( pgpBigNumIsValid( numerator ) && denominator != 0 )
	{
		result	= bnModQ( &numerator->bn, denominator);
	}
	
	return( result );
}


/* n = n^exp, modulo "mod"   "mod" *must* be odd */
	PGPError
PGPBigNumExpMod(
	PGPBigNumRef	n,
	PGPBigNumRef	exponent,
	PGPBigNumRef	mod,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( n );
	pgpValidateBigNum( exponent );
	pgpValidateBigNum( mod );
	pgpValidateBigNum( dest );
	PGPValidateParam( dest != n && dest != exponent && dest != mod );
	
	if ( IsBNError( bnExpMod( &dest->bn,
			&n->bn, &exponent->bn, &mod->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/*
 * dest = n1^e1 * n2^e2, modulo "mod".  "mod" *must* be odd.
 * dest may be the same as n1 or n2.
 */
	PGPError
PGPBigNumDoubleExpMod(
	PGPBigNumRef	n1,
	PGPBigNumRef	exponent1,
	PGPBigNumRef	n2,
	PGPBigNumRef	exponent2,
	PGPBigNumRef	mod,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( n1 );
	pgpValidateBigNum( exponent1 );
	pgpValidateBigNum( n2 );
	pgpValidateBigNum( exponent2 );
	pgpValidateBigNum( mod );
	pgpValidateBigNum( dest );
	PGPValidateParam( dest != n1 && dest != exponent1 &&
			dest != n2 && dest != exponent2 && dest != mod );
	
	
	if ( IsBNError( bnDoubleExpMod( &dest->bn,
			&n1->bn, &exponent1->bn,
			&n2->bn, &exponent2->bn,
			&mod->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = 2^exp, modulo "mod"   "mod" *must* be odd */
	PGPError
PGPBigNumTwoExpMod( 
	PGPBigNumRef	exponent,
	PGPBigNumRef	mod,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( exponent );
	pgpValidateBigNum( mod );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnTwoExpMod( &dest->bn, &exponent->bn, &mod->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = gcd(a, b).  The inputs may overlap arbitrarily. */
	PGPError
PGPBigNumGCD(
	PGPBigNumRef	a,
	PGPBigNumRef	b,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( a );
	pgpValidateBigNum( b );
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnGcd( &dest->bn, &a->bn, &b->bn ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* dest = src^-1, modulo "mod".  dest may be the same as src. */
	PGPError
PGPBigNumInv(
	PGPBigNumRef	src, 
	PGPBigNumRef	mod,
	PGPBigNumRef	dest )
{
	PGPError	err	= kPGPError_NoErr;
	int			bnError	= 0;
	
	pgpValidateBigNum( src );
	pgpValidateBigNum( mod );
	pgpValidateBigNum( dest );
	
	bnError	= bnInv( &dest->bn, &src->bn, &mod->bn );
	if ( IsBNError( bnError ) )
	{
		if ( bnError == 1 )
			err	 = kPGPError_BigNumNoInverse;
		else
			err	= kPGPError_OutOfMemory;
	}

	return( err );
}



/* Shift dest left "amt" places */
	PGPError
PGPBigNumLeftShift(
	PGPBigNumRef	dest,
	PGPUInt32		numBits )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( dest );
	
	if ( IsBNError( bnLShift( &dest->bn, numBits ) ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


/* Shift dest right "amt" places, discarding low-order bits */
	PGPError
PGPBigNumRightShift(
	PGPBigNumRef	dest,
	PGPUInt32		numBits )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateBigNum( dest );
	
	bnRShift( &dest->bn, numBits );
	
	return( err );
}


/* right shift all low order 0-bits, return number of bits shifted */
	PGPUInt16
PGPBigNumMakeOdd( PGPBigNumRef	dest )
{
	PGPUInt16	numBits	= 0;
	
	if ( pgpBigNumIsValid( dest ) )
	{
		numBits	= bnMakeOdd( &dest->bn );
	}
	
	return( numBits );
}


































/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
