/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

#include "config.h"
#include "lib/bn.h"
#include "java_math_BigNum.h"

#define GOOD 1
#define BAD 0
#define pointerFrom(x) (BIGNUM *)(unhand(x)->pointer_)

typedef struct Hjava_math_BigNum * bignumPtr;

long java_math_BigNum_hashCode( bignumPtr this )
{
	BIGNUM * thisContext = pointerFrom( this );
	if ( thisContext->top == 0 )
		return 0;
	return thisContext->d[0];
}

long java_math_BigNum_setToOne( bignumPtr this )
{
	return bn_one( pointerFrom( this ) );
}

long java_math_BigNum_setToZero( bignumPtr this )
{
	bn_zero( pointerFrom( this ) );
	return GOOD;
}

long java_math_BigNum_bignum_new( bignumPtr this )
{
	BIGNUM * ptr = bn_new();
	if (ptr)
	{
		unhand( this )->pointer_ = (long)ptr;
		return GOOD;
	}
	return BAD;
}

void java_math_BigNum_bignum_free( bignumPtr this)
{
	bn_free( pointerFrom( this ) );
}

long java_math_BigNum_byteLen( bignumPtr this )
{
	return bn_num_bytes( pointerFrom( this ) );
}

long java_math_BigNum_bitLen( bignumPtr this )
{
	return bn_num_bits( pointerFrom( this ) );
}

long java_math_BigNum_intoBytes( bignumPtr this, HArrayOfByte * bufPtr )
{
	return bn_bn2bin( pointerFrom( this ), unhand( bufPtr )->body );
}

long java_math_BigNum_fromBytes( bignumPtr this, HArrayOfByte * bufPtr )
{
	BIGNUM * ptr = bn_bin2bn( obj_length( bufPtr ), unhand( bufPtr )->body, (BIGNUM *)0 );
	if (ptr)
	{
		unhand( this )->pointer_ = (long)ptr;
		return GOOD;
	}
	return BAD;
}

long java_math_BigNum_sn_add( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr b )
{
	return bn_add( pointerFrom( r ), pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_sub( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr b )
{
	return bn_sub( pointerFrom( r ), pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_mul( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr b )
{
	return bn_mul( pointerFrom( r ), pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_mod( bignumPtr none, bignumPtr rem, bignumPtr m, bignumPtr d )
{
	return bn_mod( pointerFrom( rem ), pointerFrom( m ), pointerFrom( d ) );
}

long java_math_BigNum_sn_div( bignumPtr none, bignumPtr dv, bignumPtr rem, bignumPtr m, bignumPtr d )
{
	BIGNUM * remPtr = NULL;
	/* this is added as you can not unhand a null pointer and it is allowable to
	   use a null pointer here thus no mod returned from the divide. */
	if ( rem )
		remPtr = pointerFrom( rem );
	return bn_div( pointerFrom( dv ), remPtr, pointerFrom( m ), pointerFrom( d ) );
}

long java_math_BigNum_sn_ucmp( bignumPtr none, bignumPtr a, bignumPtr b)
{
	return bn_Ucmp( pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_cmp( bignumPtr none, bignumPtr a, bignumPtr b)
{
	return bn_cmp( pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_lshift( bignumPtr none, bignumPtr r, bignumPtr a, short n )
{
	return bn_lshift( pointerFrom ( r ), pointerFrom( a ), n );
}

long java_math_BigNum_sn_lshift1( bignumPtr none, bignumPtr r, bignumPtr a)
{
	return bn_lshift1( pointerFrom( r ), pointerFrom( a ) );
}

long java_math_BigNum_sn_rshift( bignumPtr none, bignumPtr r, bignumPtr a, short n )
{
	return bn_rshift( pointerFrom( r ), pointerFrom( a ), n );
}

long java_math_BigNum_sn_rshift1( bignumPtr none, bignumPtr r, bignumPtr a )
{
	return bn_rshift1( pointerFrom( r ), pointerFrom( a ) );
}

long java_math_BigNum_sn_mod_exp( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr p, bignumPtr m )
{
	return bn_mod_exp( pointerFrom( r ), pointerFrom( a ), pointerFrom( p ), pointerFrom( m ) );	
}

long java_math_BigNum_sn_modmul_recip( bignumPtr none, bignumPtr r, bignumPtr x, bignumPtr y, bignumPtr m, bignumPtr i, short nb )
{
	return bn_modmul_recip( pointerFrom( r ), pointerFrom( x ), pointerFrom( y ), pointerFrom( m ), pointerFrom( i ), nb);
}

long java_math_BigNum_sn_mod_mul( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr b, bignumPtr m )
{
	return bn_mul_mod( pointerFrom( r ), pointerFrom( a ), pointerFrom( b ), pointerFrom( m ) );
}

long java_math_BigNum_sn_reciprical( bignumPtr none, bignumPtr r, bignumPtr m )
{
	return bn_reciprical( pointerFrom( r ), pointerFrom( m ) );
}

long java_math_BigNum_sn_gcd( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr b )
{
	return bn_gcd( pointerFrom( r ), pointerFrom( a ), pointerFrom( b ) );
}

long java_math_BigNum_sn_copy( bignumPtr none, bignumPtr a, bignumPtr b )
{
	bn_copy( pointerFrom( a ), pointerFrom( b ) );
	return GOOD;
}

long java_math_BigNum_sn_inverse_modn( bignumPtr none, bignumPtr r, bignumPtr a, bignumPtr n )
{
	return bn_inverse_modn( pointerFrom( r ), pointerFrom( a ), pointerFrom( n ) );
}

long java_math_BigNum_sn_add_word( bignumPtr none, bignumPtr a, long w )
{
	return bn_add_word( pointerFrom( a ), w );
}

long java_math_BigNum_sn_set_word( bignumPtr none, bignumPtr a, long w )
{
	return bn_set_word( pointerFrom( a ), w );
}

