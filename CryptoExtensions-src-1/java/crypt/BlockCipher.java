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
 * This class is the base for all block ciphers.
 *
 * <p>The native code BigNum implementation used by the class was
 * developed by, and is owned by Eric Young <eay@mincom.oz.au>
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 08/12/1995
 * @author      Systemics (Michael Wynn)
 */
public abstract class BlockCipher
{
	/**
	 * Encrypt a block of data.
	 * <p><b>N.B.</b> text.length <b>must</b> be block length.<p>
	 * @param text  the data to be encrypted.
	 */
	public final void encrypt( byte text[] )
	{
		encrypt( text, 0, text, 0 );
	}

	/**
	 * Decrypt a block of data.
	 * <p><b>N.B.</b> text.length <b>must</b> be block length.<p>
	 * @param text  the data to be decrypted.
	 */
	public final void decrypt( byte text[] )
	{
		decrypt( text, 0, text, 0 );
	}

	/**
	 * Encrypt a block of data.
	 * The array in is encrypted and returned as array out.
	 * Array in and array out can be the same.
	 * <p><b>N.B.</b> in.length must equal out.length and must also equal block length.</p>
	 * @param in  the data to be encrypted.
	 * @param out the result of the encryption
	 */
	public final void encrypt( byte in[], byte out[] )
	{
		if ( in.length != out.length )
			throw new CryptoError( getClass().getName() + ": encrypt buffers must be the same size" );
		encrypt( in, 0, out, 0 );
	}

	/**
	 * Decrypt a block of data.
	 * The array in is decrypted and returned as array out.
	 * Array in and array out can be the same.
	 * <b>N.B.</b> in.length must equal out.length and must be a multiple of block length.
	 * @param in  the data to be decrypted.
	 * @param out the result of the decryption.
	 */
	public final void decrypt( byte in[], byte out[] )
	{
		if ( in.length != out.length )
			throw new CryptoError( getClass().getName() + ": decrypt output must be the same size" );
		decrypt( in, 0, out, 0 );
	}

	/**
	 * Encrypt a block of data.
	 * The data in is encrypted and returned as data out.
	 * The in and out buffers can be the same.
	 * @param in          the data to be encrypted.
	 * @param in_offset   the start of data within the in buffer.
	 * @param out         result of the encryption
	 * @param out_offset  the start of data within the out buffer.
     * @exception ArrayIndexOutOfBoundsException If the index was invalid.
	 */
	public final void encrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		int blkLength = blockLength();

		if ( in_offset < 0 || out_offset < 0 )
			throw new ArrayIndexOutOfBoundsException( getClass().getName() + ": Negative offset not allowed" );

		if ( ( in_offset + blkLength ) > in.length || ( out_offset + blkLength ) > out.length )
			throw new ArrayIndexOutOfBoundsException( getClass().getName() + ": Offset past end of array" );

		if ( ( in.length != blkLength ) || ( out.length != blkLength ) )
			throw new CryptoError( getClass().getName() + ": encrypt length must be " + blkLength );

			blockEncrypt( in, in_offset, out, out_offset );
	}

	/**
	 * Decrypt a block of data.
	 * The data in is decrypted and returned as data out.
	 * The in and out buffers can be the same.
	 * @param in          the cipher text to be decrypted.
	 * @param in_offset   the start of data within the in buffer.
	 * @param out         where the decrypted plain text will be stored.
	 * @param out_offset  the start of data within the out buffer.
     * @exception ArrayIndexOutOfBoundsException If the index was invalid.
     */

	public final void decrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		int blkLength = blockLength();

		if ( in_offset < 0 || out_offset < 0 )
			throw new ArrayIndexOutOfBoundsException( getClass().getName() + ": Negative offset not allowed" );

		if ( ( in_offset + blkLength ) > in.length || ( out_offset + blkLength ) > out.length )
			throw new ArrayIndexOutOfBoundsException( getClass().getName() + ": Offset past end of array" );

		if ( ( in.length != blkLength ) || ( out.length != blkLength ) )
			throw new CryptoError( getClass().getName() + ": decrypt length must be " + blkLength );

		blockDecrypt( in, in_offset, out, out_offset );
	}
	
	/**
	 * Perform an encryption.
	 * The in and out buffers can be the same.
	 * @param in The block to be encrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out The result of the encryption.
	 * @param out_offset  The start of data within the out buffer.
	 */
	protected abstract void blockEncrypt(byte in[], int in_offset, byte out[], int out_offset );

	/**
	 * Perform a decryption.
	 * The in and out buffers can be the same.
	 * @param in The block to be decrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out The result of the decryption.
	 * @param out_offset  The start of data within the out buffer.
	 * stored, this can be the same as in as both will be the same length.
	 */
	protected abstract void blockDecrypt(byte in[], int in_offset, byte out[], int out_offset );

	/**
	 * Return the block length of this cipher.<p>
	 * <b>N.B.</b> the library writer should also implement a 
	 * <code>public static final int BLOCK_LENGTH</code> for any classes that derive from
	 * this one
	 * @returns the block length (in bytes) of this cipher.
	 */
	public abstract int blockLength();

	/**
	 * Return the key length for this cipher.<p>
	 * <b>N.B.</b> the library writer should also implement a 
	 * <code>public static final int KEY_LENGTH </code>for any classes that derive from
	 * this one
	 * @returns   the key length (in bytes) of this cipher.
	 */
	public abstract int keyLength();
}
