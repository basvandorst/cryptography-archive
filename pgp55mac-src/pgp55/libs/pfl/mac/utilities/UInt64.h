/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: UInt64.h,v 1.9 1997/09/18 01:34:59 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include "pgpTypes.h"

class UInt64
	{
	protected:
			void	InitLong( long num )
						{
						lo	= num;
						if ( num < 0 )
							hi = -1;
						else
							hi = 0;
						}
						
			void	InitULong( unsigned long num )
						{
						lo	= num;
						hi	= 0;
						}
	public:
	unsigned long	hi;
	unsigned long	lo;

	void			Init( unsigned long hiArg, unsigned long loArg )
						{
						hi	= hiArg;
						lo	= loArg;
						}
// constructors
	UInt64( void )		{	}
	UInt64( const UnsignedWide &num )
		{
		Init( num.hi, num.lo );
		}
	UInt64( const wide &num )
		{
		Init( num.hi, num.lo );
		}
										
	UInt64( unsigned long hiNum, unsigned long loNum)
		{ 
		hi	= hiNum;
		lo	= loNum;
		}
		
	UInt64( unsigned long	num)		{	InitULong( num ); }
	UInt64( unsigned char	num)		{	InitULong( (unsigned long)num ); }
	UInt64( unsigned short	num)		{	InitULong( (unsigned long)num ); }
	UInt64( unsigned int	num)		{	InitULong( (unsigned long)num ); }
	
	
	UInt64( long theLong)		{	InitLong( (long)theLong ); }
	UInt64( char	theChar)	{	InitLong( (long)theChar ); }
	UInt64( short	theShort)	{	InitLong( (long)theShort ); }
	UInt64( int		theInt)		{	InitLong( (long)theInt ); }

	operator	UnsignedWide( void ) const
					{
					return( *(const UnsignedWide *)this );
					}
					
	operator	wide( void ) const
					{
					return( *(const wide *)this );
					}

#if PGP_DEBUG
	static void			sTest( void );
#endif
	};



inline UInt64	operator ~( const UInt64 &lhs )
					{
					return( UInt64( ~lhs.hi, ~lhs.lo ) );
					}
					
UInt64			operator +( const UInt64 &lhs, const UInt64 &rhs);

void			operator +=( UInt64 &lhs, UInt64 rhs);
					
UInt64			operator -( const UInt64 &lhs );
					
UInt64			operator -( const UInt64 &lhs, const UInt64 &rhs);

					
					
inline int		operator ==( const UInt64 &lhs, const UInt64 &rhs)
					{
					return( lhs.lo == rhs.lo && lhs.hi == rhs.hi );
					}
					
inline int		operator !=( const UInt64 &lhs, const UInt64 &rhs)
					{
					return( lhs.lo != rhs.lo || lhs.hi != rhs.hi );
					}
					
					
int				operator <( const UInt64 &lhs, const UInt64 &rhs);
int				operator <=( const UInt64 &lhs, const UInt64 &rhs);

inline int		operator >( const UInt64 &lhs, const UInt64 &rhs )
					{ return( ! ( lhs <= rhs ) ); }
inline int		operator >=( const UInt64 &lhs, const UInt64 &rhs)
					{ return( ! ( lhs < rhs ) ); }

					
					
void			operator >>=( UInt64 &lhs, const unsigned long rhs);
UInt64			operator >>( const UInt64 &lhs, const unsigned long count);

void			operator <<=( UInt64 &lhs, const unsigned long rhs);
UInt64			operator <<( const UInt64 &lhs, const unsigned long count);
















