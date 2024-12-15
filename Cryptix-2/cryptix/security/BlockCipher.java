/*
 * Copyright (C) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix development team.  All rights reserved.
 */

package cryptix.security;

/**
 * BlockCipher is an abstract superclass for ciphers that encrypt and
 * decrypt a fixed length block with a fixed secret key.
 * No memory of the text is kept between distinct operations
 * (for memory behaviour, see <code>streamCipher</code>).
 *
 * <P>
 * <b>Usage of Extended Classes</b>
 *
 * <P>
 * General usage of extended classes is to construct the object, providing
 * the <i>secret key</i> and any other details required.
 * The key is generally fixed for any given object.
 * Then, methods <code>encrypt</code> and <code>decrypt</code>
 * are called with data for processing, always using the original key:
 *
 * <pre>
 *   String plain = "Et tu, Brute?";
 *   byte plainText[] = new byte[cryptor.blockLength];
 *   plain.getBytes(0, cryptor.blockLength, plainText, 0);
 *   <br>
 *   byte cipherText[] = new byte[cryptor.blockLength];
 *   byte decrypText[] = new byte[cryptor.blockLength];
 *   <br>
 *   Caeser cryptor = new Caeser( 3 );       // 'a' becomes 'd', etc
 *   cryptor.encrypt( plainText, cipherText );
 *   cryptor.decrypt( cipherText, decrypText );
 *   <br>
 *   String decryp = new String( decrypText, 0 );
 *   String cipher = new String( cipherText, 0 );
 *   <br>
 *   if ( decryp.equals( plain ) && !cipher.equals( plain ) )
 *       out.println( "Caeser is good for protecting secrets of state" );
 *   else
 *       out.println( "Caeser has failed to keep secrets" );
 * </pre>
 *
 * In general, method <code>encrypt</code> takes <i>plaintext</i>
 * and returns <i>ciphertext</i>,
 * whilst method <code>decrypt</code> takes <i>ciphertext</i>
 * and returns <i>plaintext</i>.
 *
 * <P>
 * <b>Adding New Ciphers</b>
 * <P>
 * Extended classes should provide constructors that initialise the
 * key and any other details:
 *
 * <pre>
 *   public final class Caeser extends BlockCipher
 *   {
 *       private static final String LIBRARY_NAME = "caeser";
 *       private int rotate = 3;        // 3 is caeser, 13 is "rot13"
 *       // public Caeser() { }         // default behaviour
 *       <br>
 *       public int blockLength() { return 8; }
 *       <br>
 *        /**
 *         * Create a Caeser object with a different rotation.
 *         * @param rotate    number of positions to the right to adjust
 *         * /
 *       public Caeser( int rotate ) { this.rotate = rotate }
 *       ....
 * </pre>
 *
 * Whilst Caeser ciphers didn't deserve a real key, the amount of rotation
 * for each character is almost as good.
 * Note that the 'key' is fixed for the object; this 
 * is expected behaviour for extended classes, but not mandated.
 *
 * <P>
 * Extended classes should provide methods <code>blockEncrypt</code>
 * and <code>blockDecrypt</code> that operate on the passed data.
 * The arrays <code>in</code> and <code>out</code> may be the same array.
 * Methods <code>encrypt</code> and <code>decrypt</code> do not need
 * to be provided as they call the former from within this super class:
 *
 * <pre>
 *       protected void
 *       blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
 *       {
 *           for (int i = 0; i < rotate; i++)
 *           {
 *               out[out_offset + i] = in[in_offset + i] + rotate;
 *               if ( out[out_offset + i] > (int)'Z' )   // modulo 26
 *                   out[out_offset + i] -= 26;          // only works on [A-Z]
 *           }
 *       }
 * </pre>
 *
 * Extended classes should document the algorithm, constructors,
 * any special calls and any deviations from standard behaviour.
 * Users can refer to this superclass for standard behavior.
 * <pre>
 *        /**
 *         * Caeser is a substitution cipher recommended for Despots and jokes.
 *         * Warning: foreign characters will be thrown to the lions.
 *         * /
 *        public void Caeser
 *        ...
 * </pre>
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * See also: Shakespeare, W., <cite>Julius Caeser</cite>, The Globe, and
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
 * @see SPEED
 * @see IDEA
 */
