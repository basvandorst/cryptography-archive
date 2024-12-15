/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * This class implements the Blowfish block cipher.
 * However, it relies on linked in libraries which are not currently
 * present.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * Blowfish was written by Bruce Schneier.
 * See
 *      <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *      "Chapter 14.3 Blowfish,"
 *      <cite>Applied Cryptography</cite>,
 *      Wiley 2nd Ed, 1996,
 * and
 *      <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *      "Description of a New Variable-Length Key, 64-Bit Cipher (Blowfish),"
 *      <cite>Fast Software Encryption Cambridge Security Workshop Proceedings</cite>,
 *      Springer-Verlag, 1004, pp 191-204.
 *
 * <P>
 * The algorithm is in the public domain.
 *
 * <P>
 * <b>Copyright</b> &#169 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <p>
 * @author Systemics Ltd
 * @see BlockCipher.java
 */
public final class Blowfish extends BlockCipher
{
        private static final int REQUIRED_LIB_MAJOR_VER = 1;
        private static final int REQUIRED_LIB_MINOR_VER = 11;
        
        private static final String LIBRARY_NAME = "blowfish";

        private static boolean native_link_ok = false;
        private static boolean native_lib_loaded = false;
        private static String native_link_err = "Class never loaded";

        static 
        {
                // load the DLL or shared library that contains the native code
                // implementation of the Blowfish block cipher.
                try
                {
                        System.loadLibrary( LIBRARY_NAME );
                        native_lib_loaded = true;
                        try
                        {
                                if ( ( getLibMajorVersion() != REQUIRED_LIB_MAJOR_VER ) || 
                                        ( getLibMinorVersion() < REQUIRED_LIB_MINOR_VER ) )
                                {
                                        native_link_err = "The " + LIBRARY_NAME + " native library " + 
                                                                                getLibMajorVersion() + "." + getLibMinorVersion() +
                                                                                " is too old to use. Version " + 
                                                                                REQUIRED_LIB_MAJOR_VER + "." + REQUIRED_LIB_MINOR_VER + 
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
                                native_link_err = "The " + LIBRARY_NAME + " native library is too old to use. Version " + 
                                                                        REQUIRED_LIB_MAJOR_VER + "." + REQUIRED_LIB_MINOR_VER + 
                                                                        " required.";
                        }
                }
                catch ( UnsatisfiedLinkError ule )
                {
                        native_link_err = "The " + LIBRARY_NAME + " native library was not found.";
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
         * This is the length of a block.
         */
        public static final int BLOCK_LENGTH = 8;
        
        /**
         * This is the length of the internal buffer for the native code.
         */
        private static final int INTERNAL_KEY_LENGTH = 4240;

        /**
         * This is the internal buffer with the expanded encrypt key.
         */
        private byte ks[] = null;
 
        private int keyLength;

        /**
         * This creates a Blowfish block cipher from a byte array of a variable length.
         * @param userKey    the user key.
         * @throws UnsatisfiedLinkError if the library is not of the correct version
         */
        public Blowfish( byte userKey[] ) 
        {
                if ( !native_link_ok )
                        throw new UnsatisfiedLinkError( native_link_err );

                ks = new byte[INTERNAL_KEY_LENGTH];
//              if ( userKey.length < KEY_LEN_MIN || userKey.length < KEY_LEN_MAX )
//              throw new CryptoError( "Blowfish: User key length wrong" );
                if ( set_ks( userKey ) != 0 )
                {
                        int i;
                        for( i = 0; i < INTERNAL_KEY_LENGTH; i++ )
                                if ( ks[i] == 0 )
                                        break;
                        throw new CryptoError( "Blowfish: key expansion Error - " + new String( ks, i ) );
                }
                keyLength = userKey.length;
        }

        /**
         * Encrypt a block.
         * The in and out buffers can be the same.
         * @param in The data to be encrypted.
         * @param in_offset   The start of data within the in buffer.
         * @param out The encrypted data.
         * @param out_offset  The start of data within the out buffer.
         */
        protected void 
        blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
            if ( ks == null )
                throw new CryptoError( "Blowfish: User key not set" );

            do_blowfish( in, in_offset, out, out_offset, true );
        }

        /**
         * Decrypt a block.
         * The in and out buffers can be the same.
         * @param in The data to be decrypted.
         * @param in_offset   The start of data within the in buffer.
         * @param out The decrypted data.
         * @param out_offset  The start of data within the out buffer.
         */
        protected void
        blockDecrypt( byte in[], int in_offset, byte out[], int out_offset )
        {
            if ( ks == null )
                throw new CryptoError( "Blowfish: User key not set." );

            do_blowfish( in, in_offset, out, out_offset, false );
        }

        /**
         * Return the block length of this cipher.
         * @returns the block length of this cipher.
         */
        public int 
        blockLength()
        {
            return BLOCK_LENGTH;
        }

        /**
         * Return the key length for this cipher.
         * @returns the length of the key used when constructing this cipher.
         */
        public int 
        keyLength()
        {
            return keyLength;
        }

        // The native functions that get the library version.

        private native static int getLibMajorVersion();
        private native static int getLibMinorVersion();

        // these are the native functions that implement the Blowfish algorithm.

        /**
         * This function expands the user key to an internal form.
         */
        private native int set_ks( byte userKey[] );

        /**
         * Encrypt/decrypt the block (depending upon encrypt flag)
         */
        private native void do_blowfish( byte in[], int in_offset, byte out[], int out_offset, boolean encrypt );
}

