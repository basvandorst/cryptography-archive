/*
 * Copyright (C) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix development team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.*;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Enumeration;

import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;

import cryptix.util.EmailAddress;

/**
 * KeyRing is an abstract superclass for keyring objects -
 * stores of keys (public and private) held in a file.
 * PublicKeyRing, KeyCache and SecretKeyRing implement
 * the specific behaviour of these different kinds of keyring files.<p>
 *
 * Note that a keyring is not automatically written out when it is destroyed.
 * The application must call <code>write()</b> itself if it has added keys
 * which is wishes to keep.<p>
 *
 * KeyRing is unlikely to be useful for any other superclasses. If you wish to
 * implement a general key retrieval class, use <code>KeyStore</code> instead.
 * If you wish to add file-specific behaviour, use PublicKeyRing or SecretKeyRing.<p>
 *
 * <b>Future changes</b>
 * The next release will use a <code>File</code> object rather than a 
 * <code>String</code> to specify a filename in the constructor.<p>
 *
 * A <code>remove()</code> function may also be added.<p>
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @version 1.3, 7/2/1997
 * @author Systemics Ltd
 * @author Ian Brown
 * @author Jill Baker
 * @see PublicKeyRing
 * @see KeyCache 
 * @see cryptix.pgp.SecretKeyRing
 */
public abstract class KeyRing extends KeyStore
{
	// Only entrysById stores the public key or secret key certificate

	// users maps username to keyID
	protected Hashtable users = new Hashtable();

	// addresses maps e-mail address to KeyID
	protected Hashtable addresses = new Hashtable();

   	// this maps KeyID to KeyRingEntry;
	protected Hashtable entrysById = new Hashtable();

	protected KeyRing(){};

	protected KeyRing( InputStream in )
	throws IOException
	{
		init( new PacketInputStream( in ) );
	}

	protected KeyRing( String filename )
	throws FileNotFoundException, IOException
	{
		FileInputStream keyfile = new FileInputStream( filename );
		init( new PacketInputStream( keyfile ) );
        		keyfile.close();
	}

	/**
	 * Returns the <b>primary</b> name of the user who owns
	 * the key referenced by keyID.<p>
	 * If their key is not available, the keyID is returned as a String.
	 * This mimics PGP's behaviour.
	 */
	public final String
	getUserName( KeyID keyID )
	{
		KeyRingEntry entry =  (KeyRingEntry)entrysById.get( keyID );
		if ( entry != null ) return entry.primaryName();
		else return "Key: " + keyID.toString();
	}

	/**
	 * Adds a key to this keyring.<p>
	 * You can't just add a Certificate - you need at least
	 * a UserId and KeyRingTrust packet, and possibly
	 * signatures and further UserIds as well.<p>
	 * KeyRingEntry contains all of this.
	 */
	public final void
	add( KeyRingEntry entry)
	throws FormatException
	{
		KeyID id;
		String username;

		Certificate cert = entry.certificate();
		
		if ( cert instanceof PublicKeyCertificate )
			id = new KeyID( ((PublicKeyCertificate)cert).publicKey().id() );
		else
			id = ((SecretKeyCertificate)cert).id();

		entrysById.put( id, entry );

		UserIdCertificate[] idcerts = entry.getUserIdCertificates();

		int len = idcerts.length;
		if ( len == 0 )
			throw new FormatException( "Certificate does not have any names" );

		for ( int i = 0; i < len; i++ )
		{
			username = idcerts[i].userName();
			users.put( username, id );

			if ( username.indexOf( "@")  != -1  ) // If username contains an address
			{
				try
				{
					EmailAddress address = new EmailAddress( username );
					addresses.put( address, id );
				}
				
				catch ( IOException e ) {} // username did not contain a valid address
			}
		}
	}

	/**
	 * Get a list of all the names on the key with ID keyID.
	 */
	public final String[]
	getFullNameList( KeyID keyID )
	{
		int len = users.size();
		if ( len > 0 )
		{
			Vector v = new Vector();
			Enumeration e = users.keys();
			for ( int i = 0; i < len; i++ )
			{
				Object o;
				if ( keyID.equals( users.get( o = e.nextElement() ) ) )
					v.addElement( o );
			}
			if ( (len = v.size()) > 0 )
			{
				String[] ret;
				v.copyInto( ret = new String[len] );
				return ret;
			}
		}
		return new String[0];
	}

	/**
	 * Get a list of all the usernames of keys on this keyring.
	 */
	public final String[]
	getFullUserNameList()
	{
		int len = users.size();
		String ret[] = new String[len];
		if ( len > 0 )
		{
			Enumeration e = users.keys();
			for ( int i = 0; i < len; i++ )
				ret[i] = (String)e.nextElement();
		}
		return ret;
	}

	protected abstract void init( PacketInputStream reader ) throws IOException;

	protected final PublicKey
	getPublicKey( KeyID id )
	{
		KeyRingEntry entry;
		if ( (entry = (KeyRingEntry)entrysById.get( id )) != null )
		{
			Certificate c;
			if ( (c = entry.certificate()) instanceof PublicKeyCertificate )
				return ((PublicKeyCertificate)c).publicKey();
		}
		return null;
	}

