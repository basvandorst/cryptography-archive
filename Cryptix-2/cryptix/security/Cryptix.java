/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * This class defines the version number of the Cryptix library.
 * The major version of Cryptix is intended to signal
 * significant changes in compatibility or style.
 * The minor version of Cryptix is intended to signal
 * small changes in compatibility and new, compatible additions.
 * Another, internal minor number is also sometimes used,
 * for maintenance releases.
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
 */
public final class Cryptix
{
        private final static int MAJOR_VERSION = 2;
        private final static int MINOR_VERSION = 2;

        /**
         * @return the major version of this release of Cryptix
         */
        public final static int
        getMajorVersion()
        {
                return MAJOR_VERSION;
        }

        /**
         * @return the minor version of this release of Cryptix
         */
        public final static int
        getMinorVersion()
        {
                return MINOR_VERSION;
        }

        public final static String
        getVersionString()
        {
                return "Cryptix " + MAJOR_VERSION + "." + MINOR_VERSION;
        }
}

