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
 * This class is for any unexpected error in the native crypto library.
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version 1.00, 08/12/1995
 * @author      Systemics (Michael Wynn)
 */
public class CryptoError extends Error
{
	// If this ever occurs then java has gone VERY wrong!
	private CryptoError()
	{
		super("I thought this error was impossible to create!");
	}
	
	/**
	 * Only classes in this package can create a crypto error.
	 * @param reason   the reason the error was thrown.
	 */
	CryptoError(String reason)
	{
		super(reason);
	}	
}
