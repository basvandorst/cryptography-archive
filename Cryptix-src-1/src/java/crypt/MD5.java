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
 * This class implements the MD5 message digest.
 * <p>The native code MD5 implementation used by the class was
 * developed by, and is owned by, RSA Data Security, Inc.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.01, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class MD5 extends MessageDigest
{
	/**
	 * This is the length of the final hash (in bytes).
	 */
	public static final int HASH_LENGTH = 16;

	static
	{
		// load the DLL or shared library that contains the native code
		// implementation of the MD5 algorithm.

		System.loadLibrary( "md5" );
	}

	/**
	 * This is the amount of data required by the native code.
	 */
	private static final int INT_BUFFER_LENGTH = 88;

	/**
	 * The contextBuffer required by the native code.
	 */
    private byte contextBuf[]; /* MD5 internal data buffer */

	/**
	 * The public constructor.
	 */
	public MD5()
	{
		contextBuf = new byte[INT_BUFFER_LENGTH];
		reset();
	}

	/**
	 * Return length of the hash (in bytes).
	 * @see #HASH_LENGTH
	 * @return   The length of the hash.
	 */
	public int length()
	{
		return HASH_LENGTH;
	}

	/**
	 * Initialise (reset) the message digest.
	 */
	public synchronized void reset()
	{
		init();
	}

	/**
	 * Returns the digest of the data added and resets the digest.
	 * @return    the digest of all the data added to the message digest as a byte array.
	 */
	public synchronized byte[] digest()
	{
		byte buf[] = new byte[HASH_LENGTH];
		finish( buf );
		return buf;
	}


	/**
	 * Returns the digest of the data added and resets the digest.
	 * @return the digest of all the data added to the message digest as an object.
	 */
	public MessageHash digestAsHash()
	{
		return new HashMD5( this );
	}

	/**
	 * Returns the hash of a single string.
	 * @param msg the string to hash.
	 * @return the hash of the string.
	 */
	public static byte[]
	hash( String msg )
	{
		return hash( msg, new MD5() );
	}

	/**
	 * Returns the hash of a single byte array.
	 * @param msg the byte array to hash.
	 * @return the hash of the string.
	 */
	public static byte[]
	hash( byte msg[] )
	{
		return hash( msg, new MD5() );
	}

	/**
	 * Returns the hash of a single byte array.
	 * @param msg the byte array to hash.
	 * @return the hash of the string.
	 */
	public static HashMD5
	CreateHash( byte hash[] )
	{
		return new HashMD5( hash );
	}

	
	/**
	 * Add data to the message digest (calls the native code).
	 * @param data    The data to be added.
	 * @param offset  The start of the data in the array.
	 * @param length  The amount of data to add.
	 */
	protected synchronized void addToDigest( byte data[], int offset, int length )
	{
		update( data, offset, length );
	}

	// The native functions that implement MD5
	/**
	 * Resets the Context buffer to initial values.
	 */
	private native void init();

	/**
	 * Adds to the hash.
	 * @param data    The data to be added.
	 * @param offset  The start of the data within the array.
	 * @param length  The amount of data to add.
	 */
	private native void update( byte data[], int offset, int length );

	/**
	 * Fills the buffer with the digested output and resets the digest.
	 * @param output   The buffer where the digest is to be stored.
	 */
	private native void finish( byte output[] );
}
