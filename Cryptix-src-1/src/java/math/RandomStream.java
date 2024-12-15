/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

import java.io.InputStream;

/**
 * Abstract class that generates random numbers.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public abstract class RandomStream extends InputStream
{
	public RandomStream( byte seed0[] )
	{
		seed( seed0 );
	}

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

	protected abstract void seed( byte seed0[] );
	protected abstract byte nextByte();
}

