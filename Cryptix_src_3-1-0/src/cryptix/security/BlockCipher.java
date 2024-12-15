// $Id: BlockCipher.java,v 1.4 1997/12/07 06:40:26 hopwood Exp $
//
// $Log: BlockCipher.java,v $
// Revision 1.4  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.3.1  1997/12/04  hopwood
// + Formatting changes.
//
// Revision 1.3  1997/12/01 00:56:54  iang
// + Minor typos.
// + Copied in tests from other archive, still one bug left in SPEED.java
//
// Revision 1.2  1997/11/28 03:53:50  iang
// + Dropped bad import, name change (BlockCipher).
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.3.0.0  1997/08/14  David Hopwood
// + Instead of having cipher implementations extend this class, we now
//   keep a reference to a pure JCA Cipher. This removes the dependency
//   of the JCA code on this deprecated class.
//
// Revision 0.2.5.7  1997/08/06  David Hopwood
// + Added the state variable. It should be declared in this class
//   because it is needed in order to support the encrypt/decrypt
//   methods, and because Cipher's state variable is private, not
//   protected, in JavaSoft's implementation of the JCE.
//
// Revision 0.2.5.6  1997/07/31  David Hopwood
// + Made keyLength() public, since it was public in V2.2.
//
// Revision 0.2.5.5  1997/07/12  R. Naffah
// + Replaced the abstract qualifier in keyLength() method with
//   protected making it return as a default a -1 value. New
//   algorithm implementations can thus inherit this superclass
//   without worrying about this deprecated/unsupported method.
// + Moved the Blowfish encrypt([B,I,[B,I) and decrypt([B,I,[B,I)
//   methods to here; made them protected to allow override by
//   implementations that need to do something different; i.e. IDEA.
//
// Revision 0.2.5.4  1997/07/05  David Hopwood
// + Merged Raif's version (based on 2.2.0a) with my version.
// + Superclass CryptixCipher removed and merged into this class
//   (and StreamCipher).
// + Removed obsolete documentation.
// + This class and its methods are now deprecated.
//
// Revision 0.2.5.3  1997/07/04  R. Naffah
// + Removed final modifier from both encrypt([B,I,[B,I) and
//   decrypt([B,I,[B,I) so they can be more efficiently coded
//   in the cipher implementation.
//
// Revision 0.2.5.2  1997/04/02  Systemics
// + Wrote documentation.  Fixed apparent flaw in offsets (hopefully?).
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Systemics
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995-97 Systemics Ltd
 * on behalf of the Cryptix development team.  All rights reserved.
 */

package cryptix.security;

import java.security.Cipher;
import java.security.SecretKey;

import cryptix.provider.key.RawSecretKey;

/**
 * This is an abstract superclass for ciphers that encrypt and decrypt a
 * fixed length block with a secret key.
 * <p>
 * BlockCipher objects support only the Cryptix v2.2 API.
 * <p>
 * Note: compatibility with Cryptix 2.2 applies only to clients of this
 * class. Subclasses must be rewritten as JCE algorithms.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.4 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see java.security.Cipher
 */
public abstract class BlockCipher
{

// Variables
//...........................................................................

    /**
     * The length of the user key for this instance.
     */
    private int userKeyLength;

    /**
     * A SecretKey object corresponding to the user key for this instance.
     */
    private RawSecretKey key;

    /**
     * The JCE cipher that is delegated to.
     */
    protected Cipher cipher;

    /**
     * The algorithm's block size in bytes.
     */
    protected int blockSize;


// Constructor
//...........................................................................

    /**
     * Constructs a V2.2 block cipher from a JCE cipher.
     *
     * @param  cipher   the java.security.Cipher object.
     */
    protected BlockCipher(Cipher cipher, byte[] userKey) {
        this.cipher = cipher;
        userKeyLength = userKey.length;
        key = new RawSecretKey(cipher.getAlgorithm(), userKey);
        blockSize = cipher.blockSize();
    }


// Own methods
//...........................................................................

    /**
     * Returns the block size of this cipher.
     *
     * @return the block size (in bytes) of this cipher.
     * @deprecated Use <code>blockSize()</code>,
     *             <code>getPlaintextBlockSize()</code> or
     *             <code>getCiphertextBlockSize()</code> instead.
     */
    public final int blockLength() { return cipher.blockSize(); }

    /**
     * Returns the key length for this cipher.
     * <p>
     * The JCE equivalent is to call <code>getEncoded()</code> on the Key
     * object, and find the length of the resulting byte array. It is not
     * possible to directly get the key object that has been used to
     * initialize a Cipher.
     *
     * @return the key length (in bytes) of this cipher.
     * @deprecated
     */
    public final int keyLength() { return userKeyLength; }

    /**
     * Encrypts a block of data in place.
     * The plaintext in <i>buf</i> is encrypted and written back as
     * ciphertext. <code>buf.length</code> must be equal to the block size.
     *
     * @param  buf  the buffer holding the data
     * @exception CryptoError if the buffer was the wrong length
     * @deprecated Use <code>initEncrypt(key)</code> and
     *              <code>update(buf, 0, buf.length, text, 0)</code> instead.
     */
    public final void encrypt(byte[] buf) { encrypt(buf, buf); }

