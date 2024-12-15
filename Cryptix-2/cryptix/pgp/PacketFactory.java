/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import java.io.*;

/**
 * @author Original author not stated
 * @author Jill Baker
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
public final class PacketFactory
{
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

	private PacketFactory()
	{
	}

    public static final Packet read( DataInput in ) throws IOException
	{
            int len;
            int type;
            try
            {
                PacketHeader header = new PacketHeader(in);
                len = header.len;
                type = header.type;
            }
            catch ( EOFException e )
            {
                return null;
            }

            switch (type)
            {
                case Packet.SECRET_KEY:
                    return new SecretKeyCertificate( in, len );

                case Packet.PUBLIC_KEY:
                    return new PublicKeyCertificate( in, len );

                case Packet.TRUST:
                    return new KeyRingTrust( in, len );

                case Packet.PUBLIC_KEY_ENCRYPTED:
                    return new PublicKeyEncrypted( in, len );

                case Packet.COMPRESSED:
                    return new CompressedData( in, len );

                case Packet.CONV_ENCRYPTED:
                    return new ConvEncryptedData( in, len );

                case Packet.PLAINTEXT:
                    return new LiteralData( in, len );

                case Packet.SIGNATURE:
                    return new Signature( in, len );

                case Packet.USER_ID:
                    return new UserId( in, len );

                case Packet.COMMENT:
                    return new Comment( in, len );

                default:
                    throw new FormatException("Unknown packet type ("+type+")");
		}
	}

	public static final void
	write( DataOutput out, Packet pkt )
		throws IOException
	{
		byte buf[] = pkt.save();
		getHeaderFor( pkt, buf.length ).write( out );
		out.write( buf );
	}

	public static final byte[]
	save(Packet pkt)
		throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		write(new DataOutputStream(out), pkt);
		return out.toByteArray();
	}

	// I don't like the name 'load' - 'restore' would have been better
	public static final Packet
	load(byte buf[])
		throws IOException
	{
		return read(new DataInputStream(new ByteArrayInputStream(buf)));
	}


	private static final PacketHeader
	getHeaderFor( Packet pkt, int length )
		throws IOException
	{
        return new PacketHeader( pkt.getType(), length, pkt.getDefaultLengthSpecifier() );
	}
}
