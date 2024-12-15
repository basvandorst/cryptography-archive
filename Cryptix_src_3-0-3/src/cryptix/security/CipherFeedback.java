// $Id: CipherFeedback.java,v 1.3 1997/12/07 06:40:26 hopwood Exp $
//
// $Log: CipherFeedback.java,v $
// Revision 1.3  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 0.1.0  1997/12/07  hopwood
// + Restored from 2.2 release (and made deprecated).
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * Implements a byte-based stream cipher using n-bit CFB with an
 * n-bit-sized block cipher.
 * <p>
 * The full block size of the supplied cipher is used for the
 * Cipher Feedback Mode. The bytes supplied are processed and returned
 * immediately.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 * @see cryptix.provider.mode.CFB
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
     * @param cipher0   the block cipher to use
     * @param iv0       the initial value for the shift register (IV)
     */
    public CipherFeedback(BlockCipher cipher0, byte iv0[])
    {
        length = cipher0.blockLength();
        cipher = cipher0;
        xorBlock = new byte [length];

        reset(iv0);

        // moved to reset() for 2.2.
        // ivBlock = iv0;
        // currentByte = 0;
        // cipher.encrypt(ivBlock, xorBlock);
    }

    public int keyLength() { return cipher.keyLength(); }

    /**
     * Reset the stream feedback register with a new unique IV
     * and sets position in stream back to zero.
     *
     * @param iv0   the initial value for the shift register (IV)
     */
    public void reset(byte iv0[])
    {
        ivBlock = iv0;
        currentByte = 0;
        cipher.encrypt(ivBlock, xorBlock);
    }

    /**
     * This function encrypts a block of data.
     * The contents of the array in will not be changed,
     * but will instead store the result in the array out.
     * The arrays can, however, be the same.
     *
     * @param in            the plain text to be encrypted.
     * @param in_offset     the offset within the in buffer.
     * @param out           where the encrypted cipher text will be stored.
     * @param out_offset    the offset within the out buffer.
     * @param length        the length to encrypt.
     */
    public void 
    encrypt(byte in[], int in_offset, byte out[], int out_offset, int length)
    {
        for (int i = 0; i < length; i++)
            out[i + out_offset] = encryptByte(in[i + in_offset]);
    }

    /**
     * This function decrypts a block of data.
     * The contents of the array in will not be changed,
     * but will instead store the result in the array out.
     * The arrays can, however, be the same.
     *
     * @param in            the cipher text to be decrypted.
     * @param in_offset     the offset within the in buffer.
     * @param out           where the decrypted plain text will be stored.
     * @param out_offset    the offset within the out buffer.
     * @param length        the length to decrypt.
     */
    public void
    decrypt(byte in[], int in_offset, byte out[], int out_offset, int length)
    {
        for (int i = 0; i < length; i++)
            out[i + out_offset] = decryptByte(in[i + in_offset]);
    }

    private byte
    encryptByte(byte b)
    {
        if (currentByte >= length)
        {
            currentByte = 0;
            cipher.encrypt(ivBlock, xorBlock);
        }
        b ^= xorBlock[currentByte];
        ivBlock[currentByte] = b;
        currentByte++;
        return b;
    }
    
    private byte
    decryptByte(byte b)
    {
        if (currentByte >= length)
        {
            currentByte = 0;
            cipher.encrypt(ivBlock, xorBlock);
        }
        ivBlock[currentByte] = b;
        b ^= xorBlock[currentByte];
        currentByte++;
        return b;
    }
}

