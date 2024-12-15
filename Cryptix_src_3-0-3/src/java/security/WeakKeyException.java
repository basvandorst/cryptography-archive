/*
// $Log: WeakKeyException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package java.security;

/**
 * WeakKeyException is thrown when a weak key would have been generated
 * (e.g. by a KeyGenerator).
 * <p>
 * <strong><a href="../guide/ijce/JCEDifferences.html">This class
 * is not supported in JavaSoft's version of JCE.</a></strong>
 */
public class WeakKeyException extends KeyException {
    /**
     * Constructs a WeakKeyException without any detail message.
     */
    public WeakKeyException() {
        super();
    }

    /**
     * Constructs a WeakKeyException with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param msg     the detail message.
     */
    public WeakKeyException(String msg) {
        super(msg);
    }
}
