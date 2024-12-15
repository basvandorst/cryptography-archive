// $Id: SPEED.java,v 1.8 1998/01/11 23:18:12 iang Exp $
//
// $Log: SPEED.java,v $
// Revision 1.8  1998/01/11 23:18:12  iang
// made all the exceptions exit (1)
//
// Revision 1.7  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.6.1  1997/12/06  hopwood
// + Added missing cast to cryptix.provider.cipher.SPEED before setting
//   parameters.
// + Formatting changes.
//
// Revision 1.6  1997/12/06 13:01:00  iang
// + Dropped old lock comment.
//
// Revision 1.5  1997/12/01 00:56:54  iang
// + Minor typos.
// + Copied in tests from other archive, still one bug left in SPEED.java
//
// Revision 1.4  1997/11/28 03:21:26  iang
// + Added (really this time) the getInstance() mod.
//
// Revision 1.3  1997/11/28 03:11:25  iang
// + Added a main as tests to the algorithm classes.
// + Changed glue code to getInstance().
//
// Revision 1.2  1997/11/20 22:46:06  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1.1  1997/11/14  David Hopwood
// + Fixed imports.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.4.0.0  1997/08/15  David Hopwood
// + Split from cryptix.provider.cipher.SPEED. This class now supports only
//   the old Cryptix 2.2 API, and is deprecated.
// + Refer to the history of cryptix.provider.cipher.SPEED for previous changes.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import java.security.Cipher;

/**
 * This class implements the Cryptix V2.2 API for the SPEED block cipher.
 * New code should use the JCA implementation of SPEED, which can be obtained
 * by calling <code>Cipher.getInstance("SPEED")</code>, for example.
 * <p>
 * SPEED is a block cipher with variable key size, data block size and number
 * of rounds (in the style of RC5).
 * <P>
 * This implementation sets the key schedule and all variables when the
 * cipher is initialized:
 * <ul>
 *   <li>
 *     The key size is taken from the array passed to the constructor, and
 *     can be any even number of bytes from 6 to 32 (6, 8, 10, ...32).
 *   <li>
 *     The length of the data block defaults to 8 bytes (c.f., IDEA and DES).
 *     It can be set by calling the overloaded constructor, and can be
 *     8, 16, or 32 bytes.
 *   <li>
 *     The number of rounds can be any number from 32 upwards, divisible by
 *     4 (32, 36, ...).
 *     It defaults to 64 (which gives 'adequate' security, see paper below)
 *     and is also set by calling the overloaded constructor.
 * </ul>
 * <p>
 * SPEED was designed by <a href="mailto:yzheng@fcit.monash.edu.au">Yuliang Zheng</a>,
 * and is in the public domain.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> <a href="http://pscit-www.fcit.monash.edu.au/~yuliang/">Y. Zheng</a>
 *        "The SPEED Cipher,"
 *        <cite>Proceedings of Financial Cryptography 97</cite>,
 *        Springer-Verlag (forthcoming).
 *        FC97 held at Anguilla, BWI, 24-28 February 1997.
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.8 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see java.security.Cipher#getInstance
 */
