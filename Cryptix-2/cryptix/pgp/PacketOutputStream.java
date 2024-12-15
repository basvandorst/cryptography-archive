package cryptix.pgp;

import java.io.*;

/**
 * Adds writePacket(), writeKeyRingEntry() and writeUserIdCertificate() functions
 * to DataOutputStream.
 * @author Ian Brown
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
public class PacketOutputStream extends DataOutputStream
{
	protected DataOutputStream dout;

	public PacketOutputStream( OutputStream output )
	{
		super( output );
		dout = new DataOutputStream ( out );
	}

	public PacketOutputStream( DataOutputStream output )
	{
		super( output );
		dout = output;
	}

	public final void
	writePacket( Packet packet )
	throws IOException
	{
		PacketFactory.write( dout, packet );
	}

	public final void
	writeKeyRingEntry( KeyRingEntry entry )
	throws IOException
	{
		entry.write( dout );
	}

	public final void
	writeUserIdCertificate( UserIdCertificate uidCert )
	throws IOException
	{
		uidCert.write( dout );
	}
}
