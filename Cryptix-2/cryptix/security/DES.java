/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import java.io.PrintStream;

/**
 * DES is a block cipher with an 8 byte block size.  The key length
 * is 8 bytes, but only 56 bits are used as the parity bit in each
 * byte is ignored.
 * <P>
 * This algorithm has been seriously analysed over the last 30 years,
 * and no significant weaknesses have been reported.  It's only flaw
 * is that the key length of 56 bits makes it relatively easy to
 * brute-force it.  To overcome this near-fatal flaw, it is recommended
 * that DES be used in triple-DES mode.  Need reference here...
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * DES was written by IBM and first released in 1976.
 * See
 *      <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *      "Chapter 12 Data Encryption Standard,"
 *      <cite>Applied Cryptography</cite>,
 *      Wiley 2nd Ed, 1996
 * for detailed information.  The algorithm is freely usable.
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
 * @author Geoffrey Keating (this Java implementation)
 * @author Eric Young (SPtrans)
 * @see BlockCipher
 */
public final class DES extends BlockCipher
{
        private static final int REQUIRED_LIB_MAJOR_VER = 1;
        private static final int REQUIRED_LIB_MINOR_VER = 11;
        
        private static final String LIBRARY_NAME = "des";

        private static boolean native_link_ok = false;
        private static boolean native_lib_loaded = false;
        private static String native_link_err = "Class never loaded";