public class SPEED
extends BlockCipher
{

// Constants, and methods returning constants
//...........................................................................

    /**
     * Maximum key length for SPEED is 256 bits.
     */
    private static final int KEY_LEN_MAX = 256 / 8;

    /**
     * Minimum key length for SPEED is 48 bits.
     */
    private static final int KEY_LEN_MIN = 48 / 8;

    /**
     * Returns the maximum block length.
     * @return the maximum block length in bytes.
     */
    public static final int blockLengthMax() { return 32; }

    /**
     * Returns the minimum block length.
     * @return the minimum block length in bytes.
     */
    public static final int blockLengthMin() { return 8; }

    /**
     * Returns the modulo block length.
     * @return the modulo block length in bytes.
     */
    public static final int blockLengthMod() { return 8; }

    /**
     * Returns the maximum key length.
     * @return the maximum key length in bytes.
     */
    public static final int keyLengthMax() { return KEY_LEN_MAX; }

    /**
     * Returns the minimum key length.
     * @return the minimum key length in bytes.
     */
    public static final int keyLengthMin() { return KEY_LEN_MIN; }

    /**
     * Returns the modulo key length.
     * @return the modulo key length in bytes.
     */
    public static final int keyLengthMod() { return 2; }

    /**
     * Returns the maximum number of rounds.
     * @return the maximum number of rounds.
     */
    public static final int numRoundsMax() { return (int)(0x090000000 - 4); }

    /**
     * Returns the minimum number of rounds.
     * @return the minimum number of rounds.
     */
    public static final int numRoundsMin() { return 32; }

    /**
     * Returns the modulo number of rounds.
     * @return the modulo number of rounds.
     */
    public static final int numRoundsMod() { return 4; }


// Extra V2.2 method
//............................................................................

    /**
     * Returns the number of rounds for this cipher.
     * @return the number of rounds.
     */
    public final int numRounds() {
        return ((cryptix.provider.cipher.SPEED) cipher).getRounds();
    }


// Constructors
//............................................................................

    /**
     * This creates a SPEED block cipher instance, with key data taken
     * from a byte array of a user-defined length. The key length should
     * be an even number between 6 and 32.
     * <p>
     * Default block size (8) and number of rounds (64) are set
     * to approximate IDEA or DES, allowing for drop-in replacement.
     *
     * @param  userKey  the user key.
     * @exception CryptoError if an error occurs initializing the cipher.
     */
    public SPEED(byte[] userKey) {
        super(getCipherImpl(8, 64), userKey);
    }

    /**
     * This creates a SPEED block cipher instance, with key data taken
     * from a byte array of a user-defined length, and with a specified
     * block size and number of rounds. The key length should be an even
     * number between 6 and 32.
     * <p>
     * These are recommended settings for 'adequate' security:
     * <pre>
     *    +--------------------------------------------------+
     *    |   block size   |   key length   |     rounds     |
     *    |==================================================|
     *    |        8       |      >= 8      |     >= 64      |
     *    |--------------------------------------------------|
     *    |       16       |      >= 8      |     >= 48      |
     *    |--------------------------------------------------|
     *    |       32       |      >= 8      |     >= 48      |
     *    +--------------------------------------------------+
     * </pre>
     * <p>
     * Also see the paper referenced above.
     *
     * @param  userKey      the user key, length can be 6, 8, ... 32 bytes
     * @param  blockSize    the block size in bytes: 8, 16 or 32
     * @param  rounds       the number of rounds: >= 32, multiple of 4
     */
    public SPEED(byte[] userKey, int blockSize, int rounds) {
        super(getCipherImpl(blockSize, rounds), userKey);
    }

    private static Cipher getCipherImpl(int blockSize, int rounds) {
        try {
            cryptix.provider.cipher.SPEED cipher =
                (cryptix.provider.cipher.SPEED) Cipher.getInstance("SPEED", "Cryptix");
            cipher.setBlockSize(blockSize);
            cipher.setRounds(rounds);
            return cipher;
        } catch (Exception e) { throw new CryptoError(e.toString()); }
    }


// Test methods
//............................................................................

    /**
     * Entry point for <code>self_test</code>.
     */
    public static void main(String args[])
    {
        try { self_test(); }
        catch (Exception e) { e.printStackTrace(); System.exit(1); }
    }

    //
    // Using Hex format 0xAB results in needing casts.
    // Note that all this data is reversed - read from right to left,
    // if comparing with the paper.
    //
    private static byte[][] keys =
    {
      { 0, 0, 0, 0, 0, 0, 0, 0, },
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
      {
        65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
      },
    };

    private static byte[][] plains =
    {
      { 0, 0, 0, 0, 0, 0, 0, 0, },
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
      {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
      }
    };

    private static byte[][] ciphers =
    {
      { 109, -123, 38, -68, 25, -128, 0, 46 },
      { -24, -60, 91, -111, 22, -40, 84, -85, 113, 21, 23, -61, -71, -28, 19, 108,},
      {
        104, 16, 49, -70, -52, 49, -79, 8, -41, 97, 107, 41, -118, 85, -86, 63,
        27, -20, 63, 105, 116, 21, 78, 67, 71, 104, 98, -102, -6, 108, -31, 61,
      },
    };

    private static int[][] params =
    {
      { 8, 64 },  // bytes in block, num rounds
      { 16, 128, },
      { 32, 256 },
    };

    /**
     * Do some basic tests.
     * Three of the certification data are included only, no output,
     * success or exception.
     * If you want more, write a test program!
     * @see cryptix.examples.TestSPEED
     */
    public static void self_test() throws CryptoError
    {
        for (int i = 0; i < keys.length; i++)
        {
            SPEED cryptor = new SPEED(keys[i], params[i][0], params[i][1]);
            byte[] tmp = new byte[plains[i].length];
            cryptor.encrypt(plains[i], tmp);
            if (notEquals(ciphers[i], tmp))
                throw new CryptoError("encrypt #"+ i +" failed");
            cryptor.decrypt(ciphers[i], tmp);
            if (notEquals(plains[i], tmp))
                throw new CryptoError("decrypt #"+ i +" failed");
        }
    }

    private static boolean notEquals(byte[] a, byte[] b)
    {
        for (int i = 0; i < a.length; i++)
        {
            if (a[i] != b[i])
               return true;
            // if (a[i] == 88) // force a trip up
            //  return true;
        }
        return false;
    }
}
