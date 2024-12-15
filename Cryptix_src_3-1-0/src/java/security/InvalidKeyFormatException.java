/*
// $Log: InvalidKeyFormatException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security;

/**
 * InvalidKeyFormatException is thrown when a key format is not recognized.
 * (Current valid formats for the "SUN" provider include "X.509", "PKCS#8",
 * "RAW-BigInteger", and "RAW-DER".)
 * <p>
 * This class is documented in JavaSoft's preview APIs, for introduction
 * in Java 1.2.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0.1
 */
public class InvalidKeyFormatException extends GeneralSecurityException {
    /**
     * Constructs an InvalidKeyFormatException without any detail message.
     */
    public InvalidKeyFormatException() {
        super();
    }

    /**
     * Constructs an InvalidKeyFormatException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param msg     the detail message.
     */
    public InvalidKeyFormatException(String msg) {
        super(msg);
    }
}
