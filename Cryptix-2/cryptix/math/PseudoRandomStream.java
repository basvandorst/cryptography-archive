/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

package cryptix.math;

/**
 * class that produces psueodo random bytes
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public class PseudoRandomStream extends RandomStream
{
	protected PRSG prng;
	protected byte buf[];
	protected int ptr = 0, len = 0;

	/**
	 * Seed must be 20 bytes or greater (excess not used)
	 */
	public PseudoRandomStream(byte[] seed)
	{
		prng = new PRSG(seed);
	}

	protected void seed( byte seed[] )
	{
		prng = new PRSG(seed);
	}
	
	protected void
	nextBuffer()
	{
		prng.clock();
		buf = prng.toByteArray();
		len = buf.length;
		ptr = 0;
	}
	
	//
	// It is very important that this method
	// is synchronised, otherwise we're in big
	// trouble when using global RandomStreams
	//
	protected final synchronized byte nextByte()
	{
		if ( ptr >= len ) // time to get next set of bytes
			nextBuffer();

// Oh, what a surprise! There's a bug in MS Java ...
//		return buf[ptr++];
		byte r = buf[ptr++]; return r;

	}
}
