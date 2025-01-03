// This file is currently unlocked (change this line if you lock the file)
//
// $Log: RSAKeyGen.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.security.rsa;

import cryptix.math.BigNum;
import cryptix.math.BigInteger;
import cryptix.math.RandomStream;
import cryptix.math.TestPrime;

/**
 * This class is an RSA key pair generator.
 *
 * <p>Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public class RSAKeyGen 
{
    protected RandomStream rand;
    
    /**
     * @param rand0 the source of random numbers for the prime number
     * generation.
     */
    public RSAKeyGen(RandomStream rand0)
    {
        rand = rand0;
    }
    
    /**
     * An observer class is used to monitor progress.<p>
     * @ param bitlen the bit length of the final n
     * @ param obsrv the callback interface.
     * @ return a random prime number.
     */
    public BigInteger
    randomPrime(int bitlen, RSAKeyGenObserver obsrv)
    {
        BigInteger ret = new BigInteger();
        int bytelen = 1 + (bitlen / 8), offset = bitlen % 8;
        byte buf[] = new byte[bytelen];
        
        while (true)
        {
        // make big endian random number.
            if (rand.read(buf) != bytelen)
                throw new Error("Not using a cryptix.math.RandomStream !!!");

            buf[bytelen-1] |= 0x01; // make it odd.
        // set top bit, and clear the others.
            if (offset == 0)
            {
                buf[0] = 0;
                buf[1] |= (byte)0x80;
            }
            else
            {
                buf[0] &= (byte)(0xFF >>> (8 - offset));
                buf[0] |= (byte)(1 << (offset - 1));
            }
            ret.fromByteArray(buf);
            if (TestPrime.isPrime(ret, obsrv))
                return ret;
        }
    }

    /**
     * This function creates a Secret Key.
     * <p>
     * <b>N.B.</b> this can take a LONG time.
     *
     * @param rand the source of random data for the prime number generation.
     * @param bitlen the bit length of the final n
     * @return a random RSA SecretKey.
     */
    public static final SecretKey
    createKey(RandomStream rand, int bitlen)
    {
        return createKey(rand, bitlen, null);
    }

    /**
     * An observer class is used to monitor progress.
     *
     * @param rand the source of random data for the prime number generation.
     * @param bitlen The bit length of the final n
     * @param obsrv The callback interface.
     * @return a random RSA SecretKey.
     */
    public static final SecretKey
    createKey(RandomStream rand, int bitlen, RSAKeyGenObserver obsrv)
    {
        return new RSAKeyGen(rand).createKey(bitlen, obsrv);
    }

    /**
     * This function creates a Secret Key.
     * <p>
     * <b>N.B.</b> this can take a LONG time.
     *
     * @param bitlen the bit length of the final n
     * @return a random RSA SecretKey.
     */
    public SecretKey
    createKey(int bitlen)
    {
        return createKey(bitlen, null);
    }
    
    /**
     * An observer class is used to monitor progress.
     *
     * @param bitlen The bit length of the final n
     * @param obsrv The callback interface.
     * @return a random RSA SecretKey.
     */
    public SecretKey
    createKey(int bitlen, RSAKeyGenObserver obsrv)
    {
        int plen = bitlen/2; // PGP requires that p and q are the same length.
        int qlen = bitlen-plen;
        signalObserver(obsrv, RSAKeyGenObserver.FINDING_P);
        BigInteger p = randomPrime(plen, obsrv);
        signalObserver(obsrv, RSAKeyGenObserver.FINDING_Q);
        BigInteger q = randomPrime(qlen, obsrv);

        BigInteger e = new BigInteger(17);
        signalObserver(obsrv, RSAKeyGenObserver.KEY_RESOLVING);
        SecretKey ret = deriveKeys(p, q, e);
        signalObserver(obsrv, RSAKeyGenObserver.KEY_GEN_DONE);
        return ret;
    }

    /**
     * An observer class is used to monitor progress.
     *
     * @param p the largest factor of n
     * @param q the other factor of n
     * @param e the initial public exponent (must be odd).
     * @return a random RSA SecretKey.
     */
    protected final SecretKey
    deriveKeys(BigInteger p, BigInteger q, BigInteger e)
    {
        // Ensure p <= q
        if (p.cmp(q) > 0)
        {
            BigInteger t = p;
            p = q;
            q = t;
        }
        BigInteger t1 = new BigInteger().sub(p, BigInteger.one);
        BigInteger t2 = new BigInteger().sub(q, BigInteger.one);
        // phi(n) = (p-1)*(q-1)
        BigInteger phi = new BigInteger().mul(t1, t2);
        // G(n) = gcd(p-1,q-1)
        BigInteger gcdphi = new BigInteger().gcd(t1, t2);
        // F(n) = phi(n)/G(n)
        t1.div(phi, gcdphi);
        if (e == null) // if no e start at 3 e must be odd.
            e = new BigInteger(3);

        while(true)
        {
            t2.gcd(e, phi);
            if (t2.cmp(BigInteger.one) == 0)
                break;
            e.add(2);
        }
        // Compute d so that (e*d) mod F(n) = 1
        BigInteger d = new BigInteger();
        BigNum.inverseModN(d, e, t1);
        // Compute u so that (p*u) mod q = 1
        BigInteger u = new BigInteger();
        BigNum.inverseModN(u, p, q);
        // n = p * q
        BigInteger n = new BigInteger().mul(p, q);
        SecretKey ret = new  SecretKey(n, e, d, p, q, u);
        String test = ret.test();
        if (test != null)
            throw new Error ("RSA Key generation failed - reason:\n" + test);
        return ret;
    }

    /**
     * This informs any observer of the current status.
     *
     * @param obsrv the obect to call, null if none.
     * @param signalValue status
     */
    private static final void
    signalObserver(RSAKeyGenObserver obsrv, int signalValue)
    {
        if (obsrv != null)
            obsrv.handleEvent(signalValue);
    }
}
