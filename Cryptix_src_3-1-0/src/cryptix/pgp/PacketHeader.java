// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PacketHeader.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/03/11  Jill Baker
//   Changed from hardcoded shifts to CTB functions
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Constructors now accept type as an int, not a byte, which avoids
//   the need for the calling routine to recast.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
//   If this file is unlocked, or locked by someone other than you, then you
//   may download this file for incorporation into your build, however any
//   changes you make to your copy of this file will be considered volatile,
//   and will not be incorporated into the master build which is maintained
//   at this web site.
//
//   If you wish to make permanent changes to this file, you must wait until
//   it becomes unlocked (if it is not unlocked already), and then lock it
//   yourself. Whenever the file is locked by you, you may upload new master
//   versions to the master site. Make sure you update the history information
//   above. When your changes are complete, you should relinquish the lock to
//   make it available for other developers.
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import cryptix.util.Streamable;

/**
 * DOCUMENT ME.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @since Cryptix 2.2 or earlier
 */
public final class PacketHeader extends Streamable
{
    public int type;
    public int len;
    public int lensize;
    
    public PacketHeader(int type, int len)
    {
        this.type = type;
        this.len = len;
        lensize = 0;    // Don't care
    }
    
    //
    // PGP is brain dead in this area.
    // The problem is that packets such as UserID packets
    // must have only a one byte header, and others
    // such as PublicKeyCertificate must have a two byte
    // header.
    // That is why a length size parameter must be used.
    //
    public PacketHeader(int type, int len, int lensize)
    {
        this.type = type;
        this.len = len;
        this.lensize = lensize;
    }
    
    public PacketHeader(DataInput in)
    throws IOException
    {
        read(in);
    }

    public void read(DataInput in) throws IOException
    {
        byte ctb = in.readByte();

        type   = (byte) Packet.ctbToType(ctb);
        byte t = (byte) Packet.ctbToLengthSpecifier(ctb);
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
    write(DataOutput out)
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
            out.write(Packet.makeCTB(type, 3));
        }
        else if (lensize == 1)
        {
            out.write(Packet.makeCTB(type, 0));
            out.write(len & 0xFF);
        }
        else if (lensize == 2)
        {
            out.write(Packet.makeCTB(type, 1));
            out.writeShort(len & 0xFFFF);
        }
        else
        {
            out.write(Packet.makeCTB(type, 2));
            out.writeInt(len);
        }
    }

    // why is this static?
    public static String
    toString(PacketHeader hdr)
    {
        return "Packet header: type="+hdr.type+", len="+hdr.len+", lensize="+hdr.lensize;
    }
}
