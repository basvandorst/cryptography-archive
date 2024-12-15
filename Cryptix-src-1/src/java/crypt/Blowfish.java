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
 * This class implements the Blowfish block cipher.
 *
 * <p>The native code Blowfish algoryth was developed by Bruce Schneier
 * <p>The native code Blowfish implementation used by the class was
 * developed by Bryan Olson from Bruce Schneier's code.
 * The C source file is Copyright (C) 1995, A.M. Kuchling.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 28/06/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class Blowfish extends BlockCipher
{
    /**
     * This is the length of a block.
     */
    public static final int BLOCK_LENGTH = 8;
    
    /**
     * this loads the dll with the native code.
     */
    static
    {
        System.loadLibrary( "blowfish" );
    }

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
     */
    public Blowfish( byte userKey[] ) 
    {
        ks = new byte[INTERNAL_KEY_LENGTH];
//       if ( userKey.length < KEY_LEN_MIN || userKey.length < KEY_LEN_MAX )
//           throw new CryptoError( "Blowfish: User key length wrong" );
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
