// $Id: TestElGamal.java,v 1.5 1998/01/12 04:10:40 hopwood Exp $
//
// $Log: TestElGamal.java,v $
// Revision 1.5  1998/01/12 04:10:40  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// Revision 1.4  1997/12/15 02:40:54  hopwood
// + Committed changes below.
//
// Revision 1.3.1  1997/12/15  hopwood
// + Speed up test by only doing 512 and 768-bit primes for the time being.
//
// Revision 1.3  1997/12/14 18:00:10  hopwood
// + Committed changes below.
//
// Revision 1.2.1  1997/12/11  hopwood
// + Changed step for modulus to 256, so that the pre-computed parameters
//   will be used (when they have been computed).
// + Added new test for 384-bit parameter generation.
//
// Revision 1.2  1997/12/08 16:31:29  hopwood
// + Fix variable name that was used twice.
//
// Revision 1.1  1997/12/07 06:42:21  hopwood
// + Added TestElGamal, various other changes.
//
// Revision 0.1.0  1997/12/04  David Hopwood
// + Original version (based on TestRSA).
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */

package cryptix.test;

import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.interfaces.ElGamalPublicKey;
import java.security.interfaces.ElGamalPrivateKey;

import cryptix.util.core.BI;
import cryptix.util.core.Hex;

/**
 * This object tests the RawElGamal* and *_ElGamal_PKCS1Signature classes by
 * generating 5 keypairs with varying strength (512- to 1024-bit in 128-bit
 * increment). It then tests each newly created keypair by signing a fixed
 * string with the private key and verifying the obtained signature with the
 * public one.
 * <p>
 * In your <i>Cryptix.properties</i> file located in the <i>cryptix-lib</i>
 * package, make sure the following properties are present and active:
 * <pre>
 *   KeyPairGenerator.ElGamal =              cryptix.provider.elgamal.BaseElGamalKeyPairGenerator
 * <p>
 *   Signature.MD2/ElGamal/PKCS#1 =          cryptix.provider.elgamal.MD2_ElGamal_PKCS1Signature
 *   Signature.MD5/ElGamal/PKCS#1 =          cryptix.provider.elgamal.MD5_ElGamal_PKCS1Signature
 *   Signature.RIPEMD160/ElGamal/PKCS#1 =    cryptix.provider.elgamal.RIPEMD160_ElGamal_PKCS1Signature
 *   Signature.SHA-1/ElGamal/PKCS#1 =        cryptix.provider.elgamal.SHA1_ElGamal_PKCS1Signature
 * </pre>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.5 $</b>
 * @author  David Hopwood
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2.2
 */
class TestElGamal
extends BaseTest
{

// Constants and variables
//................................................................................

    private static final byte[] message = (
        "Je ne veux que du magnifique, et je ne travaille pas pour le " +
        "vulgaire des lecteurs --Giambattista BODONI (1740-1813)").getBytes();
    private static byte[] signature;
    private static final SecureRandom prng = new SecureRandom();


// Test methods
//................................................................................

    public static void main (String[] args) {
        new TestElGamal().commandline(args);
    }

    protected void engineTest() throws Exception {
        KeyPairGenerator kg = KeyPairGenerator.getInstance("ElGamal");
        Signature md2 = Signature.getInstance("MD2/ElGamal/PKCS#1");
        Signature md5 = Signature.getInstance("MD5/ElGamal/PKCS#1");
        Signature sha = Signature.getInstance("SHA-1/ElGamal/PKCS#1");
        Signature ripe = Signature.getInstance("RIPEMD160/ElGamal/PKCS#1");
        int s;          // key strength

        setExpectedPasses(9); // temporary hack, should be 21
        out.println("\nTest #1 (384-bit modulus + parameter generation)\n");
        test(384, kg, sha);

        for (int i = 0; i < 2; i++) { // temporary hack, should be i < 5
            s = 512 + 256 * i;
            out.println("\nTest #" + (i + 2) + " (" + s + "-bit modulus)\n");

            test(s, kg, md2);
            test(s, kg, md5);
            test(s, kg, sha);
            test(s, kg, ripe);
        }
    }

    private void test (int s, KeyPairGenerator kg, Signature alg) {
        KeyPair pair;
        PublicKey pk;
        PrivateKey sk;
        boolean ok;
        try {
            out.println("1. Generating keypair...\n");
            kg.initialize(s, prng);
            pair = kg.generateKeyPair();
            pk = pair.getPublic();
            sk = pair.getPrivate();

            out.println("2. Using keypair for signature...");
            out.println(
                "2.1 Signing with a " + s + "-bit key using " + alg.getAlgorithm() + "...");
            alg.initSign(sk);
            alg.update(message);
            signature = alg.sign();
                
            out.println(
                "2.2 Verifying with same " + s + "-bit key using " + alg.getAlgorithm() + "...\n");
            alg.initVerify(pk);
            alg.update(message);
//            ok = sig.verify(signature);
            ok = alg.verify(signature);
            passIf(ok, "Signature verification");

            if (!ok) {
                out.println("---- begin debugging -----\n");
                out.println("Computed signature: " + Hex.dumpString(signature));

                out.println("ElGamal parameters:");

                BigInteger p = ((ElGamalPublicKey) pk).getP();
                BigInteger g = ((ElGamalPublicKey) pk).getG();
                BigInteger y = ((ElGamalPublicKey) pk).getY();
                out.println("   Public key material:");
                out.println("   p: " + BI.dumpString(p));
                out.println("   g: " + BI.dumpString(g));
                out.println("   y: " + BI.dumpString(y));

                BigInteger x = ((ElGamalPrivateKey) sk).getX();
                out.println("   Private key material:");
                out.println("   x: " + BI.dumpString(x));

                BigInteger sig = new BigInteger(signature);
                out.println("   The signature as a BigInteger:");
                out.println(" sig: " + BI.dumpString(sig));

                // need correctness tests here, as for TestRSA.
                out.println("---- end debugging -----");
                
                throw new RuntimeException("PANIC...");
            }
        } catch (Exception e) {
            fail("Unexpected exception");
            e.printStackTrace();
        }
    }
}
