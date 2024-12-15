/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

package cryptix.security;
import java.io.PrintStream;

/**
 * IDEA is a block cipher with a key length of 16 bytes
 * and a block length of 8 bytes.  It is highly popular,
 * being the original cipher in PGP, and has received a
 * lot of cryptanalytic attention.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * IDEA was written by Dr. X. Lai and Prof. J. Massey.
 * See the
 * <a href="http://www.ascom.ch/Web/systec/security/idea.htm">IDEA page</a>
 * for more details.
 * The algorithm is subject to patent claims by
 * <a href="http://www.ascom.ch/systec/">Ascom Systec Ltd</a>
 * (applied for May 1991)
 * and is
 * <a href="http://www.ascom.ch/Web/systec/policy/normal/policy.html">
 * licensable</a>.
 *
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * @author Systemics Ltd
 */
public final class IDEA extends BlockCipher
{
        private static final String LIBRARY_NAME = "idea";
        private static boolean native_link_ok = false;
        private static boolean native_lib_loaded = false;
        private static String native_link_err = "Class never loaded";

        static 
        {
                //
                // load the DLL or shared library containing the native code
                // implementation of the IDEA block cipher
                //
                try
                {
                        System.loadLibrary( LIBRARY_NAME );
                        native_lib_loaded = true;
                        try
                        {
                                //
                                //      Should really do a bit more testing than this ...
                                //
                                if (idea_test() == 0)
                                {
                                        native_link_ok = true;
                                        native_link_err = null;
                                }
                                else
                                {
                                        native_link_err = "Self test failed";
                                }
                        }
                        catch ( UnsatisfiedLinkError ule )
                        {
                                native_link_err = "Errors linking to " + LIBRARY_NAME + " native library";
                        }
                }
                catch ( UnsatisfiedLinkError ule )
                {
                        native_link_err = "The " + LIBRARY_NAME + " native library was not found";
                }
        }

        public final static boolean
        hasFileLibraryLoaded()
        {
                return native_lib_loaded;
        }

        public final static boolean
        isLibraryCorrect()
        {
                return native_link_ok;
        }

        public final static String
        getLinkErrorString()
        {
                return native_link_err;
        }
        



        /**
         * The length of a block - DEPRECATED - use blockLength() instead.
         */
        public static final int BLOCK_LENGTH = 8;

        /**
         * The length of a the user key - DEPRECATED - use keyLength() instead.
         */
        public static final int KEY_LENGTH = 16;

        //
        // The length of a the internal buffer for the native code.
        //
        // KEYS_PER_ROUND * ROUNDS + 4
        private static final int INTERNAL_KEY_LENGTH = 104;

        private static final int KEYS_PER_ROUND = 6;
        private static final int ROUNDS = 8;

        /**
         * Return the block length of this cipher.
         * @return      the block length in bytes is 8
         */
        public int 
        blockLength()
        {
                return BLOCK_LENGTH;
        }

        /**
         * Return the key length of this cipher.
         * @return      the key length in bytes is 16
         */
        public int 
        keyLength()
        {
                return KEY_LENGTH;
        }




        //
        // These could be shorts, but for now leave them as ints
        // since Java doesn't have unsigned :-(
        //
        private int ks[];
        private int dks[] = null;
        
        /**
         * Create an IDEA block cipher from a key in a byte array.
         * The byte array must be keyLength bytes long.
         * @param userKey    the user key in a byte[ keyLength() ] array
         * @throws           CryptoError if length of key array is wrong
         */
        public IDEA( byte userKey[] ) 
        {
                if ( userKey.length != KEY_LENGTH )
                        throw new CryptoError( "Idea: User key length wrong" );

                if (native_link_ok) native_ks(userKey); else java_ks(userKey);
        }

        //
        //  Multiplication modulo (2**16)+1
        //
        //  Why was this public? made private.
        static private int mul(int a, int b)
        {
                int p;

                a &= 0xffff;
                b &= 0xffff;
                if (a != 0)
                {
                        if (b != 0)
                        {
                                p = a * b;
                                b = p & 0xffff;
                                a = p >>> 16;
                                return (b - a + ((b < a) ? 1 : 0)) & 0xffff;
                        }
                        else
                                return (1 - a) & 0xffff;
                }
                return (1 - b) & 0xffff; 
        }