        static 
        {
                // load the DLL or shared library that contains the native code
                // implementation of the des block cipher.
                try
                {
                        System.loadLibrary( LIBRARY_NAME );
                        native_lib_loaded = true;
                        try
                        {
                                if ( ( getLibMajorVersion() != REQUIRED_LIB_MAJOR_VER ) || 
                                        ( getLibMinorVersion() < REQUIRED_LIB_MINOR_VER ) )
                                {
                                        native_link_err = "The " +
                                            LIBRARY_NAME + " native library " + 
                                            getLibMajorVersion() + "." +
                                            getLibMinorVersion() +
                                            " is too old to use. Version " + 
                                            REQUIRED_LIB_MAJOR_VER + "." +
                                            REQUIRED_LIB_MINOR_VER + 
                                            " required.";
                                }
                                else
                                {
                                        native_link_ok = true;
                                        native_link_err = null;
                                }
                        }
                        catch ( UnsatisfiedLinkError ule )
                        {
                                native_link_err = "The " +
                                    LIBRARY_NAME +
                                    " native library is too old to use." +
                                    "Version " +
                                    REQUIRED_LIB_MAJOR_VER + "." +
                                    REQUIRED_LIB_MINOR_VER +
                                    " required.";
                        }
                }
                catch ( UnsatisfiedLinkError ule )
                {
                        native_link_err = "The " +
                            LIBRARY_NAME + " native library was not found.";
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
        public static final int KEY_LENGTH = 8;

        private static final int ROUNDS = 16;

        /* The size of the key after it has been scheduled. */
        static private final int INTERNAL_KEY_LENGTH = ROUNDS*2;
        //
        // This was in the original stub, but appears to be wrong.
        //
        // This is the length of the internal buffer for the native code.
        // private static final int INTERNAL_KEY_LENGTH = 128;

        /**
         * Return the block length of this cipher.
         * @return       the block length in bytes is 8
         */
        public int 
        blockLength()
        {
            return BLOCK_LENGTH;
        }

        /**
         * Return the key length for this cipher.
         * @returns      the key length in bytes is 8
         */
        public int 
        keyLength()
        {
            return KEY_LENGTH;
        }



        /**
         * Create a DES block cipher from a key in a byte array.
         * The byte array must be keyLength bytes long.
         * @param userKey    the user key in a byte[ keyLength() ] array
         * @throws           CryptoError if length of key array is wrong
         * @throws UnsatisfiedLinkError if the library is not of the correct version
         */
        public DES( byte userKey[] ) 
        {
                if ( userKey.length != KEY_LENGTH )
                        throw new CryptoError( "DES: User key length wrong" );

                // this was in the original stub but appears wrong.
                // ks = new byte[INTERNAL_KEY_LENGTH];

                if (native_link_ok)
                        set_ks(userKey);
                else
                        java_ks(userKey);
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
                if ( key == null )
                        throw new CryptoError( "DES: User key not set" );

                if (native_link_ok)
                        do_des( in, in_offset, out, out_offset, true );
                else
                        java_encrypt( in, in_offset, out, out_offset, true );
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
                if ( key == null )
                        throw new CryptoError( "DES: User key not set" );

                if (native_link_ok)
                        do_des( in, in_offset, out, out_offset, false );
                else
                        java_encrypt( in, in_offset, out, out_offset, false );
        }



        //
        //      The native methods that implement the DES algorithm.
        //      Synchronised for convenience of the .so developer
        //
        //      Place markers, untested, unworking.
        //      For the future: maybe change it to pass the key schedule
        //      in to save the C from allocating its own memory, a la IDEA.
        //
        private native static int getLibMajorVersion(); // get native version
        private native static int getLibMinorVersion(); // get native version

        //
        //      Expand the user key to an internal form.
        //
        private native void set_ks( byte userKey[] );

        //
        // Encrypt/decrypt the block (depending upon encrypt flag)
        //
        private native void do_des( byte in[], int in_offset, byte out[], int out_offset, boolean encrypt );
        
/////////////////////////////////// START DES CODE ///////////////



        /* The key after it has been scheduled. */
        private int key[] = new int[INTERNAL_KEY_LENGTH];
        
        /* Table for PC2 permutation in key schedule. */
        static private int skb[] = new int[8*64];
        static  {
                for (int i = 0; i < 8*64; i++)
                        skb[i] = 0;

                /* represents the bit number that each permuted bit is derived from */
                String cd = "D]PKESYM`UBJ\\@RXA`I[T`HC`LZQ"+"\\PB]TL`[C`JQ@Y`HSXDUIZRAM`EK";
                int l;
                int numdone = 0;
                int opos = 0;
                for (int c = 0; c < cd.length(); c++)
                        if ((l = cd.charAt(c) - '@') != 32)
                        {
                                int indexbit = 1 << numdone;
                                for (int i = 0; i < 64; i++)
                                        if ((indexbit & i) != 0)
                                                skb[opos+i] |= 1 << l;
                                if (++numdone == 6)
                                {
                                        opos += 64;
                                        numdone = 0;
                                }
                        }
        }
        
        private void java_ks(byte userkey[])
        {
                int c, d;       // These need to be fast

                int t;
                c = userkey[0] & 0xFF | userkey[1] << 8 & 0xFF00 |
                        userkey[2] << 16 & 0xFF0000 | userkey[3] << 24 & 0xFF000000;
                d = userkey[4] & 0xFF | userkey[5] << 8 & 0xFF00 |
                        userkey[6] << 16 & 0xFF0000 | userkey[7] << 24 & 0xFF000000;

                /* PC1, as per Earl Young. */
                t = (d>>>4 ^ c) & 0x0f0f0f0f;
                c ^= t;
                d ^= t << 4;
                t = (c<<18 ^ c) & 0xcccc0000;
                c ^= t ^ t>>>18;                
                t = (d<<18 ^ d) & 0xcccc0000;
                d ^= t ^ t>>>18;                
                t = (d>>>1 ^ c) & 0x55555555;
                c ^= t;
                d ^= t << 1;
                t = (c>>>8 ^ d) & 0x00ff00ff;
                d ^= t;
                c ^= t << 8;
                t = (d>>>1 ^ c) & 0x55555555;
                c ^= t;
                d ^= t << 1;

                d = d<<16 & 0x00ff0000 | d & 0x0000ff00 | d>>16 & 0xFF | c>>4 & 0x0F000000;
                c &= 0x0fffffff;
                
                for (int i=0; i<ROUNDS; i++)
                {
                        int s;
                        
                        if ((0x7EFC >> i & 1) == 1)
                                { c = c>>>2 | c<<26; d = d>>>2 | d<<26; }
                        else
                                { c = c>>>1 | c<<27; d = d>>>1 | d<<27; }
                        c &= 0x0fffffff;
                        d &= 0x0fffffff;

                        s= skb[        c     &0x3f                            ]|
                           skb[0x040 | c>> 6 &0x03 | c>> 7 &0x3c              ]|
                           skb[0x080 | c>>13 &0x0f | c>>14 &0x30              ]|
                           skb[0x0C0 | c>>20 &0x01 | c>>21 &0x06 | c>>22 &0x38];
                        t= skb[0x100 | d     &0x3f              ]|
                           skb[0x140 | d>> 7 &0x03 | d>> 8 &0x3c]|
                           skb[0x180 | d>>15 &0x3f              ]|
                           skb[0x1C0 | d>>21 &0x0f | d>>22 &0x30];

                        key[i<<1] = t<<16 | s&0x0000ffff;
                        s = s>>>16 | t&0xffff0000;
                        key[i<<1|1] = s<<4 | s>>>28;
                }
        }
                

        /* table for S-boxes and permutations, used in encrypt_base. */
        static private int SPtrans[] = new int[8*64];
        static  {
                /* I'd _really_ like to just say 'SPtrans =  { ... }', but
                   that would be terribly inefficient (code size + time). 
                   Instead we use a compressed representation. */
                String spt =
                   "g3H821:80:H03BA0@N1290BAA88::3112aIH8:8282@0@AH0:1W3A8P810@22;22A"+
                   "18^@9H9@129:<8@822`?:@0@8PH2H81A19:G1@03403A0B1;:0@1g192:@919AA0A"+
                   "109:W21492H@0051919811:215011139883942N8::3112A2:31981jM118::A101"+
                   "@I88:1aN0<@030128:X;811`920:;H0310D1033@W980:8A4@804A3803o1A2021B"+
                   "2:@1AH023GA:8:@81@@12092B:098042P@:0:A0HA9>1;289:@1804:40Ph=1:H0I"+
                   "0HP0408024bC9P8@I808A;@0@0PnH0::8:19J@818:@iF0398:8A9H0<13@001@11"+
                   "<8;@82B01P0a2989B:0AY0912889bD0A1@B1A0A0AB033O91182440A9P8@I80n@1"+
                   "I03@1J828212A`A8:12B1@19A9@9@8^B:0@H00<82AB030bB840821Q:8310A3021"+
                   "02::A1::20A1;8"; /* OK, try to type _that_! */
                   /* [526 chars, 3156 bits] */
                /* The theory is that each bit position in each int of SPtrans is
                   set in exactly 32 entries. We keep track of set bits. */
                for (int i = 0; i < 8*64; i++)
                        SPtrans[i] = 0; /* bits that are not set must be clear. */
                
                int stringpos = 0;
                for (int i = 0; i < 32; i++) /* each bit position */
                {
                        int index = -1; /* pretend the -1th bit was set */
                        int bit = 1 << i;
                        for (int j = 0; j < 32; j++) /* each set bit */
                        {
                                /* Each character consists of two three-bit values: */
                                int c = spt.charAt(stringpos >> 1) - '0' >> (stringpos & 1) * 3 & 7;
                                stringpos++;
                                if (c <= 4)
                                {
                                        /* values 0-4 indicate a set bit 1-5 positions from the
                                           previous set bit */
                                        SPtrans[index += c+1] |= bit;
                                        continue;
                                }
                                /* other values take at least parameter, the next value in the
                                   sequence. */
                                int param = spt.charAt(stringpos >> 1) - '0' >> (stringpos & 1) * 3 & 7;
                                stringpos++;
                                if (c == 5)
                                        /* 5 indicates a bit set param+6 positions from the
                                           previous set bit */
                                        SPtrans[index += param+6] |= bit;
                                else if (c == 6)
                                        /* 6 indicates a bit set (param*64)+1 positions from the
                                           previous set bit */
                                        SPtrans[index += (param<<6)+1] |= bit;
                                else
                                {
                                        /* 7 indicates that we should skip (param*64) positions, then process
                                           the next value which will be in the range 0-4. */
                                        index += param<<6;
                                        j--;
                                }
                        }
                }
        }

        /* DES encrypt without IP and FP. */
        long encrypt_base(int l, int r)
        {
                int u;
                int t; /* look! we fit all four variables (plus the class itself)
                        into short byte-codes! */
                
                u = r << 1 | r >>> 31;
                r = l << 1 | l >>> 31;
                l = u;
                
                for (int i = 0; i < INTERNAL_KEY_LENGTH; i += 4)
                {
                        u=r^key[i  ];
                        t=r^key[i+1];
                        t = t >>> 4 | t << 28;
                        l^=     SPtrans[0x040 | t     &0x3f]|
                                SPtrans[0x0C0 | t>> 8 &0x3f]|
                                SPtrans[0x140 | t>>16 &0x3f]|
                                SPtrans[0x1C0 | t>>24 &0x3f]|
                                SPtrans[        u     &0x3f]|
                                SPtrans[0x080 | u>> 8 &0x3f]|
                                SPtrans[0x100 | u>>16 &0x3f]|
                                SPtrans[0x180 | u>>24 &0x3f];
                        u=l^key[i+2];
                        t=l^key[i+3];
                        t = t >>> 4 | t << 28;
                        r^=     SPtrans[0x040 | t     &0x3f]|
                                SPtrans[0x0C0 | t>> 8 &0x3f]|
                                SPtrans[0x140 | t>>16 &0x3f]|
                                SPtrans[0x1C0 | t>>24 &0x3f]|
                                SPtrans[        u     &0x3f]|
                                SPtrans[0x080 | u>> 8 &0x3f]|
                                SPtrans[0x100 | u>>16 &0x3f]|
                                SPtrans[0x180 | u>>24 &0x3f];
                }
                
                l = l >>> 1 | l << 31;
                r = r >>> 1 | r << 31;
                t = u = 0;
                return l << 32L | r & 0xFFFFFFFFL;
        }
        
        /* DES decrypt without IP and FP. */
        long decrypt_base(int l, int r)
        {
                int u;
                int t; /* look! we fit all four variables (plus the class itself)
                        into short byte-codes! */
                
                u = r << 1 | r >>> 31;
                r = l << 1 | l >>> 31;
                l = u;
                
                for (int i = INTERNAL_KEY_LENGTH-2; i > 0; i -= 4)
                {
                        u=r^key[i  ];
                        t=r^key[i+1];
                        t = t >>> 4 | t << 28;
                        l^=     SPtrans[0x040 | t     &0x3f]|
                                SPtrans[0x0C0 | t>> 8 &0x3f]|
                                SPtrans[0x140 | t>>16 &0x3f]|
                                SPtrans[0x1C0 | t>>24 &0x3f]|
                                SPtrans[        u     &0x3f]|
                                SPtrans[0x080 | u>> 8 &0x3f]|
                                SPtrans[0x100 | u>>16 &0x3f]|
                                SPtrans[0x180 | u>>24 &0x3f];
                        u=l^key[i-2];
                        t=l^key[i-1];
                        t = t >>> 4 | t << 28;
                        r^=     SPtrans[0x040 | t     &0x3f]|
                                SPtrans[0x0C0 | t>> 8 &0x3f]|
                                SPtrans[0x140 | t>>16 &0x3f]|
                                SPtrans[0x1C0 | t>>24 &0x3f]|
                                SPtrans[        u     &0x3f]|
                                SPtrans[0x080 | u>> 8 &0x3f]|
                                SPtrans[0x100 | u>>16 &0x3f]|
                                SPtrans[0x180 | u>>24 &0x3f];
                }
                
                l = l >>> 1 | l << 31;
                r = r >>> 1 | r << 31;
                t = u = 0;
                return l << 32L | r & 0xFFFFFFFFL;
        }
        
        /* perform IP */
        static long initialPermutation(int l, int r)
        {
                int t;
                t = (r>>>4 ^ l) & 0x0f0f0f0f;
                l ^= t;
                r ^= t << 4;
                t = (l>>>16 ^ r) & 0x0000ffff;
                r ^= t;
                l ^= t << 16;
                t = (r>>>2 ^ l) & 0x33333333;
                l ^= t;
                r ^= t << 2;
                t = (l>>>8 ^ r) & 0x00ff00ff;
                r ^= t;
                l ^= t << 8;
                t = (r>>>1 ^ l) & 0x55555555;
                l ^= t;
                r ^= t << 1;            
                return l << 32L | r & 0xFFFFFFFFL;
        }
        
        /* perform FP */
        static long finalPermutation(int l, int r)
        {
                int t;
                t = (r>>>1 ^ l) & 0x55555555;
                l ^= t;
                r ^= t << 1;
                t = (l>>>8 ^ r) & 0x00ff00ff;
                r ^= t;
                l ^= t << 8;
                t = (r>>>2 ^ l) & 0x33333333;
                l ^= t;
                r ^= t << 2;
                t = (l>>>16 ^ r) & 0x0000ffff;
                r ^= t;
                l ^= t << 16;
                t = (r>>>4 ^ l) & 0x0f0f0f0f;
                l ^= t;
                r ^= t << 4;
                return l << 32L | r & 0xFFFFFFFFL;
        }
        
        //
        // Encrypt a block.
        // @param inbuf the block to encrypt (of length BLOCK_LENGTH)
        // @param outbuf the encrypted block (of length BLOCK_LENGTH)
        // @param enc encrypt or decrypt
        //
        private void java_encrypt(byte inbuf[], int in_offset, byte outbuf[], int out_offset, boolean enc)
        {
                long lr;
                
                /* IP */
                lr = initialPermutation(
                        inbuf[in_offset+0] & 0xFF | inbuf[in_offset+1] << 8 & 0xFF00 |
                        inbuf[in_offset+2] << 16 & 0xFF0000 | inbuf[in_offset+3] << 24 & 0xFF000000,
                        inbuf[in_offset+4] & 0xFF | inbuf[in_offset+5] << 8 & 0xFF00 |
                        inbuf[in_offset+6] << 16 & 0xFF0000 | inbuf[in_offset+7] << 24 & 0xFF000000);

                /* do it! */
                if (enc)
                        lr = encrypt_base((int)(lr >> 32),(int)lr);
                else
                        lr = decrypt_base((int)(lr >> 32),(int)lr);
                        
                /* FP */
                lr = finalPermutation((int)(lr >> 32),(int)lr);
                
                outbuf[out_offset+0] = (byte)(lr>>32); outbuf[out_offset+1] = (byte)(lr>>40);
                outbuf[out_offset+2] = (byte)(lr>>48); outbuf[out_offset+3] = (byte)(lr>>56);
                outbuf[out_offset+4] = (byte)(lr    ); outbuf[out_offset+5] = (byte)(lr>> 8);
                outbuf[out_offset+6] = (byte)(lr>>16); outbuf[out_offset+7] = (byte)(lr>>24);
        }



///////////////////////////////// T E S T ////////////////////////

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
//
//      Some tests...
//
// KEY              PLAINTEXT        CIPHERTEXT
// 0101010101010101 95f8a5e5dd31d900 8000000000000000
// 0101010101010101 dd7f121ca5015619 4000000000000000
// 0101010101010101 2e8653104f3834ea 2000000000000000
// 0101010101010101 4bd388ff6cd81d4f 1000000000000000
// 0101010101010101 20b9e767b2fb1456 0800000000000000
// 0101010101010101 55579380d77138ef 0400000000000000
// 0101010101010101 6cc5defaaf04512f 0200000000000000
// 0101010101010101 0d9f279ba5d87260 0100000000000000
// 0101010101010101 d9031b0271bd5a0a 0080000000000000
// 0101010101010101 424250b37c3dd951 0040000000000000
// 0101010101010101 b8061b7ecd9a21e5 0020000000000000
// 0101010101010101 f15d0f286b65bd28 0010000000000000
// 0101010101010101 add0cc8d6e5deba1 0008000000000000
// 0101010101010101 e6d5f82752ad63d1 0004000000000000
// 0101010101010101 ecbfe3bd3f591a5e 0002000000000000
// 0101010101010101 f356834379d165cd 0001000000000000
// 0101010101010101 2b9f982f20037fa9 0000800000000000
// 0101010101010101 889de068a16f0be6 0000400000000000
// 0101010101010101 e19e275d846a1298 0000200000000000
// 0101010101010101 329a8ed523d71aec 0000100000000000
// 0101010101010101 e7fce22557d23c97 0000080000000000
// 0101010101010101 12a9f5817ff2d65d 0000040000000000
// 0101010101010101 a484c3ad38dc9c19 0000020000000000
// 0101010101010101 fbe00a8a1ef8ad72 0000010000000000
// 0101010101010101 750d079407521363 0000008000000000
// 0101010101010101 64feed9c724c2faf 0000004000000000
// 0101010101010101 f02b263b328e2b60 0000002000000000
// 0101010101010101 9d64555a9a10b852 0000001000000000
// 0101010101010101 d106ff0bed5255d7 0000000800000000
// 0101010101010101 e1652c6b138c64a5 0000000400000000
// 0101010101010101 e428581186ec8f46 0000000200000000
// 0101010101010101 aeb5f5ede22d1a36 0000000100000000
// 0101010101010101 e943d7568aec0c5c 0000000080000000
// 0101010101010101 df98c8276f54b04b 0000000040000000
// 0101010101010101 b160e4680f6c696f 0000000020000000
// 0101010101010101 fa0752b07d9c4ab8 0000000010000000
// 0101010101010101 ca3a2b036dbc8502 0000000008000000
// 0101010101010101 5e0905517bb59bcf 0000000004000000
// 0101010101010101 814eeb3b91d90726 0000000002000000
// 0101010101010101 4d49db1532919c9f 0000000001000000
// 0101010101010101 25eb5fc3f8cf0621 0000000000800000
// 0101010101010101 ab6a20c0620d1c6f 0000000000400000
// 0101010101010101 79e90dbc98f92cca 0000000000200000
// 0101010101010101 866ecedd8072bb0e 0000000000100000
// 0101010101010101 8b54536f2f3e64a8 0000000000080000
// 0101010101010101 ea51d3975595b86b 0000000000040000
// 0101010101010101 caffc6ac4542de31 0000000000020000
// 0101010101010101 8dd45a2ddf90796c 0000000000010000
// 0101010101010101 1029d55e880ec2d0 0000000000008000
// 0101010101010101 5d86cb23639dbea9 0000000000004000
// 0101010101010101 1d1ca853ae7c0c5f 0000000000002000
// 0101010101010101 ce332329248f3228 0000000000001000
// 0101010101010101 8405d1abe24fb942 0000000000000800
// 0101010101010101 e643d78090ca4207 0000000000000400
// 0101010101010101 48221b9937748a23 0000000000000200
// 0101010101010101 dd7c0bbd61fafd54 0000000000000100
// 0101010101010101 2fbc291a570db5c4 0000000000000080
// 0101010101010101 e07c30d7e4e26e12 0000000000000040
// 0101010101010101 0953e2258e8e90a1 0000000000000020
// 0101010101010101 5b711bc4ceebf2ee 0000000000000010
// 0101010101010101 cc083f1e6d9e85f6 0000000000000008
// 0101010101010101 d2fd8867d50d2dfe 0000000000000004
// 0101010101010101 06e7ea22ce92708f 0000000000000002
// 0101010101010101 166b40b44aba4bd6 0000000000000001
// 8001010101010101 0000000000000000 95a8d72813daa94d
// 4001010101010101 0000000000000000 0eec1487dd8c26d5
// 2001010101010101 0000000000000000 7ad16ffb79c45926
// 1001010101010101 0000000000000000 d3746294ca6a6cf3
// 0801010101010101 0000000000000000 809f5f873c1fd761
// 0401010101010101 0000000000000000 c02faffec989d1fc
// 0201010101010101 0000000000000000 4615aa1d33e72f10
// 0180010101010101 0000000000000000 2055123350c00858
// 0140010101010101 0000000000000000 df3b99d6577397c8
// 0120010101010101 0000000000000000 31fe17369b5288c9
// 0110010101010101 0000000000000000 dfdd3cc64dae1642
// 0108010101010101 0000000000000000 178c83ce2b399d94
// 0104010101010101 0000000000000000 50f636324a9b7f80
// 0102010101010101 0000000000000000 a8468ee3bc18f06d
// 0101800101010101 0000000000000000 a2dc9e92fd3cde92
// 0101400101010101 0000000000000000 cac09f797d031287
// 0101200101010101 0000000000000000 90ba680b22aeb525
// 0101100101010101 0000000000000000 ce7a24f350e280b6
// 0101080101010101 0000000000000000 882bff0aa01a0b87
// 0101040101010101 0000000000000000 25610288924511c2
// 0101020101010101 0000000000000000 c71516c29c75d170
// 0101018001010101 0000000000000000 5199c29a52c9f059
// 0101014001010101 0000000000000000 c22f0a294a71f29f
// 0101012001010101 0000000000000000 ee371483714c02ea
// 0101011001010101 0000000000000000 a81fbd448f9e522f
// 0101010801010101 0000000000000000 4f644c92e192dfed
// 0101010401010101 0000000000000000 1afa9a66a6df92ae
// 0101010201010101 0000000000000000 b3c1cc715cb879d8
// 0101010180010101 0000000000000000 19d032e64ab0bd8b
// 0101010140010101 0000000000000000 3cfaa7a7dc8720dc
// 0101010120010101 0000000000000000 b7265f7f447ac6f3
// 0101010110010101 0000000000000000 9db73b3c0d163f54
// 0101010108010101 0000000000000000 8181b65babf4a975
// 0101010104010101 0000000000000000 93c9b64042eaa240
// 0101010102010101 0000000000000000 5570530829705592
// 0101010101800101 0000000000000000 8638809e878787a0
// 0101010101400101 0000000000000000 41b9a79af79ac208
// 0101010101200101 0000000000000000 7a9be42f2009a892
// 0101010101100101 0000000000000000 29038d56ba6d2745
// 0101010101080101 0000000000000000 5495c6abf1e5df51
// 0101010101040101 0000000000000000 ae13dbd561488933
// 0101010101020101 0000000000000000 024d1ffa8904e389
// 0101010101018001 0000000000000000 d1399712f99bf02e
// 0101010101014001 0000000000000000 14c1d7c1cffec79e
// 0101010101012001 0000000000000000 1de5279dae3bed6f
// 0101010101011001 0000000000000000 e941a33f85501303
// 0101010101010801 0000000000000000 da99dbbc9a03f379
// 0101010101010401 0000000000000000 b7fc92f91d8e92e9
// 0101010101010201 0000000000000000 ae8e5caa3ca04e85
// 0101010101010180 0000000000000000 9cc62df43b6eed74
// 0101010101010140 0000000000000000 d863dbb5c59a91a0
// 0101010101010120 0000000000000000 a1ab2190545b91d7
// 0101010101010110 0000000000000000 0875041e64c570f7
// 0101010101010108 0000000000000000 5a594528bebef1cc
// 0101010101010104 0000000000000000 fcdb3291de21f0c0
// 0101010101010102 0000000000000000 869efd7f9f265a09
// 1046913489980131 0000000000000000 88d55e54f54c97b4
// 1007103489988020 0000000000000000 0c0cc00c83ea48fd
// 10071034c8980120 0000000000000000 83bc8ef3a6570183
// 1046103489988020 0000000000000000 df725dcad94ea2e9
// 1086911519190101 0000000000000000 e652b53b550be8b0
// 1086911519580101 0000000000000000 af527120c485cbb0
// 5107b01519580101 0000000000000000 0f04ce393db926d5
// 1007b01519190101 0000000000000000 c9f00ffc74079067
// 3107915498080101 0000000000000000 7cfd82a593252b4e
// 3107919498080101 0000000000000000 cb49a2f9e91363e3
// 10079115b9080140 0000000000000000 00b588be70d23f56
// 3107911598090140 0000000000000000 406a9a6ab43399ae
// 1007d01589980101 0000000000000000 6cb773611dca9ada
// 9107911589980101 0000000000000000 67fd21c17dbb5d70
// 9107d01589190101 0000000000000000 9592cb4110430787
// 1007d01598980120 0000000000000000 a6b7ff68a318ddd3
// 1007940498190101 0000000000000000 4d102196c914ca16
// 0107910491190401 0000000000000000 2dfa9f4573594965
// 0107910491190101 0000000000000000 b46604816c0e0774
// 0107940491190401 0000000000000000 6e7e6221a4f34e87
// 19079210981a0101 0000000000000000 aa85e74643233199
// 1007911998190801 0000000000000000 2e5a19db4d1962d6
// 10079119981a0801 0000000000000000 23a866a809d30894
// 1007921098190101 0000000000000000 d812d961f017d320
// 100791159819010b 0000000000000000 055605816e58608f
// 1004801598190101 0000000000000000 abd88e8b1b7716f1
// 1004801598190102 0000000000000000 537ac95be69da1e1
// 1004801598190108 0000000000000000 aed0f6ae3c25cdd8
// 1002911598100104 0000000000000000 b3e35a5ee53e7b8d
// 1002911598190104 0000000000000000 61c79c71921a2ef8
// 1002911598100201 0000000000000000 e2f5728f0995013c
// 1002911698100101 0000000000000000 1aeac39a61f0a464
// 7ca110454a1a6e57 01a1d6d039776742 690f5b0d9a26939b
// 0131d9619dc1376e 5cd54ca83def57da 7a389d10354bd271
// 07a1133e4a0b2686 0248d43806f67172 868ebb51cab4599a
// 3849674c2602319e 51454b582ddf440a 7178876e01f19b2a
// 04b915ba43feb5b6 42fd443059577fa2 af37fb421f8c4095
// 0113b970fd34f2ce 059b5e0851cf143a 86a560f10ec6d85b
// 0170f175468fb5e6 0756d8e0774761d2 0cd3da020021dc09
// 43297fad38e373fe 762514b829bf486a ea676b2cb7db2b7a
// 07a7137045da2a16 3bdd119049372802 dfd64a815caf1a0f
// 04689104c2fd3b2f 26955f6835af609a 5c513c9c4886c088
// 37d06bb516cb7546 164d5e404f275232 0a2aeeae3ff4ab77
// 1f08260d1ac2465e 6b056e18759f5cca ef1bf03e5dfa575a
// 584023641aba6176 004bd6ef09176062 88bf0db6d70dee56
// 025816164629b007 480d39006ee762f2 a1f9915541020b56
// 49793ebc79b3258f 437540c8698f3cfa 6fbf1cafcffd0556
// 4fb05e1515ab73a7 072d43a077075292 2f22e49bab7ca1ac
// 49e95d6d4ca229bf 02fe55778117f12a 5a6b612cc26cce4a
// 018310dc409b26d6 1d9d5c5018f728c2 5f4c038ed12b2e41
// 1c587f1c13924fef 305532286d6f295a 63fac0d034d9f793

public static void
self_test( PrintStream out, String argv[] )
throws Exception
{
        test(out, "0101010101010101", "95f8a5e5dd31d900", "8000000000000000");
        test(out, "0101010101010101", "dd7f121ca5015619", "4000000000000000");
        test(out, "0101010101010101", "2e8653104f3834ea", "2000000000000000");
        test(out, "0101010101010101", "4bd388ff6cd81d4f", "1000000000000000");
        test(out, "0101010101010101", "20b9e767b2fb1456", "0800000000000000");
        test(out, "0101010101010101", "55579380d77138ef", "0400000000000000");
        test(out, "0101010101010101", "6cc5defaaf04512f", "0200000000000000");
        test(out, "0101010101010101", "0d9f279ba5d87260", "0100000000000000");
        test(out, "0101010101010101", "d9031b0271bd5a0a", "0080000000000000");
        test(out, "0101010101010101", "424250b37c3dd951", "0040000000000000");
        test(out, "0101010101010101", "b8061b7ecd9a21e5", "0020000000000000");
        test(out, "0101010101010101", "f15d0f286b65bd28", "0010000000000000");
        test(out, "0101010101010101", "add0cc8d6e5deba1", "0008000000000000");
        test(out, "0101010101010101", "e6d5f82752ad63d1", "0004000000000000");
        test(out, "0101010101010101", "ecbfe3bd3f591a5e", "0002000000000000");
        test(out, "0101010101010101", "f356834379d165cd", "0001000000000000");
        test(out, "0101010101010101", "2b9f982f20037fa9", "0000800000000000");
        test(out, "0101010101010101", "889de068a16f0be6", "0000400000000000");
        test(out, "0101010101010101", "e19e275d846a1298", "0000200000000000");
        test(out, "0101010101010101", "329a8ed523d71aec", "0000100000000000");
        test(out, "0101010101010101", "e7fce22557d23c97", "0000080000000000");
        test(out, "0101010101010101", "12a9f5817ff2d65d", "0000040000000000");
        test(out, "0101010101010101", "a484c3ad38dc9c19", "0000020000000000");
        test(out, "0101010101010101", "fbe00a8a1ef8ad72", "0000010000000000");
        test(out, "0101010101010101", "750d079407521363", "0000008000000000");
        test(out, "0101010101010101", "64feed9c724c2faf", "0000004000000000");
        test(out, "0101010101010101", "f02b263b328e2b60", "0000002000000000");
        test(out, "0101010101010101", "9d64555a9a10b852", "0000001000000000");
        test(out, "0101010101010101", "d106ff0bed5255d7", "0000000800000000");
        test(out, "0101010101010101", "e1652c6b138c64a5", "0000000400000000");
        test(out, "0101010101010101", "e428581186ec8f46", "0000000200000000");
        test(out, "0101010101010101", "aeb5f5ede22d1a36", "0000000100000000");
        test(out, "0101010101010101", "e943d7568aec0c5c", "0000000080000000");
        test(out, "0101010101010101", "df98c8276f54b04b", "0000000040000000");
        test(out, "0101010101010101", "b160e4680f6c696f", "0000000020000000");
        test(out, "0101010101010101", "fa0752b07d9c4ab8", "0000000010000000");
        test(out, "0101010101010101", "ca3a2b036dbc8502", "0000000008000000");
        test(out, "0101010101010101", "5e0905517bb59bcf", "0000000004000000");
        test(out, "0101010101010101", "814eeb3b91d90726", "0000000002000000");
        test(out, "0101010101010101", "4d49db1532919c9f", "0000000001000000");
        test(out, "0101010101010101", "25eb5fc3f8cf0621", "0000000000800000");
        test(out, "0101010101010101", "ab6a20c0620d1c6f", "0000000000400000");
        test(out, "0101010101010101", "79e90dbc98f92cca", "0000000000200000");
        test(out, "0101010101010101", "866ecedd8072bb0e", "0000000000100000");
        test(out, "0101010101010101", "8b54536f2f3e64a8", "0000000000080000");
        test(out, "0101010101010101", "ea51d3975595b86b", "0000000000040000");
        test(out, "0101010101010101", "caffc6ac4542de31", "0000000000020000");
        test(out, "0101010101010101", "8dd45a2ddf90796c", "0000000000010000");
        test(out, "0101010101010101", "1029d55e880ec2d0", "0000000000008000");
        test(out, "0101010101010101", "5d86cb23639dbea9", "0000000000004000");
        test(out, "0101010101010101", "1d1ca853ae7c0c5f", "0000000000002000");
        test(out, "0101010101010101", "ce332329248f3228", "0000000000001000");
        test(out, "0101010101010101", "8405d1abe24fb942", "0000000000000800");
        test(out, "0101010101010101", "e643d78090ca4207", "0000000000000400");
        test(out, "0101010101010101", "48221b9937748a23", "0000000000000200");
        test(out, "0101010101010101", "dd7c0bbd61fafd54", "0000000000000100");
        test(out, "0101010101010101", "2fbc291a570db5c4", "0000000000000080");
        test(out, "0101010101010101", "e07c30d7e4e26e12", "0000000000000040");
        test(out, "0101010101010101", "0953e2258e8e90a1", "0000000000000020");
        test(out, "0101010101010101", "5b711bc4ceebf2ee", "0000000000000010");
        test(out, "0101010101010101", "cc083f1e6d9e85f6", "0000000000000008");
        test(out, "0101010101010101", "d2fd8867d50d2dfe", "0000000000000004");
        test(out, "0101010101010101", "06e7ea22ce92708f", "0000000000000002");
        test(out, "0101010101010101", "166b40b44aba4bd6", "0000000000000001");
        test(out, "8001010101010101", "0000000000000000", "95a8d72813daa94d");
        test(out, "4001010101010101", "0000000000000000", "0eec1487dd8c26d5");
        test(out, "2001010101010101", "0000000000000000", "7ad16ffb79c45926");
        test(out, "1001010101010101", "0000000000000000", "d3746294ca6a6cf3");
        test(out, "0801010101010101", "0000000000000000", "809f5f873c1fd761");
        test(out, "0401010101010101", "0000000000000000", "c02faffec989d1fc");
        test(out, "0201010101010101", "0000000000000000", "4615aa1d33e72f10");
        test(out, "0180010101010101", "0000000000000000", "2055123350c00858");
        test(out, "0140010101010101", "0000000000000000", "df3b99d6577397c8");
        test(out, "0120010101010101", "0000000000000000", "31fe17369b5288c9");
        test(out, "0110010101010101", "0000000000000000", "dfdd3cc64dae1642");
        test(out, "0108010101010101", "0000000000000000", "178c83ce2b399d94");
        test(out, "0104010101010101", "0000000000000000", "50f636324a9b7f80");
        test(out, "0102010101010101", "0000000000000000", "a8468ee3bc18f06d");
        test(out, "0101800101010101", "0000000000000000", "a2dc9e92fd3cde92");
        test(out, "0101400101010101", "0000000000000000", "cac09f797d031287");
        test(out, "0101200101010101", "0000000000000000", "90ba680b22aeb525");
        test(out, "0101100101010101", "0000000000000000", "ce7a24f350e280b6");
        test(out, "0101080101010101", "0000000000000000", "882bff0aa01a0b87");
        test(out, "0101040101010101", "0000000000000000", "25610288924511c2");
        test(out, "0101020101010101", "0000000000000000", "c71516c29c75d170");
        test(out, "0101018001010101", "0000000000000000", "5199c29a52c9f059");
        test(out, "0101014001010101", "0000000000000000", "c22f0a294a71f29f");
        test(out, "0101012001010101", "0000000000000000", "ee371483714c02ea");
        test(out, "0101011001010101", "0000000000000000", "a81fbd448f9e522f");
        test(out, "0101010801010101", "0000000000000000", "4f644c92e192dfed");
        test(out, "0101010401010101", "0000000000000000", "1afa9a66a6df92ae");
        test(out, "0101010201010101", "0000000000000000", "b3c1cc715cb879d8");
        test(out, "0101010180010101", "0000000000000000", "19d032e64ab0bd8b");
        test(out, "0101010140010101", "0000000000000000", "3cfaa7a7dc8720dc");
        test(out, "0101010120010101", "0000000000000000", "b7265f7f447ac6f3");
        test(out, "0101010110010101", "0000000000000000", "9db73b3c0d163f54");
        test(out, "0101010108010101", "0000000000000000", "8181b65babf4a975");
        test(out, "0101010104010101", "0000000000000000", "93c9b64042eaa240");
        test(out, "0101010102010101", "0000000000000000", "5570530829705592");
        test(out, "0101010101800101", "0000000000000000", "8638809e878787a0");
        test(out, "0101010101400101", "0000000000000000", "41b9a79af79ac208");
        test(out, "0101010101200101", "0000000000000000", "7a9be42f2009a892");
        test(out, "0101010101100101", "0000000000000000", "29038d56ba6d2745");
        test(out, "0101010101080101", "0000000000000000", "5495c6abf1e5df51");
        test(out, "0101010101040101", "0000000000000000", "ae13dbd561488933");
        test(out, "0101010101020101", "0000000000000000", "024d1ffa8904e389");
        test(out, "0101010101018001", "0000000000000000", "d1399712f99bf02e");
        test(out, "0101010101014001", "0000000000000000", "14c1d7c1cffec79e");
        test(out, "0101010101012001", "0000000000000000", "1de5279dae3bed6f");
        test(out, "0101010101011001", "0000000000000000", "e941a33f85501303");
        test(out, "0101010101010801", "0000000000000000", "da99dbbc9a03f379");
        test(out, "0101010101010401", "0000000000000000", "b7fc92f91d8e92e9");
        test(out, "0101010101010201", "0000000000000000", "ae8e5caa3ca04e85");
        test(out, "0101010101010180", "0000000000000000", "9cc62df43b6eed74");
        test(out, "0101010101010140", "0000000000000000", "d863dbb5c59a91a0");
        test(out, "0101010101010120", "0000000000000000", "a1ab2190545b91d7");
        test(out, "0101010101010110", "0000000000000000", "0875041e64c570f7");
        test(out, "0101010101010108", "0000000000000000", "5a594528bebef1cc");
        test(out, "0101010101010104", "0000000000000000", "fcdb3291de21f0c0");
        test(out, "0101010101010102", "0000000000000000", "869efd7f9f265a09");
        test(out, "1046913489980131", "0000000000000000", "88d55e54f54c97b4");
        test(out, "1007103489988020", "0000000000000000", "0c0cc00c83ea48fd");
        test(out, "10071034c8980120", "0000000000000000", "83bc8ef3a6570183");
        test(out, "1046103489988020", "0000000000000000", "df725dcad94ea2e9");
        test(out, "1086911519190101", "0000000000000000", "e652b53b550be8b0");
        test(out, "1086911519580101", "0000000000000000", "af527120c485cbb0");
        test(out, "5107b01519580101", "0000000000000000", "0f04ce393db926d5");
        test(out, "1007b01519190101", "0000000000000000", "c9f00ffc74079067");
        test(out, "3107915498080101", "0000000000000000", "7cfd82a593252b4e");
        test(out, "3107919498080101", "0000000000000000", "cb49a2f9e91363e3");
        test(out, "10079115b9080140", "0000000000000000", "00b588be70d23f56");
        test(out, "3107911598090140", "0000000000000000", "406a9a6ab43399ae");
        test(out, "1007d01589980101", "0000000000000000", "6cb773611dca9ada");
        test(out, "9107911589980101", "0000000000000000", "67fd21c17dbb5d70");
        test(out, "9107d01589190101", "0000000000000000", "9592cb4110430787");
        test(out, "1007d01598980120", "0000000000000000", "a6b7ff68a318ddd3");
        test(out, "1007940498190101", "0000000000000000", "4d102196c914ca16");
        test(out, "0107910491190401", "0000000000000000", "2dfa9f4573594965");
        test(out, "0107910491190101", "0000000000000000", "b46604816c0e0774");
        test(out, "0107940491190401", "0000000000000000", "6e7e6221a4f34e87");
        test(out, "19079210981a0101", "0000000000000000", "aa85e74643233199");
        test(out, "1007911998190801", "0000000000000000", "2e5a19db4d1962d6");
        test(out, "10079119981a0801", "0000000000000000", "23a866a809d30894");
        test(out, "1007921098190101", "0000000000000000", "d812d961f017d320");
        test(out, "100791159819010b", "0000000000000000", "055605816e58608f");
        test(out, "1004801598190101", "0000000000000000", "abd88e8b1b7716f1");
        test(out, "1004801598190102", "0000000000000000", "537ac95be69da1e1");
        test(out, "1004801598190108", "0000000000000000", "aed0f6ae3c25cdd8");
        test(out, "1002911598100104", "0000000000000000", "b3e35a5ee53e7b8d");
        test(out, "1002911598190104", "0000000000000000", "61c79c71921a2ef8");
        test(out, "1002911598100201", "0000000000000000", "e2f5728f0995013c");
        test(out, "1002911698100101", "0000000000000000", "1aeac39a61f0a464");
        test(out, "7ca110454a1a6e57", "01a1d6d039776742", "690f5b0d9a26939b");
        test(out, "0131d9619dc1376e", "5cd54ca83def57da", "7a389d10354bd271");
        test(out, "07a1133e4a0b2686", "0248d43806f67172", "868ebb51cab4599a");
        test(out, "3849674c2602319e", "51454b582ddf440a", "7178876e01f19b2a");
        test(out, "04b915ba43feb5b6", "42fd443059577fa2", "af37fb421f8c4095");
        test(out, "0113b970fd34f2ce", "059b5e0851cf143a", "86a560f10ec6d85b");
        test(out, "0170f175468fb5e6", "0756d8e0774761d2", "0cd3da020021dc09");
        test(out, "43297fad38e373fe", "762514b829bf486a", "ea676b2cb7db2b7a");
        test(out, "07a7137045da2a16", "3bdd119049372802", "dfd64a815caf1a0f");
        test(out, "04689104c2fd3b2f", "26955f6835af609a", "5c513c9c4886c088");
        test(out, "37d06bb516cb7546", "164d5e404f275232", "0a2aeeae3ff4ab77");
        test(out, "1f08260d1ac2465e", "6b056e18759f5cca", "ef1bf03e5dfa575a");
        test(out, "584023641aba6176", "004bd6ef09176062", "88bf0db6d70dee56");
        test(out, "025816164629b007", "480d39006ee762f2", "a1f9915541020b56");
        test(out, "49793ebc79b3258f", "437540c8698f3cfa", "6fbf1cafcffd0556");
        test(out, "4fb05e1515ab73a7", "072d43a077075292", "2f22e49bab7ca1ac");
        test(out, "49e95d6d4ca229bf", "02fe55778117f12a", "5a6b612cc26cce4a");
        test(out, "018310dc409b26d6", "1d9d5c5018f728c2", "5f4c038ed12b2e41");
        test(out, "1c587f1c13924fef", "305532286d6f295a", "63fac0d034d9f793");
        time_test(out, "1c587f1c13924fef", "305532286d6f295a", "63fac0d034d9f793");
}

private static void
test( PrintStream out, String keyStr, String plainStr, String cipherStr )
{
        byte key[] = fromString( keyStr );
        byte plain[] = fromString( plainStr );
        byte cipher[] = fromString( cipherStr );
        DES des = new DES( key );
        byte encP[] = new byte[plain.length];
        byte decC[] = new byte[plain.length];
        des.encrypt( plain, encP );
        String a,b;
        out.println( "plain:" + toString( plain ) + " enc:" + ( a = toString( encP ) ) + " calc:" + ( b = toString( cipher) ) );
        if ( a.equals( b ) )
                out.println( "encryption good" );
        else
                out.println( " ********* DES ENC FAILED ********* " );
        des.decrypt( encP, decC );
        out.println( "  enc:" + toString( encP ) + " dec:" + ( a = toString( decC ) ) + " calc:" + ( b = toString( plain ) ) );
        if ( a.equals( b ) )
                out.println( "decryption good" );
        else
                out.println( " ********* DES DEC FAILED ********* " );

}

private static void
time_test( PrintStream out, String keyStr, String plainStr, String cipherStr )
{
        byte key[] = fromString( keyStr );
        byte plain[] = fromString( plainStr );
        byte cipher[] = fromString( cipherStr );
        DES des = new DES( key );
        out.println();
        out.println();
        out.println("Starting time tests ...");
        out.println();
        out.println("Calculating the approximate speed ...");
        out.println();

        int count=10;
        long begin;
        long lr = 8759384593857;        // Something at random
        double d;
        do      {
                des = new DES(key);
                long i;
                count *= 2;
                begin = System.currentTimeMillis();
                for (i=count; i!=0; i--)
                {
                        des.encrypt_base((int)(lr >> 32),(int)lr);
                        des.encrypt_base((int)(lr >> 32),(int)lr);
                }
                d = System.currentTimeMillis() - begin;
        } while (d < 3000);


        count *= 3;
        out.println("Doing "+count+" encryptions");

        out.println("Calculating the real speed ...");
        out.println();

        begin = System.currentTimeMillis();
        for (int i=count; i!=0; i--)
        {
                des = new DES(key);
                des.encrypt_base((int)(lr >> 32),(int)lr);
                des.encrypt_base((int)(lr >> 32),(int)lr);
        }
        d = (System.currentTimeMillis() - begin)/1000;
        out.println(count+" encryptions in "+d+" second");

        d = ((double)(count*2)/d);
        out.println("DES encryptions per sec = "+d);
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

} /* class DES */

/* The DES cipher (in ECB mode).
 *
 * Parts of this code Copyright (C) 1996 Geoffrey Keating. All rights reserved.
 *
 * Its use is FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are adhered to.
 * 
 * Copyright remains Geoffrey Keating's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Geoffrey Keating should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY GEOFFREY KEATING ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 */
/* Parts of this code (in particular, the string representing SPtrans below)
 * are Copyright (C) 1995 Eric Young (eay@mincom.oz.au). All rights reserved.
 * Its use is 
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are adhered to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
