// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyCache.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/04/02  Ian Brown
//   tidied up ready for release - doc comments added
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   imports changed to new namespace
//
// Revision 0.2.5.0  1997/02/24  Ian Brown
//   Original version
//
//   If this file is unlocked, or locked by someone other than you, then you
//   may download this file for incorporation into your build, however any
//   changes you make to your copy of this file will be considered volatile,
//   and will not be incorporated into the master build which is maintained
//   at this web site.
//
//   If you wish to make permanent changes to this file, you must wait until
//   it becomes unlocked (if it is not unlocked already), and then lock it
//   yourself. Whenever the file is locked by you, you may upload new master
//   versions to the master site. Make sure you update the history information
//   above. When your changes are complete, you should relinquish the lock to
//   make it available for other developers.
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.IOException;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;

/**
 * This class currently does nothing other than provide a particular
 * type which will be used by the next versions of the remote key-fetching
 * classes to store retrieved keys.
 * <p>
 * The next version will:
 * <ul>
 *    <li>Automatically sets expiry date of added keys to 30 days</li>
 *    <li>Be able to write itself out to disk and restore itself at next
 *        session</li>
 * </ul>
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Ian Brown
 * @since   Cryptix 2.2 or earlier
 */
public final class KeyCache extends PublicKeyRing
{
    public KeyCache() {}
}
