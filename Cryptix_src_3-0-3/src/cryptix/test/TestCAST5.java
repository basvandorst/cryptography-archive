// $Id: TestCAST5.java,v 1.3 1997/11/22 07:05:40 raif Exp $
//
// $Log: TestCAST5.java,v $
// Revision 1.3  1997/11/22 07:05:40  raif
// *** empty log message ***
//
// Revision 1.2  1997/11/22 05:59:02  iang
// core.util ==> util.core
//
// Revision 1.1  1997/11/07 05:53:25  raif
// + adapted to latest API.
//
// 1.0.1         15 Aug 1997     R. Naffah
// + use new cryptix.util classes.
// + more compact.
//
// 1.0.0           June 1997     R. Naffah
// + Original version.
//
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */
package cryptix.test;

import cryptix.util.core.ArrayUtil;
import cryptix.util.core.Hex;
import cryptix.provider.key.RawSecretKey;

import java.security.Cipher;
import java.util.Date;

/**
 * Tests the output of the CAST5 (CAST-128) cipher algorithm implementation
 * against certified pre-computed output for a given set of reference input.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  Raif S. Naffah
 */
public final class TestCAST5
{
// Variables and constants
//...........................................................................

    private static Cipher alg;
    private static final byte[] input = Hex.fromString("0123456789ABCDEF");


// Constructor
//...........................................................................
    
    private TestCAST5 () {
        System.out.println("*** CAST5 (CAST-128):\n");
        try {
            test1();
            test2();    // speed test
//            test3();    // maintenance (long) test...
        } catch (Throwable t) { t.printStackTrace(); }
    }


// main/test methods
//...........................................................................
    
    public static final void main (String[] args) {
        try {
            alg = Cipher.getInstance("CAST-128", "Cryptix");
            TestCAST5 x = new TestCAST5();
        }
        catch (Throwable t) { t.printStackTrace(); }
    }
    
    private void test1 ()
    throws Exception {
        String[][] data = {
        // key                               output
        {"0123456712345678234567893456789A", "238B4FE5847E44B2"},
        {"01234567123456782345",             "EB6A711A2C02271B"},
        {"0123456712",                       "7AC816D16E9B302E"}};

        for (int i = 0; i < data.length; i++) {
            RawSecretKey aKey = new RawSecretKey("CAST5", Hex.fromString(data[i][0]));
            byte[] output = Hex.fromString(data[i][1]);

            System.out.println("\nTest vector (" + (4 * data[i][0].length())+ "-bit key):");
            System.out.println("\nEncrypting:");

            alg.initEncrypt(aKey);
            compareIt(alg.crypt(input), output);

            System.out.println("\nDecrypting:");
            alg.initDecrypt(aKey);
            compareIt(alg.crypt(output), input);
        }
    }

    /** Encryption and Decryption speed test. Uses same data as test3 above. */
    private void test2 ()
    throws Exception {
        RawSecretKey aKey = new RawSecretKey("CAST5", Hex.fromString("0123456712"));
        byte[] a = Hex.fromString("0123456789ABCDEF");    // same as input

        alg.initEncrypt(aKey);

        System.out.println("\nSpeed test (10,000 x 8-byte w/40-bit key):\n");
        System.out.println("...Encryption\n");
        System.out.println("      start date/time: " + new Date().toString());
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 1000; j++)
                alg.crypt(a, 0, 8, a, 0);
        System.out.println("     finish date/time: " + new Date().toString());
        
        alg.initDecrypt(aKey);
        System.out.println("\n...Decryption\n");
        System.out.println("      start date/time: " + new Date().toString());
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 1000; j++)
                alg.crypt(a, 0, 8, a, 0);
        System.out.println("     finish date/time: " + new Date().toString());

        System.out.println("\n result:");
        compareIt(a, input);
    }

    /**
     * This test will take hours --no kidding! On a P133 w/JDK1.1.2 this is
     * what i get:
     *   start date/time: Fri Jun 20 10:10:33 GMT+01:00 1997
     *  finish date/time: Fri Jun 20 12:57:51 GMT+01:00 1997
     */
    private void test3 ()
    throws Exception {
        byte[]
            a = Hex.fromString("0123456712345678234567893456789A"),
            b = Hex.fromString("0123456712345678234567893456789A"),
            aOut = Hex.fromString("EEA9D0A249FD3BA6B3436FB89D6DCA92"),
            bOut = Hex.fromString("B2C95EB00C31AD7180AC05B8E83D696E");

        System.out.println("\nTest 1,000,000 encryptions with 128-bit key:\n");
        System.out.println("  start date/time: " + new Date().toString());
        for (int i = 0; i < 1000; i++)
            for (int j = 0; j < 1000; j++) {
                alg.initEncrypt(new RawSecretKey("CAST5", b));
                a = alg.crypt(a);        // aL = encrypt(aL, b); aR = encrypt(aR, b);
                alg.initEncrypt(new RawSecretKey("CAST5", a));
                b = alg.crypt(b);        // bL = encrypt(bL, a); bR = encrypt(bR, a);
            }
        System.out.println(" finish date/time: " + new Date().toString());
        System.out.println("\n result for 'a'\n");
        compareIt(a, aOut);
        System.out.println("\n result for 'b'\n");
        compareIt(b, bOut);
    }

    private void compareIt (byte[] o1, byte[] o2) {
        if (o1.length < 33) {
            System.out.println("  computed: " + Hex.toString(o1));
            System.out.println(" certified: " + Hex.toString(o2));
        } else {
            System.out.println("  computed: "); Hex.dumpString(o1);
            System.out.println(" certified: "); Hex.dumpString(o2);
        }
        System.out.println(ArrayUtil.areEqual(o1, o2) ? " *** GOOD" : " *** FAILED");
    }
}
