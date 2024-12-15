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
import cryptix.util.Streamable;

public final class PacketHeader extends Streamable
{
    public int type;
	public int len;
	public int lensize;
	
    public PacketHeader( int type_, int len_ )
	{
		type = type_;
		len = len_;
		lensize = 0;	// Don't care
	}
	
	//
	// PGP is brain dead in this area.
	// The problem is that packets such as UserID packets
	// must have only a one byte header, and others
	// such as PublicKeyCertificate must have a two byte
	// header.
	// This is why a length size parameter can be used
	// 
    public PacketHeader( int type_, int len_, int lensize_ )
	{
		type = type_;
		len = len_;
		lensize = lensize_;
	}


	
	public PacketHeader( DataInput in )
		throws IOException
	{
		read( in );
	}

        public void read( DataInput in ) throws IOException
	{
        byte ctb = in.readByte();

        type   = (byte)Packet.ctbToType( ctb );
        byte t = (byte)Packet.ctbToLengthSpecifier( ctb );
		switch (t)
		{
                    case 0: len = in.readByte()  & 0xFF;   break;
                    case 1: len = in.readShort() & 0xFFFF; break;
                    case 2: len = in.readInt();            break;
                    case 3: len = -1;                      break;
                    default:
                        throw new FormatException("Bad value for CTB ("+t+")");
		}

        lensize = 0; // don't care
	}
	
	public void
	write( DataOutput out )
		throws IOException
	{
		// If don't know and not a stream,
		// then choose a suitable value
		if (lensize == 0 && len != -1)
		{
			if (len < 256)
				lensize = 1;
			else if (len < 65536)
				lensize = 2;
			else
				lensize = 4;
		}

		if (lensize == 1 && len > 255)
			throw new FormatException("packet overflow");

		if (lensize == 2 && len > 65536)
			lensize = 3;

		if (lensize == 0)
		{
            out.write( Packet.makeCTB( type, 3 ));
		}
		else if (lensize == 1)
		{
            out.write( Packet.makeCTB( type, 0 ));
			out.write( len & 0xFF );
		}
		else if (lensize == 2)
		{
            out.write( Packet.makeCTB( type, 1 ));
			out.writeShort( len & 0xFFFF );
		}
		else
		{
            out.write( Packet.makeCTB( type, 2 ));
			out.writeInt( len );
		}
	}

	public static final String
	toString( PacketHeader hdr )
	{
		return "Packet header: type="+hdr.type+", len="+hdr.len+", lensize="+hdr.lensize;
	}
}
