/**
 * Stores all the information about a user in a keyring - 
 * their key certificate, usernames, trust packet(s) and any signatures.<p>
 * Is used by the <code>KeyRing</code> classes to read in keys, and the
 * <code>KeyClient</code> classes to convert retrieved key blocks.<p>
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @author Ian Brown
 * @version 1.0
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;

import cryptix.security.MD5;
import cryptix.security.rsa.PublicKey;

public final class KeyRingEntry
{
	private Certificate cert;
	private KeyRingTrust trust; 
	private Vector userNameCerts;	

	/**
	 * Create a new <code>KeyRingEntry</code> with these parameters.
	 */
	public KeyRingEntry( Certificate certificate, KeyRingTrust certificateTrust )
	{
		cert  = certificate;
		trust = certificateTrust;
		userNameCerts = new Vector();
	}
	
	/**
	 * Create a new <code>KeyRingEntry</code> with these parameters.
	 */
	public KeyRingEntry( Certificate certificate, KeyRingTrust certificateTrust, Vector userCerts )
	{
		cert  = certificate;
		trust = certificateTrust;
		userNameCerts = userCerts;
	}
	/**
	 * Read in a <code>KeyRingEntry</code> from <code>reader</code>.<p>
	 * If the entry does not contain any trust information (i.e. is from
	 * a secret keyring or a keyserver) assign the entry <code>defaultTrust</code>.
	 */
	public KeyRingEntry( PacketInputStream reader, KeyRingTrust defaultTrust  )
	throws IOException
	{
		userNameCerts = new Vector();
		restore( reader, defaultTrust );
	}

	/**
	 * Add these names, trust packets and signatures to this entry.
	 */
	public void addNameAndSignatures( UserId userName, KeyRingTrust utrust, Vector signatures, Vector trusts )
	throws FormatException
	{
		userNameCerts.addElement( new UserIdCertificate( userName, utrust, signatures, trusts ) );
	}

	/**
	 * How well is the key trusted overall?
	 */
	public int 
	trustValue()
	{
		return trust.trustValue();
	}
	
	/**
	 * Return the certificate contained in this entry.
	 */
	public Certificate
	certificate()
	{
		return cert;
	}
	
	/**
	 * Return the public key contained in this entry.<p>
	 * If the entry contains a secret key, <code>null</code> is returned.
	 */	
	public PublicKey
	publicKey()
	{
		if ( cert instanceof PublicKeyCertificate )
			return ( (PublicKeyCertificate)cert ).publicKey();
		else return null;
	}

	/**
	 * Return an array containing all the user ID certificates contained
	 * in this entry.
	 */
	public UserIdCertificate[]
	getUserIdCertificates()
	{
		int len;
		UserIdCertificate[] ret;
		userNameCerts.copyInto( ret = new UserIdCertificate[len = userNameCerts.size()] ); 
		return ret;
	}

	/**
	 * Keys may be associated with more than one name and e-mail address,
	 * but PGP assumes the first user ID certificate after the key is
	 * the most important one. This function returns that name.
	 */
	public String
	primaryName()
	{
		return ( (UserIdCertificate)userNameCerts.elementAt( 0 ) ).userName();
	}

	/**
	 * Convert the entry to a <code>String</code> which contains the certificate,
	 * user names and their associated trust and signature packets.
	 */
	public String
	toString()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "Cert:" ).append( cert.getClass().getName() ).append( '\n' )
		.append( trust ).append( '\n' );
		for( Enumeration e = userNameCerts.elements(); e.hasMoreElements(); )
			sb.append( e.nextElement() ).append( '\n' );
		return sb.toString();
	}

	/**
	 * Check this signatures on this entry using the keys in <code>store</code>.
	 * @param store Retrieve public keys to check the signatures on this entry from <code>store</code>
	 */
	public boolean
	checkSignatures( KeyStore store )
	{
		for( Enumeration e = userNameCerts.elements(); e.hasMoreElements(); )
			if ( !((UserIdCertificate)e.nextElement()).checkSignatures( store, cert ) )
				return false;
		return true;
	}

	private void
	restore( PacketInputStream reader, KeyRingTrust defaultTrust )
	throws IOException
	{
		Packet p;
		UserIdCertificate usercert;

		// Need to catch incomplete Entries, when in runs out of data
		p = reader.readPacket();
		if ( p == null ) throw new IOException( "Tried to read a KeyRingEntry from an empty stream" ); // Reached the end of the stream
		else if ( ! ( p instanceof Certificate ) ) throw new IOException( "KeyRingEntry must start with a Certificate." );
		else cert = (Certificate)p;
		p = reader.readPacket();
		if ( ! ( p instanceof KeyRingTrust ) )
		{
		    // We may be reading a secret keyring, or a keyserver-fetched key entry
		    // which contains no trust information
		    trust = defaultTrust;
		    if ( p != null ) reader.pushback( p );
		}
		else trust = (KeyRingTrust)p;
		while ( ( usercert = reader.readUserIdCertificate( defaultTrust ) ) != null )
			userNameCerts.addElement( usercert );
	}

	/**
	 * Write out this entry as a series of PGP packets to <code>out</code>.
	 */
	public void
	write( DataOutput out )
	throws IOException
	{
		// Is used in the KeyRing.write() function

		// Write out the certificate with its trust packet
		PacketFactory.write( out, cert );
		PacketFactory.write( out, trust );

		// Write out each userNameCert
		for ( int i = 0;  i < userNameCerts.size(); i++ )
		{
			( (UserIdCertificate)(userNameCerts.elementAt( i ) ) ).write( out );
		}
	}	
}

