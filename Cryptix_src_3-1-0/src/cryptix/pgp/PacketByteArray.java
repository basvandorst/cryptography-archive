// This file is currently locked by jbaker@madge.com
//
// $Log: PacketByteArray.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Vector;

/**
 * A PacketByteArray is a sequence of PGP packets stored as an array of bytes.
 * <p>
 * This class abstracts this idea, making it possible to specify that a
 * class that requires packets as input won't accept just <em>any old</em>
 * byte array, but only byte arrays that contain packets. It can also do
 * syntax checking, and extract information about the packets therein.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Jill Baker
 * @since  Cryptix 2.2 or earlier
 */
public class PacketByteArray
{
    protected byte[] data;
    protected Vector offsetVector;

    public PacketByteArray(byte[] buffer) throws FormatException
    {
        data = buffer;
        parseData();
    }

    public PacketByteArray(PacketInputStream in)
        throws FormatException, IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        for (int c = in.read(); c != -1; c = in.read()) out.write(c);
        data = out.toByteArray();
        parseData();
    }

    private void parseData() throws FormatException
    {
        int offset = 0;
        offsetVector = new Vector();

        try
        {
            while (offset < data.length)
            {
                int ctb = data[ offset++ ];

                // Make sure this looks like a CTB
                if (!Packet.isCTB(ctb))
                {
                    throw new FormatException("PacketByteArray: Non-CTB found where CTB expected");
                }

                // Make sure the type is one we recognise
                int type = Packet.ctbToType(ctb);
                if (!Packet.isLegalPacketType(type))
                {
                    throw new FormatException("PacketByteArray: Unknown packet type: " + type);
                }

                // Get the length and make sure it looks sensible
                int lol = Packet.ctbToLengthSpecifier(ctb);
                int length = 0;
                switch (lol)
                {
                    case 2:
                        length <<= 8; length += data[ offset++ ] & 0xFF;
                        length <<= 8; length += data[ offset++ ] & 0xFF;
                    case 1:
                        length <<= 8; length += data[ offset++ ] & 0xFF;
                    case 0:
                        length <<= 8; length += data[ offset++ ] & 0xFF;
                        break;

                    case 3:
                        length = data.length - offset;
                        break;
                }
                if (length < 0)
                {
                    throw new FormatException("PacketByteArray: Negative packet length");
                }

                // Move onto next packet
                offset += length;
                offsetVector.addElement(new Integer(offset));
            }
        }
        catch (ArrayIndexOutOfBoundsException e)
        {
            throw new FormatException("PacketByteArray: Unexpected end of data");
        }
    }

    public int numPackets()
    {
        return offsetVector.size();
    }

    private int privatePacketOffset(int packetNum)
    {
        if (packetNum == 0) return 0;
        Integer n = (Integer)offsetVector.elementAt(packetNum - 1);
        return n.intValue();
    }

    public int getPacketOffset(int packetNum)
    {
        if ((packetNum < 0) || (packetNum >= offsetVector.size()))
            throw new ArrayIndexOutOfBoundsException();

        return privatePacketOffset(packetNum);
    }

    public int getPacketLength(int packetNum)
    {
        if ((packetNum < 0) || (packetNum >= offsetVector.size()))
            throw new ArrayIndexOutOfBoundsException();

        return privatePacketOffset(packetNum + 1)
             - privatePacketOffset(packetNum);
    }

    public int getPacketType(int packetNum)
    {
        if ((packetNum < 0) || (packetNum >= offsetVector.size()))
            throw new ArrayIndexOutOfBoundsException();

        return Packet.ctbToType(data[ privatePacketOffset(packetNum) ]);
    }

    public byte[] toByteArray()
    {
        return data;
    }

    public String toString()
    {
        String s = "PacketByteArray[ ";
        for (int i=0; i<numPackets(); i++) s += getPacketType(i) + " ";
        return s + "]";
    }

    public PacketInputStream toPacketInputStream()
    {
        return new PacketInputStream(new ByteArrayInputStream(data));
    }

    /**
     * Concatenates another PacketByteArray to this one.
     */
    public void add(PacketByteArray suffix) throws FormatException
    {
        // Concatenating nothing has no effect
        if (suffix.numPackets() == 0) return;

        byte[] suffixBytes = suffix.toByteArray();
        if (numPackets() == 0)
        {
            data = suffixBytes;
        }
        else
        {
            if (Packet.ctbToLengthSpecifier(getPacketOffset(numPackets() - 1)) == 3)
                throw new FormatException(
                "PacketByteArray: Cannot concatenate onto unterminated packet");

            byte[] newBuffer = new byte[ data.length + suffixBytes.length ];
            System.arraycopy(data, 0, newBuffer, 0, data.length);
            System.arraycopy(suffixBytes, 0, newBuffer, data.length, suffixBytes.length);
            data = newBuffer;
        }
        parseData();
    }
}
