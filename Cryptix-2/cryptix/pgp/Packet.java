/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
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
 * @author Original Author not stated
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
public abstract class Packet
{
    // CTB semantics constants
    private static final int CTB_DESIGNATOR       =0x80;
    private static final int CTB_TYPE_MASK        =0x7C;
    private static final int CTB_TYPE_SHIFT       =2;
    private static final int CTB_LLEN_MASK        =0x03;

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
    public static final boolean isCTB( int ctb )
    {
        return (ctb & CTB_DESIGNATOR) == CTB_DESIGNATOR;
    }

    public static final int ctbToType( int ctb )
    {
        return (ctb & CTB_TYPE_MASK) >> CTB_TYPE_SHIFT;
    }

    public static final int ctbToLengthSpecifier( int ctb )
    {
        return (ctb & CTB_LLEN_MASK);
    }

    public static final int makeCTB( int type, int lengthSpecifier )
    {
        return CTB_DESIGNATOR | (type << CTB_TYPE_SHIFT) | lengthSpecifier;
    }

    protected Packet()
	{
	}

	protected Packet( DataInput in, int length )
		throws IOException
	{
		read( in, length );
	}

    public final static boolean isLegalPacketType( int n )
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
	load( byte buf[] )
		throws IOException
	{
		load( buf, 0, buf.length );
	}

	public final void
	load( byte buf[], int offset )
		throws IOException
	{
		int len = buf.length - offset;
		if ( len <= 0 )
			throw new IOException( "Offset too big or buffer too short." );
		load( buf, offset, len );
	}

	public final void
	load( byte buf[], int offset, int length )
		throws IOException
	{
		InputStream in = new ByteArrayInputStream( buf );
		if ( offset > 0 )
			in.skip( offset );
		read( new DataInputStream( in ), length );
	}

	public final byte[]
	save()
		throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		write( new DataOutputStream( out ) );
		return out.toByteArray();
	}

    // Subclasses may override this
    public int getDefaultLengthSpecifier() { return 2; }

    public abstract int getType();
	public abstract void read( DataInput in, int length ) throws IOException;
	public abstract int write( DataOutput out ) throws IOException;
}
