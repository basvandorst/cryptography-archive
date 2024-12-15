/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * This class is for any unexpected error in the native crypto library.
 * <P>
 * <b>References</b>
 *
 * <P>
 * <b>Copyright</b> &#169 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <p>
 * @author Systemics Ltd
 */
public class CryptoError extends Error
{
        // Should never happen
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

