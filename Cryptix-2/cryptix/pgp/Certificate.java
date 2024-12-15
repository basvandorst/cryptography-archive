/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
 
/**
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Date;

abstract class Certificate extends Packet
{
	private static final byte VERSION = 2;
	private static final int LENGTH  = 7;
	// length of one day in milliseconds.
	private static final int ONE_DAY = 1000 * 60 * 60 * 24;
	protected transient Date timeStamp;
	protected int unixTime;
	protected int daysValid;
	protected byte version;

	Certificate()
	{
		this( new Date(), 0 ); // today and forever.
	}

	Certificate( Date time0, int valid0 )
	{
		version = VERSION;
		timeStamp = time0;
		daysValid = valid0;
		unixTime = (int)( timeStamp.getTime() / 1000L );
	}
	
	Certificate( DataInput in, int length )
	throws IOException
	{
		super( in, length );
	}

	public void
	read( DataInput in, int length )
	throws IOException
	{
		if ( length < LENGTH )
			throw new FormatException( "Length too short." );
		version = in.readByte(); // version byte..
		unixTime = in.readInt();
		timeStamp = new Date( ( (long)unixTime & 0xFFFFFFFFL ) * 1000L );
		daysValid = in.readShort();
	}

	public int
	write( DataOutput out )
	throws IOException
	{
		out.write( version );
		out.writeInt( unixTime );
		out.writeShort( daysValid );
		return LENGTH;
	}
	
	public String
	toString()
	{
		return "version: " + version + " created:" + timeStamp + 
		" valid for" + ( daysValid == 0 ? "ever" : ( " " + daysValid + " days" ) );
	}

	public Date
	validUntil()
	{
		// If certificate is valid indefinitely, return null
		if ( daysValid == 0 )
			return null;
		else
			return new Date( timeStamp.getTime() + (daysValid * (long)(ONE_DAY)) );
	}

	public boolean
	expired()
	{
		/**
		 * Is this certificate still valid?
		 * @return True if it is
		 */
		Date expires;
		if ( (expires = validUntil()) == null )
			return false; // Certificate is valid indefinitely
		else // check against today's date.
			return new Date().after( expires );
	} 

    public int getType()
    {
        return Packet.PUBLIC_KEY;
    }

}
