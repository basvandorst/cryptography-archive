/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import cryptix.math.PseudoRandomStream;

/**
 * A class that generates random bytes using a psudeo random number generator.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * @author Systemics Ltd
 */
public class CSRandomStream extends PseudoRandomStream
{
        protected SHA sha = new SHA();

        /**
         * creates a new Cryptographically Secure Random Stream from a seed.
         * the security of this generator relies on a good seed
         */     
        public CSRandomStream(byte[] seed)
        {
                super(seed);
        }

        /**
         * Sets the internal buffer, length and pointer field to a new
         * set of random bits available for reading
         */
        protected void
        nextBuffer()
        {
                super.nextBuffer();             // Clock
                sha.add(buf);                   // and add buffer
                buf = sha.digest();             // get new buffer
                sha.reset();                    // reset and add ready for next time
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

