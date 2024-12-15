// $Id: $
//
// $Log: $
// Revision 1.0  1998/03/24  raif
// + start of history
//
// $Endlog$
/*
 * Copyright (c) 1995-1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */
package DEAL;

import java.io.PrintWriter;
import java.security.InvalidKeyException;

/**
 * DES is a 64-bit block cipher and a key length of 8 bytes, out of which
 * only 56 bits are used as the parity bit in each byte is ignored.<p>
 *
 * DES was designed by IBM and first released in 1976. The algorithm is
 * freely usable for both single and triple encryption.<p>
 *
 * <b>References:</b><ol>
 *    <li> <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *         "Chapter 12 Data Encryption Standard,"
 *         <cite>Applied Cryptography, 2nd edition</cite>,
 *         John Wiley &amp; Sons, 1996.<p>
 *    <li> <a href="http://www.itl.nist.gov/div897/pubs/fip46-2.htm">
 *         NIST FIPS PUB 46-2</a> (supercedes FIPS PUB 46-1),
 *         "Data Encryption Standard",
 *         U.S. Department of Commerce, December 1993.<p>
 *
 * Portions of this code are copyright &copy; 1995-1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.<p>
 *
 * Other copyright owners and contributors are:<ul>
 *  <li>Copyright &copy; 1995 <a href="mailto:eay@mincom.oz.au">Eric Young</a>
 *      --string representing SP_TRANS tables.<p>
 *  <li>Copyright &copy; 1996 Geoffrey Keating --original Java version.<p>
 *  <li><a href="jdumas@zgs.com">John F. Dumas</a> --code for crypt(3).<p>
 *
 * <b>$Revision: $</b>
 * @author  Systemics Ltd
 * @author  Geoffrey Keating
 * @author  Eric Young
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @author  John F. Dumas (jdumas@zgs.com)
 */
public final class DES_Algorithm
{
// Debugging methods and variables
//...........................................................................

    static final String NAME = "DES_Algorithm";
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


// DES constants and variables
//...........................................................................

    private static final int ROUNDS = 16;    // number of rounds
    private static final int BLOCK_SIZE = 8; // DES block size in bytes

    /** Table for PC2 permutations in key schedule computation. */
    private static final int[] SKB = new int[8 * 64];       // blank final

    /** Table for S-boxes and permutations. */
    private static final int SP_TRANS[] = new int[8 * 64];  // blank final

    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };


// Static code
//...........................................................................

    static {
        //
        // build the SKB table
        //
        // represent the bit number that each permutated bit is derived from
        // according to FIPS-46
        String cd =
            "D]PKESYM`UBJ\\@RXA`I[T`HC`LZQ"+"\\PB]TL`[C`JQ@Y`HSXDUIZRAM`EK";
        int j, s, bit;
        int count = 0;
        int offset = 0;
        for (int i = 0; i < cd.length(); i++) {
            s = cd.charAt(i) - '@';
            if (s != 32) {
                bit = 1 << count++;
                for (j = 0; j < 64; j++)
                    if ((bit & j) != 0) SKB[offset + j] |= 1 << s;
                if (count == 6) {
                    offset += 64;
                    count = 0;
                }
            }
        }
        //
        // build the SP_TRANS table
        //
        // The theory is that each bit position in each int of SP_TRANS is
        // set in exactly 32 entries. We keep track of set bits.
        //
        String spt = // [526 chars, 3156 bits]
            "g3H821:80:H03BA0@N1290BAA88::3112aIH8:8282@0@AH0:1W3A8P810@22;22"+
            "A18^@9H9@129:<8@822`?:@0@8PH2H81A19:G1@03403A0B1;:0@1g192:@919AA"+
            "0A109:W21492H@0051919811:215011139883942N8::3112A2:31981jM118::A"+
            "101@I88:1aN0<@030128:X;811`920:;H0310D1033@W980:8A4@804A3803o1A2"+
            "021B2:@1AH023GA:8:@81@@12092B:098042P@:0:A0HA9>1;289:@1804:40Ph="+
            "1:H0I0HP0408024bC9P8@I808A;@0@0PnH0::8:19J@818:@iF0398:8A9H0<13@"+
            "001@11<8;@82B01P0a2989B:0AY0912889bD0A1@B1A0A0AB033O91182440A9P8"+
            "@I80n@1I03@1J828212A`A8:12B1@19A9@9@8^B:0@H00<82AB030bB840821Q:8"+
            "310A302102::A1::20A1;8";
        offset = 0;
        int k, c, param;
        for (int i = 0; i < 32; i++) { // each bit position
            k = -1; // pretend the -1th bit was set
            bit = 1 << i;
            for (j = 0; j < 32; j++) { // each set bit
                // Each character consists of two three-bit values:
                c = spt.charAt(offset >> 1) - '0' >> (offset & 1) * 3 & 7;
                offset++;
                if (c < 5) {
                    // value 0-4 indicate a set bit 1-5 positions from
                    // previous set bit
                    k += c + 1;
                    SP_TRANS[k] |= bit;
                    continue;
                }
                // other values take at least an additional parameter:
                // the next value in the sequence.
                param = spt.charAt(offset >> 1) - '0' >> (offset & 1) * 3 & 7;
                offset++;
                if (c == 5) {
                    // bit set param+6 positions from previous set bit
                    k += param + 6;
                    SP_TRANS[k] |= bit;
                } else if (c == 6) {
                    // bit set param*64 + 1 positions from previous set bit
                    k += (param << 6) + 1;
                    SP_TRANS[k] |= bit;
                } else {
                    // should skip param*64 positions, then process next value
                    // which will be in the range 0-4.
                    k += param << 6;
                    j--;
                }
            }
        }
    }


