/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

/**
 * An output stream that hashes the data sent to it using
 * the MD5 hashing algorithm.
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
public final class MD5OutputStream extends MessageDigestOutputStream
{
        /**
         * Create a new digest output stream that uses MD5.
         */
        public MD5OutputStream()
        {
                super( new MD5() );
        }
}

