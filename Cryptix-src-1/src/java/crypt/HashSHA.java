/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.crypt;

/**
 * This class represents the output of a SHA message digestor. 
 * in such a way that it can be identifyed as such.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class HashSHA extends MessageHash
{
	/**
	 * Creates this from a SHA message digestor
	 * @param md A SHA MessageDigest.
	 */
	public HashSHA( SHA md )
	{
		super( md.digest() );
	}

	/**
	 * Creates this from a byte array that <b>MUST</b> be the the correct length
	 * @see SHA#HASH_LENGTH
	 * @param hash A byte array which represents a MD5 hash.
	 */
	public HashSHA( byte hash[] )
	{
		super( checkHash( hash ) );
	}

	/**
	 * this checks the byte array is the correct size for the a SHA hash.
	 * @param hash A byte array which represents a SHA hash.
	 */
	private static final byte[]
	checkHash( byte hash[] )
	{
		if ( hash.length != SHA.HASH_LENGTH )
			throw new RuntimeException( "Hash length incorrect " + hash.length );
		return hash;
	}
}

