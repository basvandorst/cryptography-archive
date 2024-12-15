// $Id: $
//
// $Log: $
// Revision 1.0.1  1998/04/13  raif
// + slightly improved performance.
//
// Revision 1.0  1998/04/07  raif
// + original version.
//
// $Endlog$
/*
 * Copyright (c) 1997, 1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */
package DEAL;

import java.io.PrintWriter;
import java.security.InvalidKeyException;

//...........................................................................
/**
 * DEAL is a 128-bit, variable key-size (128-, 192- and 256-bit) symmetric
 * cipher.<p>
 *
 * DEAL was designed by <a href="mailto:lars.knudsen@ii.uib.no">Lars Ramkilde
 * Knudsen</a> and is described by the designer in: DEAL -- A 128-bit Block
 * Cipher, February 21, 1998 (postscript file).<p>
 *
 * <b>Copyright</b> &copy; 1997, 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.<p>
 *
 * <b>$Revision: $</b>
 * @author  Raif S. Naffah
 */
public final class DEAL_Algorithm // implicit no-argument constructor
{
// Debugging methods and variables
//...........................................................................

    static final String NAME = "DEAL_Algorithm";
    static final boolean IN = true, OUT = false;

    static final boolean DEBUG = DEAL_Properties.GLOBAL_DEBUG;
    static final int debuglevel = DEBUG ? DEAL_Properties.getLevel(NAME) : 0;
    static final PrintWriter err = DEBUG ? DEAL_Properties.getOutput() : null;

    static final boolean TRACE = DEAL_Properties.isTraceable(NAME);

    static void debug (String s) { err.println(">>> "+NAME+": "+s); }
    static void trace (boolean in, String s) {
        if (TRACE) err.println((in?"==> ":"<== ")+NAME+"."+s);
    }
    static void trace (String s) { if (TRACE) err.println("<=> "+NAME+"."+s); }


// Constants and variables
//...........................................................................

    static final int BLOCK_SIZE = 16; // block size in bytes

    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };


