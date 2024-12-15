// $Id: TestRSA.java,v 1.8 1998/01/12 04:10:40 hopwood Exp $
//
// $Log: TestRSA.java,v $
// Revision 1.8  1998/01/12 04:10:40  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// Revision 1.7  1997/12/07 06:42:21  hopwood
// + Added TestElGamal, various other changes.
//
// Revision 1.6  1997/11/29 05:12:22  hopwood
// + Changes to use new test API (BaseTest).
//
// Revision 1.5  1997/11/22 07:05:41  raif
// + Updated the dox.
// + Replaced use of getU() with getInverseOfQModP().
//
// Revision 1.4  1997/11/20 22:31:26  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.3  1997/11/07 05:53:26  raif
// *** empty log message ***
//
// Revision 1.2  1997/11/05 08:01:56  raif
// *** empty log message ***
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.0  1997/07/23  R. Naffah
// + original version.
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
import java.security.interfaces.RSAPublicKey;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAFactors;

import cryptix.util.core.BI;
import cryptix.util.core.Hex;

/**
 * This object tests the RawRSA* and RSA_*Signature classes by generating
 * 5 keypairs with varying strength (512- to 1024-bit in 128-bit increment).
 * It then tests each newly created keypair by signing a fixed string with
 * the private key and verifying the obtained signature with the public one.
 * <p>
 * In your <i>Cryptix.properties</i> file located in the <i>cryptix-lib</i>
 * package, make sure the following properties are present and active:
 * <pre>
 *   KeyPairGenerator.RSA =              cryptix.provider.rsa.BaseRSAKeyPairGenerator
 *
 *   Signature.RSA =                     cryptix.provider.rsa.RawRSASignature
 *   Signature.MD2/RSA/PKCS#1 =          cryptix.provider.rsa.MD2_RSA_PKCS1Signature
 *   Signature.MD5/RSA/PKCS#1 =          cryptix.provider.rsa.MD5_RSA_PKCS1Signature
 *   Signature.RIPEMD160/RSA/PKCS#1 =    cryptix.provider.rsa.RIPEMD160_RSA_PKCS1Signature
 *   Signature.SHA-1/RSA/PKCS#1 =        cryptix.provider.rsa.SHA1_RSA_PKCS1Signature
 * </pre>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.8 $</b>
 * @since   Cryptix 2.2.2
 * @author  Raif S. Naffah
 */
class TestRSA
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
        new TestRSA().commandline(args);
    }

    protected void engineTest() throws Exception {
        KeyPairGenerator kg = KeyPairGenerator.getInstance("RSA");
        Signature md2 = Signature.getInstance("MD2/RSA/PKCS#1");
        Signature md5 = Signature.getInstance("MD5/RSA/PKCS#1");
        Signature sha = Signature.getInstance("SHA-1/RSA/PKCS#1");
        Signature ripe = Signature.getInstance("RIPEMD160/RSA/PKCS#1");
        int s;          // key strength

        setExpectedPasses(20);
        for (int i = 0; i < 5; i++) {
            s = 512 + 128 * i;
            out.println(
                "\nTest #" + (i + 1) + " (" + s + "-bit modulus)\n");

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

                out.println("RSA parameters:");

                BigInteger m = ((RSAPublicKey) pk).getModulus();
                BigInteger e = ((RSAPublicKey) pk).getExponent();
                out.println("   Public key material:");
                out.println("   n: " + BI.dumpString(m));
                out.println("   e: " + BI.dumpString(e));

                BigInteger n = ((RSAPrivateKey) sk).getModulus();
                BigInteger d = ((RSAPrivateKey) sk).getExponent();
                BigInteger p = ((RSAFactors) sk).getP();
                BigInteger q = ((RSAFactors) sk).getQ();
                BigInteger u = ((RSAFactors) sk).getInverseOfQModP();
                out.println("   Private key material:");
                out.println("   n: " + BI.dumpString(n));
                out.println("   d: " + BI.dumpString(d));
                out.println("   p: " + BI.dumpString(p));
                out.println("   q: " + BI.dumpString(q));
                out.println("   u: " + BI.dumpString(u));

                BigInteger x = new BigInteger(signature);
                out.println("   The signature as a BigInteger:");
                out.println("   x: " + BI.dumpString(x));

                out.println("RSA correctness tests:");
                try {
                    // 1. same modulus for both public and private keys
                    boolean yes = m.compareTo(n) == 0;
                    passIf(yes, "\t1. Same modulus?");
                    if (!yes) throw new RuntimeException();

                    // 2. n = pq
                    yes = p.multiply(q).compareTo(n) == 0;
                    passIf(yes, "\t2. n = pq?");
                    if (!yes) throw new RuntimeException();

                    // 3. x = (x ** ed) mod n
                    BigInteger y = x.modPow(e, n);
                    BigInteger z = y.modPow(d, n);
                    yes = z.compareTo(x) == 0;
                    passIf(yes, "\t3. x = (x ** ed) mod n?");
                    if (!yes) throw new RuntimeException();

                    // 4. same with CRT-based formula
                    BigInteger ONE = BigInteger.valueOf(1L);
                    BigInteger ep = d.mod(p.subtract(ONE));
                    BigInteger eq = d.mod(q.subtract(ONE));
                    BigInteger p2 = y.mod(p).modPow(ep, p);
                    BigInteger q2 = y.mod(q).modPow(eq, q);
                    q2 = q2.subtract(p2);
                    if (q2.signum() == -1) q2 = q2.add(q);  // 0 <= q2 - p2 < q
                    z = p2.add(p.multiply(q2.multiply(u).mod(q)));
                    yes = z.compareTo(x) == 0;
                    passIf(yes, "\t4. (x ** e) mod n = (y ** d) mod pq?");
                    if (!yes) throw new RuntimeException();
                    out.println();
                }
                catch (Exception ex) {
                    fail("Exception during pure RSA tests");
                    throw new RuntimeException("PANIC: Failed pure RSA tests...");
                }
                // passed RSA tests. problem with formatting
                out.println("---- end debugging -----");
                
                throw new RuntimeException("PANIC...");
            }
        } catch (Exception e) {
            fail("Unexpected exception");
            e.printStackTrace();
        }
    }
}
