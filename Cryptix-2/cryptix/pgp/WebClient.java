package cryptix.pgp;

import java.io.*;
import java.net.*;

import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * @version 2.0 29/12/1996
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
public final class WebClient extends KeyClient
{
	private String pathToPage;
	
	public WebClient( String name, int port_, String path  )
	throws IOException, UnknownHostException
	{
		super( name, port_ );
		pathToPage = path;
	}


	/**
	 * Called by KeyClient.getKey() to fetch a key using a specific protocol
	 * Should NOT be called directly - this would bypass checks and caching
	 */
	protected PublicKey fetch( String userName )
	throws IOException 
	{
		boolean fullKeyReceived = false;
		DataInputStream serverIn = new DataInputStream( ( new URL( "http", serverName, port, pathToPage + "?op=get&exact=on&search=" + userName ) ).openStream() );
		String line = null;
		String keyBlock = "";
		
		// Skip HTML formatting and information about key
		do
		{
			line = serverIn.readLine();
			if ( line == null ) return null; // No key returned
		} while ( !line.equals( "-----BEGIN PGP PUBLIC KEY BLOCK-----" ) );
		keyBlock += line + Armoury.LINEFEED;	
		
		// Get rest of key
		while ( ( line = serverIn.readLine() ) != null )
		{
			if ( line.equals( "-----END PGP PUBLIC KEY BLOCK-----" ) )
			{
				keyBlock += line;
				fullKeyReceived = true;
				break; // End of key
			}
			else keyBlock += line + Armoury.LINEFEED;
		}
		try
		{
			if ( ! fullKeyReceived ) return null; // Problem retrieving key
			else return new KeyRingEntry( new PacketInputStream( new ByteArrayInputStream( Armoury.strip( keyBlock ) ) ), KeyRingTrust.UNDEF ).publicKey();
		}

		catch( InvalidChecksumException ice )
		{
			// We can't throw one as KeyStore (the ultimate parent class ) doesn't allow it
			throw new IOException( "Message's checksum is incorrect." );
		}
	}

	/**
	 * WebClient doesn't need an address as it queries a
	 * master keyserver.
	 */
	protected PublicKey fetch( EmailAddress address, KeyID keyID )
	throws IOException
	{
		return fetch( keyID.toString() );
	}
	
	protected PublicKey fetch( EmailAddress address )
	throws IOException
	{
		return fetch( address.toString() );
	}
	
	protected PublicKey fetch( KeyID keyID )
	throws IOException
	{
		return fetch( null, keyID );
	}

}
