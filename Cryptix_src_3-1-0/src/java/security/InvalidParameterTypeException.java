/*
// $Log: InvalidParameterTypeException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security;

/**
 * This is the exception for invalid parameter types.
 * <p>
 * This class is documented in JavaSoft's preview APIs, for introduction
 * in Java 1.2.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author David Hopwood
 * @since  IJCE 1.0.1
 * @see java.security.AlgorithmParameters
 * @see java.security.interfaces.DSADiscreteLogParameters
 */
public class InvalidParameterTypeException extends GeneralSecurityException {
    /**
     * Constructs an InvalidParameterTypeException without any detail message.
     */
    public InvalidParameterTypeException() {
        super();
    }

    /**
     * Constructs an InvalidParameterTypeException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param msg     the detail message.
     */
    public InvalidParameterTypeException(String msg) {
        super(msg);
    }
}
