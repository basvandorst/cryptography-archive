/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.security.rsa;

import cryptix.math.BigNum;
import cryptix.math.BigInteger;
import cryptix.math.RandomStream;
import cryptix.math.TestPrime;
import cryptix.math.TestPrimeObserver;

/**
 * This class is an RSA key generator callback for monitoring progress
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
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

