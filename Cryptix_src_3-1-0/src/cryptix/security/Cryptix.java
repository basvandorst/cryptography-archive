// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Cryptix.java,v $
// Revision 1.2  1997/11/20 22:46:06  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1.1  1997/11/14  David Hopwood
// + Replaced references to cryptix.core.Cryptix with
//   cryptix.CryptixProperties.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.2  1997/08/15  David Hopwood
// + Replaced references to cryptix.provider.Cryptix (?) with
//   cryptix.core.Cryptix.
//
// Revision 0.1.0.1  1997/07/12  David Hopwood
// + Deprecated this class in favour of cryptix.provider.Cryptix (?)
// + Removed methods that were not in Cryptix 2.2.
//
// Revision 0.1.0.0  1997/07/12  David Hopwood
// + No history before this date.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import cryptix.CryptixProperties;

/**
 * This class used to define the version number of the Cryptix library.
 * The class <samp>cryptix.CryptixProperties</samp> should now be used in
 * preference.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @author  Jill Baker
 * @since   Cryptix 2.2
 * @deprecated
 * @see cryptix.CryptixProperties
 */
public class Cryptix
{
    private Cryptix() {} // static methods only

    /**
     * Returns the major version of this release of Cryptix.
     * @deprecated Use <code>CryptixProperties.getMajorVersion()</code>.
     */
    public static int getMajorVersion() {
        return CryptixProperties.getMajorVersion();
    }

    /**
     * Returns the minor version of this release of Cryptix.
     * @deprecated Use <code>CryptixProperties.getMinorVersion()</code>.
     */
    public static int getMinorVersion() {
        return CryptixProperties.getMinorVersion();
    }

    /**
     * Returns a string describing this version of Cryptix.
     * @deprecated Use <code>CryptixProperties.getVersionString()</code>.
     */
    public static String getVersionString() {
        return CryptixProperties.getVersionString();
    }
}
