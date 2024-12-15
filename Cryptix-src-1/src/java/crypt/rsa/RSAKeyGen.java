/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.crypt.rsa;

import java.math.BigNum;
import java.math.BigInteger;
import java.math.RandomStream;
import java.math.TestPrime;

/**
 * This class is an RSA key pair generator.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class RSAKeyGen 
{
	protected RandomStream rand;
	
	/**
	 * @param rand0 the source of random numbers for the prime number
	 * generation.
	 */
	public RSAKeyGen( RandomStream rand0 )
	{
		rand = rand0;
	}
	
	/**
	 * An observer class is used to monitor progress.<p>
	 * @ param bitlen the bit length of the final n
	 * @ param obsrv the callback interface.
	 * @ return a random prime number.
	 */
	public BigInteger
	randomPrime( int bitlen, RSAKeyGenObserver obsrv )
	{
		BigInteger ret = new BigInteger();
		int bytelen = 1 + ( bitlen / 8 ), offset = bitlen % 8;
		byte buf[] = new byte[bytelen];
		
		while ( true )
		{
		// make big endian random number.
			if ( rand.read( buf ) != bytelen )
				throw new Error( "Not using a java.math.RandomStream !!!" );

			buf[bytelen-1] |= 0x01; // make it odd.
		// set top bit, and clear the others.
			if ( offset == 0 )
			{
				buf[0] = 0;
				buf[1] |= (byte)0x80;
			}
			else
			{
				buf[0] &= (byte)( 0xFF >>> ( 8 - offset ) );
				buf[0] |= (byte)( 1 << ( offset - 1 ) );
			}
			ret.fromByteArray( buf );
			if ( TestPrime.isPrime( ret, obsrv ) )
				return ret;
		}
	}

	/**
	 * This function creates a Secret Key
	 * <p>
	 *<b>N.B.</b> this can take a LONG time.
	 * @ param rand the source of random data for the prime number generation.
	 * @ param bitlen the bit length of the final n
	 * @ return a random RSA Secret Ket.
	 */
	public static final SecretKey
	createKey( RandomStream rand, int bitlen )
	{
		return createKey( rand, bitlen, null );
	}

	/**
	 * An observer class is used to monitor progress.<p>
	 * @ param rand the source of random data for the prime number generation.
	 * @ param bitlen The bit length of the final n
	 * @ param obsrv The callback interface.
	 * @ return a random RSA Secret key.
	 */
	public static final SecretKey
	createKey( RandomStream rand, int bitlen, RSAKeyGenObserver obsrv )
	{
		return new RSAKeyGen( rand ).createKey( bitlen, obsrv );
	}

	/**
	 * This function creates a Secret Key
	 * <p>
	 *<b>N.B.</b> this can take a LONG time.
	 * @ param bitlen the bit length of the final n
	 * @ return a random RSA Secret key.
	 */
	public SecretKey
	createKey( int bitlen )
	{
		return createKey( bitlen, null );
	}
	


	/**
	 * An observer class is used to monitor progress.<p>
	 * @ param bitlen The bit length of the final n
	 * @ param obsrv The callback interface.
	 * @ return a random RSA Secret key.
	 */
	public SecretKey
	createKey( int bitlen, RSAKeyGenObserver obsrv )
	{
		int plen = bitlen/2; // PGP requires that p and q are the same length.
		int qlen = bitlen-plen;
		signalObserver( obsrv, RSAKeyGenObserver.FINDING_P );
		BigInteger p = randomPrime( plen, obsrv );
		signalObserver( obsrv, RSAKeyGenObserver.FINDING_Q );
		BigInteger q = randomPrime( qlen, obsrv );

		BigInteger e = new BigInteger( 17 );
		signalObserver( obsrv, RSAKeyGenObserver.KEY_RESOLVING );
		SecretKey ret = deriveKeys( p, q, e );
		signalObserver( obsrv, RSAKeyGenObserver.KEY_GEN_DONE );
		return ret;
	}

	/**
	 * An observer class is used to monitor progress.<p>
	 * @ param p one of the factors of n
	 * @ param q the other factors of n
	 * @ param e the initial public exponent (must be odd).
	 * @ return a random RSA Secret key.
	 */
	protected final SecretKey
	deriveKeys( BigInteger p, BigInteger q, BigInteger e )
	{
		// Ensure p <= q
		if ( p.cmp( q ) > 0 )
		{
			BigInteger t = p;
			p = q;
			q = t;
		}
		BigInteger t1 = new BigInteger().sub( p, BigInteger.one );
		BigInteger t2 = new BigInteger().sub( q, BigInteger.one );
		// phi(n) = (p-1)*(q-1)
		BigInteger phi = new BigInteger().mul( t1, t2 );
		// G(n) = gcd(p-1,q-1)
		BigInteger gcdphi = new BigInteger().gcd( t1, t2 );
		// F(n) = phi(n)/G(n)
		t1.div( phi, gcdphi );
		if ( e == null ) // if no e start at 3 e must be odd.
			e = new BigInteger( 3 );

		while( true )
		{
			t2.gcd( e, phi );
			if ( t2.cmp( BigInteger.one ) == 0 )
				break;
			e.add( 2 );
		}
		// Compute d so that (e*d) mod F(n) = 1
		BigInteger d = new BigInteger();
		BigNum.inverseModN( d, e, t1 );
		// Compute u so that (p*u) mod q = 1
		BigInteger u = new BigInteger();
		BigNum.inverseModN( u, p, q );
		// n = p * q
		BigInteger n = new BigInteger().mul( p, q );
		SecretKey ret = new  SecretKey( n, e, d, p, q, u );
		String test = ret.test();
		if ( test != null )
			throw new Error ( "RSA Key generation failed reason :\n" + test );
		return ret;
	} 
	
	
	/**
	 * this informs any observer of the current status
	 * @param obsrv the obect to call, null if none.
	 * @param signalValue status
	 */
	private static final void
	signalObserver( RSAKeyGenObserver obsrv, int signalValue )
	{
		if ( obsrv != null )
			obsrv.handleEvent( signalValue );
	}
}