// Implementation of Basic API methods
//...........................................................................

    /**
     * Expand a user-supplied key material into a session key.
     *
     * @param key  The 128/192/256-bit user-key to use.
     * @exception InvalidKeyException  If the key is invalid.
     */
    public static synchronized Object makeKey (byte[] k)
    throws InvalidKeyException {
        int i = 0;
        int L = (k[i++] & 0xFF)       | (k[i++] & 0xFF) <<  8 |
                (k[i++] & 0xFF) << 16 | (k[i++] & 0xFF) << 24;
        int R = (k[i++] & 0xFF)       | (k[i++] & 0xFF) <<  8 |
                (k[i++] & 0xFF) << 16 | (k[i++] & 0xFF) << 24;

        int t = ((R >>> 4) ^ L) & 0x0F0F0F0F;
        L ^= t;
        R ^= t << 4;
        t = ((L << 18) ^ L) & 0xCCCC0000;
        L ^= t ^ t >>> 18;        
        t = ((R << 18) ^ R) & 0xCCCC0000;
        R ^= t ^ t >>> 18;        
        t = ((R >>> 1) ^ L) & 0x55555555;
        L ^= t;
        R ^= t << 1;
        t = ((L >>> 8) ^ R) & 0x00FF00FF;
        R ^= t;
        L ^= t << 8;
        t = ((R >>> 1) ^ L) & 0x55555555;
        L ^= t;
        R ^= t << 1;

        R = (R & 0x000000FF) <<  16 |
            (R & 0x0000FF00)        |
            (R & 0x00FF0000) >>> 16 |
            (L & 0xF0000000) >>>  4;
        L &= 0x0FFFFFFF;

        int s;
        int j = 0;
        int[] sKey = new int[ROUNDS * 2];

        for (i = 0; i < ROUNDS; i++) {
            if ((0x7EFC >> i & 1) == 1) {
                L = (L >>> 2 | L << 26) & 0x0FFFFFFF;
                R = (R >>> 2 | R << 26) & 0x0FFFFFFF;
            } else {
                L = (L >>> 1 | L << 27) & 0x0FFFFFFF;
                R = (R >>> 1 | R << 27) & 0x0FFFFFFF;
            }
            s = SKB[           L         & 0x3F                        ] |
                SKB[0x040 | (((L >>>  6) & 0x03) | ((L >>>  7) & 0x3C))] |
                SKB[0x080 | (((L >>> 13) & 0x0F) | ((L >>> 14) & 0x30))] |
                SKB[0x0C0 | (((L >>> 20) & 0x01) | ((L >>> 21) & 0x06)
                                                 | ((L >>> 22) & 0x38))];
            t = SKB[0x100 | (  R         & 0x3F                       )] |
                SKB[0x140 | (((R >>>  7) & 0x03) | ((R >>>  8) & 0x3C))] |
                SKB[0x180 | ( (R >>> 15) & 0x3F                       )] |
                SKB[0x1C0 | (((R >>> 21) & 0x0F) | ((R >>> 22) & 0x30))];

            sKey[j++] = t <<  16 | (s & 0x0000FFFF);
            s         = s >>> 16 | (t & 0xFFFF0000);
            sKey[j++] = s <<   4 |  s >>> 28;
        }
        return sKey;
    }

    /**
     * Encrypt exactly one block of plaintext.
     *
     * @param in          The plaintext.
     * @param inOffset    Index of in from which to start considering data.
     * @param sessionKey  The session key to use for encryption.
     * @return The ciphertext generated from a plaintext using the session key.
     */
    public static byte[]
    blockEncrypt (byte[] in, int inOffset, Object sessionKey) {
        int[] L_R = {
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset  ] & 0xFF) << 24};

        IP(L_R);
        encrypt(L_R, sessionKey);
        FP(L_R);

        int L = L_R[0];
        int R = L_R[1];
        byte[] result = new byte[] {
            (byte) L, (byte)(L >>  8), (byte)(L >> 16), (byte)(L >> 24),
            (byte) R, (byte)(R >>  8), (byte)(R >> 16), (byte)(R >> 24)
        };
        return result;
    }

    /**
     * Decrypt exactly one block of ciphertext.
     *
     * @param in          The ciphertext.
     * @param inOffset    Index of in from which to start considering data.
     * @param sessionKey  The session key to use for decryption.
     * @return The plaintext generated from a ciphertext using the session key.
     */
    public static byte[]
    blockDecrypt (byte[] in, int inOffset, Object sessionKey) {
        int[] L_R = {
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset++] & 0xFF) << 24,
            (in[inOffset++] & 0xFF)       | (in[inOffset++] & 0xFF) <<  8 |
            (in[inOffset++] & 0xFF) << 16 | (in[inOffset  ] & 0xFF) << 24 };

        IP(L_R);
        decrypt(L_R, sessionKey);
        FP(L_R);

        int L = L_R[0];
        int R = L_R[1];
        byte[] result = new byte[] {
            (byte) L, (byte)(L >>  8), (byte)(L >> 16), (byte)(L >> 24),
            (byte) R, (byte)(R >>  8), (byte)(R >> 16), (byte)(R >> 24)
        };
        return result;
    }

    /**
     * Basic symmetric encryption/decryption and a reduced KAT set tests.
     *
     * @return True iff all tests pass.
     */
    public static boolean self_test() {
if (DEBUG) trace(IN, "self_test()");
        boolean ok = false;
        try {
            byte[] kb = new byte[BLOCK_SIZE];
            byte[] pt = new byte[BLOCK_SIZE];
            int i;

            for (i = 0; i < BLOCK_SIZE; i++) kb[i] = (byte) i;
            for (i = 0; i < BLOCK_SIZE; i++) pt[i] = (byte) i;

            Object key = makeKey(kb);

            byte[] ct =  blockEncrypt(pt, 0, key);
            byte[] tmp = blockDecrypt(ct, 0, key);

            ok = areEqual(pt, tmp);
            if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("       key: " + toString(kb));
    debug(" plaintext: " + toString(pt));
    debug("ciphertext: " + toString(ct));
    debug("  computed: " + toString(tmp));
}
                throw new RuntimeException(
                    "Symmetric operation in ECB mode failed");
            }