    /**
     * Encrypts a block of data.
     * The plaintext in <i>in</i> is encrypted and the ciphertext is written
     * into <i>out</i>.
     * Note that <i>in</i> and <i>out</i> can be the same array.
     * <code>in.length</code> and <code>out.length</code> must be equal to the
     * block size.
     *
     * @param  in   the plaintext to be encrypted
     * @param  out  the ciphertext result of the encryption
     * @exception CryptoError if a buffer was the wrong length
     * @deprecated Use <code>initEncrypt(key)</code> and
     *              <code>update(in, 0, in.length, out, 0)</code> instead.
     */
    public final void encrypt(byte[] in, byte[] out) {
        if (in.length != blockSize || out.length != blockSize)
            throw new CryptoError(cipher.getAlgorithm() +
                ": Encrypt buffer length is incorrect");

        encrypt(in, 0, out, 0);
    }

    /**
     * Encrypts a block of data within an array.
     * The plaintext in <code>in[inOffset..inOffset+blockSize-1]</code>
     * is encrypted, and the resulting ciphertext is written to
     * <code>out[outOffset..outOffset+blockSize-1]</code>.
     * Note that there must be sufficient bytes left in each array.
     * <p>
     * The <i>in</i> and <i>out</i> arrays can be the same, in which
     * case the behaviour is the same as if the input array was copied
     * before any output is written.
     *
     * @param  in           buffer holding the plaintext to be encrypted
     * @param  inOffset     the start of plaintext within <code>in</code>
     * @param  out          buffer to write the encrypted ciphertext to
     * @param  outOffset    the start of ciphertext within <code>out</code>
     * @exception ArrayIndexOutOfBoundsException if an offset was invalid.
     * @deprecated Use <code>initEncrypt(key)</code> and <code>update(in, inOffset,
     *                      blockSize, out, outOffset)</code> instead.
     */
    public void encrypt(byte[] in, int inOffset, byte[] out, int outOffset) {
        if (inOffset < 0 || outOffset < 0)
            throw new ArrayIndexOutOfBoundsException(
                cipher.getAlgorithm() + ": Negative offset not allowed");
        if ((long)inOffset + blockSize > in.length ||
            (long)outOffset + blockSize > out.length)
            throw new ArrayIndexOutOfBoundsException(
                cipher.getAlgorithm() + ": Offset past end of array");

        try {
            if (cipher.getState() != Cipher.ENCRYPT)
                cipher.initEncrypt(key);

            cipher.update(in, inOffset, blockSize, out, outOffset);
        } catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Decrypts an array of data in place.
     * The ciphertext in <i>buf</i> is decrypted and written back as
     * plaintext. <code>buf.length</code> must be equal to the block size.
     *
     * @param  buf  the buffer holding the data
     * @exception CryptoError if the buffer was the wrong length
     * @deprecated Use <code>initDecrypt(key)</code> and
     *              <code>update(buf, 0, buf.length, text, 0)</code> instead.
     */
    public final void decrypt(byte[] text) { decrypt(text, text); }

    /**
     * Decrypts a block of data.
     * The ciphertext in <i>in</i> is decrypted and the plaintext is written
     * into <i>out</i>.
     * Note that <i>in</i> and <i>out</i> can be the same array.
     * <code>in.length</code> and <code>out.length</code> must be equal to the
     * block size.
     *
     * @param  in   the plaintext to be encrypted
     * @param  out  the ciphertext result of the encryption
     * @exception CryptoError if a buffer was the wrong length
     * @deprecated Use <code>initDecrypt(key)</code> and
     *              <code>update(in, 0, in.length, out, 0)</code> instead.
     */
    public final void decrypt(byte[] in, byte[] out) {
        if (in.length != blockSize || out.length != blockSize)
            throw new CryptoError(cipher.getAlgorithm() +
                ": Decrypt buffer length is incorrect");

        decrypt(in, 0, out, 0);
    }

    /**
     * Decrypts a block of data within an array.
     * The ciphertext in <code>in[inOffset..inOffset+blockSize-1]</code>
     * is decrypted, and the resulting plaintext is written to
     * <code>out[outOffset..outOffset+blockSize-1]</code>.
     * Note that there must be sufficient bytes left in each array.
     * <p>
     * The <i>in</i> and <i>out</i> arrays can be the same, in which
     * case the behaviour is the same as if the input array was copied
     * before any output is written.
     *
     * @param  in           buffer holding the ciphertext to be decrypted
     * @param  inOffset     the start of ciphertext within <code>in</code>
     * @param  out          buffer to write the decrypted plaintext to
     * @param  outOffset    the start of plaintext within <code>out</code>
     * @exception ArrayIndexOutOfBoundsException if an offset was invalid.
     * @deprecated Use <code>initDecrypt(key)</code> and <code>update(in, inOffset,
     *                      blockSize, out, outOffset)</code> instead.
     */
    public void decrypt(byte[] in, int inOffset, byte[] out, int outOffset) {
        if (inOffset < 0 || outOffset < 0)
            throw new ArrayIndexOutOfBoundsException(
                cipher.getAlgorithm() + ": Negative offset not allowed");
        if ((long)inOffset + blockSize > in.length ||
            (long)outOffset + blockSize > out.length)
            throw new ArrayIndexOutOfBoundsException(
                cipher.getAlgorithm() + ": Offset past end of array");

        try {
            if (cipher.getState() != Cipher.DECRYPT)
                cipher.initDecrypt(key);

            cipher.update(in, inOffset, blockSize, out, outOffset);
        } catch (Exception e) { throw new CryptoError(e.toString()); }
    }
}
