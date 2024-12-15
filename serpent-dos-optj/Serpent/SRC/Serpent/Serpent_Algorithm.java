// $Id: $
//
// $Log: $
// Revision 1.1.1  1998/04/10  raif
// + added code to generate Intermediate Values KAT.
// + cosmetics.
//
// Revision 1.1  1998/04/07  Serpent authors
// + revised slightly (endianness, and key schedule for variable lengths)
//
// Revision 1.0  1998/04/06  raif
// + original version.
//
// $Endlog$
/*
 * Copyright (c) 1997, 1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */
package Serpent;

import java.io.PrintWriter;
import java.security.InvalidKeyException;

//...........................................................................
/**
 * Serpent is a 128-bit 32-round block cipher with variable key lengths,
 * including 128-, 192- and 256-bit
 * keys conjectured to be at least as secure as three-key triple-DES.<p>
 *
 * Serpent was designed by Ross Anderson, Eli Biham and Lars Knudsen as a
 * candidate algorithm for the NIST AES Quest.<p>
 *
 * References:<ol>
 *  <li>Serpent: A New Block Cipher Proposal. This paper was published in the
 *  proceedings of the "Fast Software Encryption Workshop No. 5" held in
 *  Paris in March 1998. LNCS, Springer Verlag.<p>
 *  <li>Reference implementation of the standard Serpent cipher written in C
 *  by <a href="http://www.cl.cam.ac.uk/~fms/">Frank Stajano</a>.</ol><p>
 *
 * <b>Copyright</b> &copy; 1997, 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.<p>
 *
 * <b>$Revision: $</b>
 * @author  Raif S. Naffah
 * @author  Serpent authors (Ross Anderson, Eli Biham and Lars Knudsen)
 */
public final class Serpent_Algorithm // implicit no-argument constructor
{
// Debugging methods and variables
//...........................................................................

    static final String NAME = "Serpent_Algorithm";
    static final boolean IN = true, OUT = false;

    static final boolean DEBUG = Serpent_Properties.GLOBAL_DEBUG;
    static final int debuglevel = DEBUG ? Serpent_Properties.getLevel(NAME) : 0;
    static final PrintWriter err = DEBUG ? Serpent_Properties.getOutput() : null;

    static final boolean TRACE = Serpent_Properties.isTraceable(NAME);

    static void debug (String s) { err.println(">>> "+NAME+": "+s); }
    static void trace (boolean in, String s) {
        if (TRACE) err.println((in?"==> ":"<== ")+NAME+"."+s);
    }
    static void trace (String s) { if (TRACE) err.println("<=> "+NAME+"."+s); }


// Constants and variables
//...........................................................................

    /**
     * This variable acts as a selector to allow the user to choose between
     * a bit-slice implementation (Serpent_BitSlice) and a standard one
     * (Serpent_Standard). When set to true the bit-slice implementation
     * code is used, otherwise it's the standard implementation code.<p>
     *
     * IMPORTANT: When you change the value of this variable, make sure
     * you re-compile all the classes in the Serpent package.
     */
    static final boolean USE_BIT_SLICE_IMPLEMENTATION = true;

    static final int BLOCK_SIZE = 16; // bytes in a Serpent data-block

    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };


// Basic API methods
//...........................................................................

    /**
     * Expand a user-supplied key material into a session key.
     *
     * @param key  The user-key bytes (multiples of 4) to use.
     * @exception  InvalidKeyException  If the key is invalid.
     */
    public static synchronized Object makeKey (byte[] key)
    throws InvalidKeyException {
        if (key == null)
            throw new InvalidKeyException("Empty key");
/*
        if (!(key.length == 16 || key.length == 24 || key.length == 32))
             throw new InvalidKeyException("Incorrect key length");

	// Although it checks for size of 16, 24 or 32, it really
	// supports all multiples of 4 up to 32 in the code of makeKey.
*/
        if ((key.length % 4) != 0 || (key.length / 4) > 8)
             throw new InvalidKeyException("Incorrect key length");

        return USE_BIT_SLICE_IMPLEMENTATION ?
            Serpent_BitSlice.makeKey(key) :
            Serpent_Standard.makeKey(key);
    }

    /**
     * Encrypt exactly one block of plaintext.
     *
     * @param  in         The plaintext.
     * @param  inOffset   Index of in from which to start considering data.
     * @param  sessionKey The session key to use for encryption.
     * @return The ciphertext generated from a plaintext using the session key.
     */
    public static byte[]
    blockEncrypt (byte[] in, int inOffset, Object sessionKey) {
        return USE_BIT_SLICE_IMPLEMENTATION ?
            Serpent_BitSlice.blockEncrypt(in, inOffset, sessionKey) :
            Serpent_Standard.blockEncrypt(in, inOffset, sessionKey);
    }

    /**
     * Decrypt exactly one block of ciphertext.
     *
     * @param  in         The ciphertext.
     * @param  inOffset   Index of in from which to start considering data.
     * @param  sessionKey The session key to use for decryption.
     * @return The plaintext generated from a ciphertext using the session key.
     */
    public static byte[]
    blockDecrypt (byte[] in, int inOffset, Object sessionKey) {
        return USE_BIT_SLICE_IMPLEMENTATION ?
            Serpent_BitSlice.blockDecrypt(in, inOffset, sessionKey) :
            Serpent_Standard.blockDecrypt(in, inOffset, sessionKey);
    }

    /** A basic symmetric encryption/decryption test. */ 
    public static boolean self_test() {
if (DEBUG) trace(IN, "self_test()");
        boolean ok = false;
        try {
            byte[] kb = new byte[BLOCK_SIZE*2]; // all zeroes
// kb=fromString("0000000000000000000000000000000000000000000000000000000000000000");
            byte[] pt = fromEvenLengthString("00000003000000020000000100000000");
            int i;

            Object key = makeKey(kb);

            byte[] ct =  blockEncrypt(pt, 0, key);
            byte[] cpt = blockDecrypt(ct, 0, key);

            ok = areEqual(pt, cpt);
            if (!ok) {
if (DEBUG && debuglevel > 7) {
    debug("  plain: "+toReversedString(pt));
    debug(" cipher: "+toReversedString(ct));
    debug(" plain2: "+toReversedString(cpt));
}
                throw new RuntimeException("Symmetric operation failed");
            }
            ok = self_test(BLOCK_SIZE);
        } catch (Exception x) {
if (DEBUG && debuglevel > 0) {
    debug("Exception encountered during self-test: " + x.getMessage());
    x.printStackTrace();
}
        }
if (DEBUG && debuglevel > 0) debug("Self-test OK? " + ok);
if (DEBUG) trace(OUT, "self_test()");
        return ok;
    }


