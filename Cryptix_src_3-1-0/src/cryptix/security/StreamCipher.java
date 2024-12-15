// $Id: StreamCipher.java,v 1.2 1997/12/07 06:40:26 hopwood Exp $
//
// $Log: StreamCipher.java,v $
// Revision 1.2  1997/12/07 06:40:26  hopwood
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
 * This abstract class is the basis for a stream cipher of any form.
 * <p>
 * StreamCipher objects support only the Cryptix v2.2 API.
 * <p>
 * Note: compatibility with Cryptix 2.2 applies only to clients of this
 * class. Subclasses must be rewritten as JCE algorithms.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 * @see java.security.Cipher
 */
public abstract class StreamCipher
{
    /**
     * This function encrypts a block of data.
     * The contents of the array will be changed.
     *
     * @param text  the plaintext to be encrypted.
     */
    public final void encrypt(byte text[])
    {
        encrypt(text, 0, text, 0, text.length);
    }

    /**
     * This function decrypts a block of data.
     * The contents of the array will be changed.
     *
     * @param text  the ciphertext to be decrypted.
     */
    public final void decrypt(byte text[])
    {
        decrypt(text, 0, text, 0, text.length);
    }

    /**
     * This function encrypts a block of data.
     * The contents of the array in remain unchanged,
     * and the result is stored in the array out.
     * However, array in and array out can be the same.
     *
     * @param in    the plaintext to be encrypted.
     * @param out   where the ciphertext will be stored.
     */
    public final void encrypt(byte in[], byte out[])
    {
        if (in.length > out.length)
            throw new CryptoError(getClass().getName() +
                    ": encrypt output buffer too small");
        encrypt(in, 0, out, 0, in.length);
    }

    /**
     * This function decrypts a block of data.
     * The contents of the array in remain unchanged,
     * and the result is stored in the array out.
     * However, array in and array out can be the same.
     *
     * @param in    the ciphertext to be decrypted.
     * @param out   where the plaintext will be stored.
     */
    public final void decrypt(byte in[], byte out[])
    {
        if (in.length != out.length)
            throw new CryptoError(getClass().getName() +
                ": decrypt output buffer too small");
        decrypt(in, 0, out, 0, in.length);
    }

    /**
     * This function encrypts a block of data.
     * The contents of the array in remain unchanged,
     * and the result is stored in the array out.
     * However, array in and array out can be the same.
     *
     * @param in            the plaintext to be encrypted.
     * @param in_offset     the start of data within the in buffer.
     * @param out           where the ciphertext will be stored.
     * @param off_offset    the start of data within the out buffer.
     * @param length        the length to encrypt.
     */
    public abstract void encrypt(byte in[], int in_offset, byte out[],
                                 int out_offset, int length);

    /**
     * This function decrypts a block of data.
     * The contents of the array in remain unchanged,
     * and the result is stored in the array out.
     * However, array in and array out can be the same.
     *
     * @param in            the ciphertext to be decrypted.
     * @param in_offset     the start of data within the in buffer.
     * @param out           where the plaintext will be stored.
     * @param off_offset    the start of data within the out buffer.
     * @param length        the length to decrypt.
     */
    public abstract void decrypt(byte in[], int in_offset, byte out[],
                                 int out_offset, int length);

    /**
     * This function returns the length of the key for this cipher.
     *
     * @return the length (in bytes) of the key used with this cipher
     */
    public abstract int keyLength();
}
