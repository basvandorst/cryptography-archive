// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyStore.java,v $
// Revision 1.2  1999/06/30 23:02:00  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/04/03  Ian Brown
//   added doc comments
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   imports changed to new namespace
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
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
import cryptix.security.MD5;
import cryptix.security.rsa.PublicKey;

/**
 * Abstract base class for PGP keyrings and keyservers.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author  Ian Brown
 * @since   Cryptix 2.2 or earlier
 * @see cryptix.pgp.KeyRing
 */
public abstract class KeyStore
{
    protected abstract PublicKey getKey(String username) throws IOException;
    protected abstract PublicKey getKey(KeyID keyID) throws IOException;

    /**
     * Checks the signature <i>sig</i> on <i>cert</i>
     * and <i>username</i> using the keys in this keystore.
     */
    public final boolean 
    checkSignature(Signature sig, Certificate cert, UserId username)
    throws IOException
    {
        return checkSignature(this, sig, cert, username);
    }
    
    /**
     * Checks the signature <i>sig</i> on <i>cert</i>
     * and <i>username</i> using the keys in <i>store</i>.
     */
    public static boolean 
    checkSignature(KeyStore store, Signature sig, Certificate cert, UserId username)
    throws IOException
    {
        PublicKey key;
        if ((key = store.getKey(sig.getKeyId())) != null)
            return checkSignature(key, sig, cert, username);
        throw new IOException("Key id " + sig.getKeyId() + " not found");
    }
    
    /**
     * Checks the signature <i>sig</i> on <i>key</i>'s <i>cert</i>
     * and <i>username</i> using the keys in <i>store</i>.
     */
    public static boolean 
    checkSignature(PublicKey key, Signature sig, Certificate cert, UserId username)
    throws IOException
    {
        MD5 md = new MD5();
        md.add(PacketFactory.save(cert));
        md.add(username.save());
        return sig.check(key, md);
    }
}