        //
        // Compute inverse of x, modulo (2**16)+1, using Euclidean gcd algorithm
        //
        // The Euclidean part of this algorithm could live in a
        // general purpose math library, but then it would probably
        // end up too slow.
        //
        //  Why was this public?
        static private int inv(int x)
        {
                int t0, t1, q, y;

                x &= 0xffff;

                if (x <= 1)     /* Since zero and one are self inverse */
                        return x;

                t1 = 0x10001 / x;       /* Since x >= 2, the result is 16bit */
                y = 0x10001 % x;
                if (y == 1)
                        return ((1 - t1) & 0xffff);

                t0 = 1;
                do
                {
                        q = x / y;
                        x %= y;
                        t0 = (t0 + q * t1) & 0xffff;
                        if (x == 1)
                                return t0;
                        q = y / x;
                        y = y % x;
                        t1 += q * t0;
                } while (y != 1);

                return (1-t1) & 0xffff;
        }

        //
        // Expand user key of 128 bits to full of 832 bits
        //
        //  Why was this public?
        private void java_ks(byte userKey[])
        {
                int i, j;

                ks = new int[INTERNAL_KEY_LENGTH/2];

                for(i = 0; i < KEY_LENGTH/2; i++)
                        ks[i] = (((userKey[i*2] & 0xff) << 8)| (userKey[i*2+1] & 0xff));

                j = 0;
                int koff = 0;
                for(; i < INTERNAL_KEY_LENGTH/2; i++)
                {
                        j++;
                        ks[koff+j+7] = ((ks[koff+(j&7)] << 9) | (ks[koff+((j+1)&7)] >>> 7)) & 0xffff;
                        koff += j & 8;
                        j &= 7;
                }
        }

        //
        // Create decryption key.
        // Note that this is only called if decryption is asked for.
        // We rely on the constructor doing the encryption key schedule.
        //
        //  Why was this public?
        private void java_dks()
        {
                int i;
                int j = 0;

                dks = new int[INTERNAL_KEY_LENGTH/2];

                dks[KEYS_PER_ROUND * ROUNDS + 0] = inv(ks[j++]);
                dks[KEYS_PER_ROUND * ROUNDS + 1] = -ks[j++];
                dks[KEYS_PER_ROUND * ROUNDS + 2] = -ks[j++];
                dks[KEYS_PER_ROUND * ROUNDS + 3] = inv(ks[j++]);

                for (i = KEYS_PER_ROUND * (ROUNDS-1); i >= 0; i -= KEYS_PER_ROUND)
                {
                        dks[i+4] = ks[j++];
                        dks[i+5] = ks[j++];
                        dks[i+0] = inv(ks[j++]);
                        if (i > 0)
                        {
                                dks[i+2] = -ks[j++];
                                dks[i+1] = -ks[j++];
                        }
                        else
                        {
                                dks[i+1] = -ks[j++];
                                dks[i+2] = -ks[j++];
                        }
                        dks[i+3]=inv(ks[j++]);
                }
        }

        //
        // Encryption and decryption
        //
        //  Why was this public?
        private void
        java_encrypt(byte in[], int in_offset, byte out[], int out_offset, int[] key)
        {
                int k = 0;
                int t0, t1;

                int x0 = in[in_offset++] << 8;
                x0 |= in[in_offset++] & 0xff;
                int x1 = in[in_offset++] << 8;
                x1 |= in[in_offset++] & 0xff;
                int x2 = in[in_offset++] << 8;
                x2 |= in[in_offset++] & 0xff;
                int x3 = in[in_offset++] << 8;
                x3 |= in[in_offset] & 0xff;

                for (int i = 0; i < ROUNDS; ++i)
                {
                        x0 = mul(x0, key[k++]);
                        x1 += key[k++];
                        x2 += key[k++];
                        x3 = mul(x3, key[k++]);

                        t0 = x2;
                        x2 = mul(x0^x2, key[k++]);
                        t1 = x1;
                        x1 = mul((x1^x3)+x2, key[k++]);
                        x2 += x1;

                        x0 ^= x1;
                        x3 ^= x2;
                        x1 ^= t0;
                        x2 ^= t1;
                }

                x0 = mul(x0, key[k++]);
                t0 = x1;
                x1 = x2 + key[k++];
                x2 = t0 + key[k++];
                x3 = mul(x3, key[k]);

                out[out_offset++] = (byte)(x0 >>> 8);
                out[out_offset++] = (byte)(x0);
                out[out_offset++] = (byte)(x1 >>> 8);
                out[out_offset++] = (byte)(x1);
                out[out_offset++] = (byte)(x2 >>> 8);
                out[out_offset++] = (byte)(x2);
                out[out_offset++] = (byte)(x3 >>> 8);
                out[out_offset] = (byte)(x3);
        }

