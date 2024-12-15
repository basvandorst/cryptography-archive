package cryptix.pgp;

import java.io.*;
import java.net.*;

import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * Base for the classes that fetch keys from keyservers.<p>
 * Next version will cache keys, within and between sessions.<p>
 * WebClient is the only class which currently extends this. Others
 * can be easily written - they simply need to implement the <code>fetch()</code>
 * function.<p>
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @version 1.0 19/12/1996
 * @author Ian Brown
 */
public abstract class KeyClient extends KeyStore
{
	protected String serverName;
	protected int port;	
	
	/**
	 * Set up variables for server connection
	 */
	protected KeyClient( String name, int port_ )
	throws IOException, UnknownHostException
	{
		serverName = name;
		port = port_;
	}

	public PublicKey getKey( String userName ) throws IOException
	{
		return fetch( userName );
	}

	public PublicKey getKey( EmailAddress address) throws IOException
	{
		return fetch( address );
	}

	public PublicKey getKey( EmailAddress address, KeyID keyID ) throws IOException
	{
		return fetch( address, keyID );
	}

	public PublicKey getKey( KeyID keyID ) throws IOException
	{
		return null;
	}

	public final String
	getUserName( KeyID keyID )
	{
		return null;
	}

	/**
	 * fetch() is overridden by child classes to provide the 'meat' of these classes
	 */
	protected abstract PublicKey fetch( String userName ) throws IOException;
	protected abstract PublicKey fetch( EmailAddress address, KeyID keyID ) throws IOException;
	protected abstract PublicKey fetch( EmailAddress address ) throws IOException;
}
