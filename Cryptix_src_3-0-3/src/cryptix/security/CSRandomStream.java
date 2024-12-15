// $Id: CSRandomStream.java,v 1.1 1997/12/07 06:40:26 hopwood Exp $
//
// $Log: CSRandomStream.java,v $
// Revision 1.1  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 0.1.0  1997/12/07  hopwood
// + Restored from 2.2 release (and made deprecated).
//
// $Endlog$
/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import cryptix.math.PseudoRandomStream;

/**
 * A class that generates random bytes using a pseudo random number generator.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 * @see java.security.SecureRandom
 */
public class CSRandomStream extends PseudoRandomStream
{
    protected SHA sha = new SHA();

    /**
     * Creates a new Cryptographically Secure Random Stream from a seed.
     * The security of this generator relies on a good seed.
     */     
    public CSRandomStream(byte[] seed)
    {
        super(seed);
    }

    /**
     * Sets the internal buffer, length and pointer field to a new
     * set of random bits available for reading.
     */
    protected void
    nextBuffer()
    {
        super.nextBuffer();     // Clock
        sha.add(buf);           // and add buffer
        buf = sha.digest();     // get new buffer
        sha.reset();            // reset and add ready for next time
        sha.add(buf);
    }

    /**
     * Reset the seed and the PRNG.
     */
    protected void
    seed(byte[] seed)
    {
        sha.reset();
        super.seed(seed);
    }

    /**
     * Add some entropy to the generator.
     */
    public synchronized void
    add(byte[] entropy)
    {
        sha.add(entropy);
        nextBuffer();   // Mix it in right away
    }
}

