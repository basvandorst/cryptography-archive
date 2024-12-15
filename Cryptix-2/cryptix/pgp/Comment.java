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

public final class Comment extends Packet
{
	protected String data;

	public Comment( String comment )
	{
		data = comment;
	}

	public Comment( DataInput in, int length )
	throws IOException
	{
		super( in, length );
	}

	public void 
	read( DataInput in, int length )
		throws IOException
	{
		if (length > 0 )
		{
			byte buf[] = new byte[length];
			in.readFully( buf );
			data = new String( buf, 0 );
		}
		else 
			data = "";
	}

	public int
	write( DataOutput out )
		throws IOException
	{
		out.writeBytes( data );
		return data.length();
	}
	
	public String
	toString()
	{
		return data;
	}

    public int getType()
    {
        return Packet.COMMENT;
    }

    public int getDefaultLengthSpecifier()
    {
        return 1;
    }

}
