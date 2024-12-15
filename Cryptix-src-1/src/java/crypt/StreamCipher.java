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
 * This abstract class is the basis for a stream cipher of any form.
 * <p><b>N.B.</b> Even though there is no constructor, there is a key length
 * method so that the user can find the key length.  There is no key in the base 
 * class because the key storage varies with cipher type, and is left to
 * the designer of the derived class.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 08/12/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public abstract class StreamCipher
{
	/**
	 * This function encrypts a block of data.
	 * The contents of the array will be changed.
	 * @param text  The plaintext to be encrypted.
	 */
	public final void encrypt( byte text[] )
	{
		encrypt( text, 0, text, 0, text.length );
	}

	/**
	 * This function decrypts a block of data.
	 * The contents of the array will be changed.
	 * @param text  The ciphertext to be decrypted.
	 */
	public final void decrypt( byte text[] )
	{
		decrypt( text, 0, text, 0, text.length );
	}

	/**
	 * This function encrypts a block of data.
	 * The contents of the array in remain unchanged,
	 * and the result is stored in the array out.
	 * However, array in and array out can be the same.
	 * @param in  The plaintext to be encrypted.
	 * @param out Where the ciphertext will be stored.
	 */
	public final void encrypt( byte in[], byte out[] )
	{
		if ( in.length > out.length )
			throw new CryptoError( getClass().getName() + ": encrypt output buffer too small" );
		encrypt( in, 0, out, 0, in.length );
	}

	/**
	 * This function decrypts a block of data.
	 * The contents of the array in remain unchanged,
	 * and the result is stored in the array out.
	 * However, array in and array out can be the same.
	 * @param in  The ciphertext to be decrypted.
	 * @param out Where the plaintext will be stored.
	 */
	public final void decrypt( byte in[], byte out[] )
	{
		if ( in.length != out.length )
			throw new CryptoError( getClass().getName() + ": decrypt output buffer too small" );
		decrypt( in, 0, out, 0, in.length );
	}

	/**
	 * This function encrypts a block of data.
	 * The contents of the array in remain unchanged,
	 * and the result is stored in the array out.
	 * However, array in and array out can be the same.
	 * @param in  The plaintext to be encrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out Where the ciphertext will be stored.
	 * @param off_offset   The start of data within the out buffer.
	 * @param length      The length to encrypt.
	 */
	public abstract void encrypt( byte in[], int in_offset, byte out[], int out_offset, int length );

	/**
	 * This function decrypts a block of data.
	 * The contents of the array in remain unchanged,
	 * and the result is stored in the array out.
	 * However, array in and array out can be the same.
	 * @param in  The ciphertext to be decrypted.
	 * @param in_offset   The start of data within the in buffer.
	 * @param out Where the plaintext will be stored.
	 * @param off_offset   The start of data within the out buffer.
	 * @param length      The length to decrypt.
	 */
	public abstract void decrypt( byte in[], int in_offset, byte out[], int out_offset, int length );
	
	/**
	 * This function returns the length of the key for this cipher.<p>
	 * <b>N.B.</b> The library writer should also implement a 
	 * public static final int KEY_LENGTH for any classes that derive from
	 * this.
	 * @returns   the length (in bytes) of the key used with this cipher
	 */
	public abstract int keyLength();
}

