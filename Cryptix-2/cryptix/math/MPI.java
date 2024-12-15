/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.math;

import java.io.InputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * This class is used to read and write MPI big integers.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public final class MPI
{
	public static final BigInteger
	read( DataInput in )
	throws IOException
	{
		int bitlen = in.readShort() & 0xFFFF;
		byte buffer[] = new byte[( bitlen + 7 ) / 8];
		in.readFully( buffer );
		return new BigInteger( buffer );
	}
	
	public static final void
	write( DataOutput out, BigInteger bigInt )
	throws IOException
	{
		out.writeShort( bigInt.bitLength() );
		out.write( bigInt.toByteArray() );
	}

	public static final BigInteger
	load( byte buf[] )
	throws IOException
	{
		return load( buf, 0 );
	}

	public static final BigInteger
	load( byte buf[], int offset )
	throws IOException
	{
		InputStream in = new ByteArrayInputStream( buf );
		if ( offset > 0 )
			in.skip( offset );
		return read( new DataInputStream( in ) );
	}

	public static final byte[]
	save( BigInteger bigInt )
	throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		write( new DataOutputStream( out ), bigInt );
		return out.toByteArray();
	}
}
