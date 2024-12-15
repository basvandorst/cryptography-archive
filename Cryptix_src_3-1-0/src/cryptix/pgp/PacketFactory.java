// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PacketFactory.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/03/10  Jill Baker
//   Replaced long instanceof chain with one line in getHeaderFor()
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Replaced constants to denote packet types with those in Packet.
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

import java.io.*;

/**
 * This class contains static methods to read and write encoded packets.
 * When a packet is read, an instance of the correct subclass of Packet
 * is created according to the encoded packet's type field.
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
 *
 * @see cryptix.pgp.Packet
 */
public final class PacketFactory
{
    private PacketFactory() {} // static methods only

    // @deprecated
    // @see constants in Packet
    // public static final byte PublicKeyEncryptedType = 1;
    // public static final byte SignatureType = 2;
    // public static final byte SecretKeyCertificateType = 5;
    // public static final byte PublicKeyCertificateType = 6;
    // public static final byte CompressedDataType = 8;
    // public static final byte ConvEncryptedDataType = 9;
    // public static final byte LiteralDataType = 11;
    // public static final byte KeyRingTrustType = 12;
    // public static final byte UserIdType = 13;
    // public static final byte CommentType = 14;

    public static Packet read(DataInput in) throws IOException
    {
        int len;
        int type;
        try
        {
            PacketHeader header = new PacketHeader(in);
            len = header.len;
            type = header.type;
        }
        catch (EOFException e)
        {
            return null;
        }

        switch (type)
        {
            case Packet.SECRET_KEY:
                return new SecretKeyCertificate(in, len);

            case Packet.PUBLIC_KEY:
                return new PublicKeyCertificate(in, len);

            case Packet.TRUST:
                return new KeyRingTrust(in, len);

            case Packet.PUBLIC_KEY_ENCRYPTED:
                return new PublicKeyEncrypted(in, len);

            case Packet.COMPRESSED:
                return new CompressedData(in, len);

            case Packet.CONV_ENCRYPTED:
                return new ConvEncryptedData(in, len);

            case Packet.PLAINTEXT:
                return new LiteralData(in, len);

            case Packet.SIGNATURE:
                return new Signature(in, len);

            case Packet.USER_ID:
                return new UserId(in, len);

            case Packet.COMMENT:
                return new Comment(in, len);

            default:
                throw new FormatException("Unknown packet type ("+type+")");
        }
    }

    public static void
    write(DataOutput out, Packet pkt)
    throws IOException
    {
        byte buf[] = pkt.save();
        getHeaderFor(pkt, buf.length).write(out);
        out.write(buf);
    }

    public static byte[]
    save(Packet pkt)
    throws IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        write(new DataOutputStream(out), pkt);
        return out.toByteArray();
    }

    // I don't like the name 'load' - 'restore' would have been better.
    public static Packet
    load(byte[] buf)
    throws IOException
    {
        return read(new DataInputStream(new ByteArrayInputStream(buf)));
    }

    private static PacketHeader
    getHeaderFor(Packet pkt, int length)
    throws IOException
    {
        return new PacketHeader(pkt.getType(), length,
            pkt.getDefaultLengthSpecifier());
    }
}
