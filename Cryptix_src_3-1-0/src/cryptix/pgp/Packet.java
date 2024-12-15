// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Packet.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/03/10  Jill Baker
//   Added methods getType() and getDefaultLengthSpecifier()
//   Added packet type MESSAGE_DIGEST
//   Added CTB functions
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Added constants to denote packet types.
//   Added the method isLegalPacketType().
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

import java.io.InputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import cryptix.util.StreamInterface;
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
 * @author original author not stated
 * @author Jill Baker
 * @since  Cryptix 2.2 or earlier
 */
public abstract class Packet
{
    // CTB semantics constants
    private static final int CTB_DESIGNATOR      = 0x80;
    private static final int CTB_TYPE_MASK       = 0x7C;
    private static final int CTB_TYPE_SHIFT      = 2;
    private static final int CTB_LLEN_MASK       = 0x03;

    // Legal packet types
    public static final int PUBLIC_KEY_ENCRYPTED = 1;
    public static final int SIGNATURE            = 2;
    public static final int MESSAGE_DIGEST       = 3;   // new
    public static final int SECRET_KEY           = 5;
    public static final int PUBLIC_KEY           = 6;
    public static final int COMPRESSED           = 8;
    public static final int CONV_ENCRYPTED       = 9;
    public static final int PLAINTEXT            = 11;
    public static final int TRUST                = 12;
    public static final int USER_ID              = 13;
    public static final int COMMENT              = 14;

    // methods to extract information from a CTB
    public static final boolean isCTB(int ctb)
    {
        return (ctb & CTB_DESIGNATOR) == CTB_DESIGNATOR;
    }

    public static final int ctbToType(int ctb)
    {
        return (ctb & CTB_TYPE_MASK) >> CTB_TYPE_SHIFT;
    }

    public static final int ctbToLengthSpecifier(int ctb)
    {
        return (ctb & CTB_LLEN_MASK);
    }

    public static final int makeCTB(int type, int lengthSpecifier)
    {
        return CTB_DESIGNATOR | (type << CTB_TYPE_SHIFT) | lengthSpecifier;
    }

    protected Packet()
    {
    }

    protected Packet(DataInput in, int length)
    throws IOException
    {
        read(in, length);
    }

    public static boolean isLegalPacketType(int n)
    {
        switch (n)
        {
            case PUBLIC_KEY_ENCRYPTED:
            case SIGNATURE:
            case MESSAGE_DIGEST:
            case SECRET_KEY:
            case PUBLIC_KEY:
            case COMPRESSED:
            case CONV_ENCRYPTED:
            case PLAINTEXT:
            case TRUST:
            case USER_ID:
            case COMMENT:
                return true;

            default:
                return false;
            }
        }

    public final void
    load(byte buf[])
        throws IOException
    {
        load(buf, 0, buf.length);
    }

    public final void
    load(byte buf[], int offset)
        throws IOException
    {
        int len = buf.length - offset;
        if (len <= 0)
            throw new IOException("Offset too big or buffer too short.");
        load(buf, offset, len);
    }

    public final void
    load(byte buf[], int offset, int length)
        throws IOException
    {
        InputStream in = new ByteArrayInputStream(buf);
        if (offset > 0)
            in.skip(offset);
        read(new DataInputStream(in), length);
    }

    public final byte[]
    save()
        throws IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        write(new DataOutputStream(out));
        return out.toByteArray();
    }

    // Subclasses may override this
    public int getDefaultLengthSpecifier() { return 2; }

    public abstract int getType();
    public abstract void read(DataInput in, int length) throws IOException;
    public abstract int write(DataOutput out) throws IOException;
}
