// $Id: $
//
// $Log: $
// Revision 1.1.3  1998/04/14  raif
// + further performance improvement by reducing multi-dim array references.
// + performance improvement by inlining function calls.
// + added code to generate Intermediate Values KAT.
// + cosmetics.
//
// Revision 1.1  1998/04/07  Serpent authors
// + revised slightly (endianness, and key schedule for variable lengths)
//
// Revision 1.0  1998/04/06  raif
// + start of history.
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
 * A bit-slice implementation in Java of the Serpent cipher.<p>
 *
 * Serpent is a 128-bit 32-round block cipher with variable key lengths,
 * including 128-, 192- and 256-bit
 * keys conjectured to be at least as secure as three-key triple-DES.<p>
 *
 * Serpent was designed by Ross Anderson, Eli Biham and Lars Knudsen as a
 * candidate algorithm for the NIST AES Quest.<p>
 *
 * References:<ol>
 *  <li>Serpent: A New Block Cipher Proposal. This paper was published in the
 *  proceedings of the "Fast Software Encryption Workshop No. 6" held in
 *  Paris in March 1998. LNCS, Springer Verlag.<p>
 *  <li>Reference implementation of the standard Serpent cipher written in C
 *  by <a href="http://www.cl.cam.ac.uk/~fms/"> Frank Stajano</a>.</ol><p>
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
public final class Serpent_BitSlice // implicit no-argument constructor
{
// Debugging methods and variables
//...........................................................................

    static final String NAME = "Serpent_BitSlice";
    static final boolean IN = true, OUT = false;

    static final boolean DEBUG = Serpent_Properties.GLOBAL_DEBUG;
    static final int debuglevel =
        DEBUG ? Serpent_Properties.getLevel("Serpent_Algorithm") : 0;
    static final PrintWriter err =
        DEBUG ? Serpent_Properties.getOutput() : null;

    static final boolean TRACE =
        Serpent_Properties.isTraceable("Serpent_Algorithm");

    static void debug (String s) { err.println(">>> "+NAME+": "+s); }
    static void trace (boolean in, String s) {
        if (TRACE) err.println((in?"==> ":"<== ")+NAME+"."+s);
    }
    static void trace (String s) { if (TRACE) err.println("<=> "+NAME+"."+s); }


// Constants and variables
//...........................................................................

    static final int BLOCK_SIZE =  16; // bytes in a data-block

    static final int ROUNDS = 32;              // nbr of rounds
    static final int PHI = 0x9E3779B9; // (sqrt(5) - 1) * 2**31

