/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * This abstract class is the basis for a stream cipher of any form.
 * <p>
 * Even though there is no constructor, there is a key length
 * method so that the user can find the key length.
 * There is no key in the base 
 * class because the key storage varies with cipher type, and is left to
 * the designer of the derived class.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * See also:
 * Schneier, B., <cite>Applied Cryptography</cite>, Wiley, 1996, 2nd Ed.
 *
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <p>
 * @author Systemics Ltd
 * @see BlockCipher
 * @see CipherFeedback
 * @see CFB
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
                        throw new CryptoError( getClass().getName() +
                                        ": encrypt output buffer too small" );
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
                        throw new CryptoError( getClass().getName() +
                                        ": decrypt output buffer too small" );
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
         * This function returns the length of the key for this cipher.
         *
         * @returns   the length (in bytes) of the key used with this cipher
         */
        public abstract int keyLength();
}