        //
        // Encrypt a block.
        // The in and out buffers can be the same.
        // @param in The data to be encrypted.
        // @param in_offset   The start of data within the in buffer.
        // @param out The encrypted data.
        // @param out_offset  The start of data within the out buffer.
        //
        protected void 
        blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
                if ( ks == null )
                        throw new CryptoError( "Idea: User key not set." );

                if (native_link_ok)
                        native_encrypt(in, in_offset, out, out_offset, ks);
                else
                        java_encrypt(in, in_offset, out, out_offset, ks);
        }

        //
        // Decrypt a block.
        // The in and out buffers can be the same.
        // @param in The data to be decrypted.
        // @param in_offset   The start of data within the in buffer.
        // @param out The decrypted data.
        // @param out_offset  The start of data within the out buffer.
        //
        protected void
        blockDecrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
                if ( dks == null )
                {
                        //
                        //      Don't create the decrypt schedule unless
                        //      it is needed.
                        //      It is made from the encrypt schedule.
                        //
                        dks = new int [INTERNAL_KEY_LENGTH/2];
                        if (native_link_ok) native_dks(); else java_dks();
                }

                if (native_link_ok)
                        native_encrypt(in, in_offset, out, out_offset, dks);
                else
                        java_encrypt(in, in_offset, out, out_offset, dks);
        }



        //
        //      This style - passing the key schedule into the encrypt -
        //      is used to avoid having the C allocate its own buffers.
        //
        //      For now, just do key schedule in java.  Not much
        //      would be gained regardless.
        //
        /**
         *      Place marker, untested, unworking.
         */
        public void native_ks(byte userKey[])
        {
                java_ks( userKey );
                // ks = new int [INTERNAL_KEY_LENGTH/2];
        }
        /**
         *      Place marker, untested, unworking.
         */
        public void native_dks()
        {
                java_dks();
        }
        /**
         *      Place marker, untested, unworking.
         */
        public void native_encrypt(byte in[], int in_offset, byte out[], int out_offset, int[] key) {}


        //
        // The native functions that implement the IDEA algorithm.
        //      Synchronised for convenience of the .so developer
        //

        private static synchronized native int idea_test();
        /**
         * Expand the user key.
         */
        private synchronized native void idea_ks( byte userKey[] );

        /**
         * Expand the decryption key.
         */
        private synchronized native void idea_dks();

        /**
         * Encrypt/decrypt the block (depending upon encrypt flag)
         */
        private synchronized native void idea_encrypt( byte in[], int in_offset, byte out[], int out_offset, byte[] key );



///////////////////////////////// T E S T /////////////////////////
        /**
         * Entry point for <code>self_test</code>.
         */
        public static final void
        main(String argv[]) 
        {
                try {
                        self_test(System.out, argv);
                }
                catch(Throwable t)
                {
                        t.printStackTrace();
                }
        }

