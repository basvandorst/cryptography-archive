// $Id: TestSquare.java,v 1.5 1997/11/27 05:41:31 iang Exp $
//
// $Log: TestSquare.java,v $
// Revision 1.5  1997/11/27 05:41:31  iang
// Add static to block - crashes javadoc without.
//
// Revision 1.4  1997/11/20 22:31:26  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1  1997/11/17  David Hopwood
// + Changed occurrence of
//   "if (x.length < 33) .. Hex.toString(x) .. else .. Hex.dumpString(x)"
//   to "Hex.dumpString(x)". Hex.dumpString has been changed so that
//   this will have the same effect.
//
// Revision 1.3  1997/11/07 05:53:26  raif
// *** empty log message ***
//
// Revision 1.2  1997/11/05 08:01:57  raif
// *** empty log message ***
//
// Revision 0.1.0.2  1997/08/20  R. Naffah
// + Use RawKeyGenerator and SquareKeyGenerator classes.
//
// Revision 0.1.0.1  1997/08/13  R. Naffah
// + Modified to use/test new Mode implementation.
// + Use RawSecretKey instead of ByteArrayKey.
// + Use cryptix.util.
//
// Revision 0.1.0.0  1997/07/30  R. Naffah
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */

package cryptix.test;

import cryptix.util.core.ArrayUtil;
import cryptix.util.core.Hex;
import cryptix.provider.key.RawKeyGenerator;
import cryptix.provider.key.RawSecretKey;

import java.security.Cipher;
import java.security.FeedbackCipher;
import java.security.KeyGenerator;
import java.security.Mode;
import java.security.SecretKey;

/**
 * Tests the output of the Square cipher algorithm implementation against
 * certified pre-computed values for a given set of reference input.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.5 $</b>
 * @author Raif S. Naffah
 */
public final class TestSquare
{

// Variables and constants
//...........................................................................

    private static final byte[] key = new byte[16];
    static {
        for (int i = 0; i < 16; i++) key[i] = (byte)i;
    }
    private static RawKeyGenerator rkg;
    private static SecretKey aKey;


// Constructor
//...........................................................................
    
    private TestSquare () {
        try {
//          aKey = new RawSecretKey("Square", key);
            rkg = (RawKeyGenerator) KeyGenerator.getInstance("Square", "Cryptix");
            aKey = rkg.generateKey(key);

            test1();
            test2();
            test3();
            test4();
            test5();
        } catch (Throwable t) { t.printStackTrace(); }
    }


// main/test methods
//...........................................................................
    
    public static final void main (String[] args) {
        System.out.println("\n*** Square:");
        try { TestSquare x = new TestSquare(); }
        catch (Exception e) { e.printStackTrace(); }
    }
    
    private void test1 ()
    throws Exception {
        byte[] input = new byte[16];
        {
            for (int i = 0; i < 16; i++) input[i] = (byte) i;
        }
        byte[] output = {
            (byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
            (byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
            (byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
            (byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F
        };
        System.out.println("\nTest vector (raw/ECB):\nEncrypting:");
        Cipher alg = Cipher.getInstance("Square", "Cryptix");
            
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }
    
    private void test2 ()
    throws Exception {
        byte[] input = new byte[32];
        {
            for (int i = 0; i < 16; i++)
                input[i] = input[i + 16] = (byte) i;
        }
        byte[] output = {
            (byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
            (byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
            (byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
            (byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F,

            (byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
            (byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
            (byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
            (byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F
        };
        System.out.println("\nTest vector (ECB):\nEncrypting:");
        Cipher alg = Cipher.getInstance("Square/ECB", "Cryptix");
            
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }

    private void test3 ()
    throws Exception {
        byte[] input = new byte[32];
        {
            for (int i = 0; i < 16; i++)
                input[i] = input[i + 16] = (byte) i;
        }
        byte[] output = {
            (byte)0x7C, (byte)0x34, (byte)0x91, (byte)0xD9,
            (byte)0x49, (byte)0x94, (byte)0xE7, (byte)0x0F,
            (byte)0x0E, (byte)0xC2, (byte)0xE7, (byte)0xA5,
            (byte)0xCC, (byte)0xB5, (byte)0xA1, (byte)0x4F,

            (byte)0x41, (byte)0xD2, (byte)0xF1, (byte)0x9D,
            (byte)0x7E, (byte)0x87, (byte)0x8D, (byte)0xB5,
            (byte)0x6C, (byte)0x74, (byte)0x46, (byte)0xD4,
            (byte)0x24, (byte)0xC3, (byte)0xAD, (byte)0xFC
        };
        System.out.println("\nTest vector (CBC):\nEncrypting:");
        Mode alg = (Mode)(Cipher.getInstance("Square/CBC", "Cryptix"));
        FeedbackCipher fbc = (FeedbackCipher) alg;
        fbc.setInitializationVector(new byte[fbc.getInitializationVectorLength()]);
            
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }

    private void test4 ()
    throws Exception {
        byte[] input = new byte[32];
        {
            for (int i = 0; i < 16; i++)
                input[i] = input[i + 16] = (byte) i;
        }
        byte[] output = {
            (byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
            (byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
            (byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
            (byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,

            (byte)0xd5, (byte)0xcb, (byte)0x53, (byte)0x8e,
            (byte)0xea, (byte)0x28, (byte)0x97, (byte)0x4f,
            (byte)0x7c, (byte)0x75, (byte)0xe7, (byte)0x9b,
            (byte)0xcb, (byte)0x0d, (byte)0x4d, (byte)0x0e
        };
        System.out.println("\nTest vector (CFB):\nEncrypting:");
        Mode alg = (Mode)(Cipher.getInstance("Square/CFB", "Cryptix"));
        ((FeedbackCipher) alg).setInitializationVector(new byte[16]);
            
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }

    private void test5 ()
    throws Exception {
        byte[] input = new byte[32];
        {
            for (int i = 0; i < 16; i++)
                input[i] = input[i + 16] = (byte) i;
        }
        byte[] output = {
            (byte)0xff, (byte)0x58, (byte)0x6d, (byte)0xa5,
            (byte)0x6c, (byte)0xba, (byte)0xc5, (byte)0x06,
            (byte)0x4a, (byte)0x09, (byte)0xa4, (byte)0x0a,
            (byte)0xee, (byte)0xb6, (byte)0xae, (byte)0xaf,

            (byte)0x35, (byte)0xc8, (byte)0x33, (byte)0xd3,
            (byte)0x5c, (byte)0x29, (byte)0x44, (byte)0x37,
            (byte)0x35, (byte)0xd2, (byte)0x25, (byte)0xbc,
            (byte)0x95, (byte)0x28, (byte)0xc3, (byte)0xc8
        };
        System.out.println("\nTest vector (OFB):\nEncrypting:");
        Mode alg = (Mode)(Cipher.getInstance("Square/OFB", "Cryptix"));
        FeedbackCipher fbc = (FeedbackCipher) alg;
        fbc.setInitializationVector(new byte[fbc.getInitializationVectorLength()]);
            
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }

    private void compareIt (byte[] o1, byte[] o2) {
        System.out.println("  computed: " + Hex.dumpString(o1));
        System.out.println(" certified: " + Hex.dumpString(o2));
        System.out.println(ArrayUtil.areEqual(o1, o2) ? " *** GOOD" : " *** FAILED");
    }
}
