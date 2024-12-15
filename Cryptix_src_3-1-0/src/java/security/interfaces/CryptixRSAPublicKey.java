/*
// $Log: CryptixRSAPublicKey.java,v $
// Revision 1.1  1999/07/12 20:35:35  edwin
// renaming java.security.interfaces.RSAPrivateKey and RSAPublicKey to CryptixRSAPrivateKey and CryptixRSAPublicKey. This is one more step to JDK1.2 compatibility.
//
// Revision 1.1.1.1  1997/11/03 22:36:58  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security.interfaces;

import java.security.PublicKey;

/**
 * The interface to an RSA public key.
 *
 * <p><b>$Revision: 1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0
 * @see java.security.Cipher
 * @see java.security.Signature
 */
public interface CryptixRSAPublicKey extends RSAKey, PublicKey {
}
