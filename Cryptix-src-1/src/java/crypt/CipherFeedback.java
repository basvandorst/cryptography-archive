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
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 08/12/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class CipherFeedback extends StreamCipher
{
	protected byte ivBlock[];
	protected byte xorBlock[];
	protected BlockCipher cipher;
	protected int currentByte;
	protected int length;

	public CipherFeedback( BlockCipher cipher0, byte iv0[] )
	{
		ivBlock = iv0;
		cipher = cipher0;
		currentByte = 0;
		length = cipher0.blockLength();
		xorBlock = new byte [length];
		cipher.encrypt( ivBlock, xorBlock );
	}

	/**
	 * This function encrypts a block of data.
	 * The contents of the array in will not be changed,
	 * but will instead store the result in the array out.
	 * The arrays can, however, be the same.
	 * @param in          The plain text to be encrypted.
	 * @param in_offset   The offset within the in buffer.
	 * @param out         Where the encrypted cipher text will be stored.
	 * @param out_offset  The offset within the out buffer.
	 * @param length      The length to encrypt.
	 */
	public void 
	encrypt( byte in[], int in_offset, byte out[], int out_offset, int length )
	{
		for ( int i = 0; i < length; i++ )
			out[i + out_offset] = encryptByte( in[i + in_offset] );
	}

	/**
	 * This function decrypts a block of data.
	 * The contents of the array in will not be changed,
	 * but will instead store the result in the array out.
	 * The arrays can, however, be the same.
	 * @param in          The cipher text to be decrypted.
	 * @param in_offset   The offset within the in buffer.
	 * @param out         Where the decrypted plain text will be stored.
	 * @param out_offset  The offset within the out buffer.
	 * @param length      The length to decrypt.
	 */
	public void
	decrypt( byte in[], int in_offset, byte out[], int out_offset, int length )
	{
		for ( int i = 0; i < length; i++ )
			out[i + out_offset] = decryptByte( in[i + in_offset] );
	}

	private byte
	encryptByte( byte b )
	{
		if ( currentByte >= length )
		{
			currentByte = 0;
			cipher.encrypt( ivBlock, xorBlock );
		}
		b ^= xorBlock[currentByte];
		ivBlock[currentByte] = b;
		currentByte++;
		return b;
	}
	
	private byte
	decryptByte( byte b )
	{
		if ( currentByte >= length )
		{
			currentByte = 0;
			cipher.encrypt( ivBlock, xorBlock );
		}
		ivBlock[currentByte] = b;
		b ^= xorBlock[currentByte];
		currentByte++;
		return b;
	}

	public int keyLength()
	{
		return -1; // this has no meaning with this form of cipher.
	}
}