if (DEBUG && debuglevel > 0) debug("Symmetric operation in ECB mode OK? " + ok);

            String[][] kat = {                     // KAT for a reduced set
            // KEY,               PLAINTEXT,          CIPHERTEXT
            {"0101010101010101", "95f8a5e5dd31d900", "8000000000000000"},
            {"0101010101010101", "dd7f121ca5015619", "4000000000000000"},
            {"0101010101010101", "2e8653104f3834ea", "2000000000000000"},
            // outer's data
            {"0123456789abcdef", "0123456789abcde7", "c95744256a5ed31d"},
            {"0123456710325476", "89abcdef98badcfe", "f0148eff050b2716"},
            };
            
            for (i = 0; i < kat.length; ) {
                key = makeKey(fromString(kat[i][0]));
                pt = fromString(kat[i][1]);
                ct = fromString(kat[i][2]);
                i++;

                tmp = blockEncrypt(pt, 0, key);
                ok = areEqual(ct, tmp);
                if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("KAT triple #"+i);
    debug("       key: " + toString(kb));
    debug(" plaintext: " + toString(pt));
    debug("ciphertext: " + toString(ct));
    debug("  computed: " + toString(tmp));
}
                    throw new RuntimeException("ECB mode encryption #"+i+" failed");
                }
                tmp = blockDecrypt(ct, 0, key);
                ok = areEqual(pt, tmp);
                if (!ok) {
if (DEBUG && debuglevel > 0) {
    debug("KAT triple #"+i);
    debug("       key: " + toString(kb));
    debug("ciphertext: " + toString(ct));
    debug(" plaintext: " + toString(pt));
    debug("  computed: " + toString(tmp));
}
                    throw new RuntimeException("ECB mode decryption #"+i+" failed");
                }
            }
