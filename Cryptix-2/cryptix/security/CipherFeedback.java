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
 *
 * <P>
 * Note that this is <i>Variant A</i>.
 * It needs to be rationalised with #CFB
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
 * @see StreamCipher
 */
public class CipherFeedback extends StreamCipher
{
        protected byte ivBlock[];
        protected byte xorBlock[];
        protected BlockCipher cipher;
        protected int currentByte;
        protected int length;

        /**
         * This implements variant A: a unique initial value for the
         * feedback register must be provided in the constructor.
         * If the IV is not unique, the plaintext can be recovered.
         *
         * @param cipher0     the block cipher to use
         * @param iv0         the initial value for the shift register (IV)
         */
        public CipherFeedback( BlockCipher cipher0, byte iv0[] )
        {
                length = cipher0.blockLength();
                cipher = cipher0;
                xorBlock = new byte [length];

                reset( iv0 );

                // moved to reset() for 2.5.2.
                // ivBlock = iv0;
                // currentByte = 0;
                // cipher.encrypt( ivBlock, xorBlock );
        }

        public int keyLength() { return cipher.keyLength(); }

        /**
         * Reset the stream feedback register with a new unique IV
         * and sets position in stream back to zero.
         *
         * @param iv0         the initial value for the shift register (IV)
         */
        public void reset( byte iv0[] )
        {
                ivBlock = iv0;
                currentByte = 0;
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

// added meaningful length above in 2.5.2
//      public int keyLength()
//      {
//              return -1; // meaningless for this form of cipher.
//      }
}

