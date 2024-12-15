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
 * @version     1.00, 12/02/1996
 * @author      Systemics (Michael Wynn)
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

		System.loadLibrary("md5");
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
	public void reset()
	{
		init();
	}

	/**
	 * Returns the digest of the data added and resets the digest.
	 * @return    the digest of all the data added to the message digest.
	 */
	public byte[] digest()
	{
		byte buf[] = new byte[HASH_LENGTH];
		finish( buf );
		return buf;
	}

	/**
	 * Add data to the message digest (calls the native code).
	 * @param data    The data to be added.
	 * @param offset  The start of the data in the array.
	 * @param length  The amount of data to add.
	 */
	protected void addToDigest( byte data[], int offset, int length )
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