if (DEBUG && debuglevel > 0) debug("KAT (reduced set) in ECB mode OK? " + ok);
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

    /** @return the length in bytes of an input block for this cipher. */
    public static int engineBlockSize() { return BLOCK_SIZE; }

    /**
     * Encrypt exactly one block of plaintext without applying IP nor FP.
     *
     * @param io         The plaintext (on input) and ciphertext (on output)
     *                   (as an array of 32-bit entities).
     * @param sessionKey The session key to use for encryption.
     */
    public static final void encrypt (int[] io, Object sessionKey) {
        int[] sKey = (int[]) sessionKey;
        int L = io[0];
        int R = io[1];

        int u = R << 1 | R >>> 31;
        R = L << 1 | L >>> 31;
        L = u;
        int t;
        int n = sKey.length;
        for (int i = 0; i < n; ) {
            u = R ^ sKey[i++];
            t = R ^ sKey[i++];
            t = t >>> 4 | t << 28;
            L ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                  SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                  SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                  SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                  SP_TRANS[          u         & 0x3F ] |
                  SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                  SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                  SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);

            u = L ^ sKey[i++];
            t = L ^ sKey[i++];
            t = t >>> 4 | t << 28;
            R ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                  SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                  SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                  SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                  SP_TRANS[          u         & 0x3F ] |
                  SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                  SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                  SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);
        }
        io[0] = L >>> 1 | L << 31;
        io[1] = R >>> 1 | R << 31;
    }

    /**
     * Encrypt in situ exactly one block of plaintext without applying IP
     * nor FP.
     *
     * @param io         The plaintext (on input) and ciphertext (on output)
     *                   (as an array of 8-bit entities).
     * @param sessionKey The session key to use for encryption.
     */
    public static final void encrypt (byte[] io, Object sessionKey) {
        int[] L_R = { (io[0] & 0xFF)       | (io[1] & 0xFF) <<  8 |
                      (io[2] & 0xFF) << 16 | (io[3] & 0xFF) << 24,
                      (io[4] & 0xFF)       | (io[5] & 0xFF) <<  8 |
                      (io[6] & 0xFF) << 16 | (io[7] & 0xFF) << 24 };

        encrypt(L_R, sessionKey);

        int L = L_R[0];
        int R = L_R[1];
        io[0] = (byte) L;
        io[1] = (byte)(L >>  8);
        io[2] = (byte)(L >> 16);
        io[3] = (byte)(L >> 24);
        io[4] = (byte) R;
        io[5] = (byte)(R >>  8);
        io[6] = (byte)(R >> 16);
        io[7] = (byte)(R >> 24);
    }

    /**
     * Decrypt in situ exactly one block of ciphertext without applying IP
     * nor FP.
     *
     * @param io         The ciphertext (on input) and plaintext (on output)
     *                   (as an array of 32-bit entities).
     * @param sessionKey The session key to use for decryption.
     */
    public static final void decrypt (int[] io, Object sessionKey) {
        int[] sKey = (int[]) sessionKey;
        int L = io[0];
        int R = io[1];
        int u = R << 1 | R >>> 31;
        R = L << 1 | L >>> 31;
        L = u;
        int t;
        int n = sKey.length;
        for (int i = n - 1; i > 0; ) {
            t = R ^ sKey[i--];
            u = R ^ sKey[i--];
            t = t >>> 4 | t << 28;
            L ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                  SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                  SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                  SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                  SP_TRANS[          u         & 0x3F ] |
                  SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                  SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                  SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);

            t = L ^ sKey[i--];
            u = L ^ sKey[i--];
            t = t >>> 4 | t << 28;
            R ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                  SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                  SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                  SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                  SP_TRANS[          u         & 0x3F ] |
                  SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                  SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                  SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);
        }
        io[0] = L >>> 1 | L << 31;
        io[1] = R >>> 1 | R << 31;
    }

    /**
     * Decrypt exactly one block of ciphertext without applying IP nor FP.
     *
     * @param io         The ciphertext (on input) and plaintext (on output)
     *                   (as an array of 8-bit entities).
     * @param sessionKey The session key to use for decryption.
     */
    public static final void decrypt (byte[] io, Object sessionKey) {
        int[] L_R = { (io[0] & 0xFF)       | (io[1] & 0xFF) <<  8 |
                      (io[2] & 0xFF) << 16 | (io[3] & 0xFF) << 24,
                      (io[4] & 0xFF)       | (io[5] & 0xFF) <<  8 |
                      (io[6] & 0xFF) << 16 | (io[7] & 0xFF) << 24 };

        decrypt(L_R, sessionKey);

        int L = L_R[0];
        int R = L_R[1];
        io[0] = (byte) L;
        io[1] = (byte)(L >>  8);
        io[2] = (byte)(L >> 16);
        io[3] = (byte)(L >> 24);
        io[4] = (byte) R;
        io[5] = (byte)(R >>  8);
        io[6] = (byte)(R >> 16);
        io[7] = (byte)(R >> 24);
    }

    /**
     * Apply in situ the DES IP trnasformation to a DES block.
     *
     * @param io  DES block as an array of 32-bit entities.
     */
    public static final void IP (int[] io) {
        int L = io[0];
        int R = io[1];

        int t = ((R >>> 4) ^ L) & 0x0F0F0F0F;
        L ^= t;
        R ^= t << 4;
        t = ((L >>> 16) ^ R) & 0x0000FFFF;
        R ^= t;
        L ^= t << 16;
        t = ((R >>> 2) ^ L) & 0x33333333;
        L ^= t;
        R ^= t << 2;
        t = ((L >>> 8) ^ R) & 0x00FF00FF;
        R ^= t;
        L ^= t << 8;
        t = ((R >>> 1) ^ L) & 0x55555555;

        io[0] = L ^ t;
        io[1] = R ^ (t << 1);
    }

    /**
     * Apply in situ the DES IP trnasformation to a DES block.
     *
     * @param io  DES block as an array of 8-bit entities.
     */
    public static final void IP (byte[] io) {
        int[] L_R = { (io[0] & 0xFF)       | (io[1] & 0xFF) <<  8 |
                      (io[2] & 0xFF) << 16 | (io[3] & 0xFF) << 24,
                      (io[4] & 0xFF)       | (io[5] & 0xFF) <<  8 |
                      (io[6] & 0xFF) << 16 | (io[7] & 0xFF) << 24 };
        IP(L_R);

        int L = L_R[0];
        int R = L_R[1];
        io[0] = (byte) L;
        io[1] = (byte)(L >>  8);
        io[2] = (byte)(L >> 16);
        io[3] = (byte)(L >> 24);
        io[4] = (byte) R;
        io[5] = (byte)(R >>  8);
        io[6] = (byte)(R >> 16);
        io[7] = (byte)(R >> 24);
    }

    /**
     * Apply in situ the DES FP trnasformation (IP<sup>-1</sup>) to a DES
     * block.
     *
     * @param io  DES block as an array of 32-bit entities.
     */
    public static final void FP (int[] io) {
        int L = io[0];
        int R = io[1];

        int t = (R >>> 1 ^ L) & 0x55555555;
        L ^= t;
        R ^= t << 1;
        t = (L >>> 8 ^ R) & 0x00FF00FF;
        R ^= t;
        L ^= t << 8;
        t = (R >>> 2 ^ L) & 0x33333333;
        L ^= t;
        R ^= t << 2;
        t = (L >>> 16 ^ R) & 0x0000FFFF;
        R ^= t;
        L ^= t << 16;
        t = (R >>> 4 ^ L) & 0x0F0F0F0F;

        io[0] = L ^ t;
        io[1] = R ^ (t << 4);
    }

    /**
     * Apply in situ the DES FP trnasformation (IP<sup>-1</sup>) to a DES
     * block.
     *
     * @param io  DES block as an array of 32-bit entities.
     */
    public static final void FP (byte[] io) {
        int[] L_R = { (io[0] & 0xFF)       | (io[1] & 0xFF) <<  8 |
                      (io[2] & 0xFF) << 16 | (io[3] & 0xFF) << 24,
                      (io[4] & 0xFF)       | (io[5] & 0xFF) <<  8 |
                      (io[6] & 0xFF) << 16 | (io[7] & 0xFF) << 24 };
        FP(L_R);

        int L = L_R[0];
        int R = L_R[1];
        io[0] = (byte) L;
        io[1] = (byte)(L >>  8);
        io[2] = (byte)(L >> 16);
        io[3] = (byte)(L >> 24);
        io[4] = (byte) R;
        io[5] = (byte)(R >>  8);
        io[6] = (byte)(R >> 16);
        io[7] = (byte)(R >> 24);
    }

    /**
     * Implements the Unix crypt(3) algorithm.<p>
     *
     * @param L0          Left 32 bits of a DES block.
     * @param R0          Right 32 bits of a DES block.
     * @param sessionKey  The session key to use for decryption.
     * @return The encrypted DES block.
     */
    public static int[] crypt3 (int L0, int R0, Object sessionKey) {
        int[] sKey = (int[]) sessionKey;
        int L = 0;
        int R = 0;
        int t, u, v;
        int n = sKey.length;

        for (int i = 0; i < 25; i++) {
            for (int j = 0; j < n; ) {
                v = R ^ (R >>> 16);
                u = v & L0;
                v &= R0;
                u ^= (u << 16) ^ R ^ sKey[j++];
                t = v ^ (v << 16) ^ R ^ sKey[j++];
                t = t >>> 4 | t << 28;
                L ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                      SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                      SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                      SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                      SP_TRANS[          u         & 0x3F ] |
                      SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                      SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                      SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);
                    
                v = L ^ (L >>> 16);
                u = v & L0;
                v &= R0;
                u ^= (u << 16) ^ L ^ sKey[j++];
                t = v ^ (v << 16) ^ L ^ sKey[j++];
                t = t >>> 4 | t << 28;
                R ^= (SP_TRANS[0x040 | ( t         & 0x3F)] |
                      SP_TRANS[0x0C0 | ((t >>>  8) & 0x3F)] |
                      SP_TRANS[0x140 | ((t >>> 16) & 0x3F)] |
                      SP_TRANS[0x1C0 | ((t >>> 24) & 0x3F)] |
                      SP_TRANS[          u         & 0x3F ] |
                      SP_TRANS[0x080 | ((u >>>  8) & 0x3F)] |
                      SP_TRANS[0x100 | ((u >>> 16) & 0x3F)] |
                      SP_TRANS[0x180 | ((u >>> 24) & 0x3F)]);
            }
            t = L; L = R; R = t;
        }
        int[] result = { R >>> 1 | R << 31, L >>> 1 | L << 31 };
        FP(result);
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

    /** Returns a byte array from a string of hexadecimal digits. */
    private static final byte[] fromString (String hex) {
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
    public static final int fromDigit (char ch) {
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
}

/* --------------------------------------------------------------------------
 * Parts of this code (in particular, the string representing SP_TRANS)
 * are Copyright (C) 1995 Eric Young (eay@mincom.oz.au). All rights reserved.
 *
 * Its use is FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are adhered to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * --------------------------------------------------------------------------
 * The DES cipher (in ECB mode).
 *
 * Parts of this code Copyright (C) 1996 Geoffrey Keating. All rights reserved.
 *
 * Its use is FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are adhered to.
 * 
 * Copyright remains Geoffrey Keating's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Geoffrey Keating should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Geoffrey Keating
 *    (geoffk@discus.anu.edu.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY GEOFFREY KEATING ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
