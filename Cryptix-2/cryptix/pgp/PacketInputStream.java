package cryptix.pgp;

import java.io.*;
import java.util.Vector;

/**
 * Adds readPacket(), readKeyRingEntry() and readUserIdCertificate() functions
 * to DataInputStream.
 * Also allows ONE packet to be pushed back onto the stream.
 * @author Mike Wynn
 * @author Ian Brown
 * @author Jill Baker
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
public class PacketInputStream extends DataInputStream
{
	/**@variable store Stores a packet that has been pushed back*/
	protected Packet store = null;
	protected DataInputStream din;

	public PacketInputStream( InputStream input )
	{
		super( input );
		din = new DataInputStream ( in );
	}

	public PacketInputStream( DataInputStream input )
	{
		super( input );
		din = input;
	}

	/**
	 * Reads any packet, including comment packets.
	 */
	public synchronized Packet readAnyPacket() throws IOException
	{
		Packet temp;
		if ( store == null ) return PacketFactory.read( din );
		else
		{
			temp = store;
			store = null;
			return temp;
		}
	}

	/**
	 * Reads any packet apart from Comment packets.
	 * This greatly simplifies the processing packet streams,
	 * which can contain Comments anywhere.
	 */
	public synchronized Packet readPacket() throws IOException
	{
       		Packet p;
        	do p = readAnyPacket();
        	while ( (p != null) && ( p instanceof Comment ) );
        	return p;
	}

	/**
	 * Push back a packet which has been read onto the stream.
	 * This is essential for parsing multi-packet structures such as
	 * a KeyRingEntry, where you don't know how many packets of what
	 * type to expect.
	 */
	public synchronized void
	pushback( Packet p )
	throws IOException
	{
		if ( store == null ) store = p;
		else throw new IOException( "Only one packet may be pushed back." );
	}

	/**
	 * Used by KeyRing to load in key certificate and accompanying user IDs and signatures
	 * and KeyServer to decipher keys retrieved from remote keyservers.
	 */
	public synchronized KeyRingEntry
	readKeyRingEntry( KeyRingTrust defaultTrust)
	throws IOException
	{
		Packet p;
		Certificate cert;
		UserIdCertificate usercert;
		KeyRingTrust trust;
		Vector userCerts = new Vector();

		// Need to catch incomplete Entries, when in runs out of data
		// This doesn't check for Comment packets, as PGP doesn't generate them
		p = readPacket();
		if ( p == null ) return null; // Reached the end of the stream
		else if ( ! ( p instanceof Certificate ) ) throw new IOException( "KeyRingEntry must start with a Certificate." );
		else cert = (Certificate)p;
		p = readPacket();
		if ( p == null ) return null; // Reached the end of the stream
		else if ( ! ( p instanceof KeyRingTrust ) )
		{
		    // We may be reading a secret keyring, or a keyserver-fetched key entry
		    // which contains no trust information
		    trust = defaultTrust;
		    pushback( p );
		}
		else trust = (KeyRingTrust)p;
		while ( ( usercert = readUserIdCertificate( defaultTrust ) ) != null )
			userCerts.addElement( usercert );

		return new KeyRingEntry( cert, trust, userCerts );
	}

	public synchronized UserIdCertificate
	readUserIdCertificate( KeyRingTrust defaultTrust )
	throws IOException
	{
		UserId userid;
		KeyRingTrust trust;
		Vector signatures = new Vector();
		Vector trusts = new Vector();
		Packet p = null;

		// Need to catch incomplete certificates, when in runs out of data
		p = readPacket();
		if ( ! ( p instanceof UserId ) )
		{
			// Reached end of UserIdCertificates section
			if ( p != null ) pushback( p );
			return null;
		}		
		else userid = (UserId)p;
		
		p = readPacket();		
		if ( ! ( p instanceof KeyRingTrust ) )
		{
			// Reached end of UserIdCertificates section
			// or reading from a no-trust-given source
            		trust = defaultTrust;
            		if ( p != null ) pushback( p );
        	}
		else trust = (KeyRingTrust)p;
		
		while ( ( p = readPacket () ) instanceof Signature )
		{
			signatures.addElement( p );
			p = readPacket();
			if ( ! ( p instanceof KeyRingTrust ) )
			{
				p = defaultTrust;
				if ( p != null ) pushback ( p );
			}
			trusts.addElement( p );
		}
		// p will now be the UserId at the start of the next UserIdCertificate
		if ( p != null ) pushback ( p );
		return new UserIdCertificate( userid, trust, signatures, trusts );
	}

    public boolean markSupported()
    {
        return false;
    }
}
