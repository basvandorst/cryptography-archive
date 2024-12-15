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
 * This class implements the DES block cipher.
 *
 * <p>The native code DES implementation used by the class was
 * developed by, and is owned by, Eric Young <eay@mincom.oz.au>
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 14/12/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class DES extends BlockCipher
{
    /**
     * This is the length of a block.
     */
    public static final int BLOCK_LENGTH = 8;

    /**
     * This is the length of a the user key.
     */
    public static final int KEY_LENGTH = 8;

    
    /**
     * this loads the dll with the native code.
     */
    static
    {
        System.loadLibrary( "des" );
    }

    /**
     * This is the length of the internal buffer for the native code.
     */
    private static final int INTERNAL_KEY_LENGTH = 128;

    /**
     * This is the internal buffer with the expanded encrypt key.
     */
    private byte ks[] = null;
 
    /**
     * This creates a DES block cipher from a byte array of the correct length.
     * @param userKey    the user key.
     */
    public DES( byte userKey[] ) 
    {
        ks = new byte[INTERNAL_KEY_LENGTH];
        if ( userKey.length != KEY_LENGTH )
            throw new CryptoError( "DES: User key length wrong" );

        set_ks( userKey );
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
            throw new CryptoError( "DES: User key not set" );

        do_des( in, in_offset, out, out_offset, true );
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
            throw new CryptoError( "DES: User key not set." );

        do_des( in, in_offset, out, out_offset, false );
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
     * @returns the key length of this cipher.
     */
    public int 
    keyLength()
    {
        return KEY_LENGTH;
    }

    // these are the native functions that implement the DES algorithm.
    /**
     * This function expands the user key to an internal form.
     */
    private native void set_ks( byte userKey[] );

    /**
     * Encrypt/decrypt the block (depending upon encrypt flag)
     */
    private native void do_des( byte in[], int in_offset, byte out[], int out_offset, boolean encrypt );
}
