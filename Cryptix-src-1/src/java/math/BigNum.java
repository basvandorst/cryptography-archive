/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

/**
 * This class is public, but the constructor is protected.
 * It is intended that this class be used as a base for the BigInteger
 * wrapper class, and not used directly.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.01, 27/06/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class BigNum implements Cloneable
{
	private static BigNum staticOne;

	static 
	{
		// load the DLL or shared library that contains the native code
		// implementation of the big integer routines.
		System.loadLibrary( "bignum" );

		// create a static BigNum that will be set to one.
		staticOne = new BigNum();
		staticOne.setToOne();
	}

	private int pointer_; // N.B. may need to be long if we are running on a 64bit machine

	protected BigNum()
	{
		bignum_new();
	}
	
	public Object
	clone()
	{
		BigNum r = new BigNum();
		copy( r, this );
		return r;
	}
	
	public void
	copy( Object src )
	{
		copy( this, (BigNum)src );
	}
	
	protected synchronized static void
	copy( BigNum dst, BigNum src )
	{
		if ( ( sn_copy( dst, src ) ) == 0 )
			throw new MathError("copy failed");
	}

	public synchronized static void
	one( BigNum a )
	{
		if ( a.setToOne() == 0 )
			throw new MathError("set to one failed");
	}

	public synchronized static void
	zero( BigNum a )
	{
		if ( a.setToZero() == 0 )
			throw new MathError("set to zero failed");
	}

	public static void
	inc(BigNum a) // N.B. add is synchronized 
	{
		add( a, a, staticOne );
	}

	public static void
	dec(BigNum a) // N.B. sub is synchronized 
	{
		sub( a, a, staticOne );
	}

	public synchronized static void
	add(BigNum r ,BigNum a, BigNum b)
	{
		if ( sn_add( r, a, b ) == 0 )
			throw new MathError("addition failed");
	}

	public synchronized static void
	add( BigNum r, int val )
	{
		if ( sn_add_word( r, val ) == 0 )
			throw new MathError("add integer failed");
	}

	public synchronized static void
	sub(BigNum r ,BigNum a, BigNum b)
	{
		if ( sn_sub( r, a, b ) == 0 )
			throw new MathError("subtraction failed");
	}

	/**
	 * <b>WARNING</b> the result must not be a <b>or</b> b.
	 * See the bn_mul.c source file for reason.
	 */
	public synchronized static void
	mul(BigNum r ,BigNum a, BigNum b)
	{
		if ( r == a || r == b )
			throw new MathError("Result must no be either a to b multiply");
		if ( sn_mul( r, a, b ) == 0 )
			throw new MathError("multiply failed");
	}

	public synchronized static void
	mod(BigNum r ,BigNum m, BigNum d)
	{
		if ( sn_mod( r, m, d ) == 0 )
			throw new MathError("modulo failed");
	}

	public synchronized static void
	div(BigNum dv ,BigNum m, BigNum d)
	{
		if ( sn_div( dv, null, m, d ) == 0 )
			throw new MathError("divide failed");
	}

	public synchronized static void
	div(BigNum dv ,BigNum rem, BigNum m, BigNum d)
	{
	System.out.println( "calling sn_div" );
		if ( sn_div( dv, rem, m, d ) == 0 )
			throw new MathError("divide failed");
	}
	
	public synchronized static int
	ucmp( BigNum a, BigNum b)
	{
		return sn_ucmp( a, b );
	}

	// returns 0 if a==b
	// returns -ve if a<b
	// returns +ve if a>b
	//
	public synchronized static int
	cmp( BigNum a, BigNum b)
	{
		return sn_cmp( a, b );
	}

	public synchronized static void
	shiftLeft(BigNum r, BigNum a, short n)
	{
		if ( sn_lshift( r, a, n ) == 0 )
			throw new MathError( "shift left failed" );
	}

	public synchronized static void
	shiftLeftOnce(BigNum r, BigNum a)
	{
		if ( sn_lshift1( r, a ) == 0 )
			throw new MathError( "shift left once failed" );
	}
	
	public synchronized static void
	shiftRight(BigNum r, BigNum a, short n)
	{
		if ( sn_rshift( r, a, n ) == 0 )
			throw new MathError( "shift right failed" );
	}

	public synchronized static void
	shiftRightOnce(BigNum r, BigNum a)
	{
		if ( sn_rshift1( r, a ) == 0 )
			throw new MathError( "shift right once failed" );
	}

	public synchronized static void
	modExp( BigNum r, BigNum a, BigNum power, BigNum modulo)
	{
		if ( sn_mod_exp( r, a, power, modulo ) == 0 )
			throw new MathError("modulo exp failed");
	}

	public synchronized static void
	modMul( BigNum r, BigNum a, BigNum b, BigNum modulo)
	{
		if ( sn_mod_mul( r, a, b, modulo ) == 0 )
			throw new MathError("modulo multiply failed");
	}

	public synchronized static int
	recip( BigNum r, BigNum m )
	{
		int returnValue;
		if ( ( returnValue = sn_reciprical( r, m ) ) == -1 )
			throw new MathError("reciprical failed");
		return returnValue;
	}

	public synchronized static void
	gcd( BigNum r, BigNum a, BigNum b )
	{
		if ( sn_gcd( r, a, b ) == 0 )
			throw new MathError("gcd failed");
	}

	public synchronized static void
	modMulRecip(BigNum r, BigNum x, BigNum y, BigNum m, BigNum i, short nb )
	{
		if ( sn_modmul_recip( r, x, y, m, i, nb ) == 0 )
			throw new MathError("modulo reciprical failed");
	}

	public synchronized static void
	inverseModN( BigNum r, BigNum a, BigNum n )
	{
		if ( sn_inverse_modn( r, a, n ) == 0 )
			throw new MathError("inverse modulo n failed");
	}

	public synchronized static void
	assign( BigNum r, int val )
	{
		if ( sn_set_word( r, val ) == 0 )
			throw new MathError("set to integer failed");
	}

	public int
	intoBinary(byte buffer[])
	{
		int r;
		if ( ( buffer.length < ( byteLen() ) ) )
			throw new MathError("into-binary buffer too small");
		if ( ( r = intoBytes( buffer ) ) == 0 )
			throw new MathError("into-binary failed");
		return r;
	}

	protected void 
	fromBinary( byte buffer[])
	{
		if ( ( fromBytes( buffer ) ) == 0 )
			throw new MathError("from-binary failed");
	}

	public String
	toString()
	{
		throw new MathError("BigNums can not natively be strings.");
	}
	
	public int byteLength()
	{
		return byteLen();
	}

	public int bitLength()
	{
		return bitLen();
	}

	protected void finalize()
	{
		bignum_free();
	}
	
	public native int hashCode(); // just returns the MSB word but this allows use in hashtables etc.

	private native int setToOne();
	private native int setToZero();
	private native int bignum_new();
	private native void bignum_free();
	private native int byteLen();
	private native int bitLen();
	private native int intoBytes( byte[] buffer );
	private native int fromBytes( byte[] buffer );

	private native static int sn_add( BigNum r, BigNum a, BigNum b );
	private native static int sn_sub( BigNum r, BigNum a, BigNum b );
	private native static int sn_mul( BigNum r, BigNum a, BigNum b );
	private native static int sn_mod( BigNum rem, BigNum m, BigNum d );
	private native static int sn_div( BigNum dv, BigNum rem, BigNum m, BigNum d );
	private native static int sn_ucmp( BigNum a, BigNum b );
	private native static int sn_cmp( BigNum a, BigNum b );
	private native static int sn_lshift( BigNum r, BigNum a, short n );
	private native static int sn_lshift1( BigNum r, BigNum a );
	private native static int sn_rshift( BigNum r, BigNum a, short n );
	private native static int sn_rshift1( BigNum r, BigNum a );
	private native static int sn_mod_exp( BigNum r, BigNum a, BigNum p, BigNum m );
	private native static int sn_modmul_recip( BigNum r, BigNum x, BigNum y, BigNum m, BigNum i, short nb );
	private native static int sn_mod_mul( BigNum r, BigNum a, BigNum b, BigNum m );
	private native static int sn_reciprical( BigNum r, BigNum m );
	private native static int sn_gcd( BigNum r, BigNum a, BigNum b );
	private native static int sn_copy( BigNum a, BigNum b );
	private native static int sn_inverse_modn( BigNum r, BigNum a, BigNum n );
	private native static int sn_add_word( BigNum a, int w );
	private native static int sn_set_word( BigNum a, int w );
}

