// $Log: IDEA.java,v $
// Revision 1.5  1998/01/11 23:18:12  iang
// made all the exceptions exit (1)
//
// Revision 1.4  1997/12/01 00:56:54  iang
// minor typos  copied in from other archive,
// still one bug left in SPEED.java
//
// Revision 1.3  1997/11/28 03:21:26  iang
// added (really this time) the getInstance() mod
//
// Revision 1.2  1997/11/28 03:11:24  iang
// added a main as tests to the algorithm classes
// changed glue code to getInstance()
//
// Revision 1.1.1.1  1997/11/03 22:36:56  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.4.0.0  1997/08/15  David Hopwood
// + Split from cryptix.provider.cipher.IDEA. This class now supports only
//   the old Cryptix 2.2 API, and is deprecated.
// + Refer to the history of cryptix.provider.cipher.IDEA for previous changes.
//
// $Endlog$
/*
 * Copyright (c) 1995-97 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

import java.security.Cipher;

/**
 * This class implements the Cryptix V2.2 API for the IDEA block cipher.
 * New code should use the JCA implementation of IDEA, which can be obtained
 * by calling <code>Cipher.getInstance("IDEA")</code>, for example.
 * <p>
 * IDEA is a block cipher with a key length of 16 bytes and a block length of
 * 8 bytes. It is highly popular, being the original cipher in PGP, and has
 * received a lot of cryptanalytic attention.
 * <p>
 * IDEA was written by Dr. X. Lai and Prof. J. Massey.
 * <p>
 * <b>References</b>
 * <p>
 * <blockquote>
 *    See the <a href="http://www.ascom.ch/Web/systec/security/idea.htm">IDEA page</a>
 *    for more details.
 *    <p>
 *    The algorithm is subject to patent claims by
 *    <a href="http://www.ascom.ch/systec/">Ascom Systec Ltd</a>
 *    (applied for May 1991), and is
 *    <a href="http://www.ascom.ch/Web/systec/policy/normal/policy.html">licensable</a>.
 * </blockquote>
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.5 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see java.security.Cipher#getInstance
 */
public class IDEA
extends BlockCipher
{

// Constants
//............................................................................

    /**
     * The length of a block (8 bytes for IDEA).
     */
    public static final int BLOCK_LENGTH = 8;

    /**
     * The length of a user key (16 bytes for IDEA).
     */
    public static final int KEY_LENGTH = 16;


// Constructor
//............................................................................

    /**
     * This creates an IDEA block cipher instance, with key data taken
     * from a 16-byte array.
     *
     * @param  userKey  the user key.
     * @exception CryptoError if an error occurs initializing the cipher.
     */
    public IDEA(byte[] userKey) {
        super(getCipherImpl(), userKey);
    }

    private static Cipher getCipherImpl() {
        try { return Cipher.getInstance("IDEA", "Cryptix"); }
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
// This is (apparently) the official certification data.
// Use ints as Java grumbles about negative hex values.
//
static final private byte[][][] tests =
{
  { // cert 1
    { 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, }, // key
    { 0, 0, 0, 1, 0, 2, 0, 3, },                         // plain
    { 17, -5, -19, 43, 1, -104, 109, -27, },             // cipher
  },
  { // cert 8
    { 58, -104, 78, 32, 0, 25, 93, -77, 46, -27, 1, -56, -60, 124, -22, 96, },
    { 1, 2, 3, 4, 5, 6, 7, 8, },
    { -105, -68, -40, 32, 7, -128, -38, -122, },
  },
  { // cert 9
    { 0, 100, 0, -56, 1, 44, 1, -112, 1, -12, 2, 88, 2, -68, 3, 32, }, // key
    { 5, 50, 10, 100, 20, -56, 25, -6, },          // plain
    { 101, -66, -121, -25, -94, 83, -118, -19, },   // cipher
  },
};

/**
 * Do some basic tests.
 * Three of the certification data are included only, no output,
 * success or exception.
 * If you want more, write a test program!
 * @see cryptix.examples.IDEA
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

        IDEA cryptor = new IDEA(key);
        byte[] tmp = new byte[cryptor.blockLength()];

        cryptor.encrypt(plain, tmp);
        if (notEquals(cipher, tmp))
            throw new CryptoError("encrypt #"+ i +" failed");
        cryptor.decrypt(cipher, tmp);
        if (notEquals(plain, tmp))
            throw new CryptoError("decrypt #"+ i +" failed");
    }
}

private static final boolean
notEquals(byte[] a, byte[] b)
{
    for (int i = 0; i < a.length; i++)
    {
        if (a[i] != b[i])
            return true;
        // if (a[i] == 83) // force a trip up
        //     return true;
    }
    return false;
}

} /* class */
