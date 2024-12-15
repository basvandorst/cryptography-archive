/*
// $Log: RSAPrivateKey.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:58  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.security.PrivateKey;

/**
 * The interface to an RSA private key.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0
 * @see java.security.Cipher
 * @see java.security.Signature
 */
public interface RSAPrivateKey extends RSAKey, PrivateKey {
}
