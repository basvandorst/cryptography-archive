package cryptix.pgp;

import java.io.InputStream;
import java.io.IOException;
import java.io.FileNotFoundException;

import java.util.Vector;
import java.util.Enumeration;

import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;

import cryptix.util.EmailAddress;

/**
 * Represents a public keyring. Keys on it can be retrieved using the
 * <code>getKey()</code> functions.<p>
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @version 1.2, 7/2/1997
 * @author Mike Wynn
 * @author Ian Brown
 */
public class PublicKeyRing extends KeyRing
{
	public PublicKeyRing() {} // Use add()

	/**
	 * Create a new public keyring object from <code>in</code>.
	 */
	public PublicKeyRing( InputStream in )
	throws IOException
	{
		super( in );
	}

	/**
	 * Create a new public keyring object from the file <code>filename</code>.
	 */
	public PublicKeyRing( String filename )
	throws FileNotFoundException, IOException
	{
		super( filename );
	}

	/** Retrieve a key by any of the user names on it.*/
	public PublicKey
	getKey( String username )
	throws IOException
	{
		KeyID toID;
		if ( (toID = (KeyID)(users.get( username ))) != null )
		{
			PublicKey publicKey = getKey( toID );
			if ( publicKey != null )
				return publicKey;
			// there SHOULD never be a time when you can get the ID but not the key !!
		}
		return getIfBegins( users.keys(), username );
	}
	
	/** Retrieve a key by its ID.*/
	public PublicKey
	getKey( KeyID keyID )
	throws IOException
	{
		return getPublicKey( keyID );
	}

	/** Retrieve a key by any of the e-mail addresses on it.*/
	public PublicKey
	getKey( EmailAddress address )
	throws IOException
	{
		return getPublicKey( address );
	}

	protected final void
	init( PacketInputStream reader )
	throws IOException
	{
		KeyRingEntry entry;
		// By default, key entries without trust information will be given undefined trust
		while ( ( entry = reader.readKeyRingEntry( KeyRingTrust.UNDEF ) ) != null )
			if ( ! entry.certificate().expired() ) add( entry );
	}

}

