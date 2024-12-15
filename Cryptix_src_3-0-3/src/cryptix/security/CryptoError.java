// This file is currently unlocked (change this line if you lock the file)
//
// $Log: CryptoError.java,v $
// Revision 1.2  1997/11/21 05:30:19  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/08/14  David Hopwood
// + Undo changes made in 2.5.2.
//
// Revision 0.2.5.2  1997/07/05  David Hopwood
// + Made constructor public, so that it is accessible from
//   cryptix.CryptixException.
// + Changed this class to extend java.security.ProviderException.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

/**
 * This class is for any unexpected error thrown by classes in
 * <samp>cryptix.security.*</samp> (i.e. when the Cryptix V2.2-compatible APIs
 * are being used).
 * <p>
 * The equivalent exception for the JCA APIs is
 * <samp>cryptix.CryptixException</samp>.
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 *
 * @see cryptix.CryptixException
 */
public class CryptoError extends Error
{
    /**
     * Constructs a CryptoError with the specified
     * detail message. A detail message is a String that describes
     * this particular exception.
     *
     * @param message   the detail message.
     */
    CryptoError(String message)
    {
        super(message);
    }       
}
