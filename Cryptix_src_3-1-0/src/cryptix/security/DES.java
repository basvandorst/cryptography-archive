// This file is currently unlocked (change this line if you lock the file)
//
// $Log: DES.java,v $
// Revision 1.4  1998/01/11 23:18:11  iang
// made all the exceptions exit (1)
//
// Revision 1.3  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.2  1997/11/28 03:11:24  iang
// added a main as tests to the algorithm classes
// changed glue code to getInstance()
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.4.0.0  1997/08/15  David Hopwood
// + Split from cryptix.provider.cipher.DES. This class now supports only
//   the old Cryptix 2.2 API, and is deprecated.
// + Refer to the history of cryptix.provider.cipher.DES for previous changes.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import java.security.Cipher;

/**
 * This class implements the Cryptix V2.2 API for the DES block cipher.
 * New code should use the JCA implementation of DES, which can be obtained
 * by calling <code>Cipher.getInstance("DES")</code>, for example.
 * <p>
 * DES is a block cipher with an 8 byte block size. The key length
 * is 8 bytes, but only 56 bits are used as the parity bit in each
 * byte is ignored.
 * <p>
 * This algorithm has been seriously analysed over the last 30 years,
 * and no significant weaknesses have been reported. Its only known
 * flaw is that the key length of 56 bits makes it relatively easy to
 * brute-force it.
 * <p>
 * To overcome this near-fatal flaw, it is recommended that DES be
 * used in Triple DES mode. The JCA algorithm name for the recommended
 * form of Triple DES is "DES-EDE3/CBC", which is implemented by the
 * <code><a href="cryptix.provider.cipher.DES_EDE3.html">DES_EDE3</a></code>
 * and <code><a href="cryptix.provider.mode.CBC.html">CBC</a></code> classes.
 * <p>
 * DES was written by IBM and first released in 1976. The algorithm is
 * freely usable.
 * <p>
 * <b>References</b>
 * <p>
 * <blockquote>
 *    <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *    "Chapter 12 Data Encryption Standard,"
 *    <cite>Applied Cryptography</cite>,
 *    Wiley 2nd Ed, 1996
 * </blockquote>
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.4 $</b>
 * @author  Systemics Ltd
 * @author  Geoffrey Keating
 * @author  Eric Young
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @author  John F. Dumas
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see java.security.Cipher#getInstance
 */
public class DES
extends BlockCipher
{

// Constants
//...........................................................................

    /**
     * The length of a block (8 bytes for DES).
     */
    public static final int BLOCK_LENGTH = 8;

    /**
     * The length of a user key (8 bytes for DES).
     */
    public static final int KEY_LENGTH = 8;


// Constructors
//............................................................................

    /**
     * This creates a DES block cipher instance, with key data taken
     * from a byte array of a user-defined length.
     *
     * @param  userKey  the user key.
     * @exception CryptoError if an error occurs initializing the cipher.
     */
    public DES(byte[] userKey) {
        super(getCipherImpl(), userKey);
    }

    private static Cipher getCipherImpl() {
        try { return Cipher.getInstance("DES", "Cryptix"); }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }
// // // // // // // // //  T E S T  // // // // // // // // // // // // 

    //
    // Don't expand this code please without thinking about it,
    // much better to write a separate program.
    //
    /**
     * Entry point for very basic <code>self_test</code>.
     */
    public static final void
    main(String argv[]) 
    {
        try { self_test(); }
        catch(Throwable t) { t.printStackTrace(); System.exit(1); }
    }


//
// Not sure if these are official, probably not.
// Use ints as Java grumbles about negative hex values.
//
static final private byte[][][] tests =
{
  {
    { 1, 1, 1, 1, 1, 1, 1, 1, },
    { -107, -8, -91, -27, -35, 49, -39, 0, },
    {  -128, 0, 0, 0, 0, 0, 0, 0, },
  },
  {
    { 1, 1, 1, 1, 1, 1, 1, 2, },
    { 0, 0, 0, 0, 0, 0, 0, 0, },
    { -122, -98, -3, 127, -97, 38, 90, 9, },
  },
  {
    { 56, 73, 103, 76, 38, 2, 49, -98, },
    { 81, 69, 75, 88, 45, -33, 68, 10, },
    { 113, 120, -121, 110, 1, -15, -101, 42, },
  },
};

/**
 * Do some basic tests.
 * Three of the certification data are included only, no output,
 * success or exception.
 * If you want more, write a test program!
 * @see cryptix.examples.DES
 */
public static final void
self_test()
    throws CryptoError
{
    for (int i = 0; i < tests.length; i++)
    {
        byte[][] test = tests[i];
        byte[] key    = test[0];
        byte[] plain  = test[1];
        byte[] cipher = test[2];

        DES cryptor = new DES(key);
        byte[] tmp = new byte[cryptor.blockLength()];

        cryptor.encrypt(plain, tmp);
        if (notEquals(cipher, tmp))
            throw new CryptoError("encrypt #"+ i +" failed");
        cryptor.decrypt(cipher, tmp);
        if (notEquals(plain, tmp))
            throw new CryptoError("decrypt #"+ i +" failed");
    }
}
private static boolean
notEquals(byte[] a, byte[] b)
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