//
//      Test data
//
// 00010002000300040005000600070008  0000000100020003  11FBED2B01986DE5
// 00010002000300040005000600070008  0102030405060708  540E5FEA18C2F8B1
// 00010002000300040005000600070008  0019324B647D96AF  9F0A0AB6E10CED78
// 00010002000300040005000600070008  F5202D5B9C671B08  CF18FD7355E2C5C5
// 00010002000300040005000600070008  FAE6D2BEAA96826E  85DF52005608193D
// 00010002000300040005000600070008  0A141E28323C4650  2F7DE750212FB734
// 00010002000300040005000600070008  050A0F14191E2328  7B7314925DE59C09
// 0005000A000F00140019001E00230028  0102030405060708  3EC04780BEFF6E20
// 3A984E2000195DB32EE501C8C47CEA60  0102030405060708  97BCD8200780DA86
// 006400C8012C019001F4025802BC0320  05320A6414C819FA  65BE87E7A2538AED
// 9D4075C103BC322AFB03E7BE6AB30006  0808080808080808  F5DB1AC45E5EF9F9

        /**
         * Runs algorithm through test data.
         */
        public static void
        self_test( PrintStream out, String argv[] )
        throws Exception
        {
                test(out, "00010002000300040005000600070008", "0000000100020003", "11FBED2B01986DE5" );
                test(out, "0005000A000F00140019001E00230028", "0102030405060708", "3EC04780BEFF6E20" );
                test(out, "3A984E2000195DB32EE501C8C47CEA60", "0102030405060708", "97BCD8200780DA86" );
                test(out, "006400C8012C019001F4025802BC0320", "05320A6414C819FA", "65BE87E7A2538AED" );
                test(out, "9D4075C103BC322AFB03E7BE6AB30006", "0808080808080808", "F5DB1AC45E5EF9F9" );
        }
        
        private static void
        test( PrintStream out, String keyStr, String plainStr, String cipherStr )
        {
                byte key[] = fromString( keyStr );
                byte plain[] = fromString( plainStr );
                byte cipher[] = fromString( cipherStr );
                IDEA idea = new IDEA( key );
                byte encP[] = new byte[plain.length];
                byte decC[] = new byte[plain.length];
                idea.encrypt( plain, encP );
                String a,b;
                out.println( "plain:" + toString( plain ) + " enc:" + ( a = toString( encP ) ) + " calc:" + ( b = toString( cipher) ) );
                if ( a.equals( b ) )
                        out.println( "encryption good" );
                else
                        out.println( " ********* IDEA ENC FAILED ********* " );
                idea.decrypt( encP, decC );
                out.println( "  enc:" + toString( encP ) + " dec:" + ( a = toString( decC ) ) + " calc:" + ( b = toString( plain ) ) );
                if ( a.equals( b ) )
                        out.println( "decryption good" );
                else
                        out.println( " ********* IDEA DEC FAILED ********* " );
        }

        private static byte[]
        fromString( String inHex )
        {
                int len=inHex.length();
                int pos =0;
                byte buffer[] = new byte [( ( len + 1 ) / 2 )];
                if ( ( len % 2 ) == 1 )
                {
                        buffer[0]=(byte)asciiToHex(inHex.charAt(0));
                        pos=1;
                        len--;
                }

                for(int ptr = pos; len > 0; len -= 2 )
                        buffer[pos++] = (byte)( 
                                        ( asciiToHex( inHex.charAt( ptr++ ) ) << 4 ) |
                                        ( asciiToHex( inHex.charAt( ptr++ ) ) )
                                        );
                return buffer;
        }

        private static final String
        toString( byte buffer[] )
        {
                StringBuffer returnBuffer = new StringBuffer();
                for ( int pos = 0, len = buffer.length; pos < len; pos++ )
                        returnBuffer.append( hexToAscii( ( buffer[pos] >>> 4 ) & 0x0F ) )
                                                .append( hexToAscii( buffer[pos] & 0x0F ) );
                return returnBuffer.toString();
        }

        private static final int
        asciiToHex(char c)
        {
                if ( ( c >= 'a' ) && ( c <= 'f' ) )
                        return ( c - 'a' + 10 );
                if ( ( c >= 'A' ) && ( c <= 'F' ) )
                        return ( c - 'A' + 10 );
                if ( ( c >= '0' ) && ( c <= '9' ) )
                        return ( c - '0' );
                throw new Error("ascii to hex failed");
        }

        private static char
        hexToAscii(int h)
        {
                if ( ( h >= 10 ) && ( h <= 15 ) )
                        return (char)( 'A' + ( h - 10 ) );
                if ( ( h >= 0 ) && ( h <= 9 ) )
                        return (char)( '0' + h );
                throw new Error("hex to ascii failed");
        }
}

