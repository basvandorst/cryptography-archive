/*
// $Log: PacketOutputStream.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.pgp;

import java.io.*;

/**
 * Adds <code>writePacket()</code>, <code>writeKeyRingEntry()</code> and
 * <code>writeUserIdCertificate()</code> methods to DataOutputStream.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Ian Brown
 * @since  Cryptix 2.2 or earlier
 */
public class PacketOutputStream extends DataOutputStream
{
    protected DataOutputStream dout;

    public PacketOutputStream(OutputStream output)
    {
        super(output);
        dout = new DataOutputStream (out);
    }

    public PacketOutputStream(DataOutputStream output)
    {
        super(output);
        dout = output;
    }

    public final void
    writePacket(Packet packet)
    throws IOException
    {
        PacketFactory.write(dout, packet);
    }

    public final void
    writeKeyRingEntry(KeyRingEntry entry)
    throws IOException
    {
        entry.write(dout);
    }

    public final void
    writeUserIdCertificate(UserIdCertificate uidCert)
    throws IOException
    {
        uidCert.write(dout);
    }
}
