/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

/**
 * This class is an Output stream that hashes the data sent to it using
 * the SHA hashing algorithm.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * <b>Copyright</b> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * @author Systemics Ltd
 * @see MessageDigestOutputStream
 */
public final class SHAOutputStream extends MessageDigestOutputStream
{
        /**
         * Create a new digest output stream that uses SHA.
         */
        public SHAOutputStream()
        {
                super( new SHA() );
        }
}
