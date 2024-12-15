// This file is currently unlocked (change this line if you lock the file)
//
// $Log: UserId.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.mime.LegacyString;

/**
 * This class represents a User ID Packet.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public final class UserId extends Packet
{
    protected String data;

    public UserId(String data_)
    {
        data = data_;
    }

    public UserId(DataInput in, int length)
    throws IOException
    {
        super(in, length);
    }

    public void read(DataInput in, int length)
    throws IOException
    {
        if (length > 0)
        {
            byte buf[] = new byte[length];
            in.readFully(buf);
            data = LegacyString.toString(buf);
        }
        else 
            data = "";
    }

    public int write(DataOutput out)
    throws IOException
    {
        out.writeBytes(data);
        return data.length();
    }
    
    public String toString()
    {
        return data;
    }

    public int getType()
    {
        return Packet.USER_ID;
    }

    public int getDefaultLengthSpecifier()
    {
        return 1;
    }
}