    /**
     * An array of 32 (number of rounds) S boxes.<p>
     *
     * An S box is an array of 16 distinct quantities, each in the range 0-15.
     * A value v at position p for a given S box, implies that if this S box
     * is given on input a value p, it will return the value v.
     */
    static final byte[][] Sbox = new byte[][] {
        {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
        { 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
        { 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
        {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13},
        {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
        { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
        { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
        {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9},
        {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
        {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
        {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
        { 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12},
        { 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
        {13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
        {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
        { 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14},
        { 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
        {14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
        { 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
        {11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3},
        {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
        {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
        { 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
        { 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13},
        { 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
        {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
        { 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
        { 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12},
        {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
        { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
        { 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
        { 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
    };

    static final byte[][] SboxInverse = new byte[][] {
        {14,  3,  4,  8,  1, 12, 10, 15,  7, 13,  9,  6, 11,  2,  0,  5},
        { 0,  7,  5, 14,  3, 13,  9,  2, 15, 12,  8, 11, 10,  6,  4,  1},
        {15,  1,  6, 12,  0, 14,  5, 11,  3, 10, 13,  7,  9,  4,  2,  8},
        {13,  6,  3, 10,  4,  8, 14,  7,  2,  5, 12,  9,  1, 15, 11,  0},
        {13,  1, 10,  6,  7, 14,  4,  9,  2,  8, 15,  5, 12, 11,  3,  0},
        { 9, 10,  5,  0,  2, 15, 12,  3,  6, 13, 11, 14,  8,  1,  7,  4},
        { 0,  7, 14, 13,  5,  8, 11,  2,  9, 12,  4,  3, 10,  6,  1, 15},
        {12,  3,  7,  4,  6, 13,  9, 10,  1, 15,  2,  8, 11,  0, 14,  5},
        { 1,  8, 14,  5, 13,  7,  4, 11, 15,  2,  0, 12, 10,  9,  3,  6},
        { 2, 15,  8,  4,  5, 10,  6,  1,  9,  3,  7, 13, 12,  0, 11, 14},
        { 7,  9, 10,  6,  2, 12,  1, 15,  4,  3, 13,  8, 11,  0, 14,  5},
        { 3,  0, 14, 11,  8, 13,  4,  7,  6,  5,  1, 12, 15,  2, 10,  9},
        { 4,  8,  9,  3, 14, 11,  5,  0, 10,  6,  7, 12, 13,  1,  2, 15},
        { 6, 12, 10,  7,  8,  3,  4,  9,  1, 15, 13,  2, 11,  0, 14,  5},
        { 3,  9, 13, 10, 15, 12,  1,  6, 14,  2,  0,  5,  4,  7, 11,  8},
        { 2,  5, 14,  0,  9, 10,  3, 13,  7,  8,  4, 11, 12,  6, 15,  1},
        {13,  3,  0, 10,  2,  9,  7,  4,  8, 15,  5,  6,  1, 12, 14, 11},
        { 9,  7,  2, 12,  4,  8, 15,  5, 14, 13, 11,  1,  3,  6,  0, 10},
        {14,  2,  1, 13,  0, 11, 12,  6,  7,  9,  4,  3, 10,  5, 15,  8},
        {10,  4,  6, 15, 13, 14,  8,  3,  1, 11, 12,  0,  2,  7,  5,  9},
        { 8,  1,  5, 10, 11, 14,  6, 13,  7,  4,  2, 15,  0,  9, 12,  3},
        {12,  9,  3, 14,  2,  7,  8,  4, 15,  6,  0, 13,  5, 10, 11,  1},
        { 9, 12,  4,  7, 10,  3, 15,  8,  5,  0, 11, 14,  6, 13,  1,  2},
        {13, 10,  2,  1,  0,  5, 12, 11, 14,  4,  7,  8,  3, 15,  9,  6},
        { 5, 15,  2,  8,  0, 12, 14, 11,  6, 10, 13,  1,  9,  7,  3,  4},
        { 1,  6, 12,  9,  4, 10, 15,  3, 14,  5,  7,  2, 11,  0,  8, 13},
        {12,  0, 15,  5,  1, 13, 10,  6, 11, 14,  8,  2,  4,  3,  7,  9},
        {10,  4, 13, 14,  5,  9,  0,  7,  3,  8,  6,  1, 15,  2, 12, 11},
        {13,  7,  1, 10,  3, 12,  4, 15,  2,  9,  8,  6, 14,  0, 11,  5},
        {12,  0, 15,  5,  7,  9, 10,  6,  3, 14,  4, 11,  8,  2, 13,  1},
        { 8,  3,  7, 13,  2, 14,  9,  0, 15,  4, 10,  1,  5, 11,  6, 12},
        {11,  1,  0, 12,  4, 13, 14,  3,  6, 10,  5, 15,  9,  7,  2,  8}
    };

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
if (DEBUG) trace(IN, "makeKey("+key+")");
if (DEBUG && debuglevel > 7) {
System.out.println("Intermediate Bit-slice Session Key Values");
System.out.println();
System.out.println("Raw="+toString(key));
}
        // compute prekeys w[]:
        // (a) from user key material
        int[] w = new int[4 * (ROUNDS + 1)];
        int limit = key.length / 4;
        int i, j, t, offset = 0;
        for (i = 0; i < limit; i++)
            w[i] = (key[offset++] & 0xFF) |
                   (key[offset++] & 0xFF) <<  8 |
                   (key[offset++] & 0xFF) << 16 |
                   (key[offset++] & 0xFF) << 24;

        if (i < 8)
            w[i++] = 1;
//        for (; i < 8; i++)
//            w[i] = 0;

        // (b) and expanding them to full 132 x 32-bit material
        // this is a literal implementation of the Serpent paper
        // (section 4 The Key Schedule, p.226)
        //
        // start by computing the first 8 values using the second
        // lot of 8 values as an intermediary buffer
        for (i = 8, j = 0; i < 16; i++) {
            t = w[j] ^ w[i-5] ^ w[i-3] ^ w[i-1] ^ PHI ^ j++;
            w[i] = t << 11 | t >>> 21;
        }
        // translate the buffer by -8
        for (i = 0, j = 8; i < 8; ) w[i++] = w[j++];
        limit = 4 * (ROUNDS + 1); // 132 for a 32-round Serpent
        // finish computing the remaining intermediary subkeys
        for ( ; i < limit; i++) {
            t = w[i-8] ^ w[i-5] ^ w[i-3] ^ w[i-1] ^ PHI ^ i;
            w[i] = t << 11 | t >>> 21;
        }
        // compute intermediary key. use the same array as prekeys
        int x0, x1, x2, x3, y0, y1, y2, y3, z;
        byte[] sb;
        for (i = 0; i < ROUNDS + 1; i++) {
            x0 = w[i     ];
            x1 = w[i + 33];
            x2 = w[i + 66];
            x3 = w[i + 99];
            y0 = y1 = y2 = y3 = 0;
            sb = Sbox[(ROUNDS + 3 - i) % ROUNDS];
            for (j = 0; j < 32; j++) {
                z = sb[((x0 >>> j) & 0x01)      |
                       ((x1 >>> j) & 0x01) << 1 |
                       ((x2 >>> j) & 0x01) << 2 |
                       ((x3 >>> j) & 0x01) << 3];
                y0 |= ( z        & 0x01) << j;
                y1 |= ((z >>> 1) & 0x01) << j;
                y2 |= ((z >>> 2) & 0x01) << j;
                y3 |= ((z >>> 3) & 0x01) << j;
            }
            w[i     ] = y0;
            w[i + 33] = y1;
            w[i + 66] = y2;
            w[i + 99] = y3;
        }
        // instead of a 2-dim array use a 1-dim array for better preformance
if (DEBUG && debuglevel > 7) {
System.out.println("K[]:"); for(i=0;i<ROUNDS+1;i++){for(j=0;j<4;j++) System.out.print("0x"+intToString(w[i*4+j])+", "); System.out.println();}
System.out.println();
}
if (DEBUG) trace(OUT, "makeKey()");
        return w;
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
        int[] K = (int[]) sessionKey;
        int x0 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x1 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x2 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x3 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int y0, y1, y2, y3, z;
        int n = ROUNDS - 1;
        int j = 0;
        byte[] sb;
        for (int i = 0; i < n; i++) {
            x0 ^= K[j++];
            x1 ^= K[j++];
            x2 ^= K[j++];
            x3 ^= K[j++];

            y0 = y1 = y2 = y3 = 0;
            sb = Sbox[i];
            for (int k = 0; k < 32; k++) {
                z = sb[((x0 >>> k) & 0x01)      |
                       ((x1 >>> k) & 0x01) << 1 |
                       ((x2 >>> k) & 0x01) << 2 |
                       ((x3 >>> k) & 0x01) << 3];
                y0 |= ( z        & 0x01) << k;
                y1 |= ((z >>> 1) & 0x01) << k;
                y2 |= ((z >>> 2) & 0x01) << k;
                y3 |= ((z >>> 3) & 0x01) << k;
            }
            x0 = y0 << 13 | y0 >>> 19;
            x2 = y2 <<  3 | y2 >>> 29;
            x1 = y1 ^ x0 ^ x2;
            x3 = y3 ^ x2 ^ (x0 << 3);
            x1 = x1 << 1 | x1 >>> 31;
            x3 = x3 << 7 | x3 >>> 25;
            x0 ^= x1 ^ x3;
            x2 ^= x3 ^ (x1 << 7);
            x0 = x0 <<  5 | x0 >>> 27;
            x2 = x2 << 22 | x2 >>> 10;
if (DEBUG && debuglevel > 6) System.out.println("CT"+i+"="+intToString(x0)+intToString(x1)+intToString(x2)+intToString(x3));
        }
        x0 ^= K[j++];
        x1 ^= K[j++];
        x2 ^= K[j++];
        x3 ^= K[j++];

        y0 = y1 = y2 = y3 = 0;
        sb = Sbox[n];
        for (int k = 0; k < 32; k++) {
            z = sb[((x0 >>> k) & 0x01)      |
                   ((x1 >>> k) & 0x01) << 1 |
                   ((x2 >>> k) & 0x01) << 2 |
                   ((x3 >>> k) & 0x01) << 3];
            y0 |= ( z        & 0x01) << k;
            y1 |= ((z >>> 1) & 0x01) << k;
            y2 |= ((z >>> 2) & 0x01) << k;
            y3 |= ((z >>> 3) & 0x01) << k;
        }
        x0 = y0 ^ K[j++];
        x1 = y1 ^ K[j++];
        x2 = y2 ^ K[j++];
        x3 = y3 ^ K[j++];

        byte[] result = new byte[] {
            (byte)(x0), (byte)(x0 >>> 8), (byte)(x0 >>> 16), (byte)(x0 >>> 24),
            (byte)(x1), (byte)(x1 >>> 8), (byte)(x1 >>> 16), (byte)(x1 >>> 24),
            (byte)(x2), (byte)(x2 >>> 8), (byte)(x2 >>> 16), (byte)(x2 >>> 24),
            (byte)(x3), (byte)(x3 >>> 8), (byte)(x3 >>> 16), (byte)(x3 >>> 24)
        };
if (DEBUG && debuglevel > 6) {
System.out.println("CT="+toString(result));
System.out.println();
}
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
        int[] K = (int[]) sessionKey;
        int x0 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x1 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x2 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;
        int x3 = (in[inOffset++] & 0xFF)       |
                 (in[inOffset++] & 0xFF) <<  8 |
                 (in[inOffset++] & 0xFF) << 16 |
                 (in[inOffset++] & 0xFF) << 24;

        int z, y0, y1, y2, y3;
        int j = ROUNDS << 2;
        x0 ^= K[j++];
        x1 ^= K[j++];
        x2 ^= K[j++];
        x3 ^= K[j++];
        y0 = y1 = y2 = y3 = 0;
        byte[] sb = SboxInverse[ROUNDS-1];
        for (int k = 0; k < 32; k++) {
            z = sb[((x0 >>> k) & 0x01)      |
                   ((x1 >>> k) & 0x01) << 1 |
                   ((x2 >>> k) & 0x01) << 2 |
                   ((x3 >>> k) & 0x01) << 3];
            y0 |= ( z        & 0x01) << k;
            y1 |= ((z >>> 1) & 0x01) << k;
            y2 |= ((z >>> 2) & 0x01) << k;
            y3 |= ((z >>> 3) & 0x01) << k;
        }
        j = (ROUNDS-1) << 2;
        x0 = y0 ^ K[j++];
        x1 = y1 ^ K[j++];
        x2 = y2 ^ K[j++];
        x3 = y3 ^ K[j++];

        for (int i = ROUNDS-2; i >= 0; i--) {
if (DEBUG && debuglevel > 6) System.out.println("PT"+i+"="+intToString(x0)+intToString(x1)+intToString(x2)+intToString(x3));
            x2 = x2 << 10 | x2 >>> 22;
            x0 = x0 << 27 | x0 >>>  5;
            x2 ^= x3 ^ (x1 << 7);
            x0 ^= x1 ^ x3;
            x3 = x3 << 25 | x3 >>> 7;
            x1 = x1 << 31 | x1 >>> 1;
            x3 ^= x2 ^ (x0 << 3);
            x1 ^= x0 ^ x2;
            x2 = x2 << 29 | x2 >>>  3;
            x0 = x0 << 19 | x0 >>> 13;

            y0 = y1 = y2 = y3 = 0;
            sb = SboxInverse[i];
            for (int k = 0; k < 32; k++) {
                z = sb[((x0 >>> k) & 0x01)      |
                       ((x1 >>> k) & 0x01) << 1 |
                       ((x2 >>> k) & 0x01) << 2 |
                       ((x3 >>> k) & 0x01) << 3];
                y0 |= ( z        & 0x01) << k;
                y1 |= ((z >>> 1) & 0x01) << k;
                y2 |= ((z >>> 2) & 0x01) << k;
                y3 |= ((z >>> 3) & 0x01) << k;
            }
            j = i << 2;
            x0 = y0 ^ K[j++];
            x1 = y1 ^ K[j++];
            x2 = y2 ^ K[j++];
            x3 = y3 ^ K[j++];
        }
        byte[] result = new byte[] {
            (byte)(x0), (byte)(x0 >>> 8), (byte)(x0 >>> 16), (byte)(x0 >>> 24),
            (byte)(x1), (byte)(x1 >>> 8), (byte)(x1 >>> 16), (byte)(x1 >>> 24),
            (byte)(x2), (byte)(x2 >>> 8), (byte)(x2 >>> 16), (byte)(x2 >>> 24),
            (byte)(x3), (byte)(x3 >>> 8), (byte)(x3 >>> 16), (byte)(x3 >>> 24)
        };
if (DEBUG && debuglevel > 6) {
System.out.println("PT="+toString(result));
System.out.println();
}
if (DEBUG) trace(OUT, "blockDecrypt()");
        return result;
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

    /**
     * Returns a string of 8 hexadecimal digits (most significant
     * digit first) corresponding to the integer <i>n</i>, which is
     * treated as unsigned.
     */
    public static final String intToString (int n) {
        char[] buf = new char[8];
        for (int i = 7; i >= 0; i--) {
            buf[i] = HEX_DIGITS[n & 0x0F];
            n >>>= 4;
        }
        return new String(buf);
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

    /**
     * Returns a string of hexadecimal digits from an integer array. Each
     * int is converted to 4 hex symbols.
     */
    private static final String toString (int[] ia) {
        int length = ia.length;
        char[] buf = new char[length * 8];
        for (int i = 0, j = 0, k; i < length; i++) {
            k = ia[i];
            buf[j++] = HEX_DIGITS[(k >>> 28) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 24) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 20) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 16) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>> 12) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  8) & 0x0F];
            buf[j++] = HEX_DIGITS[(k >>>  4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k         & 0x0F];
        }
        return new String(buf);
    }
}
