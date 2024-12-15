/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.crypt;

import java.math.PsudeoRandomStream;

/**
 * class that generates random bytes using a psudeo random number generator.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public class CSRandomStream extends PsudeoRandomStream
{
	SHA sha = new SHA();

	/**
	 * creates a new Cryptographicly Secure Random Stream from a seed.
	 * the security of this generator is based on the security of the seed.
	 */	
	public CSRandomStream( byte seed0[] )
	{
		super( seed0 );
	}

	/**
	 * Sets the internal buffer, length and pointer field to a new
	 * set of random bits available for reading
	 */
	protected void
	nextBuffer()
	{
		super.nextBuffer();
		sha.add( buf );
		buf = sha.digest();
		sha.reset();
		sha.add( buf );
	}
}