	protected final PublicKey
	getPublicKey( EmailAddress address )
	{
		KeyRingEntry entry;
		KeyID id = (KeyID)addresses.get( address );
		if ( id == null ) return null;
		else if ( (entry = (KeyRingEntry)entrysById.get( id ) ) != null )
		{
			Certificate c;
			if ( (c = entry.certificate()) instanceof PublicKeyCertificate )
				return ((PublicKeyCertificate)c).publicKey();
		}
		return null;
	}

	protected final SecretKey
	getSecretKey( KeyID id, Passphrase passphrase )
	throws DecryptException
	{
		KeyRingEntry entry;
		if ( (entry = (KeyRingEntry)entrysById.get( id )) != null )
		{
			Certificate c;
			if ( (c = entry.certificate()) instanceof SecretKeyCertificate )
				return ((SecretKeyCertificate)c).getSecretKey( passphrase );
		}
		return null;
	}

	protected final SecretKey
	getSecretKey( EmailAddress address, Passphrase passphrase )
	throws DecryptException
	{
		KeyRingEntry entry;
		KeyID id = (KeyID)addresses.get( address );
		if ( id == null ) return null;
		else if ( (entry = (KeyRingEntry)entrysById.get( id ) ) != null )
		{
			Certificate c;
			if ( (c = entry.certificate()) instanceof SecretKeyCertificate )
				return ((SecretKeyCertificate)c).getSecretKey( passphrase );
		}
		return null;
	}

	protected final PublicKey
	getIfBegins( Enumeration e, String username )
	throws IOException
	{
		while( e.hasMoreElements() )
		{
			String key = (String)e.nextElement();
			// Looks up key indirectly using keyID as a 'pointer'
			// Could this be if ( key.indexOf( username ) != -1 )
			// to make a getIfContains?
			if ( key.startsWith( username ) )
				return getKey( key );
		}
		return null;
	}

	/**
	 * Writes out all KeyRingEntries contained in this keyring
	 * to outputStream.
	 */
	public void
	write( OutputStream outputStream )
	throws IOException
	{
		/**
		  * Should this not write out expired KeyRingEntries?
		  * It might be better to leave expired certificates initially in the file.
		  * When it is next read in with this class, it will be ignored and so
		  * will disappear when the keyring is next written out.
		   */
		PacketOutputStream pos;
		if ( outputStream instanceof PacketOutputStream) pos = (PacketOutputStream)outputStream;
		else pos = new PacketOutputStream( outputStream );
		KeyID id;
		for ( Enumeration e = entrysById.keys(); e.hasMoreElements(); )
		{
			id = (KeyID)e.nextElement();
			pos.writeKeyRingEntry( (KeyRingEntry)entrysById.get( id ) );
		}
	}

	/** this will check the signatures assuming all the keys are in this ring */
	public boolean
	checkSignatures()
	{
		return checkSignatures( this );
	}

	/** this will check the signature assuming that the KeyStore contains all the nec keys */
	public boolean
	checkSignatures( KeyStore store )
	{
		for ( Enumeration e = entrysById.elements(); e.hasMoreElements(); )
		{
			if ( !((KeyRingEntry)e.nextElement()).checkSignatures( store ) )
				return false;
		}
		return true;
	}

	/** debug dump of key ring */
	public final void
	dump( PrintStream out )
	{
		Hashtable h = users;
		out.println( "Username to KeyID mappings ..." );
		for ( Enumeration e = h.keys(); e.hasMoreElements(); )
		{
			Object o = e.nextElement();
			out.println( o + " maps to " + h.get( o ) );
		}

		out.println();
		out.println( "KeyID to Certificate/Key mappings ..." );
		h = entrysById;
		for ( Enumeration e = h.keys(); e.hasMoreElements(); )
		{
			Object o = e.nextElement();
			out.println( o + " maps to " + h.get( o ) );
		}
		out.println( checkSignatures() ? "Signatures O.K." : "Signatures BAD" );
	}

	/** debug list of signatures etc */
	public final void
	list( PrintStream out )
	{
		out.println();
		out.println( "Key signature checking ..." );
		for ( Enumeration e = entrysById.keys(); e.hasMoreElements(); )
		{
			KeyID id = (KeyID)e.nextElement();
			out.println( id );
			KeyRingEntry entry = (KeyRingEntry)entrysById.get( id );
			UserIdCertificate[] names = entry.getUserIdCertificates();
			Certificate cert = entry.certificate();
			out.println( "Cert:" + cert );
			for( int i = 0; i < names.length; i++ )
			{
				UserId name;
				out.println( "User name:" + (name = names[i].userId()).toString() );
				Signature[] sigs = names[i].signatures();
				for ( int j = 0; j < sigs.length; j++ )
				{
					KeyID sigid = sigs[j].getKeyId();
					out.println( "Signed by " + sigid + " User name " + getUserName( sigid ) );
					try
					{
						out.println( checkSignature( sigs[j], cert, name ) ? "GOOD signature" : "*** BAD signature ***" );
					}
					catch( IOException ioe )
					{
						out.println( "Signature unchecked " + ioe.getClass().getName() + ":" + ioe.getMessage() );
					}
				}
			}
		}
		return;
	}

	/**
	 * How much is the key with ID keyId trusted by this user?
	 */
        public int
        getKeyTrust( KeyID keyId )
        {
            KeyRingEntry kre = (KeyRingEntry) entrysById.get( keyId );
            if (kre == null) return -1;
            UserIdCertificate[] uidcert = kre.getUserIdCertificates();
            return uidcert[0].getTrustValue();
        }
}

