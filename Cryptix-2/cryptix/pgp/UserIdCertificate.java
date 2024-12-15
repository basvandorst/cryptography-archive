/*
 * Stores all the information about a user in a keyring - 
 * their key certificate, usernames, trust packet(s) and any signatures.
 * @author Ian Brown
 * @version 1.0
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;

import cryptix.security.rsa.PublicKey;

/**
 * Each user ID can contain any number of signatures and corresponding trust packets
 * This class is used by KeyRingEntry to deal with this.
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
public final class UserIdCertificate
{
	
	private UserId         name;
	private KeyRingTrust   usertrust;
	private Signature[]    sigs;
	private KeyRingTrust[] sigtrust;

	UserIdCertificate( UserId userName, KeyRingTrust utrust, Vector signatures, Vector trusts )
	throws FormatException
	{
		name = userName;
		usertrust = utrust;
		initSigs( signatures, trusts );
	}

	UserIdCertificate( PacketInputStream reader )
	throws IOException
	{
		restore( reader );
	}

	private final void
	initSigs( Vector signatures, Vector trusts )
	throws FormatException
	{
		if ( (signatures != null) && (trusts != null) )
		{
			int len;
			if ( (len = trusts.size()) != signatures.size() )
				throw new FormatException( "Number of trust packets incorrect for number of signatures" );
			if ( len > 0 )
			{
				signatures.copyInto( sigs = new Signature[len] );
				trusts.copyInto( sigtrust = new KeyRingTrust[len] );
				return;
			}
		}
		sigs = new Signature[0];
		sigtrust = new KeyRingTrust[0];
		return;
	}

	public String
	userName()
	{
		return name.toString();
	}

	public UserId
	userId()
	{
		return name;
	}

	public Signature[]
	signatures()
	{
		int len = sigs.length;
		Signature[] ret;
		System.arraycopy( sigs, 0, ret = new Signature[len], 0, len );
		return ret;
	}

	boolean
	checkSignatures( KeyStore store, Certificate cert )
	{
		int len = sigs.length;
		for ( int i = 0; i < len; i++ )
		{
			try
			{
				PublicKey key = store.getKey( sigs[i].getKeyId() );
				if ( key != null )
				{
					if ( !KeyStore.checkSignature( key, sigs[i], cert, name ) )
						return false;
				}
				else
					System.err.println( "Key id " + sigs[i].getKeyId() + " not found (Signature ignored)" );
			}
			catch( IOException e )
			{
				System.err.println( "IOException Signature ignored" );
			}
		}
		return true;
	}

	public String
	toString()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "Username:" ).append( name ).append( '\n' )
		.append( usertrust ).append( '\n' );
		int len = sigs.length;
		for ( int i = 0; i < len; i++ )
			sb.append( sigs[i] ).append( '\n' ).append( sigtrust[i] ).append( '\n' );
		return sb.toString();
	}

	void
	restore( PacketInputStream reader )
	throws IOException
	{
		Packet p;
		if ( !((p = reader.readPacket()) instanceof UserId) )
			throw new IOException( "UserIdCetrificate must start with an UserId packet" );
		name = (UserId)p;
		if ( !((p = reader.readPacket()) instanceof KeyRingTrust) )
			throw new IOException( "KeyRingTrust expected not " + p.getClass().getName() );
		usertrust = (KeyRingTrust)p;
		Vector signatures = new Vector(), trusts = new Vector();
		while ( (p = reader.readPacket()) instanceof Signature ) 
		{
			if ( p == null )
				break;
			signatures.addElement( p );
			if ( !((p = reader.readPacket()) instanceof KeyRingTrust) )
				throw new IOException( "KeyRingTrust expected not " + p.getClass().getName() );
			trusts.addElement( p );
		}
		reader.pushback( p );
		initSigs( signatures, trusts );
	}
	
	void
	write( DataOutput out )
	throws IOException
	{
		PacketFactory.write( out, name );
		PacketFactory.write( out, usertrust ); // Overall trust for UserId
		int len = sigs.length;
		for ( int i = 0; i < len; i++ )
		{
			PacketFactory.write( out, sigs[i] );
			PacketFactory.write( out, sigtrust[i] );
		}
	}

        int
        getTrustValue()
        {
            return usertrust.trustValue() & 3;
        }
}


