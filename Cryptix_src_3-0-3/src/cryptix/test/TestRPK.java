// $Id: TestRPK.java,v 1.5 1998/01/12 04:10:40 hopwood Exp $
//
// $Log: TestRPK.java,v $
// Revision 1.5  1998/01/12 04:10:40  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// Revision 1.4  1997/11/29 05:12:22  hopwood
// + Changes to use new test API (BaseTest).
//
// Revision 1.3  1997/11/22 07:05:41  raif
// + Use isSameValue() instead of equals().
// + Use new naming scheme.
//
// Revision 1.2  1997/11/20 22:31:26  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1  1997/11/17  David Hopwood
// + Fixed imports.
// + Documentation changes.
// + Renamed RPK signature algorithms to follow Sun's convention
//   (digest name first).
//
// Revision 1.1  1997/11/08 02:24:32  raif
// *** empty log message ***
//
// Revision 0.1.0.0  1997/09/25  R. Naffah
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
import cryptix.provider.rpk.*;

import java.security.Cipher;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;

/**
 * This class tests the RPK*, *_RPK_PKCS1Signature and rpk.StreamCipher classes.
 * <p>
 * In your <samp>Cryptix.properties</samp> file located in the
 * <i>cryptix-lib</i> directory, make sure the following properties are
 * present and active:
 * <pre>
 *    Cipher.RPK.stream.cipher =          cryptix.provider.rpk.StreamCipher
 *
 *    KeyGenerator.RPK =                  cryptix.provider.rpk.RPKKeyGenerator
 *    KeyPairGenerator.RPK =              cryptix.provider.rpk.RPKKeyPairGenerator
 *
 *    Signature.MD5/RPK/PKCS#1 =          cryptix.provider.rpk.MD5_RPK_PKCS1Signature
 *    Signature.SHA-1/RPK/PKCS#1 =        cryptix.provider.rpk.SHA1_RPK_PKCS1Signature
 *    Signature.RIPEMD160/RPK/PKCS#1 =    cryptix.provider.rpk.RIPEMD160_RPK_PKCS1Signature
 * </pre>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.5 $</b>
 * @author  Raif S. Naffah
 */
class TestRPK
extends BaseTest
{

// Variables and constants
//...........................................................................

    private static final byte[] message = (
        "Je ne veux que du magnifique, et je ne travaille pas pour le " +
        " vulgaire des lecteurs --Giambattista BODONI (1740-1813)").getBytes();

    private static byte[] signature;


// Test methods
//................................................................................

    public static void main(String[] args) {
        new TestRPK().commandline(args);
    }

    protected void engineTest() throws Exception {
        KeyPairGenerator kg = KeyPairGenerator.getInstance("RPK");

        Signature siga = Signature.getInstance("MD5/RPK/PKCS#1");
        // alternatively you can use one of these two
//        Signature siga = Signature.getInstance("SHA-1/RPK/PKCS#1");
//        Signature siga = Signature.getInstance("RIPEMD160/RPK/PKCS#1");

        test(kg, siga);
    }

    void test(KeyPairGenerator kg, Signature md) {
        try {
            out.println("1. Generating a keypair...\n");
            kg.initialize(0);        // will use default granularity = 8 (highest)
            KeyPair pair = kg.generateKeyPair();
            RPKKey E = (RPKKey) pair.getPublic();
            RPKKey D = (RPKKey) pair.getPrivate();
            out.println("\n1.1 Public part of the keypair: " + E);
            out.println("\n1.2 Private part of the keypair: " + D);

            // test basic RPK formula: E**R = Q**D
            // get another pair with same params
            out.println("\n1.3 Testing validity of basic RPK formula...");
            KeyPair pair2 = kg.generateKeyPair();
            RPKKey Q = (RPKKey) pair2.getPublic();
            RPKKey R = (RPKKey) pair2.getPrivate();
            RPKKey K1 = RPKKey.exponentiate(E, R);
            RPKKey K2 = RPKKey.exponentiate(Q, D);
            if (K1.isSameValue(K2))
                pass("Basic RPK operation");
            else {
                fail("Basic RPK operation");
                out.println("     E: " + Hex.dumpString(E.toByteArray()));
                out.println("     R: " + Hex.dumpString(R.toByteArray()));
                out.println("     Q: " + Hex.dumpString(Q.toByteArray()));
                out.println("     D: " + Hex.dumpString(D.toByteArray()));
                out.println("  E**R: " + Hex.dumpString(K1.toByteArray()));
                out.println("  Q**D: " + Hex.dumpString(K2.toByteArray()));
                return;
            }

            out.println("\n2. Testing RPK stream cipher...");
            Cipher rpk = Cipher.getInstance("RPK.stream.cipher", "Cryptix");
            rpk.initEncrypt(K1);
            byte[] computed = rpk.crypt(message);
            rpk.initDecrypt(K2);
            computed = rpk.crypt(computed);
            if (ArrayUtil.areEqual(computed, message))
                pass("RPK stream cipher symmetry");
            else {
                fail("RPK stream cipher symmetry");
                out.println("  Computed: " + Hex.dumpString(computed));
                out.println("    Actual: " + Hex.dumpString(message));
                return;
            }

            out.println("\n3. Using keypair for signature...");
            out.println("\n3.1 Signing with " + md.getAlgorithm() + " key...");
            md.initSign((PrivateKey) E);            // role reversal in RPK
            md.update(message);
            signature = md.sign();

            out.println("\n3.2 Verifying with same " + md.getAlgorithm() + " key...\n");
            md.initVerify((PublicKey) D);
            md.update(message);
            passIf(md.verify(signature), "Signature verification");
        }
        catch (Exception e) {
            fail("Unexpected exception");
            e.printStackTrace();
        }
    }
}
