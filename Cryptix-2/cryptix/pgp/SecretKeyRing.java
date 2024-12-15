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
 * Represents a secret keyring. Use the <code>getKey()</code> functions
 * to retrieve a key.<p>
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @version 1.2 1/2/97
 * @author Mike Wynn
 * @author Ian Brown
 */
public final class SecretKeyRing extends KeyRing
{
	public SecretKeyRing() {} // Use add()
	
	/**
	 * Create a new secret keyring object from <code>in</code>.
	 */
	public SecretKeyRing( InputStream in )
	throws IOException
	{
		super( in );
	}
	
	/**
	 * Create a new secret keyring object from the file <code>filename</code>.
	 */
	public SecretKeyRing( String filename )
	throws FileNotFoundException, IOException
	{
		super( filename );
	}

	/**
	 *<b>DON'T USE THIS</b> - it's only in to allow SecretKeyRing to fit into
	 * this class hierarchy. <code>null</code> is always returned as a 
	 * passphrase is needed to decrypt the key first.
	 */
	public final PublicKey
	getKey( String username)
	throws FormatException
	{
		// See getKey( byte[] keyID ) for an explanation.
		return null;
	}
	
	/** Retrieve a key by any of the user names on it.*/
	public final SecretKey
	getKey( String username, Passphrase passphrase )
	throws IOException, DecryptException
	{
		KeyID id = (KeyID)( users.get( username ) ); 
		if ( id == null ) return null;
		else return getKey( id, passphrase );
	}
		
	/**
	 *<b>DON'T USE THIS</b> - it's only in to allow SecretKeyRing to fit into
	 * this class hierarchy. <code>null</code> is always returned as a 
	 * passphrase is needed to decrypt the key first.
	 */
	public final PublicKey
	getKey( KeyID keyID)
	{
		return null;
	}
	
	/** Retrieve a key by its ID.*/
	public final SecretKey
	getKey( KeyID keyID, Passphrase passphrase )
	throws DecryptException
	{
		return getSecretKey( keyID, passphrase );
	}

	/** Retrieve a key by any of the e-mail addresses on it.*/
	public final SecretKey
	getKey( EmailAddress address, Passphrase passphrase )
	throws DecryptException
	{
		return getSecretKey( address, passphrase );
	}
	
	public final int
	getKeyTrust( KeyID keyId )
	{
		// If we have this secret key, we must trust it totally...
		if ( entrysById.get( keyId ) != null ) return 3;
		// Otherwise we don't know
		else return -1;
	}

	protected final void
	init( PacketInputStream reader )
	throws IOException
	{
		KeyRingEntry entry;
		// Logically, you must trust completely all entries on a secret keyring
		while ( ( entry = reader.readKeyRingEntry( KeyRingTrust.SECRING ) ) != null )
			if ( ! entry.certificate().expired() ) add( entry );
	}
}