// Basic API methods
//...........................................................................

    /**
     * Expand a user-supplied key material into a session key.
     *
     * @param key  The 128/192/256-bit user-key to use.
     * @exception  InvalidKeyException  If the key is invalid.
     */
    public static synchronized Object makeKey (byte[] k)
    throws InvalidKeyException {
if (DEBUG) trace(IN, "makeKey("+k+")");
        if (k == null)
            throw new InvalidKeyException("Empty key");
        int s = k.length / 8;
        if (!(s == 2 || s == 3 || s == 4))
             throw new InvalidKeyException("Incorrect key length");
if (DEBUG && debuglevel > 7) {
System.out.println("Intermediate Session Key Values");
System.out.println();
System.out.println("Raw="+toString(k));
}
        //
        // format user key material into K[] DES key material objects
        // each being 64-bit wide; ie. an array of 8 bytes
        //
        byte[][] K = new byte[s][8];
        int i, j, offset = 0;
        for (i = 0; i < s; i++)
            for (j = 0; j < 8; j++)
                K[i][j] = k[offset++];
        //
        // out of these s DES keys make r new DES keys RK[]; r being
        // the appropriate number of rounds for the given user key
        // material supplied
        //
        // NOTE:                                         --RSN 1998/04/15
        //    Although the next 2 lines are correct, the code generation
        //    (tested with jdk-115 on win95B) may yield incorrect results
        //    depending on wether the -O switch is set or not when compiling
        //    with javac. Not using the -O switch always result in correct
        //    values.
        //
        //    The current code seems to work correctly with and without the
        //    -O switch of javac.
        //
//        int r = getRounds(k.length);
//        byte[][] RK = new byte[r][8];
        byte[][] RK = null;
        // the formulae for computing RK[] use a fixed DES key, we'll
        // note it as Kf
        byte[] Kf = new byte[] {
                   0x01,        0x23,        0x45,        0x67,
            (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF
        };
        Object Ek = DES_Algorithm.makeKey(Kf);
        // three cases are possible depending on the value of s
        switch (s) {
        case 2:
            RK = new byte[6][8];
            RK[0] = DES_Algorithm.blockEncrypt(        K[0],             0, Ek);
            RK[1] = DES_Algorithm.blockEncrypt(xor(    K[1],     RK[0]), 0, Ek);
            RK[2] = DES_Algorithm.blockEncrypt(xor(xor(K[0], 1), RK[1]), 0, Ek);
            RK[3] = DES_Algorithm.blockEncrypt(xor(xor(K[1], 2), RK[2]), 0, Ek);
            RK[4] = DES_Algorithm.blockEncrypt(xor(xor(K[0], 3), RK[3]), 0, Ek);
            RK[5] = DES_Algorithm.blockEncrypt(xor(xor(K[1], 4), RK[4]), 0, Ek);
            break;
        case 3:
            RK = new byte[6][8];
            RK[0] = DES_Algorithm.blockEncrypt(        K[0],             0, Ek);
            RK[1] = DES_Algorithm.blockEncrypt(xor(    K[1],     RK[0]), 0, Ek);
            RK[2] = DES_Algorithm.blockEncrypt(xor(    K[2],     RK[1]), 0, Ek);
            RK[3] = DES_Algorithm.blockEncrypt(xor(xor(K[0], 1), RK[2]), 0, Ek);
            RK[4] = DES_Algorithm.blockEncrypt(xor(xor(K[1], 2), RK[3]), 0, Ek);
            RK[5] = DES_Algorithm.blockEncrypt(xor(xor(K[2], 3), RK[4]), 0, Ek);
            break;
        case 4:
            RK = new byte[8][8];
            RK[0] = DES_Algorithm.blockEncrypt(        K[0],             0, Ek);
            RK[1] = DES_Algorithm.blockEncrypt(xor(    K[1],     RK[0]), 0, Ek);
            RK[2] = DES_Algorithm.blockEncrypt(xor(    K[2],     RK[1]), 0, Ek);
            RK[3] = DES_Algorithm.blockEncrypt(xor(    K[3],     RK[2]), 0, Ek);
            RK[4] = DES_Algorithm.blockEncrypt(xor(xor(K[0], 1), RK[3]), 0, Ek);
            RK[5] = DES_Algorithm.blockEncrypt(xor(xor(K[1], 2), RK[4]), 0, Ek);
            RK[6] = DES_Algorithm.blockEncrypt(xor(xor(K[2], 3), RK[5]), 0, Ek);
            RK[7] = DES_Algorithm.blockEncrypt(xor(xor(K[3], 4), RK[6]), 0, Ek);
            break;
        }
if (DEBUG && debuglevel > 7) {
//for (i=0;i<r;i++) System.out.println("RK"+i+"="+toString(RK[i]));
for (i=0;i<RK.length;i++) System.out.println("RK"+i+"="+toString(RK[i]));
System.out.println();
}
        // DEAL session keys become then the set of the r session keys built
        // using the RK[]
//        Object[] sessionKey = new Object[r];
        Object[] sessionKey = new Object[RK.length];
//        for (i = 0; i < r; i++)
        for (i = 0; i < RK.length; i++)
            sessionKey[i] = DES_Algorithm.makeKey(RK[i]);
if (DEBUG) trace(OUT, "makeKey()");
        return sessionKey;
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
if (DEBUG) trace(IN, "blockEncrypt("+in+", "+inOffset+", "+sessionKey+")");
        Object[] rKey = (Object[]) sessionKey; // re-cast sessionKey to round keys
        int rounds = rKey.length;

        byte[] L0 = new byte[8];       // divide input into 2 64-bit entities
        byte[] R0 = new byte[8];
        byte[] L1, R1; // temp arrays
        System.arraycopy(in, inOffset,     L0, 0, 8);
        System.arraycopy(in, inOffset + 8, R0, 0, 8);

if (DEBUG && debuglevel > 6) System.out.println("PT="+toString(L0)+" "+toString(R0));

        DES_Algorithm.IP(L0);                   // apply DES IP to both sides
        DES_Algorithm.IP(R0);

if (DEBUG && debuglevel > 6) System.out.println("IP(PT)="+toString(L0)+" "+toString(R0));

        R1 = (byte[]) L0.clone();               // j = 1
        DES_Algorithm.encrypt(L0, rKey[0]);
        L1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("CT1="+toString(L1)+" "+toString(R1));
        R0 = (byte[]) L1.clone();               // j = 2
        DES_Algorithm.encrypt(L1, rKey[1]);
        L0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("CT2="+toString(L0)+" "+toString(R0));
        R1 = (byte[]) L0.clone();               // j = 3
        DES_Algorithm.encrypt(L0, rKey[2]);
        L1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("CT3="+toString(L1)+" "+toString(R1));
        R0 = (byte[]) L1.clone();               // j = 4
        DES_Algorithm.encrypt(L1, rKey[3]);
        L0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("CT4="+toString(L0)+" "+toString(R0));
        R1 = (byte[]) L0.clone();               // j = 5
        DES_Algorithm.encrypt(L0, rKey[4]);
        L1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("CT5="+toString(L1)+" "+toString(R1));
        R0 = (byte[]) L1.clone();               // j = 6
        DES_Algorithm.encrypt(L1, rKey[5]);
        L0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("CT6="+toString(L0)+" "+toString(R0));
        if (rounds == 8) {
            R1 = (byte[]) L0.clone();           // j = 7
            DES_Algorithm.encrypt(L0, rKey[6]);
            L1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("CT7="+toString(L1)+" "+toString(R1));
            R0 = (byte[]) L1.clone();           // j = 8
            DES_Algorithm.encrypt(L1, rKey[7]);
            L0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("CT8="+toString(L0)+" "+toString(R0));
        }
        DES_Algorithm.FP(L0);                   // apply DES FP to both sides
        DES_Algorithm.FP(R0);
if (DEBUG && debuglevel > 6) {
System.out.println("CT="+toString(L0)+" "+toString(R0));
System.out.println();
}
        byte[] result = new byte[16];
        System.arraycopy(L0, 0, result, 0, 8);
        System.arraycopy(R0, 0, result, 8, 8);
if (DEBUG) trace(OUT, "blockEncrypt()");
        return result;
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
if (DEBUG) trace(IN, "blockDecrypt("+in+", "+inOffset+", "+sessionKey+")");
        Object[] rKey = (Object[]) sessionKey; // re-cast sessionKey to round keys
        int rounds = rKey.length;

        byte[] L1 = new byte[8];       // divide input into 2 64-bit entities
        byte[] R1 = new byte[8];
        byte[] L0, R0; // temp arrays
        System.arraycopy(in, inOffset,     L1, 0, 8);
        System.arraycopy(in, inOffset + 8, R1, 0, 8);

if (DEBUG && debuglevel > 6) System.out.println("CT="+toString(L1)+" "+toString(R1));

        DES_Algorithm.IP(L1);                   // apply DES IP to both sides
        DES_Algorithm.IP(R1);
if (DEBUG && debuglevel > 6) System.out.println("IP(CT)="+toString(L1)+" "+toString(R1));

        if (rounds == 8) {
            L0 = (byte[]) R1.clone();           // j = 8
            DES_Algorithm.encrypt(R1, rKey[7]);
            R0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("PT8="+toString(L0)+" "+toString(R0));
            L1 = (byte[]) R0.clone();           // j = 7
            DES_Algorithm.encrypt(R0, rKey[6]);
            R1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("PT7="+toString(L1)+" "+toString(R1));
        }
        L0 = (byte[]) R1.clone();               // j = 6
        DES_Algorithm.encrypt(R1, rKey[5]);
        R0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("PT6="+toString(L0)+" "+toString(R0));
        L1 = (byte[]) R0.clone();               // j = 5
        DES_Algorithm.encrypt(R0, rKey[4]);
        R1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("PT5="+toString(L1)+" "+toString(R1));
        L0 = (byte[]) R1.clone();               // j = 4
        DES_Algorithm.encrypt(R1, rKey[3]);
        R0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("PT4="+toString(L0)+" "+toString(R0));
        L1 = (byte[]) R0.clone();               // j = 3
        DES_Algorithm.encrypt(R0, rKey[2]);
        R1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("PT3="+toString(L1)+" "+toString(R1));
        L0 = (byte[]) R1.clone();               // j = 2
        DES_Algorithm.encrypt(R1, rKey[1]);
        R0 = xor(L1, R1);
if (DEBUG && debuglevel > 6) System.out.println("PT2="+toString(L0)+" "+toString(R0));
        L1 = (byte[]) R0.clone();               // j = 1
        DES_Algorithm.encrypt(R0, rKey[0]);
        R1 = xor(L0, R0);
if (DEBUG && debuglevel > 6) System.out.println("PT1="+toString(L1)+" "+toString(R1));

        DES_Algorithm.FP(L1);                   // apply DES FP to both sides
        DES_Algorithm.FP(R1);
if (DEBUG && debuglevel > 6) {
System.out.println("PT="+toString(L1)+" "+toString(R1));
System.out.println();
}
        byte[] result = new byte[16];
        System.arraycopy(L1, 0, result, 0, 8);
        System.arraycopy(R1, 0, result, 8, 8);
if (DEBUG) trace(OUT, "blockDecrypt()");
        return result;
    }

    /** A basic symmetric encryption/decryption test. */ 
    public static boolean self_test() {
        boolean ok = DES_Algorithm.self_test();
        try {
            if (!ok)
                throw new RuntimeException("DES self-test failed");
            byte[] kb = new byte[] {
                (byte) 0x01, (byte) 0x23, (byte) 0x45, (byte) 0x67,
                (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF,
                (byte) 0x10, (byte) 0x32, (byte) 0x54, (byte) 0x76,
                (byte) 0x98, (byte) 0xBA, (byte) 0xDC, (byte) 0xFE };
            byte[] pt = fromString("0123456789ABCDEF1032547698BADCFE");
            byte[] ct = fromString("419C64EC7C579C4CAC81A4ED78D9374B");
            int i;

            Object key = makeKey(kb);

            byte[] tmp = blockEncrypt(pt, 0, key);
            ok = areEqual(ct, tmp);
            if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("KAT single triple");
    debug("       key: " + toString(kb));
    debug(" plaintext: " + toString(pt));
    debug("ciphertext: " + toString(ct));
    debug("  computed: " + toString(tmp));
}
                throw new RuntimeException("Encryption failed");
            }
            tmp = blockDecrypt(ct, 0, key);
            ok = areEqual(pt, tmp);
            if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("KAT single triple");
    debug("       key: " + toString(kb));
    debug("ciphertext: " + toString(ct));
    debug(" plaintext: " + toString(pt));
    debug("  computed: " + toString(tmp));
}
                throw new RuntimeException("Decryption failed");
            }
