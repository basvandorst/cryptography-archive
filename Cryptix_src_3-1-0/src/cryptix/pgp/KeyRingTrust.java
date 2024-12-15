// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyRingTrust.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/04/03  Ian Brown
//   tidied up and added doc comments
//
// Revision 0.2.5.1  1997/03/10  Jill Baker
//   added methods getType() and getDefaultLengthSpecifier()
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

/**
 * Represents a PGP trust packet. Also contains some public constants
 * for the different levels of trust defined by PGP for keys.
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
public final class KeyRingTrust extends Packet
{
    public final static KeyRingTrust UNDEF = new KeyRingTrust((byte)0);
    public final static KeyRingTrust UNKNOWN = new KeyRingTrust((byte)1);
    public final static KeyRingTrust NOTUSUALLY = new KeyRingTrust((byte)2);
    public final static KeyRingTrust INTRODUCER1= new KeyRingTrust((byte)3);
    public final static KeyRingTrust INTRODUCER2 = new KeyRingTrust((byte)4);
    public final static KeyRingTrust USUALLY = new KeyRingTrust((byte)5);
    public final static KeyRingTrust ALWAYS = new KeyRingTrust((byte)6);
    public final static KeyRingTrust SECRING = new KeyRingTrust((byte)7);

    private byte trust;

    /**
     * Creates a new object with the trust level <i>trust</i>.
     */
    public KeyRingTrust(byte trust)
    {
        this.trust = trust;
    }
    
    /**
     * Reads and creates a trust packet from <i>in</i> with the given
     * length.
     */
    public KeyRingTrust(DataInput in, int length)
        throws IOException
    {
        super(in, length);
    }

    public int 
    trustValue()
    {
        return trust;
    }
    
    /**
     * Reads a trust packet from <i>in</i> with the given length.
     */
    public void 
    read(DataInput in, int length)
        throws IOException
    {
        if (length != 1)
            throw new FormatException("Length invalid.");
        trust = in.readByte();
    }

    /**
     * Writes this trust packet to <i>out</i>.
     */
    public int
    write(DataOutput out)
        throws IOException
    {
        out.write(trust);
        return 1;
    }
    
    public String
    toString()
    {
        StringBuffer sb = new StringBuffer();
        sb.append("trust packet: ");
        if ((trust & 0x20) != 0)
            return sb.append("DISABLED").toString();
        switch(trust & 7)
        {
        case 0:
            sb.append("undef trust");
            break;
        case 1:
            sb.append("unknown trust");
            break;
        case 2:
            sb.append("usually NOT trusted");
            break;
        case 3:
            sb.append("introducer 1");
            break;
        case 4:
            sb.append("introducer 2");
            break;
        case 5:
            sb.append("usually DO trust");
            break;
        case 6:
            sb.append("always trust");
            break;
        case 7:
            sb.append("secring trust");
            break;
        }

        if ((trust & 0x80) != 0)
            sb.append(" BUCK STOPS here");
        return sb.toString();
    }

    /**
     * What type of packet is this?
     * (used by PacketFactory).
     */
    public int getType()
    {
        return Packet.TRUST;
    }
    
    /**
     * What is this packet's default length?
     * (used by PacketFactory).
     */
    public int getDefaultLengthSpecifier()
    {
        return 1;
    }
}
