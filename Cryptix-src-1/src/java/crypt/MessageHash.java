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
 * this class reprosents the output from a message digestor in a form
 * where the type and be asertained.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class MessageHash
{
	private byte hash[];
	
	/**
	 * You can not create an instance of this object
	 * @see java.crypt.HashSHA
	 * @see java.crypt.HashMD5
	 */
	protected MessageHash( byte hash0[] )
	{
		hash = new byte[hash0.length];
		System.arraycopy( hash0, 0, hash, 0, hash0.length );
	}
	
	/**
	 * @return the hash as a new byte array.
	 */
	public final byte[]
	toByteArray()
	{
		byte buf[] = new byte[hash.length];
		System.arraycopy( hash, 0, buf, 0, hash.length );
		return buf;
	}
	
	/**
	 * @return the hash length.
	 */
	public final int
	length()
	{
		return hash.length;
	}
}

