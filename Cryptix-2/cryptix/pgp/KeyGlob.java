package cryptix.pgp;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Vector;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * Stores all of the key sources known to a program - 
 * all public and secret keyrings, and a Web master keyserver.
 * If setKeyServer is not used, the object will not use one.<p>
 * Can be passed to objects which decrypt and verify PGP messages
 * such as ArmouredMessage.<p>
 * NB Keyrings are searched in the order in which they are added.
 * Therefore, <b>always add the most highly trusted keyrings first.</b><p>
 * The master keyserver is only consulted if a key is not available locally.<p> 
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @author Ian Brown
 * @author Jill Baker
 */
public final class KeyGlob extends KeyStore
{

	private Vector publicKeyRings;
	private Vector secretKeyRings;
	private KeyClient keyClient;
	
	/**
	 * Initialise an empty KeyGlob
	 * Use addKeyRing and setKeyServer to fill it up
	 */
	public KeyGlob()
	{
		publicKeyRings = new Vector();
		secretKeyRings = new Vector();
		keyClient = null;
	}

	/**
	 * Add publicKeyRing to the KeyGlob's collection
	 */
	public void
	addKeyRing( PublicKeyRing publicKeyRing )
	{

		publicKeyRings.addElement( publicKeyRing );
	}

	/**
	 * Add secretKeyRing to the KeyGlob's collection.
	 * By allowing more than one secret keyring, users can keep a standard secret key available,
	 * and have a high-security key stored on a floppy disk which applications only access
	 * transparently through this class when it's needed.
	 */
	public void
	addKeyRing( SecretKeyRing secretKeyRing )
	{
		secretKeyRings.addElement( secretKeyRing );
	}
	
	/**
	 * Give KeyGlob keyclient object it can use to fetch keys remotely.
	 * This will be searched for public keys if they are not available locally.
	 * @param client An instantiated KeyClient subclass object.
	 */
	public void
	setKeyServer( KeyClient client )
	throws IOException, UnknownHostException
	{
		keyClient = client;
	}
	
	/**
         * Get the trust value for a given key ID
         * @param keyID the key ID you are interested in
         * @param publicRings search public rings (true), or secret rings (false)
         * @return trust value, or -1 if key not present
         */
        public int getKeyTrust( KeyID keyID, boolean publicRings )
        {

            int trust;

            // Cycle through all public keyrings to look for key ID
            if (publicRings)
            {
                for ( int i = 0; i < publicKeyRings.size(); i++ )
		{
                    PublicKeyRing ring = (PublicKeyRing) publicKeyRings.elementAt( i );
                    trust = ring.getKeyTrust( keyID );
                    if (trust != -1) return trust;
		}
            }
            else
            {
                for ( int i = 0; i < secretKeyRings.size(); i++ )
		{
                    SecretKeyRing ring = (SecretKeyRing) secretKeyRings.elementAt( i );
                    trust = ring.getKeyTrust( keyID );
                    if (trust != -1) return trust;
		}
            }
            return -1;
        }

	/**
	 * Try and find a public key for name.
	 * @param name The e-mail address of the user
	 * @return Their public key, or null if one can't be found
	 */
	public PublicKey
	getKey( String name )
	throws IOException
	{
		PublicKey publicKey = null;

		// Cycle through all public keyrings to look for name
		for ( int i = 0; i < publicKeyRings.size(); i++ )
		{
			publicKey = ( (PublicKeyRing) publicKeyRings.elementAt( i ) ).getKey( name );
			// If we've found it, return immediately
			if ( publicKey != null )
				return publicKey;
		}

		// If user has specified a keyserver...
		if ( keyClient != null )
		// then try it for the key
		return keyClient.getKey( name );
		else return null;
	}

	/**
	 * Try and find a public key for id.
	 * @param name The key ID of the user (the low 8 bits of their key pair's public modulus)
	 * @return Their public key, or null if one can't be found
	 */
	public PublicKey
	getKey ( KeyID id )
	throws IOException
	{
		PublicKey publicKey = null;

		// Cycle through all public keyrings to look for id
		for ( int i = 0; i < publicKeyRings.size(); i++ )
		{
			publicKey = ( (PublicKeyRing) publicKeyRings.elementAt( i ) ).getKey( id );
			// If we've found it, return immediately
			if ( publicKey != null )
				return publicKey;
		}

		// If user has specified a keyserver...
		if ( keyClient != null )
		// then try it for the key
		return keyClient.getKey( id );
		else return null;
	}
	
	/**
	 * Try and find a public key for address
	 * @param address The e-mail address of the user
	 * @return Their public key, or null if one can't be found
	 */
	public final PublicKey
	getKey ( EmailAddress address )
	throws IOException
	{
		PublicKey publicKey = null;

		// Cycle through all public keyrings to look for address
		for ( int i = 0; i < publicKeyRings.size(); i++ )
		{
			publicKey = ( (PublicKeyRing) publicKeyRings.elementAt( i ) ).getKey( address );
			// If we've found it, return immediately
			if ( publicKey != null )
				return publicKey;
		}
		
		// If user has specified a keyserver...
		if ( keyClient != null )
		// then try it for the key
		return keyClient.getKey( address );
		else return null;
	}

	/**
	 * Try and find a secret key for name and decrypt it with passphrase
	 * @param name The e-mail address of the user
	 * @param passphrase The passphrase protecting their secret key
	 * @return Their secret key, or null if one can't be found
	 */
	public SecretKey
	getSecretKey( String name, Passphrase passphrase )
	throws IOException, DecryptException
	{
		SecretKey secretKey = null;
		// Cycle through all secret keyrings to look for name
		for ( int i = 0; i < secretKeyRings.size(); i++ )
		{
			secretKey = ( (SecretKeyRing) secretKeyRings.elementAt( i ) ).getKey( name, passphrase );
			// If we've found it, return immediately
			if ( secretKey != null )
				return secretKey;
		}
		// It's not in any of them
		return null;
	}

	/**
	 * Try and find a secret key for id and decrypt it with passphrase
	 * @param name The key ID of the user's key
	 * @param passphrase The passphrase protecting their secret key
	 * @return Their secret key, or null if one can't be found
	 */
	public SecretKey
	getSecretKey ( KeyID id, Passphrase passphrase )
	throws IOException, DecryptException
	{
		SecretKey secretKey = null;
		// Cycle through all secret keyrings to look for id
		for ( int i = 0; i < secretKeyRings.size(); i++ )
		{
			secretKey = ( (SecretKeyRing) secretKeyRings.elementAt( i ) ).getKey( id, passphrase );
			// If we've found it, return immediately
			if ( secretKey != null )
				return secretKey;
		}
		// It's not in any of them
		return null;
	}

	/**
	 * Returns the name of a user whose key has ID keyID.<p>
	 * Only checks local keyrings.
	 */
	public final String
	getUserName( KeyID keyID )
	{
		String name = null;
		// Cycle through all public keyrings to look for name
		for ( int i = 0; i < publicKeyRings.size(); i++ )
		{
			name = ( (PublicKeyRing) publicKeyRings.elementAt( i ) ).getUserName( keyID );
			// If we've found it, return immediately
			if ( name != null )
				return name;
		}

		// We could look it up from the keyserver, but that would be overkill
		// so just follow PGP's example and give the key ID as a string
		return keyID.toString();
	}	
}
