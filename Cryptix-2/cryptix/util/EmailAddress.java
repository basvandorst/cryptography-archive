package cryptix.util;

import java.io.IOException;
import java.util.Vector;

/**
 * @author Ian Brown
 * @version 1.0
 * Represents an Internet e-mail address - username and host can be retrieved separately.
 * This may seem rather banal.
 * It's useful for three reasons, however:
 *  - To fetch a user's public key from a distributed keyserver system, you need to know the hostname - this 
 *    class saves duplication of effort
 *  - It allows KeyRing.getKey() to be overloaded rather than having a separate getKeyByAddress()
 *  - 'Friendly' e-mail addresses of the form "Friendly Name <dfh343@host.domain>" are automatically dealt with
 */
public final class EmailAddress
{
	private String address;
	private int atPos;
	private Vector hostpath = null;

	public EmailAddress( String friendlyAddress )
	throws IOException
	{
		// Actual address may  be between < and >
		int startAddress = friendlyAddress.indexOf( "<") + 1;
		int endAddress = friendlyAddress.lastIndexOf( ">");
		// indexOf returns -1 if char can't be found
		if ( startAddress != 0 && endAddress != -1 )
			address = friendlyAddress.substring( startAddress, endAddress );
		else address = friendlyAddress;
		if ( ( atPos = address.indexOf( "@") ) == -1  )
			throw new IOException( "Not a valid Internet e-mail address" );
	}

	public String
	toString()
	{
		return address;
	}

	public String
	user()
	{
		return address.substring( 0, atPos );
	}

	public String
	host()
	{
		return address.substring( atPos + 1, address.length() );
	}

	/**
	 * Useful for clients of distributed keyservers who need to query each host in turn
	 */
	public Vector
	hostPath()
	{
		if ( hostpath == null )
		{
			// Do initialisation here rather than in constructor as most
			// instantiations of this class won't need these details
			hostpath = new Vector();
			int lastDot = atPos;
			int thisDot;
			while ( ( thisDot = address.indexOf( ".", lastDot + 1 ) ) != -1 )
			{
				hostpath.addElement( address.substring( lastDot + 1, thisDot ) );
				lastDot = thisDot;
			}
			hostpath.addElement( address.substring( lastDot + 1 ) ); // Get last domain
		}
		return hostpath;
	}

	public boolean equals( Object obj )
	{
		if ( obj instanceof EmailAddress )
		{
			EmailAddress check = (EmailAddress)obj;
			return ( check.toString().equals( toString() ) );
		}
		else return false;
	}

	public int hashCode()
	{
		return address.hashCode();
	}

	public static final void main( String args[] )
	throws IOException
	{
		EmailAddress myAddress = new EmailAddress( "Ian Brown <Ian.Brown@newcastle.ac.uk>" );
		System.out.println( "User: " + myAddress.user() );
		Vector hosts = myAddress.hostPath();
		for ( int i = 0; i < hosts.size(); i++ )
		{
			System.out.println( "Next domain: " + hosts.elementAt( i ) );
		}
	}
}
