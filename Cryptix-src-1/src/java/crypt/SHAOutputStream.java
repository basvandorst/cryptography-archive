/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */
 
package java.crypt;

/**
 * This class is an output stream that hashes the data sent using the
 * SHA hashing algorithm.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 22/05/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class SHAOutputStream extends MessageDigestOutputStream
{
	/**
	 * create a new digest output stream that uses SHA.
	 */
	public SHAOutputStream()
	{
		super( new SHA() );
	}
}

