/*
// $Log: GeneralSecurityException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security;

/**
 * This is the general security exception class, which serves to group all
 * the exception classes of the <samp>java.security</samp> package that extend
 * from it.
 * <p>
 * Exceptions (no pun intended) are <samp>AccessControlException</samp>
 * and <samp>CertificateException</samp>, which subclass from
 * <samp>java.lang.SecurityException</samp>, and <samp>ProviderException</samp>
 * and <samp>InvalidParameterException</samp>, which subclass from
 * <samp>java.lang.RuntimeException</samp>. 
 * <p>
 * This class is documented in JavaSoft's preview APIs, for introduction
 * in Java 1.2.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0.1
 */
public class GeneralSecurityException extends Exception {
    /**
     * Constructs a GeneralSecurityException without any detail message.
     */
    public GeneralSecurityException() {
        super();
    }

    /**
     * Constructs a GeneralSecurityException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param msg     the detail message.
     */
    public GeneralSecurityException(String msg) {
        super(msg);
    }
}
