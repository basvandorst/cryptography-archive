/*____________________________________________________________________________
	Copyright (C) 1996 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/
#include "UInt64.h"




#if ! GENERATINGCFM

	#pragma parameter __D0 PrimAdd32WithCarry(__D0, __A0)
	UInt32		PrimAdd32WithCarry(UInt32 src, UInt32 * result)
	 THREEWORDINLINE(0xD190, 0x7000, 0xD180);
	 
	 
	#pragma parameter PrimAdd64(__A0, __A1)
	void		PrimAdd64(const UInt64 * src, UInt64 * result)
	 NINEWORDINLINE(0x2028, 0x0004, 0xD0A9,\
		 0x0004, 0x2210, 0x2411, 0xD382, 0x22C1, 0x2280);
	
	
#else

	static inline UInt32
PrimAdd32WithCarry(
	UInt32		src,
	UInt32 *		dest)
	{
	UInt32	result;
	UInt32	carry = 0;
	
	result	= src + *dest;
	*dest	= result;
	if ( result < src )
		carry	= 1;
		
	return( carry );
	}
	
	
	static inline void
PrimAdd64(
	const UInt64 *	src,
	UInt64 *		result)
	{
	result->hi	+= src->hi + PrimAdd32WithCarry( src->lo, &result->lo );
	}
	
#endif



#pragma mark -


	UInt64
operator -( const UInt64 &lhs )
	{
	UInt64	result( ~lhs.hi, ~lhs.lo );
	result.hi	+= PrimAdd32WithCarry( 1, &result.lo );
	return( result );
	}
	
	
	UInt64
operator -(
	const UInt64 &lhs,
	const UInt64 &rhs)
	{
	// form negative first
	UInt64	result( ~rhs.hi,~rhs.lo );
	result.hi	+= PrimAdd32WithCarry( 1, &result.lo );
	
	// now add together
	PrimAdd64( &lhs, &result );
	
	return( result );
	}
	
	
	UInt64
operator +(
	const UInt64 &lhs,
	const UInt64 &rhs)
	{
	UInt64	result	= rhs;
	
	PrimAdd64( &lhs, &result );
	
	return( result );
	}



	void
operator +=(
	UInt64 &	lhs,
	UInt64		rhs)
	{
	PrimAdd64( &rhs, &lhs );
	}
	
	
	int
operator <(
	const UInt64 &	lhs,
	const UInt64 &	rhs)
	{
	if ( lhs.hi < rhs.hi )
		{
		return( 1 );
		}
		
	if ( lhs.hi == rhs.hi )
		{
		return( lhs.lo < rhs.lo );
		}
		
	return( 0 );
	}
	
	
	int
operator <=(
	const UInt64 &	lhs,
	const UInt64 &	rhs)
	{
	if ( lhs.hi < rhs.hi )
		{
		return( 1 );
		}
		
	if ( lhs.hi == rhs.hi )
		{
		return( lhs.hi <= rhs.hi );
		}
		
	return( 0 );
	}



	void
operator <<=(
	UInt64 &		lhs,
	UInt32			count)
	{
	if ( count < 32 )
		{
		lhs.hi	<<= count;
		lhs.hi	|= lhs.lo >> ( 32 - count );
		lhs.lo	<<= count;
		}
	else
		{
		lhs.hi	= lhs.lo << ( count - 32 );
		lhs.lo	= 0;
		}
	}


	
	UInt64
operator <<(
	const UInt64 &	lhs,
	UInt32			count)
	{
	UInt64	result	= lhs;
	
	result	<<= count;
	return( result );
	}
	
	
	
	void
operator >>=(
	UInt64 &		lhs,
	UInt32			count)
	{
	if ( count < 32 )
		{
		lhs.lo	>>= count;
		lhs.lo	|= lhs.hi << ( 32 - count );
		lhs.hi	>>= count;
		}
	else
		{
		lhs.lo	= lhs.hi >> ( count - 32 );
		lhs.hi	= 0;
		}
	}
	
	
	
	UInt64
operator >>(
	const UInt64 &	lhs,
	UInt32			count)
	{
	UInt64	result	= lhs;
	
	result	>>= count;
	return( result );
	}
	
	

#if PGP_DEBUG	// [

	static void
DebugCString( const char *cString)
	{
	pgpDebugMsg( cString );
	}
	

	void
UInt64::sTest( void )
	{
	const UInt64	zero	= 0UL;
	UInt64			negOne	= -1;

#define TestAssert( cond ) \
	do { if ( ! (cond ) ) {DebugCString( #cond ); } } while ( FALSE )

	pgpAssert( zero.hi == 0 && zero.lo == 0 );
	
	// test for assignment of negative char, short int, long
	negOne	= (char)-1;
	TestAssert( negOne.hi == -1 && negOne.lo == -1 );
	negOne	= (int)-1;
	TestAssert( negOne.hi == -1 && negOne.lo == -1 );
	negOne	= (short)-1;
	TestAssert( negOne.hi == -1 && negOne.lo == -1 );
	negOne	= (long)-1;
	TestAssert( negOne.hi == -1 && negOne.lo == -1 );
	
	
	// test for assignment of unsigned char, short int, long
	UInt64	temp;
	temp	= (uchar)-1;
	TestAssert( temp.hi == 0 && temp.lo == (uchar)-1 );
	temp	= (uint)-1;
	TestAssert( temp.hi == 0 && temp.lo == (uint)-1 );
	temp	= (UInt16)-1;
	TestAssert( temp.hi == 0 && temp.lo == (UInt16)-1 );
	temp	= (UInt32)-1;
	TestAssert( temp.hi == 0 && temp.lo == (UInt32)-1 );
	
	
	// test addition
	TestAssert( UInt64( 0, 0 ) + UInt64( 0, 0 ) == zero );
	TestAssert( UInt64( 0, 0 ) + UInt64( 0, 0 ) == 0 );
	TestAssert( negOne + 1 == zero );
	TestAssert( 1 + negOne == zero );
	TestAssert( UInt64( 0, -1) + 1 == UInt64( 1, 0) );
	TestAssert( UInt64( 1 ) + UInt64( 1 ) == UInt64( 2 ) );
	TestAssert( UInt64( 1, 0) + UInt64( 1, 0) == UInt64( 2, 0) );
	TestAssert( UInt64( 100, 0) + UInt64( 0, 100) == UInt64( 100, 100) );


	// test subtraction
	TestAssert( UInt64( 1 ) - UInt64( 1 ) == zero );
	TestAssert( UInt64( 2 ) - UInt64( 1 ) == UInt64( 1 ) );
	TestAssert( UInt64( 1, 0) - (UInt64( 1, 0) - 1) == UInt64( 1 ) );
	
	
	// test comparisons
	TestAssert( UInt64( 0 ) == UInt64( 0 ) );
	TestAssert( UInt64( 1 ) == UInt64( 1 ) );
	TestAssert( UInt64( 1, 1) == UInt64( 1, 1) );
	TestAssert( UInt64( 1 ) != UInt64( 2 ) );
	TestAssert( UInt64( 1, 1) != UInt64( 2, 2) );
	TestAssert( UInt64( 1, 1) != UInt64( 1, 2) );
	TestAssert( UInt64( 1, 1) != UInt64( 2, 1) );
	
	TestAssert( UInt64( 1, 1) < UInt64( 1, 2) );
	TestAssert( UInt64( 1, 1) < UInt64( 2, 1) );
	TestAssert( UInt64( 1, 1) <= UInt64( 1, 1) );
	TestAssert( UInt64( 1, 1) <= UInt64( 1, 2) );
	TestAssert( UInt64( 1, 1) <= UInt64( 2, 1) );
	
	// test shifts
	TestAssert( UInt64( 0, 1 ) << 1 == UInt64( 0, 2 ) );
	TestAssert( UInt64( 0, 1 ) << 32 == UInt64( 1, 0 ) );
	TestAssert( UInt64( 0, 0x80000000 ) << 1 == UInt64( 1, 0 ) );
	TestAssert( UInt64( -1, -1 ) << 64 == UInt64( 0, 0 ) );
	TestAssert( UInt64( -1, -1 ) << 32 == UInt64( -1, 0 ) );
	TestAssert( UInt64( -1, -1 ) << 63 == UInt64( 0x80000000, 0 ) );
	TestAssert( UInt64( -1, -1 ) << 64 == UInt64( 0, 0 ) );
	
	TestAssert( UInt64( 0, 2 ) >> 1 == UInt64( 0, 1 ) );
	TestAssert( UInt64( 1, 0 ) >> 32 == UInt64( 0, 1 ) );
	TestAssert( UInt64( 1, 1 ) >> 1 == UInt64( 0, 0x80000000 ) );
	TestAssert( UInt64( -1, -1 ) >> 64 == UInt64( 0, 0 ) );
	TestAssert( UInt64( -1, -1 ) >> 32 == UInt64( 0, -1 ) );
	TestAssert( UInt64( -1, -1 ) >> 63 == UInt64( 0, 1 ) );
	}



class TestUInt64
	{
	public:
		TestUInt64( void )
			{
			UInt64::sTest();
			}
	};
static TestUInt64	sTest;

#endif	// ]
	
	
