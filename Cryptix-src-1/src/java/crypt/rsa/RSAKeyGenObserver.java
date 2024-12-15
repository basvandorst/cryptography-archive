/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.crypt.rsa;

import java.math.BigNum;
import java.math.BigInteger;
import java.math.RandomStream;
import java.math.TestPrime;
import java.math.TestPrimeObserver;

/**
 * This class is an RSA key generator callback for monitoring progress
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1996
 * @author      Systemics Ltd. (Michael Wynn)
 */
public interface RSAKeyGenObserver extends TestPrimeObserver
{
	/** the RSA Key Generator is searching for a suitable P */
	public static final int FINDING_P = 5;
	/** the RSA Key Generator is searching for a suitable Q */
	public static final int FINDING_Q = 6;
	/** the RSA Key Generator is generation a key from P and Q */
	public static final int KEY_RESOLVING = 7;
	/** the RSA Key Generator has finished */
	public static final int KEY_GEN_DONE = 8;
}

