/*
// $Log: ElGamalParams.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/
 
package java.security.interfaces;

import java.math.BigInteger;

/**
 * Interface to an ElGamal-specific set of key parameters, which defines
 * an ElGamal <em>key family</em>.
 * <p>
 * The same key parameters apply to both the signature and encryption
 * algorithms.
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
 * @see java.security.Key
 * @see java.security.Cipher
 * @see java.security.Signature
 */
public interface ElGamalParams {
    /**
     * Returns the prime, <i>p</i>.
     * 
     * @return the prime as a java.math.BigInteger
     */
    BigInteger getP();

    /**
     * Returns the base, <i>g</i>.
     * 
     * @return the base as a java.math.BigInteger 
     */
    BigInteger getG();
}
