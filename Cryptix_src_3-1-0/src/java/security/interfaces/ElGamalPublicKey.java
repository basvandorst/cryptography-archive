/*
// $Log: ElGamalPublicKey.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.security.PublicKey;

/**
 * The interface to an ElGamal public key.
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
public interface ElGamalPublicKey extends ElGamalKey, PublicKey {
}
