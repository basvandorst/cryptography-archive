/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

/**
 * Implements a byte-based stream cipher using n-bit CFB with an
 * n-bit-sized block cipher.
 *
 * <P>
 * The full block size of the supplied cipher is used for the
 * Cipher Feedback Mode.
 * The bytes supplied are processed and returned immediately.
 * <P>
 * Note that this is <i>Variant B</i>.
 * It needs to be rationalised with #CipherFeedback.
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
 * @see StreamCipher
 */
public class CFB extends StreamCipher
{
        protected BlockCipher cipher;
        protected int blocksize;

        protected byte iv[];
        protected byte k[];
        protected int bytesLeft;

        /**
         * This implements variant B: a unique IV for the
         * feedback register must be provided as the first blockLength
         * characters in the stream.
         * If the IV is not unique, the plaintext can be recovered.
         *
         * @param cipher0     the block cipher to use
         */
        public CFB( BlockCipher cipher_)
        {
                cipher = cipher_;
                blocksize = cipher_.blockLength();
                iv = new byte [blocksize];
                k = new byte [blocksize];
                reset();
        }

        public int keyLength() { return cipher.keyLength(); }

        public void next_block() { bytesLeft = 0; }

        /**
         * Reset the stream and the IV.  Note that a new IV is now
         * needed as described above.
         */
        public void reset()
        {
                bytesLeft = 0;
                for (int i = blocksize-1; i >= 0; --i) { iv[i] = 0; }
        }



        /**
         * This function encrypts a block of data.
         * The contents of the array in remain unchanged,
         * with the result stored in the array out.
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
                if (bytesLeft <= 0)
                {
                        bytesLeft = blocksize;
                        cipher.encrypt(iv, 0, k, 0);
                }
                b ^= k[blocksize-bytesLeft];
                // what's wrong with iv[blocksize-bytesLeft] = b; ???
                System.arraycopy(iv, 1, iv, 0, blocksize-1);
                iv[blocksize-1] = b;
                bytesLeft--;
                return b;
        }

        private byte
        decryptByte( byte b )
        {
                if (bytesLeft <= 0)
                {
                        bytesLeft = blocksize;
                        cipher.encrypt(iv, 0, k, 0);
                }
                byte t = b;
                b ^= k[blocksize-bytesLeft];
                System.arraycopy(iv, 1, iv, 0, blocksize-1);
                iv[blocksize-1] = t;
                bytesLeft--;
                return b;
        }

}
