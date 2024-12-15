/*
// $Log: InvalidChecksumException.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.pgp;

/**
 * This exception is thrown when a checksum is invalid.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @since Cryptix 2.2 or earlier
 */
public class InvalidChecksumException extends Exception
{
    InvalidChecksumException(String reason)
    {
        super(reason);
    }
}
