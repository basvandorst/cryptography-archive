// $Id: Blowfish.java,v 1.7 1998/01/11 23:18:11 iang Exp $
//
// $Log: Blowfish.java,v $
// Revision 1.7  1998/01/11 23:18:11  iang
// made all the exceptions exit (1)
//
// Revision 1.6  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.5  1997/11/28 03:11:23  iang
// + Added a main as tests to the algorithm classes.
// + Changed glue code to getInstance().
//
// Revision 1.4  1997/11/28 01:20:13  iang
// + Added David's change of getInstance().
//
// Revision 1.3  1997/11/26 08:49:22  iang
// + Added main() for test.
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
// + Split from cryptix.provider.cipher.Blowfish. This class now supports only
//   the old Cryptix 2.2 API, and is deprecated.
// + Refer to the history of cryptix.provider.cipher.Blowfish for previous changes.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import java.security.Cipher;

/**
 * This class implements the Cryptix V2.2 API for the Blowfish block cipher.
 * New code should use the JCA implementation of Blowfish, which can be obtained
 * by calling <code>Cipher.getInstance("Blowfish")</code>, for example.
 * <p>
 * Blowfish was designed by <a href="mailto:schneier@counterpane.com">Bruce
 * Schneier</a>. The algorithm is in the public domain.
 * <p>
 * This implementation throws a CryptoError if the user key length is out of range,
 * or if any one of the two self-tests fail. The self-tests are as follows:
 * <ol>
 *   <li> encrypt 0 ten times using the initial S boxes and P values. Check the
 *        result against a known value. If equal decrypt the result 10 times and
 *        compare it to 0.
 *   <li> complete the key expansion process and use the newly formed key to
 *        encrypt 10 times a block of all zeroes; decrypt the result 10 times and
 *        compare.
 * </ol>
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Bruce Schneier,
 *        "Section 14.3 Blowfish,"
 *        <cite>Applied Cryptography, 2nd edition</cite>,
 *        John Wiley &amp; Sons, 1996
 *        <p>
 *   <li> Bruce Schneier,
 *        "Description of a New Variable-Length Key, 64-Bit Cipher (Blowfish),"
 *        <cite>Fast Software Encryption Cambridge Security Workshop Proceedings</cite>,
 *        Springer-Verlag, 1004, pp 191-204.
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.7 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2
 * @deprecated
 * @see java.security.Cipher#getInstance
 */
public class Blowfish
extends BlockCipher
{

// Constants
//...........................................................................

    /**
     * The length of a block (8 bytes for Blowfish).
     */
    public static final int BLOCK_LENGTH = 8;
    public static final int BLOCK_LENGTH_MIN = 8;
    public static final int BLOCK_LENGTH_MAX = 8;

    /**
     * The default length of a user key (16 bytes for Blowfish).
     */
    public static final int KEY_LENGTH = 16;
    public static final int KEY_LENGTH_MAX = 448 / 8;
    public static final int KEY_LENGTH_MIN = 40 / 8;
    public static final int KEY_LENGTH_MOD = 1;

    /**
     * The 2.2 API only supports the default number of rounds, 16.
     */
    public static final int NUM_ROUNDS = 16;


// Constructor
//............................................................................

    /**
     * This creates a Blowfish block cipher instance, with key data taken
     * from a byte array of a user-defined length.
     *
     * @param  userKey  the user key.
     * @exception CryptoError if an error occurs initialising the cipher.
     */
    public Blowfish(byte[] userKey) {
        super(getCipherImpl(), userKey);
    }

    private static Cipher getCipherImpl() {
        try { return Cipher.getInstance("Blowfish", "Cryptix"); }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }


// Test methods
//...........................................................................
//
// Don't expand this code please without thinking about it,
// much better to write a separate program.
//

    /**
     * Entry point for very basic <code>self_test</code>.
     */
    public static void main(String argv[]) 
    {
        try { self_test(); }
        catch (Exception e) { e.printStackTrace(); System.exit(1); }
    }

    /**
     * This is (apparently) the official certification data.
     * Use ints as Java grumbles about negative hex values.
     */
    static final private byte[][][] tests =
    {
      { // cert 1
        { // key
          97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
          111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
        },
        { 66, 76, 79, 87, 70, 73, 83, 72, },   // plain
        { 50, 78, -48, -2, -12, 19, -94, 3, }, // cipher
      },
      { // cert 2
        {
          87, 104, 111, 32, 105, 115, 32, 74, 111, 104, 110, 32, 71, 97, 108,
          116, 63,
        },
        { -2, -36, -70, -104, 118, 84, 50, 16, },
        { -52, -111, 115, 43, -128, 34, -10, -124, },
      },
      { // cert 3 - Ayn Rand with 4th ( ) and 8th (d) chars hi bit set.
        { 65, 121, 110, -96, 82, 97, 110, -28, }, // key
        { -2, -36, -70, -104, 118, 84, 50, 16, }, // plain
        { -31, 19, -12, 16, 44, -4, -50, 67, },   // cipher
      },
    };

    /**
     * Do some basic tests.
     * Three of the certification data are included only, no output,
     * success or exception.
     * If you want more, write a test program!
     * @see cryptix.examples.TestBlowfish
     */
    public static void self_test() throws CryptoError
    {
        for (int i = 0; i < tests.length; i++)
        {
            byte[][] test = tests[i];
            byte[] key    = test[0];
            byte[] plain  = test[1];
            byte[] cipher = test[2];

            Blowfish cryptor = new Blowfish(key);
            byte[] tmp = new byte[cryptor.blockLength()];

            cryptor.encrypt(plain, tmp);
            if (notEquals(cipher, tmp))
                throw new CryptoError("encrypt #"+ i +" failed");
            cryptor.decrypt(cipher, tmp);
            if (notEquals(plain, tmp))
                throw new CryptoError("decrypt #"+ i +" failed");
        }
    }

    private static boolean notEquals(byte[] a, byte[] b)
    {
        for (int i = 0; i < a.length; i++)
        {
            if (a[i] != b[i])
                return true;
            // if (a[i] == -4) // force a trip up
            //  return true;
        }
        return false;
    }
}