if (DEBUG && debuglevel > 0) debug("KAT (single triple) OK? " + ok);
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


// own methods
//...........................................................................
    
    /** @return The length in bytes of DEAL's input block. */
    public static int blockSize() { return BLOCK_SIZE; }

    /**
     * Return The number of rounds for a given DEAL's user key size.
     *
     * @param keySize  The size of the user key material in bytes.
     * @return The number of rounds for this DEAL.
     */
    public static final int getRounds (int keySize) {
if (DEBUG) trace(IN, "getRounds("+keySize+")");
        switch (keySize) {
        case 16:
        case 24:
if (DEBUG) trace(OUT, "getRounds() --> 6");
            return 6;
        case 32:
        default:
if (DEBUG) trace(OUT, "getRounds() --> 8");
            return 8;
        }
    }

    /** @return A 64-bit entity = x ^ y. */
    private static final byte[] xor (byte[] x, byte[] y) {
        byte[] result = (byte[]) x.clone();
        for (int i = 0; i < 8; i++)
            result[i] ^= y[i];
        return result;
    }

    /**
     * @param x  A DES key as a byte[].
     * @param y  A bit number/position in a 64-bit ordinal string where the
     *      <i>y</i><sup>th</sup> bit (and no other) is set. Allowed values
     *      for <i>y</i> are 1, 2, 3 and 4. The bits are numbered, as described
     *      in FIPS 46-2 (Data Encryption Standard --DES), from left to right,
     *      ie. the left most bit of the block is bit zero.
     * @return  A 64-bit entity result of xoring both input values together.
     */
    private static final byte[] xor (byte[] x, int y) {
        byte[] result = (byte[]) x.clone();
        result[0] ^= (0x80 >>> (y-1)) & 0xFF;
        return result;
    }

    /** A basic symmetric encryption/decryption test for a given key size. */
    private static boolean self_test (int keysize) {
if (DEBUG) trace(IN, "self_test("+keysize+")");
        boolean ok = false;
        try {
            byte[] kb = new byte[keysize];
            byte[] pt = new byte[BLOCK_SIZE];
            int i;

//            kb[0] = (byte) 0x80;
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
}
            byte[] ct =  blockEncrypt(pt, 0, key);

if (DEBUG && debuglevel > 6) {
System.out.println("Intermediate Plaintext Values (Decryption)");
System.out.println();
}
            byte[] cpt = blockDecrypt(ct, 0, key);

            ok = areEqual(pt, cpt);
            if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("       key: " + toString(kb));
    debug(" plaintext: " + toString(pt));
    debug("ciphertext: " + toString(ct));
    debug("  computed: " + toString(cpt));
}
                throw new RuntimeException("Symmetric operation failed");
            }
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
    private static final boolean areEqual (byte[] a, byte[] b) {
        int aLength = a.length;
        if (aLength != b.length)
            return false;
        for (int i = 0; i < aLength; i++)
            if (a[i] != b[i])
                return false;
        return true;
    }

    /** Returns a byte array from a string of hexadecimal digits. */
    private static byte[] fromString (String hex) {
        int len = hex.length();
        byte[] buf = new byte[((len + 1) / 2)];
        int i = 0, j = 0;
        if ((len % 2) == 1)
            buf[j++] = (byte) fromDigit(hex.charAt(i++));
        while (i < len) {
            buf[j++] = (byte)(
                (fromDigit(hex.charAt(i++)) << 4) |
                 fromDigit(hex.charAt(i++))
            );
        }
        return buf;
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
    private static final String toString (byte[] ba) {
        int length = ba.length;
        char[] buf = new char[length * 2];
        for (int i = 0, j = 0, k; i < length; ) {
            k = ba[i++];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }


// main(): use to generate the Intermediate Values KAT
//...........................................................................

    public static void main (String[] args) {
if (DEBUG && debuglevel > 6) {
System.out.println("Algorithm Name: "+DEAL_Properties.FULL_NAME);
System.out.println("Electronic Codebook (ECB) Mode");
System.out.println();
}
        self_test();
        self_test(24);
        self_test(32);
    }
}
