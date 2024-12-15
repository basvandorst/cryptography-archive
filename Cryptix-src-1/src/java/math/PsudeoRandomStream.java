/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

/**
 * class that generates random bytes using s psudeo randon number generator.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class PsudeoRandomStream extends RandomStream
{
	protected PRNG prng;
	protected byte buf[];
	protected int ptr = 0, len = 0;

	public PsudeoRandomStream( byte seed0[] )
	{
		super( seed0 );
	}


	protected void seed( byte seed0[] )
	{
		prng = new PRNG( seed0 );
	}
	
	protected void
	nextBuffer()
	{
		prng.clock();
		buf = prng.toByteArray();
		len = buf.length;
		ptr = 0;
	}
	
	protected final synchronized byte nextByte()
	{
		if ( ptr >= len ) // time to get next bytes
			nextBuffer();
		return buf[ptr++];
	}
}

