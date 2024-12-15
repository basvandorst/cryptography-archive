/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 * Represents a PGP trust packet. Also contains some public constants
 * for the different levels of trust defined by PGP for keys.<p>
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
public final class KeyRingTrust extends Packet
{
	public final static KeyRingTrust UNDEF = new KeyRingTrust( (byte)0 );
	public final static KeyRingTrust UNKNOWN = new KeyRingTrust( (byte)1);
	public final static KeyRingTrust NOTUSUALLY = new KeyRingTrust( (byte)2 );
	public final static KeyRingTrust INTRODUCER1= new KeyRingTrust( (byte)3 );
	public final static KeyRingTrust INTRODUCER2 = new KeyRingTrust( (byte)4 );
	public final static KeyRingTrust USUALLY = new KeyRingTrust( (byte)5 );
	public final static KeyRingTrust ALWAYS = new KeyRingTrust( (byte)6 );
	public final static KeyRingTrust SECRING = new KeyRingTrust( (byte)7 );

	private byte trust;

	/**
	 * Create a new object with the trust level <code>trust0</code>.
	 */
	public KeyRingTrust( byte trust0 )
	{
		trust = trust0;
	}
	
	/**
	 * Read and create a trust packet from <code>in</code>.
	 */
	public KeyRingTrust( DataInput in, int length )
		throws IOException
	{
		super( in, length );
	}

	public int 
	trustValue()
	{
		return trust;
	}
	
	/**
	 * Read a trust packet from <code>in</code> with <code>length</code>.
	 */
	public void 
	read( DataInput in, int length )
		throws IOException
	{
		if ( length != 1 )
			throw new FormatException( "Length invalid." );
		trust = in.readByte();
	}

	/**
	 * Write this trust packet to <code>out</code>.
	 */
	public int
	write( DataOutput out )
		throws IOException
	{
		out.write( trust );
		return 1;
	}
	
	public String
	toString()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "trust packet: " );
		if ( (trust & 0x20) != 0 )
			return sb.append( "DISABLED" ).toString();
		switch( trust & 7 )
		{
		case 0:
			sb.append( "undef trust" );
			break;
		case 1:
			sb.append( "unknown trust" );
			break;
		case 2:
			sb.append( "usually NOT trusted" );
			break;
		case 3:
			sb.append( "introducer 1" );
			break;
		case 4:
			sb.append( "introducer 2" );
			break;
		case 5:
			sb.append( "usually DO trust" );
			break;
		case 6:
			sb.append( "always trust" );
			break;
		case 7:
			sb.append( "secring trust" );
			break;
		}

		if ( (trust & 0x80) != 0 )
			sb.append( " BUCK STOPS here" );
		return sb.toString();
	}

	/**
	 * What type of packet is this?<p>
	 * (Used by <code>PacketFactory</code>).
	 */
	public int getType()
	{
        	return Packet.TRUST;
	}
	
	/**
	 * What is this packet's default length?<p>
	 * (Used by <code>PacketFactory</code>).
	 */
	public int getDefaultLengthSpecifier()
	{
		return 1;
	}

}

