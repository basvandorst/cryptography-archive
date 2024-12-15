/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
 
/**
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

package cryptix.pgp;

import java.util.Date;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.util.Streamable;

public final class LiteralData extends Packet
{
	private Date timestamp;
	private String filename;
	private char mode;
	private byte buffer[];

	public LiteralData( byte data[] )
	{
		this( data, "data", 'b' );
	}

	public LiteralData( byte data[], String filename0, char mode0 )
	{
		filename = filename0.length() < 255 ? filename0 : filename0.substring( 0, 255 );
		mode = mode0;
		timestamp = new Date(); // now.
		buffer = data;
	}

	public LiteralData( DataInput in, int length )
		throws IOException
	{
		super( in, length );
	}

	public Date timestamp() { return timestamp; }
	public String filename() { return filename; }
	public char mode() { return mode; }
	public boolean modeIsText() { return mode == 't'; }
	public boolean modeIsBinary() { return mode == 'b'; }

        public byte[] getBuffer()
        {
            return buffer;
        }

	public byte[]
	data()
	{
		int len;
		byte tmp[] = new byte[len = buffer.length];
		System.arraycopy( buffer, 0, tmp, 0, len );
		return tmp;
	}

	public void 
	read( DataInput in, int length )
		throws IOException
	{
		if (length < 6)
			throw new FormatException("Length too short");

		mode = (char)in.readByte(); // mode
		if (mode != 'b' && mode != 't')
			throw new FormatException("Invalid mode");

		int nameLength = in.readByte() & 0xFF;
		if ( nameLength > 0 )
		{
			byte tmp[] = new byte[nameLength];
			in.readFully( tmp );
			filename = new String( tmp, 0 );
		}
		else
			filename = "";

		timestamp = new Date((in.readInt() & 0xFFFFFFFFL) * 1000L);

		buffer = new byte[length-nameLength-6];
		in.readFully(buffer);
	}

	public int
	write( DataOutput out )
		throws IOException
	{
		int nameLength = filename.length();
		out.writeByte( (byte)mode );
		out.writeByte( (byte)nameLength );
		out.writeBytes( filename );
		out.writeInt((int)(timestamp.getTime()/1000L));
		out.write( buffer );
		return 6 + nameLength + buffer.length;
	}

        public int getType()
        {
                return Packet.PLAINTEXT;
        }
}