public abstract class BlockCipher
{
        /**
         * Encrypt a block of data in place.
         * The plaintext in <code>text</code> is encrypted
         * and written back as ciphertext.
         * The length of <code>text</code> must be the
         * block length as returned by <code>blockLength</code>.<p>
         * <p>
         * @param text    the buffer holding the data
         */
        public final void
        encrypt( byte text[] )
        {
                encrypt( text, text );
        }

        /**
         * Decrypt a block of data in place.
         * The ciphertext in <code>text</code> is decrypted
         * and written back as plaintext.
         * The length of <code>text</code> must be the
         * block length as returned by <code>blockLength</code>.<p>
         * <p>
         * @param text    the buffer holding the data
         */
        public final void
        decrypt( byte text[] )
        {
                decrypt( text, text );
        }

        /**
         * Encrypt a block of data.
         * The plaintext in <code>in</code> is encrypted and the ciphertext
         * is written into <code>out</code>.
         * Note that <code>in</code> and <code>out</code> can be the same array.
         * The length of both <code>in</code> and <code>out</code> must be the
         * block length as returned by <code>blockLength</code>.<p>
         * <p>
         * @param in              the plaintext to be encrypted
         * @param out             the ciphertext result of the encryption
         * @exception CryptoError if a buffer was the wrong length
         */
        public final void
        encrypt( byte in[], byte out[] )
        {
                int len = blockLength();
                if ( (in.length != len) || (out.length != len) )
                        throw new CryptoError( getClass().getName() +
                        ": encrypt buffers must be the same size as cipher length" );
                encrypt( in, 0, out, 0 );
        }

        /**
         * Decrypt a block of data.
         * The ciphertext in <code>in</code> is decrypted and
         * the plaintext is written into <code>out</code>.
         * Note that <code>in</code> and <code>out</code> can be the same array.
         * The length of both <code>in</code> and <code>out</code> must be the
         * block length as returned by <code>blockLength</code>.<p>
         * <p>
         * @param in              the ciphertext to be decrypted
         * @param out             the plaintext result of the encryption
         * @exception CryptoError if a buffer was the wrong length
         */
        public final void
        decrypt( byte in[], byte out[] )
        {
                int len = blockLength();
                if ( (in.length != len) || (out.length != len) )
                        throw new CryptoError( getClass().getName() +
                        ": decrypt buffers must be the same size as cipher length" );
                decrypt( in, 0, out, 0 );
        }

        /**
         * Encrypt a block of data within an array.
         * The plaintext in <code>in</code> is encrypted
         * from <code>in_offset</code>
         * to <code>(in_offset + blockLength - 1)</code>
         * and the ciphertext
         * is written into <code>out</code>
         * from <code>out_offset</code>
         * to <code>(out_offset + blockLength - 1)</code>
         * Note that there must be at least blockLength bytes left in each
         * array at the supplied offsets.
         * <p>
         * @param in          buffer holding the plaintext to be encrypted
         * @param in_offset   the start of plaintext within <code>in</code>
         * @param out         buffer to hold the encrypted ciphertext result
         * @param out_offset  the start of cyphertext within <code>out</code>
         * @exception ArrayIndexOutOfBoundsException if an offset was invalid.
         */
        public final void
        encrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
                int blkLength = blockLength();

                if ( in_offset < 0 || out_offset < 0 )
                {
                        throw new ArrayIndexOutOfBoundsException(
                                getClass().getName() +
                                ": Negative offset not allowed" );
                }

                if ( ( in_offset + blkLength ) > in.length ||
                     ( out_offset + blkLength ) > out.length )
                {
                        throw new ArrayIndexOutOfBoundsException(
                                getClass().getName() +
                                ": Offset past end of array" );
                }

                //
                // This is wrong, I think.  Now in other methods.  Iang.
                // if ( ( in.length != blkLength ) ||
                //      ( out.length != blkLength ) )
                //      throw new CryptoError( getClass().getName() +
                //              ": encrypt length must be " + blkLength );

