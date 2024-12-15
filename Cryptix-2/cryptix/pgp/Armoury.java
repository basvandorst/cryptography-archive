/**
 * Converts data to and from the 64-bit encoding, with a 24-bit checksum,
 * that PGP uses as 'transport armour'.
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */

package cryptix.pgp;

import java.io.*;
import cryptix.mime.Base64;

public final class Armoury
{
	public static final String LINEFEED = "\r\n";

	private Armoury()
	{
	}

	/**
	 * Armour and return the data in <code>buf[]</code>
	 * <i>without</i> the PGP framing lines.
	 */
	public static String
	messageWithChecksum(byte buf[])
	{
		byte checksum[] = new byte[3];
		int csum = CRC.checksum(buf);
		checksum[0] = (byte)(csum >>> 16);
		checksum[1] = (byte)(csum >>> 8);
		checksum[2] = (byte)(csum);

		return Base64.encode(buf)+"="+Base64.encode(checksum);
	}
	
	/**
	 * Armour and write the <code>name</code> packet in <code>buf</code>
	 * to <code>out</code>.
	 */
	public static void
	message( PrintStream out, String name, byte buf[])
	{
		out.println("-----BEGIN PGP "+name+"-----");
		out.println("Version: Cryptix: 0.01");
		out.println();
		out.print(messageWithChecksum(buf));
		out.println("-----END PGP "+name+"-----");
	}
	
	/**
	 * Armour and return the <code>name</code> packet in <code>buf</code>.
	 */
	public static String
	message( String name, byte buf[] )
	{
		String framed;
		framed = "-----BEGIN PGP " + name + "-----" + LINEFEED;
		framed += "Version: Cryptix: 0.01" + LINEFEED + LINEFEED;
		framed += messageWithChecksum(buf);
		framed += "-----END PGP " + name + "-----" + LINEFEED;
		return framed;
	}

	/**
	 * Armour and write <code>pkt</code> to <code>out</code>.
	 */
	public static void
	message( PrintStream out, Packet pkt)
	throws IOException
	{
		message(out, name(pkt), pkt.save());
	}

 	/**
 	 * 'Strips' armour off PGP binary data
 	 * and checks 24-bit checksum
 	 */
 	public static byte[]
	strip( String buf )
	throws FormatException, IOException, InvalidChecksumException
	{
		String checksum, armour;
		int pos;
		// Find start of ASCII armour
		pos = buf.indexOf( "-----BEGIN PGP" ) ;

		if ( pos == -1 ) throw new FormatException( "Couldn't find PGP part in message." );

		// Now get rid of version / comments
		pos = buf.indexOf( LINEFEED + LINEFEED, pos );

		// Get message up to checksum
		armour = buf.substring( pos + 4, ( pos = buf.indexOf( LINEFEED + "=" ) ) );
		checksum = buf.substring( pos + 2, pos + 7 );
		// We now have message in armour and its checksum
		int givenChecksum = decodeChecksum( checksum );
		int realChecksum = CRC.checksum( Base64.decode( armour ) );
		// Do the two checksums match?
		if ( givenChecksum != realChecksum ) throw new InvalidChecksumException( "Message checksum wrong." );

		// OK - checksum correct, so return decoded armour
		return Base64.decode( ArmouredMessage.byteEncode( armour ) );
 	}

	/**
	 * Returns the name PGP uses in its framing lines for this type
	 * of <code>pkt</code>.
	 */
	public static String
	name( Packet pkt )
	{
		if ( pkt instanceof Signature )
			return "SIGNATURE";
		else if ( pkt instanceof SecretKeyCertificate )
			return "SECRET KEY BLOCK";
		else if ( pkt instanceof PublicKeyCertificate )
			return "PUBLIC KEY BLOCK";

		throw new IllegalArgumentException("PGP Packet type "+pkt.getClass().getName()+" not valid for this method");
	}

	/**
	 * Read from <code>dis</code> until a PGP section appears.
	 */
	public static void
	readUntilBegin(DataInputStream dis)
	throws IOException
	{
		String line;
		// Read from stream until a PGP section appears
		while ( ! ( ( line = dis.readLine() ).startsWith( "-----BEGIN PGP" ) ) );
		// Now get rid of version / comments
		while ( ! ( ( line = dis.readLine() ).equals( "" ) ) );
	}

	/**
	 * Decode the checksum contained in <code>s</code>.
	 */
	public static int
	decodeChecksum(String s)
		//throws InvalidChecksumException
	{
		if (s.length() != 5)
		{
		}

		byte[] asc_sum = new byte[4];
		s.getBytes(1, 5, asc_sum, 0);
		byte[] binsum = Base64.decode(asc_sum);
		int sum = ((((int)binsum[0] & 0xFF) << 16)
			| (((int)binsum[1] & 0xFF) << 8)
			| ((int)binsum[2]) & 0xFF);
		return sum;
	}
}

