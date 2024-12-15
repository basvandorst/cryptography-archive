/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.security;

/**
 * This class represents the output of a SHA0 message digestor. 
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public final class HashSHA0 extends MessageHash
{
	/**
	 * Creates this from an SHA0 message digestor
	 * @param md An SHA0 MessageDigest.
	 */
	public HashSHA0( SHA0 md )
	{
		super( md.digest() );
	}

	/**
	 * Creates this from a byte array that must be the the correct length
	 * @param hash A byte array which represents an SHA0 hash.
	 */
	public HashSHA0( byte hash[] )
	{
		super( checkHash( hash ) );
	}

	/**
	 * Returns a big endian Hex string prefixed with "SHA0:",
	 *	showing the value of the hash.
	 * @return a string reprosenting the hash.
	 */
	public String
	toString()
	{
		return "SHA0:" + super.toString();
	}

	/**
	 * this checks the byte array is the correct size for the an SHA0 hash.
	 * @param hash A byte array which represents an SHA0 hash.
	 */
	//
	// Hang on! This shouldn't throw an exception!
	// The caller of this function (eg. HashSHA0) should
	// throw the exception.
	//
	private static final byte[]
	checkHash( byte hash[] )
	{
		if ( hash.length != SHA0.HASH_LENGTH )
			throw new RuntimeException( "Hash length incorrect " + hash.length );
		return hash;
	}
}
