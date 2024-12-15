/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */
 
package cryptix.pgp;

import cryptix.security.BlockCipher;

/**
 *
 * This class implements PGPs (ie. Zimmermans) non-standard CFB mode.
 * (For standard method, see Schneier, AppCrypto 9.6).
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
public class CFB extends cryptix.security.CFB
{
	public CFB( BlockCipher cipher_) { super(cipher_); }

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
		next_block();
		super.encrypt(in, in_offset, out, out_offset, length);
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
		next_block();
		super.decrypt(in, in_offset, out, out_offset, length);
	}

}
