/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

package cryptix.math;

import java.io.InputStream;

/**
 * Abstract class that generates random numbers.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public abstract class RandomStream extends InputStream
{
    public int
	read()
	{
		return nextByte() & 0xFF;
	}

    public int
	read( byte b[] )
	{
		return read(b, 0, b.length);
    }

    public int
	read( byte b[], int off, int len )
	{
		for ( int i = off ; i < len ; i++ )
			b[i] = nextByte();
		return len - off;
	}

    public long
	skip( long n )
	{
		for ( long i = n; i > 0; i-- )
			nextByte();
		return n;
    }

	protected abstract void seed(byte seed[]);
	protected abstract byte nextByte();
}
