/*
// $Log: ElGamalPrivateKey.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.math.BigInteger;
import java.security.PrivateKey;

/**
 * The interface to an ElGamal private key.
 * <p>
 * <b>References</b>
 * <p>
 * <blockquote>
 *    <a href="mailto:schneier@counterpane.com">Bruce Schneier</a>,
 *    "Section 19.6 ElGamal,"
 *    <cite>Applied Cryptography</cite>,
 *    Wiley 2nd Ed, 1996.
 * </blockquote>
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0.1
 * @see ElGamalKey
 * @see java.security.Cipher
 * @see java.security.Signature
 */
public interface ElGamalPrivateKey extends ElGamalKey, PrivateKey {
    /**
     * Returns the value of <i>x</i> (the private key).
     *
     * @return x as a java.math.BigInteger
     */
    BigInteger getX();
}
