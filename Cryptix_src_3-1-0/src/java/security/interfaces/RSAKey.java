/*
// $Log: RSAKey.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:58  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.math.BigInteger;

/**
 * The interface to an RSA public or private key.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0
 */
public interface RSAKey {
    /**
     * Returns the modulus.
     */
    BigInteger getModulus();

    /**
     * Returns the exponent.
     */
    BigInteger getExponent();
}
