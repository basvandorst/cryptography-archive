// This file is currently unlocked (change this line if you lock the file)
//
// $Log: RSAKeyGenObserver.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
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
 * <p>Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
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

