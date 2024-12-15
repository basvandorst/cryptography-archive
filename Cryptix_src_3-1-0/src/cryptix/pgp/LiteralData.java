// This file is currently unlocked (change this line if you lock the file)
//
// $Log: LiteralData.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.util.Date;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.util.Streamable;
import cryptix.mime.LegacyString;

/**
 * This class represents a literal data Packet.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since Cryptix 2.2 or earlier
 */
public final class LiteralData extends Packet
{
    private Date timestamp;
    private String filename;
    private char mode;
    private byte[] buffer;

    public LiteralData(byte[] data)
    {
        this(data, "data", 'b');
    }

    public LiteralData(byte[] data, String filename, char mode)
    {
        this.filename = filename.length() < 255 ? filename
                                                : filename.substring(0, 255);
        this.mode = mode;
        timestamp = new Date(); // now.
        buffer = data;
    }

    public LiteralData(DataInput in, int length)
    throws IOException
    {
        super(in, length);
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

    public byte[] data()
    {
        int len;
        byte[] tmp = new byte[len = buffer.length];
        System.arraycopy(buffer, 0, tmp, 0, len);
        return tmp;
    }

    public void read(DataInput in, int length)
    throws IOException
    {
        if (length < 6)
            throw new FormatException("Length too short");

        mode = (char) in.readByte(); // mode
        if (mode != 'b' && mode != 't')
            throw new FormatException("Invalid mode");

        int nameLength = in.readByte() & 0xFF;
        if (nameLength > 0)
        {
            byte[] tmp = new byte[nameLength];
            in.readFully(tmp);
            filename = LegacyString.toString(tmp);
        }
        else
            filename = "";

        timestamp = new Date((in.readInt() & 0xFFFFFFFFL) * 1000L);

        buffer = new byte[length-nameLength-6];
        in.readFully(buffer);
    }

    public int write(DataOutput out)
    throws IOException
    {
        int nameLength = filename.length();
        out.writeByte((byte)mode);
        out.writeByte((byte)nameLength);
        out.writeBytes(filename);
        out.writeInt((int) (timestamp.getTime()/1000L));
        out.write(buffer);
        return 6 + nameLength + buffer.length;
    }

    public int getType()
    {
        return Packet.PLAINTEXT;
    }
}
