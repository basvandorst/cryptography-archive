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
/**
 * This class is an RSA secret key pair.
 * It can also be used as a public key.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 05/01/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class SecretKey extends PublicKey
{
	/** the private exponent */
	protected BigInteger d_;
	/** one of the factors of n */
	protected BigInteger p_;
	/** the other factors of n */
	protected BigInteger q_;
	/** the multiplic inverse of p mod q */
	protected BigInteger u_;
	
	protected SecretKey()
	{
		super();
	}
	
	/**
	 * Creates a secret key pair from the component parts
	 * @param n0 The public modulus
	 * @param e0 The public exponent
	 * @param d0 The private exponent
	 * @param p0 One of the factors of n
	 * @param q0 The other Factor of n
	 * @param u0 The multiplic inverse of p mod q
	 */
	public SecretKey( BigInteger n0, BigInteger e0, BigInteger d0, BigInteger p0, BigInteger q0, BigInteger u0 )
	{
		super( n0, e0 );
		d_ = new BigInteger( d0 );
		p_ = new BigInteger( p0 );
		q_ = new BigInteger( q0 );
		u_ = new BigInteger( u0 );
	}

	/**
	 * @return the private exponent d
	 */
	public final BigInteger
	d()
	{
		return new BigInteger( d_ );
	}

	/**
	 * @return the smallest factor of n
	 */
	public final BigInteger
	p()
	{
		return new BigInteger( p_ );
	}

	/**
	 * @return the largest factor of n
	 */
	public final BigInteger
	q()
	{
		return new BigInteger( q_ );
	}

	/**
	 * @return the multiplicative inverse of p mod q
	 */
	public final BigInteger
	u()
	{
		return new BigInteger( u_ );
	}
	
	/**
	 * decrypt a number
	 * used for decryption or signing
	 * @param encrypted the number to decrypt.
	 * @return a decrypted number.
	 */
	public BigInteger
	decrypt( BigInteger encrypted )
	{
		return cryptFast( d_, encrypted );
	}

	/**
	 * encrypt a number
	 * used for encryption or signature verification.
	 * <B>N.B.</B> this is quicker that the public key method,
	 * since secret components are available.
	 * @param plain the number to encrypt.
	 * @return an encrypted number.
	 */
	public BigInteger
	encrypt( BigInteger plain )
	{
		return cryptFast( e_, plain );
	}

	/**
	 * perform a fast encryption/decryption
	 * @param key either e if encrypting or d if decrypting
	 * @param msg the message to crypt.
	 * @return the encrypted/decrypted message
	 */
	protected final BigInteger
	cryptFast( BigInteger key, BigInteger msg )
	{
		BigInteger p1 = (BigInteger)p_.clone();
		BigInteger q1 = (BigInteger)q_.clone();

		p1.dec();
		q1.dec();

		BigInteger dmp1 = new BigInteger().mod( key, p1 );
		BigInteger dmq1 = new BigInteger().mod( key, q1 );

		// m1 = ((msg mod p) ^ dmq1) ) mod p
		BigInteger m = new BigInteger().mod( msg, p_ ).modExp( dmp1, p_ );

		//r = ((msg mod q) ^ dmp1) ) mod q
		BigInteger r = new BigInteger().mod( msg, q_ ).modExp( dmq1, q_ );

		BigInteger r1 = new BigInteger().sub( r, m );
	
		BigInteger zero = BigInteger.zero();

		if ( r1.cmp( zero ) < 0 )
			r1.add( r1, q_ );

		r.mul( r1, u_ );
		r1.mod( r, q_ );
		r.mul( r1, p_ );
		r1.add( r, m );
	
		return r1;
	}
	

	/**
	 * perform a sanity check on the key
	 * @return null if the key is O.K., otherwise an error message.
	 */
	public final String insane()
	{
		BigInteger t = new BigInteger().mul( p_, q_ );
		if ( n_.cmp( t ) != 0 )
			return "pq != n";
		if ( p_.cmp( q_ ) >= 0 )
			return " p >= q";
		BigNum.modMul( t, p_, u_, q_ );
		if ( t.cmp( BigInteger.one ) != 0 )
			return "(p*u) mod q != 1";
		return null;
	}


	/**
	 * Perform a basic test on the key
	 * @return null if the key is O.K., otherwise an error message.
	 */
	public final String test()
	{
		String ret = insane();
		if ( ret != null )
			return ret;
		StringBuffer sb = new StringBuffer();
		byte tmp = 7,buf[] = new byte[16];
		for ( int i = 0; i < 16; tmp += 3 )
			buf[i++] = tmp;
		BigInteger msg = new BigInteger( buf );
		System.out.println( "test string" + msg.toString() );
		BigInteger encmsg = encrypt( msg );
		BigInteger msg2 = decrypt( encmsg );
		if ( msg.cmp( msg2 ) != 0 )
			return "Key test failed " + msg + "\nis not" + msg2;
		return null;
	}

	/**
	 * Access all parts of the secret key.
	 * This function should be used with caution,
	 * in order that the secret key is not compromised.
	 * @return a string representation of the key.
	 */
	public String
	toString()
	{
		return super.toString() + "\nd:" + d_ + "\np:" + p_ + "\nq:" + q_ + "\nu:" + u_;
	}
}

