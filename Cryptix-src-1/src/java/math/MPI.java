/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 * This class is used to read and write MPI big integers.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 02/07/1995
 * @author      Systemics Ltd. (Michael Wynn)
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
}

