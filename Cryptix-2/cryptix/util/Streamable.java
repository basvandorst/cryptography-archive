/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.util;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * This is a Streamable abstract base class that
 * allows provides the load and save methods.
 * Only the read and write methods need defining for use.
 * @see cryptic.util.StreamInterface
 */
public abstract class Streamable implements StreamInterface
{
	private static final int DEFAULT_BUFFER_SIZE = 2048;
	
	public Streamable()
	{
	}

	public Streamable( byte buf[] )
		throws IOException
	{
		load( buf );
	}

	public Streamable( DataInput in )
		throws IOException
	{
		read( in );
	}
	
	public final void
	load( byte buf[] )
		throws IOException
	{
		read( new DataInputStream( new ByteArrayInputStream( buf ) ) );
	}

	public final byte[]
	save()
		throws IOException
	{
		ByteArrayOutputStream out = new ByteArrayOutputStream( DEFAULT_BUFFER_SIZE );
		write( new DataOutputStream( out ) );
		return out.toByteArray();
	}
}
