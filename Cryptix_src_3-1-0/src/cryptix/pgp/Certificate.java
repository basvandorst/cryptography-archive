// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Certificate.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/05/07  Ian Brown
//   added method timestamp() for Jill's PGP app
//
// Revision 0.2.5.1  1997/03/10  Jill Baker
//   added method getType()
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
import java.util.Date;

/**
 * This class represents a Packet of type PUBLIC_KEY.
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
        this(new Date(), 0); // today and forever.
    }

    Certificate(Date time0, int valid0)
    {
        version = VERSION;
        timeStamp = time0;
        daysValid = valid0;
        unixTime = (int)(timeStamp.getTime() / 1000L);
    }
    
    Certificate(DataInput in, int length)
    throws IOException
    {
        super(in, length);
    }

    public void
    read(DataInput in, int length)
    throws IOException
    {
        if (length < LENGTH)
            throw new FormatException("Length too short.");
        version = in.readByte(); // version byte..
        unixTime = in.readInt();
        // Y2038 date bug here?
        timeStamp = new Date(((long)unixTime & 0xFFFFFFFFL) * 1000L);
        daysValid = in.readShort();
    }

    public int
    write(DataOutput out)
    throws IOException
    {
        out.write(version);
        out.writeInt(unixTime);
        out.writeShort(daysValid);
        return LENGTH;
    }
    
    public String
    toString()
    {
        return "version: " + version + " created:" + timeStamp + 
        " valid for" + (daysValid == 0 ? "ever" : (" " + daysValid + " days"));
    }

    public Date
    validUntil()
    {
        // If certificate is valid indefinitely, return null
        if (daysValid == 0)
            return null;
        else
            return new Date(timeStamp.getTime() + (daysValid * (long)(ONE_DAY)));
    }

    public boolean
    expired()
    {
        /**
         * Is this certificate still valid?
         * @return True if it is
         */
        Date expires;
        if ((expires = validUntil()) == null)
            return false; // Certificate is valid indefinitely
        else // check against today's date.
            return new Date().after(expires);
    } 

    public int getType()
    {
        return Packet.PUBLIC_KEY;
    }
}