                blockEncrypt( in, in_offset, out, out_offset );
        }

        /**
         * Decrypt a block of data within an array.
         * The cyphertext in <code>in</code> is decrypted
         * from <code>in_offset</code>
         * to <code>(in_offset + blockLength - 1)</code>
         * and the plaintext
         * is written into <code>out</code>
         * from <code>out_offset</code>
         * to <code>(out_offset + blockLength - 1)</code>
         * Note that there must be at least blockLength bytes left in each
         * array at the supplied offsets.
         * <p>
         * @param in          buffer holding the cyphertext to be decrypted
         * @param in_offset   the start of cyphertext within <code>in</code>
         * @param out         buffer to hold the decrypted plaintext result
         * @param out_offset  the start of plaintext within <code>out</code>
         * @exception ArrayIndexOutOfBoundsException if an offset was invalid.
         */
        public final void
        decrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
                int blkLength = blockLength();

                if ( in_offset < 0 || out_offset < 0 )
                {
                        throw new ArrayIndexOutOfBoundsException(
                                getClass().getName() +
                                ": Negative offset not allowed" );
                }

                if ( ( in_offset + blkLength ) > in.length ||
                     ( out_offset + blkLength ) > out.length )
                {
                        throw new ArrayIndexOutOfBoundsException(
                                getClass().getName() +
                                ": Offset past end of array" );
                }

                // if ( ( in.length != blkLength ) ||
                //      ( out.length != blkLength ) )
                //      throw new CryptoError( getClass().getName() +
                //              ": decrypt length must be " + blkLength );

                blockDecrypt( in, in_offset, out, out_offset );
        }
        
        /**
         * Perform an encryption in the extended class.
         * The plaintext in <code>in</code> is encrypted
         * from <code>in_offset</code>
         * to <code>(in_offset + blockLength - 1)</code>
         * and the ciphertext
         * is written into <code>out</code>
         * from <code>out_offset</code>
         * to <code>(out_offset + blockLength - 1)</code>
         * Note that there will be at least blockLength bytes left in each
         * array at the supplied offsets, this is checked in the superclass.
         * The <code>in</code> and <code>out</code> buffers can be the same.
         *
         * <p>
         * @param in          buffer holding the plaintext to be encrypted
         * @param in_offset   the start of plaintext within <code>in</code>
         * @param out         buffer to hold the encrypted ciphertext result
         * @param out_offset  the start of cyphertext within <code>out</code>
         */
        protected abstract void
        blockEncrypt(byte in[], int in_offset, byte out[], int out_offset );

        /**
         * Perform a decryption in the extended class.
         * The cyphertext in <code>in</code> is decrypted
         * from <code>in_offset</code>
         * to <code>(in_offset + blockLength - 1)</code>
         * and the plaintext
         * is written into <code>out</code>
         * from <code>out_offset</code>
         * to <code>(out_offset + blockLength - 1)</code>
         * Note that there will be at least blockLength bytes left in each
         * array at the supplied offsets, this is checked in the superclass.
         * The <code>in</code> and <code>out</code> buffers can be the same.
         * <p>
         * @param in          buffer holding the cyphertext to be decrypted
         * @param in_offset   the start of cyphertext within <code>in</code>
         * @param out         buffer to hold the decrypted plaintext result
         * @param out_offset  the start of plaintext within <code>out</code>
         */
        protected abstract void
        blockDecrypt(byte in[], int in_offset, byte out[], int out_offset );



        //
        // This comment was here originally, but I don't think
        // it is appropriate because of variable key algorithms.  iang.
        //     N.B. the library writer should also implement a 
        //     <code>public static final int BLOCK_LENGTH</code>
        //     for any classes that derive from this one.
        //

        /**
         * Return the block length of this cipher.
         * Note that for variable block length ciphers, this will return
         * the length of the block as initiated by the constructor.
         * (For variable length ciphers, consider implementing
         * blockLengthMin, blockLengthMax and blockLengthMod).
         * <p>
         * @return            the block length (in bytes) of this object
         */
        public abstract int blockLength();

        /**
         * Return the key length for this cipher.
         * Note that for variable key length ciphers, this will return
         * the length of the block as initiated by the constructor.
         * <p>
         * @return            the key length (in bytes) of this object
         */
        public abstract int keyLength();
}