// Serpent own methods
//...........................................................................
    
    /** @return The length in bytes of the Algorithm input block. */
    public static int blockSize() { return BLOCK_SIZE; }

    /** A basic symmetric encryption/decryption test for a given key size. */
    private static boolean self_test (int keysize) {
if (DEBUG) trace(IN, "self_test("+keysize+")");
        boolean ok = false;
        try {
            byte[] kb = new byte[keysize];
            byte[] pt = new byte[BLOCK_SIZE];
            int i;

            for (i = 0; i < keysize; i++)
                kb[i] = (byte) i;
            for (i = 0; i < BLOCK_SIZE; i++)
                pt[i] = (byte) i;

if (DEBUG && debuglevel > 6) {
System.out.println("==========");
System.out.println();
System.out.println("KEYSIZE="+(8*keysize));
System.out.println("KEY="+toString(kb));
System.out.println();
}
            Object key = makeKey(kb);

if (DEBUG && debuglevel > 6) {
System.out.println("Intermediate Ciphertext Values (Encryption)");
System.out.println();
System.out.println("PT="+toString(pt));
}
            byte[] ct =  blockEncrypt(pt, 0, key);

if (DEBUG && debuglevel > 6) {
System.out.println("Intermediate Plaintext Values (Decryption)");
System.out.println();
System.out.println("CT="+toString(ct));
}
            byte[] cpt = blockDecrypt(ct, 0, key);

            ok = areEqual(pt, cpt);
            if (!ok)
                throw new RuntimeException("Symmetric operation failed");
        } catch (Exception x) {
if (DEBUG && debuglevel > 0) {
    debug("Exception encountered during self-test: " + x.getMessage());
    x.printStackTrace();
}
        }
if (DEBUG && debuglevel > 0) debug("Self-test OK? " + ok);
if (DEBUG) trace(OUT, "self_test()");
        return ok;
    }


// utility static methods (from cryptix.util.core ArrayUtil and Hex classes)
//...........................................................................
    
    /**
     * Compares two byte arrays for equality.
     *
     * @return true if the arrays have identical contents
     */
    private static boolean areEqual (byte[] a, byte[] b) {
        int aLength = a.length;
        if (aLength != b.length)
            return false;
        for (int i = 0; i < aLength; i++)
            if (a[i] != b[i])
                return false;
        return true;
    }

    /**
     * Returns a number from 0 to 15 corresponding to the hex
     * digit <i>ch</i>.
     */
    public static int fromDigit (char ch) {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'A' && ch <= 'F')
            return ch - 'A' + 10;
        if (ch >= 'a' && ch <= 'f')
            return ch - 'a' + 10;
        throw new IllegalArgumentException("Invalid hex digit '"+ch+"'");
    }

    /**
     * Returns a string of hexadecimal digits from a byte array. Each
     * byte is converted to 2 hex symbols.
     */
    private static String toString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = 0, j = 0, k; i < length; ) {
            k = ba[i++];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }


// other utility static methods
//...........................................................................

    /**
     * Returns an hexadecimal number (respresented as a string of hexadecimal 
     * digits from a byte array). Each byte is converted to 2 hex symbols.
     * The order is however, as of printing a number from a little-endian
     * internal representation (i.e., reverse order).
     */
    public static String toReversedString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = length-1, j = 0, k; i >=0; ) {
            k = ba[i--];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }
       
    /**
     * Returns a byte array from an hexadecimal number.
     */
    public static byte[] fromEvenLengthString (String hex) {
        int len = hex.length();
        byte[] buf = new byte[((len + 1) / 2)];

        int j = 0;
        if ((len % 2) == 1) throw new IllegalArgumentException(
            "string must have an even number of digits");

        while (len > 0) {
            buf[j++] = (byte) (fromDigit(hex.charAt(--len)) |
                              (fromDigit(hex.charAt(--len)) << 4));
        }
        return buf;
    }


// main(): use to generate the Intermediate Values KAT
//...........................................................................

    public static void main (String[] args) {
if (DEBUG && debuglevel > 6) {
System.out.println("Algorithm Name: "+Serpent_Properties.FULL_NAME);
System.out.println("Electronic Codebook (ECB) Mode");
System.out.println();
}
        self_test(16);
        self_test(24);
        self_test(32);
    }
}
