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
 * This class represents the output of a MD5 message digestor. 
 * in such a way that it can be identifyed as such.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class HashMD5 extends MessageHash
{

	/**
	 * Creates this from a MD5 message digestor.
	 * @see MD5#HASH_LENGTH
	 * @param md A MD5 message digestor.
	 */
	public HashMD5( MD5 md )
	{
		super( md.digest() );
	}

	/**
	 * Creates this from a byte array that <b>MUST</b> be the the correct length
	 * @see MD5#HASH_LENGTH
	 * @param hash A byte array which represents a MD5 hash.
	 */
	public HashMD5( byte hash[] )
	{
		super( checkHash( hash ) );
	}

	/**
	 * this checks the byte array is the correct size for the a MD5 hash.
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

