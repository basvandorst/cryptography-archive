// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PublicKeyRing.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/06/01  Ian Brown
//   fixed to work with changes to KeyRing
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

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.Vector;
import java.util.Enumeration;

import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * Represents a public keyring. Keys on it can be retrieved using the
 * <code>getKey()</code> functions.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Mike Wynn
 * @author  Ian Brown
 * @since   Cryptix 2.2 or earlier
 */
public class PublicKeyRing extends KeyRing
{
    /**
     * Creates an empty public keyring object. Use the <code>add</code>
     * method to add new entries.
     */
    public PublicKeyRing() {}

    /**
     * Creates a new public keyring object from <i>in</i>.
     * @exception IOException if there was an I/O error
     */
    public PublicKeyRing(InputStream in)
    throws IOException
    {
        super(in);
    }

    /**
     * Creates a new public keyring object from the file <i>filename</i>.
     * @exception FileNotFoundException if <i>filename</i> was not found
     * @exception IOException if there was an I/O error
     */
    // REMIND SECURITY: need to check this.
    public PublicKeyRing(String filename)
    throws FileNotFoundException, IOException
    {
        SecurityManager security = System.getSecurityManager();
        if (security != null) security.checkRead(filename);

        FileInputStream keyfile = new FileInputStream(filename);
        init(new PacketInputStream(keyfile));
        keyfile.close();
    }

    /**
     * Retrieves a key by any of the user names on it.
     */
    public PublicKey getKey(String username)
    throws IOException
    {
        KeyID toID;
        if ((toID = (KeyID) (users.get(username))) != null)
        {
            PublicKey publicKey = getKey(toID);
            if (publicKey != null)
                return publicKey;
            // there SHOULD never be a time when you can get the ID but
            // not the key !!
        }
        return getIfBegins(users.keys(), username);
    }
    
    /**
     * Retrieves a key by its ID.
     */
    public PublicKey getKey(KeyID keyID)
    throws IOException
    {
        return getPublicKey(keyID);
    }

    /**
     * Retrieves a key by any of the e-mail addresses on it.
     */
    public PublicKey getKey(EmailAddress address)
    throws IOException
    {
        return getPublicKey(address);
    }

    protected final void init(PacketInputStream reader)
    throws IOException
    {
        KeyRingEntry entry;
        // By default, key entries without trust information will be given
        // undefined trust.
        while ((entry = reader.readKeyRingEntry(KeyRingTrust.UNDEF)) != null)
            if (! entry.certificate().expired()) add(entry);
    }
}
