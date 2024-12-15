// $Id: SecretKey.java,v 1.3 1999/07/12 20:32:37 edwin Exp $
//
// $Log: SecretKey.java,v $
// Revision 1.3  1999/07/12 20:32:37  edwin
// making cryptFast public instead of protected
// renaming java.security.interfaces.RSAPrivateKey and RSAPublicKey to CryptixRSAPrivateKey and CryptixRSAPublicKey. This is one more step to JDK1.2 compatibility.
//
// Revision 1.2  1997/12/07 07:47:04  hopwood
// + Removed extends clauses for java.security.interfaces.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/07/29  David Hopwood
// + Added support for RSAPrivateKey and RSAFactors interfaces.
// + Deprecated this class.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security.rsa;

import java.security.interfaces.CryptixRSAPrivateKey;
import java.security.interfaces.RSAFactors;

import cryptix.math.BigNum;
import cryptix.math.BigInteger;

/**
 * This class represents an RSA secret key pair. It can also be used as a public key.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated This class may be removed in a future version of Cryptix.
 * @see cryptix.provider.rsa.BaseRSAPrivateKey
 */
public class SecretKey
extends PublicKey
{
    /** the private exponent */
    protected BigInteger d_;
    /** one of the factors of n */
    protected BigInteger p_;
    /** the other factor of n */
    protected BigInteger q_;
    /** the multiplicative inverse of p mod q */
    protected BigInteger u_;

    /**
     * A constructor that does no initialization, for use only by subclasses.
     */
    protected SecretKey()
    {
        super();
    }
    
    /**
     * Constructs a secret key pair from the component parts.
     *
     * @param n the public modulus
     * @param e the public (encryption) exponent
     * @param d the private (decryption) exponent
     * @param p the smaller factor of <i>n</i>
     * @param q the larger factor of <i>n</i>
     * @param u the multiplicative inverse of <i>p</i> mod <i>q</i>
     */
    public SecretKey(BigInteger n, BigInteger e, BigInteger d, BigInteger p,
                      BigInteger q, BigInteger u)
    {
        super(n, e);
        d_ = new BigInteger(d);
        p_ = new BigInteger(p);
        q_ = new BigInteger(q);
        u_ = new BigInteger(u);
    }

    /**
     * Returns the private exponent, <i>d</i>.
     */
    public final BigInteger d()
    {
        return new BigInteger(d_);
    }

    /**
     * Returns the smaller factor of <i>n</i>, <i>p</i>.
     */
    public final BigInteger p()
    {
        return new BigInteger(p_);
    }

    /**
     * Returns the larger factor of <i>n</i>, <i>q</i>.
     */
    public final BigInteger q()
    {
        return new BigInteger(q_);
    }

    /**
     * Returns the multiplicative inverse of <i>p</i> mod <i>q</i>.
     */
    public final BigInteger u()
    {
        return new BigInteger(u_);
    }

    /**
     * Decrypts a number. This is used for decryption or signing.
     *
     * @param encrypted the number to decrypt.
     * @return a decrypted number.
     */
    public BigInteger
    decrypt(BigInteger encrypted)
    {
        return cryptFast(d_, encrypted);
    }

    /**
     * Encrypts a number. This is used for encryption or signature verification.
     * <b>N.B.</b> this is quicker than the public key method, since secret
     * components are available.
     *
     * @param plain the number to encrypt.
     * @return an encrypted number.
     */
    public BigInteger
    encrypt(BigInteger plain)
    {
        return cryptFast(e_, plain);
    }

    /**
     * Performs a fast encryption/decryption, using the Chinese Remainder Theorem.
     *
     * @param exponent  either e if encrypting or d if decrypting.
     * @param input     the value to encrypt or decrypt.
     * @return the result of the operation.
     */
    public final BigInteger cryptFast(BigInteger exponent, BigInteger input)
    {
        BigInteger p1 = (BigInteger) p_.clone();
        BigInteger q1 = (BigInteger) q_.clone();

        p1.dec();
        q1.dec();

        BigInteger dmp1 = new BigInteger().mod(exponent, p1);
        BigInteger dmq1 = new BigInteger().mod(exponent, q1);

        // m1 = ((input mod p) ^ dmq1)) mod p
        BigInteger m = new BigInteger().mod(input, p_).modExp(dmp1, p_);

        // r = ((input mod q) ^ dmp1)) mod q
        BigInteger r = new BigInteger().mod(input, q_).modExp(dmq1, q_);

        BigInteger r1 = new BigInteger().sub(r, m);
    
        BigInteger zero = BigInteger.zero();

        if (r1.cmp(zero) < 0)
            r1.add(r1, q_);

        r.mul(r1, u_);
        r1.mod(r, q_);
        r.mul(r1, p_);
        r1.add(r, m);
    
        return r1;
    }
    
    /**
     * Performs a sanity check on the key.
     *
     * @return null if the key is O.K., otherwise an error message.
     */
    public final String insane()
    {
        BigInteger t = new BigInteger().mul(p_, q_);
        if (n_.cmp(t) != 0)
            return "pq != n";
        if (p_.cmp(q_) >= 0)
            return "p >= q";
        BigNum.modMul(t, p_, u_, q_);
        if (t.cmp(BigInteger.one) != 0)
            return "(p*u) mod q != 1";
        return null;
    }

    /**
     * Performs a basic test on the key.
     *
     * @return null if the key is O.K., otherwise an error message.
     */
    public final String test()
    {
        String ret = insane();
        if (ret != null)
            return ret;
        StringBuffer sb = new StringBuffer();
        byte tmp = 7,buf[] = new byte[16];
        for (int i = 0; i < 16; tmp += 3)
            buf[i++] = tmp;
        BigInteger msg = new BigInteger(buf);
//        System.out.println("test string" + msg.toString());
        BigInteger encmsg = encrypt(msg);
        BigInteger msg2 = decrypt(encmsg);
        if (msg.cmp(msg2) != 0)
            return "Key test failed " + msg + "\nis not" + msg2;
        return null;
    }

    /**
     * Returns a string representation of all parts of the secret key.
     * This method should be used with caution, in order that the secret key
     * is not compromised.
     *
     * @return a string representation of the key.
     */
    public String toString()
    {
        return super.toString() + "\nd:" + d_ + "\np:" + p_ + "\nq:" + q_ + "\nu:" + u_;
    }
}
