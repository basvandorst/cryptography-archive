/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.security.rsa;

import cryptix.security.MD5;
import cryptix.math.BigInteger;

/**
 * This class is an RSA public key, it can only verify signatures or encrypt.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public class PublicKey
{
	/** the public modulus */
	protected BigInteger n_;

	/** the public exponent */
	protected BigInteger e_;

	protected PublicKey()
	{
	}
	
	/**
	 * create a public key from a modulus and exponent
	 * @param n0 The public modulus
	 * @param e0 The public exponent
	 */
	public PublicKey( BigInteger n0, BigInteger e0 )
	{
		n_= new BigInteger( n0 );
		e_= new BigInteger( e0 );
	}

	/**
	 * create a new public key from an existing key.
	 * @param from0 the key to copy.
	 */
	public PublicKey( PublicKey from0 )
	{
		copy( from0 );
	}

	/**
	 * @return a new BigInteger equal to the public modulus
	 */
	public final BigInteger
	n()
	{
		return new BigInteger( n_ );
	}

	/**
	 * @return a new BigInteger equal to the public exponent
	 */
	public final BigInteger
	e()
	{
		return new BigInteger( e_ );
	}

	/**
	 * copies a key if it is an instance of cryptix.security.rsa.PublicKey.
	 * @param src this object to copy
	 */
	public void
	copy( Object src )
	{
		PublicKey from = (PublicKey)src;
		n_ = new BigInteger( from.n_ );
		e_ = new BigInteger( from.e_ );
	}

	/**
	 * tests if an object is equal to this one.
	 * @param obj object to test
	 * @return true if the object is equal, otherwise false.
	 */
	public boolean equals( Object obj )
	{
		if ( obj instanceof PublicKey )
		{
			PublicKey from = (PublicKey)obj;
			if ( n_.equals( from.n_ ) && e_.equals( from.e_ ) )
				return true;
		}
		return false;
	}
	
	/**
	 * gets the key id.
	 * @returns The lower 8 bytes of n, the public modulus.
	 */
	public final byte[]
	id()
	{
		byte nBuffer[] = n_.toByteArray();
		byte ident[] = new byte[8];
		System.arraycopy( nBuffer, nBuffer.length - 8, ident, 0, 8 );
		return ident;
	}

	/**
	 * get the keys fingerprint.
	 * @return a MD5 hash of n and e.
	 */
	public final byte[]
	fingerPrint()
	{
		byte eBuffer[] = e_.toByteArray();
		byte nBuffer[] = n_.toByteArray();
		MD5 md = new MD5();
		md.add( nBuffer );
		md.add( eBuffer );
		return md.digest();
	}

	/**
	 * get the keys bit length.
	 * @return the length in bits of n, the public modulus.
	 */
	public final int bitLength()
	{
		return n_.bitLength();
	}

	/**
	 * encrypts a number with this key.
	 * @param plain the number to encrypt.
	 * @return an encrypted number.
	 */
	public BigInteger
	encrypt( BigInteger plain )
	{
		return new BigInteger( plain ).modExp( e_, n_ );
	}
	
	/**
	 * @return this key in hex.
	 */
	public String
	toString()
	{
		return "n:" + n_ + "\ne:" + e_;
	}
}
