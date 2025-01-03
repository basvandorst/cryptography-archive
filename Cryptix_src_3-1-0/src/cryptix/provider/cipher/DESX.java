// $Id: DESX.java,v 1.2 1998/05/31 14:00:24 kettler Exp $
//
// $Log: DESX.java,v $
// Revision 1.2  1998/05/31 14:00:24  kettler
// + DESX now used two indepent XOR keys
//
// Revision 1.1  1998/05/27 14:05:53  kettler
// + Added DESX another variant of using DES with 56+64 bit keys
//
//
// $Endlog$
/*
 * Copyright (c) 1998 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.provider.cipher;

import cryptix.util.core.Debug;
import cryptix.CryptixException;
import cryptix.provider.key.RawSecretKey;

import java.io.PrintWriter;
import java.security.Cipher;
import java.security.Key;
import java.security.KeyException;
import java.security.InvalidKeyException;
import java.security.SymmetricCipher;

/**
 * This class implements DESX encryption with two independent
 * keys. DESX acts as a block cipher with an 8 byte block size.
 * <p>
 * The encoded form of the DESX key should be a 24-byte array,
 * consisting of one 8-byte DES key K and two XOR keys K1 and K2 
 * in order - K, K1 and K2.
 * Encryption and decryption are done as follows:
 * <ul>
 *   <li> C = E(P XOR K1) XOR K2
 *   <li> P = D(C XOR K2) XOR K1
 * </ul>
 * <p>
 * When DESX is used with the CBC mode class (algorithm name
 * "DESX/CBC"), the result is Outer-CBC, and only one IV is used.
 * <p>
 * <b>References:</b>
 * <ol>
 *    <li> <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *         "Chapter 12 Data Encryption Standard," and
 *         "Section 15.2 Triple Encryption,"
 *         <cite>Applied Cryptography, 2nd edition</cite>,
 *         John Wiley &amp; Sons, 1996
 *         <p>
 *    <li> R.C. Merkle and M. Hellman,
 *         "On the Security of Multiple Encryption,"
 *         <cite>Communications of the ACM</cite>,
 *         vol. 24 no. 7, 1981, pages 465-467.
 *         <p>
 *    <li> P Karn, P Metzger, W A Simpson
 *         "The ESP Triple DES Transform,"
 *         <cite>Internet Draft</cite>,
 *         draft-simpson-esp-des3-x-01.txt
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Sascha Kettler
 * @since   Cryptix 3.0.4
 */
public class DESX
extends Cipher
implements SymmetricCipher
{

// Debugging methods and vars.
//...........................................................................

    private static final boolean DEBUG = Debug.GLOBAL_DEBUG;
    private static final boolean DEBUG_SLOW = Debug.GLOBAL_DEBUG_SLOW;
    private static final int debuglevel = DEBUG ? Debug.getLevel("DESX") : 0;
    private static final PrintWriter err = DEBUG ? Debug.getOutput() : null;
    private static void debug(String s) { err.println("DESX: " + s); }


// DESX constants and variables
//............................................................................

    private static final int
        BLOCK_SIZE = 8,
        KEY_LENGTH = 24,
        SINGLE_KEY_LENGTH = 8;

    private Cipher des;

    private byte[] XORkey1;
    private byte[] XORkey2;


// Constructor
//............................................................................

    /**
     * Constructs a DESX cipher object, in the UNINITIALIZED state.
     * This calls the Cipher constructor with <i>implBuffering</i> false,
     * <i>implPadding</i> false and the provider set to "Cryptix".
     */
    public DESX() {
        super(false, false, "Cryptix");
        des = new DES();
	XORkey1 = null;
        XORkey2 = null;
    }


// Implementation of JCE methods
//............................................................................
    
    /**
     * <b>SPI</b>: Returns the length of an input block, in bytes.
     *
     * @return the length in bytes of an input block for this cipher.
     */
    public int engineBlockSize () { return BLOCK_SIZE; }

    /**
     * <b>SPI</b>: Initializes this cipher for encryption, using the
     * specified key.
     *
     * @param  key  the key to use for encryption.
     * @exception KeyException if one of the following occurs: <ul>
     *                <li> key.getEncoded() == null;
     *                <li> The length of the user key array is invalid.
     *              </ul>
     */
    public void engineInitEncrypt (Key key)
    throws KeyException {
        byte[][] keys = splitKey(key);
	RawSecretKey DESkey = new RawSecretKey("DES", keys[0]);

        des.initEncrypt(DESkey);

        XORkey1 = keys[1];
        XORkey2 = keys[2];
    }

    /**
     * <b>SPI</b>: Initializes this cipher for decryption, using the
     * specified key.
     *
     * @param  key  the key to use for decryption.
     * @exception KeyException if one of the following occurs: <ul>
     *                <li> key.getEncoded() == null;
     *                <li> The length of the user key array is invalid.
     *              </ul>
     */
    protected void engineInitDecrypt (Key key)
    throws KeyException {
        byte[][] keys = splitKey(key);
	RawSecretKey DESkey = new RawSecretKey("DES", keys[0]);

        des.initDecrypt(DESkey);

        XORkey1 = keys[2];
        XORkey2 = keys[1];
    }


    /**
     * <b>SPI</b>: This is the main engine method for updating data.
     * <p>
     * <i>in</i> and <i>out</i> may be the same array, and the input and output
     * regions may overlap.
     *
     * @param  in           the input data.
     * @param  inOffset     the offset into in specifying where the data starts.
     * @param  inLen        the length of the subarray.
     * @param  out          the output array.
     * @param  outOffset    the offset indicating where to start writing into
     *                      the out array.
     * @return the number of bytes written.
     * @exception CryptixException if the native library is being used, and it
     *                      reports an error.
     */
    protected int
    engineUpdate (byte[] in, int inOffset, int inLen, byte[] out, int outOffset) {
        int i;

        byte[] temp = new byte[inLen];

	for (i=0; i<inLen; i++) {
	  temp[i] = (byte) (in[i+inOffset] ^ XORkey2[i % SINGLE_KEY_LENGTH]);
	}

	des.update(temp, 0, inLen, temp, 0);

	for (i=0; i<inLen; i++) {
	  out[i+outOffset] = (byte) (temp[i] ^ XORkey1[i % SINGLE_KEY_LENGTH]);
	}

        return inLen;
    }


// Own methods
//............................................................................

    /**
     * Splits a Triple DES key into three single DES keys.
     *
     * @param  key  the key to be split.
     * @return a 3-element array containing the DES key and the two XOR keys,
     *              in the order
     *              in which they were encoded.
     * @exception InvalidKeyException if one of the following occurs: <ul>
     *                <li> key.getEncoded() == null;
     *                <li> The length of the user key array is invalid.
     *              </ul>
     */
    private byte[][] splitKey(Key key) throws InvalidKeyException {
        byte[] userkey = key.getEncoded();
        if (userkey == null)
            throw new InvalidKeyException(getAlgorithm() + ": Null user key");

        if (userkey.length != KEY_LENGTH)
            throw new InvalidKeyException(getAlgorithm() + ": Invalid user key length");

        byte[][] keys = new byte[3][SINGLE_KEY_LENGTH];
        for (int i = 0; i < 3; i++)
            System.arraycopy (userkey, i*SINGLE_KEY_LENGTH,
			      keys[i], 0, SINGLE_KEY_LENGTH);

        return keys;
    }
}
