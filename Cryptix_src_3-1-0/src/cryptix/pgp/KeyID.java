/*
// $Log: KeyID.java,v $
// Revision 1.2  1997/11/20 23:02:06  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.pgp;

import cryptix.util.ByteArray;
import cryptix.util.core.Hex;

/**
 * A wrapper around the byte[] key ID.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author  Ian Brown
 * @since   Cryptix 2.2 or earlier
 */
public final class KeyID extends ByteArray
{
    public KeyID(byte[] id)
    {
        // Could perhaps check that id[] is the right length here?
        // Although future PGP versions may use different KeyID lengths...
        super(id);
    }

    public String
    toString()
    {
        return "id: 0x" + Hex.toString(toByteArray());
    }
}
