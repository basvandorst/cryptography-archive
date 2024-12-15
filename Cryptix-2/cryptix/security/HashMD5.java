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
 * This class represents the output of a MD5 message digestor. 
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public final class HashMD5 extends MessageHash
{
	/**
	 * Creates this from a MD5 message digestor.
	 * @param md A MD5 message digestor.
	 */
	public HashMD5( MD5 md )
	{
		super( md.digest() );
	}

	/**
	 * Creates this from a byte array that must be of the correct length
	 * @param hash A byte array which represents a MD5 hash.
	 */
	public HashMD5( byte hash[] )
	{
		super( checkHash( hash ) );
	}

	/**
	 * Returns a big endian Hex string prefixed with "MD5:",
	 *	showing the value of the hash.
	 * @return a string reprosenting the hash.
	 */
	public String
	toString()
	{
		return "MD5:" + super.toString();
	}

	/**
	 * Check the byte array is the correct size for the MD5 hash.
	 * @param hash A byte array which represents a MD5 hash.
	 */
	private static final byte[]
	checkHash( byte hash[] )
	{
		if ( hash.length != MD5.HASH_LENGTH )
			throw new RuntimeException( "Hash length incorrect " + hash.length );
		return hash;
	}
}
