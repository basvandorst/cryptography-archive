/*
// $Log: NoSuchParameterException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security;

/**
 * NoSuchParameterException is thrown when a particular parameter is not
 * available in an algorithm parameter set. 
 * <p>
 * This class is documented in JavaSoft's preview APIs, for introduction
 * in Java 1.2.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0.1
 * @see java.security.AlgorithmParameters
 */
public class NoSuchParameterException extends GeneralSecurityException {
    /**
     * Constructs a NoSuchParameterException without any detail message.
     */
    public NoSuchParameterException() {
        super();
    }

    /**
     * Constructs a NoSuchParameterException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param msg     the detail message.
     */
    public NoSuchParameterException(String msg) {
        super(msg);
    }
}
