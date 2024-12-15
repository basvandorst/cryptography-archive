// $Id: PublicKey.java,v 1.2 1997/12/07 07:47:03 hopwood Exp $
//
// $Log: PublicKey.java,v $
// Revision 1.2  1997/12/07 07:47:03  hopwood
// + Removed extends clauses for java.security.interfaces.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security.rsa;

import cryptix.security.MD5;
import cryptix.math.BigInteger;

/**
 * This class represents an RSA public key. It can only verify signatures or
 * encrypt.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 * @deprecated This class may be removed in a future version of Cryptix.
 * @see cryptix.provider.rsa.BaseRSAPublicKey
 */
public class PublicKey
{
    /**
     * The public modulus as a cryptix.math.BigInteger.
     */
    protected BigInteger n_;

    /**
     * The public exponent as a cryptix.math.BigInteger.
     */
    protected BigInteger e_;

    /**
     * A constructor that does nothing, for use by subclasses.
     */
    protected PublicKey()
    {
    }
    
    /**
     * Creates a public key from a modulus and exponent.
     *
     * @param n the public modulus
     * @param e the public exponent
     */
    public PublicKey(BigInteger n, BigInteger e)
    {
        n_= new BigInteger(n);
        e_= new BigInteger(e);
    }

    /**
     * Creates a new public key from an existing key.
     *
     * @param from the key to copy.
     */
    public PublicKey(PublicKey from)
    {
        copy(from);
    }

    /**
     * Returns a new cryptix.math.BigInteger equal to the public modulus.
     */
    public final BigInteger n()
    {
        return new BigInteger(n_);
    }

    /**
     * Returns a new cryptix.math.BigInteger equal to the public exponent.
     */
    public final BigInteger e()
    {
        return new BigInteger(e_);
    }

    /**
     * Copies a key if it is an instance of cryptix.security.rsa.PublicKey.
     *
     * @param src the object to copy from
     * @exception ClassCastException if !(src instanceof cryptix.security.rsa.PublicKey)
     */
    public void copy(Object src)
    {
        PublicKey from = (PublicKey) src;
        n_ = new BigInteger(from.n_);
        e_ = new BigInteger(from.e_);
    }

    /**
     * Tests whether another object is equal to this one.
     * <p>
     * Note: since PublicKey objects are mutable (because of the <code>copy</code>
     * method), it was a mistake to override <code>equals</code>. Also,
     * <code>equals</code> and <code>hashCode</code> should always be overridden
     * together.
     *
     * @param obj object to test
     * @return true if the object is equal, otherwise false.
     */
    public boolean equals(Object obj)
    {
        if (obj instanceof PublicKey)
        {
            PublicKey from = (PublicKey) obj;
            if (n_.equals(from.n_) && e_.equals(from.e_))
                return true;
        }
        return false;
    }

    /**
     * Gets the key's ID.
     *
     * @returns the lower 8 bytes of n, the public modulus.
     */
    public final byte[] id()
    {
        byte[] nBuffer = n_.toByteArray();
        byte[] ident = new byte[8];
        System.arraycopy(nBuffer, nBuffer.length - 8, ident, 0, 8);
        return ident;
    }

    /**
     * Gets the key's fingerprint.
     * <p>
     * WARNING: because the lengths of n and e are not encoded, it is feasible to
     * create distinct keys with the same fingerprint.
     * See <a href="http://www.hotlava.com/doc/fag-pgp/">
     * http://www.hotlava.com/doc/fag-pgp/</a>.
     *
     * @return an MD5 hash of n and e.
     */
    public final byte[] fingerPrint()
    {
        byte[] eBuffer = e_.toByteArray();
        byte[] nBuffer = n_.toByteArray();
        MD5 md = new MD5();
        md.add(nBuffer);
        md.add(eBuffer);
        return md.digest();
    }

    /**
     * Gets the key's bit length.
     *
     * @return the length in bits of n, the public modulus.
     */
    public final int bitLength()
    {
        return n_.bitLength();
    }

    /**
     * Encrypts a number using this key. This can also be used to verify
     * RSA signatures.
     * <p>
     * Note: using RSA directly without random padding can be insecure.
     *
     * @param plain the number to encrypt.
     * @return the result of the encryption.
     */
    public BigInteger encrypt(BigInteger plain)
    {
        return new BigInteger(plain).modExp(e_, n_);
    }

    /**
     * Returns a string representation of this key in hex.
     */
    public String toString()
    {
        return "n:" + n_ + "\ne:" + e_;
    }
}
