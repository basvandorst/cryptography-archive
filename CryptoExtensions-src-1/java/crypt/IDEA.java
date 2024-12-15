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
 * This class Implements the IDEA block cipher.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 08/12/1995
 * @author      Systemics (Michael Wynn)
 */

public final class IDEA extends BlockCipher
{
	/**
	 * This is the length of a block.
	 */
	public static final int BLOCK_LENGTH = 8;

	/**
	 * This is the length of a the user key.
	 */
	public static final int KEY_LENGTH = 16;

	
	/**
	 * this loads the dll with the native code.
	 */
	static
	{
		System.loadLibrary( "idea" );
	}

	/**
	 * This is the length of a the internal buffer for the native code.
	 */
	private static final int INTERNAL_KEY_LENGTH = 104;

	/**
	 * This is the internal buffer with the expanded encrypt key.
	 */
    private byte ks[] = new byte[INTERNAL_KEY_LENGTH];

	/**
	 * This is the internal buffer with the expanded decrypt key.
	 * this is only used when decrypting.
	 */
    private byte dks[] = null;
	
	/**
	 * This creates a Idea block cipher from a byte array of the correct length.
	 * @param userKey    the user key.
	 */
	public IDEA( byte userKey[] ) 
	{
		if ( userKey.length != KEY_LENGTH )
			throw new CryptoError( "Idea: User key length wrong" );

		set_ks( userKey );
	}

	/**
	 * Encrypt a block.
	 * The in and out buffers can be the same.
	 * @param in The data to be encrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out The encrypted data.
	 * @param out_offset  The start of data within the out buffer.
	 */
	protected void 
	blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		if ( ks == null )
			throw new CryptoError( "Idea: User key not set." );

		do_idea( in, in_offset, out, out_offset, ks );
	}

	/**
	 * Decrypt a block.
	 * The in and out buffers can be the same.
	 * @param in The data to be decrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out The decrypted data.
	 * @param out_offset  The start of data within the out buffer.
	 */
	protected void
	blockDecrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		if ( dks == null )
		{
			dks = new byte [INTERNAL_KEY_LENGTH];
			set_dks();
		}

		do_idea( in, in_offset, out, out_offset, dks );
	}

	/**
	 * Return the block length of this cipher.
	 * @returns the block length of this cipher.
	 */
	public int 
	blockLength()
	{
		return BLOCK_LENGTH;
	}

	/**
	 * Return the key length of this cipher.
	 * @returns the key length of this cipher.
	 */
	public int 
	keyLength()
	{
		return KEY_LENGTH;
	}

	// these are the native functions that implement the IDEA algorithm.
	/**
	 * This function expands the user key.
	 */
	private native void set_ks( byte userKey[] );

	/**
	 * This function expands the decryption key.
	 */
	private native void set_dks();

	/**
	 * Encrypt/decrypt the block (depending upon encrypt flag)
	 */
	private native void do_idea( byte in[], int in_offset, byte out[], int out_offset, byte[] key );
}

