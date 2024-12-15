// This file is currently unlocked (change this line if you lock the file)
//
// $Log: SecretKeyRing.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.4  1997/06/01  Ian Brown
//   fixed to work with changes to KeyRing
//
// Revision 0.2.5.3  1997/04/06  Ian Brown
//   Added getKeyTrust to override KeyRing.getKeyTrust
//   If you possess a secret key, you must trust it totally (as it's yours!)
//   Secret keys never have signatures or trust packets attached, so
//   there's no point computing a trust value.
//   Therefore, 3 is returned if you possess the key, -1 (unknown) if you don't
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
 * Represents a secret keyring. Use the <code>getKey()</code> functions
 * to retrieve a key.
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
public final class SecretKeyRing extends KeyRing
{
    /**
     * Creates an empty secret keyring object. Use the <code>add</code>
     * method to add new entries.
     */
    protected SecretKeyRing() {}
    
    /**
     * Creates a new secret keyring object from <i>in</i>.
     * @exception IOException if there was an I/O error
     */
    public SecretKeyRing(InputStream in)
    throws IOException
    {
        super(in);
    }
    
    /**
     * Create a new secret keyring object from the file <i>filename</i>.
     * @exception FileNotFoundException if <i>filename</i> was not found
     * @exception IOException if there was an I/O error
     */
    // REMIND SECURITY: need to check this.
    public SecretKeyRing(String filename)
    throws FileNotFoundException, IOException
    {
        SecurityManager security = System.getSecurityManager();
        if (security != null) security.checkRead(filename);

        FileInputStream keyfile = new FileInputStream(filename);
        init(new PacketInputStream(keyfile));
        keyfile.close();
    }

    /**
     * <b>DON'T USE THIS</b> - it's only here to allow SecretKeyRing to fit
     * into this class hierarchy. <code>null</code> is always returned as a 
     * passphrase is needed to decrypt the key first.
     */
    // wouldn't it be better for this to throw an exception?
    public final PublicKey getKey(String username)
    throws FormatException
    {
        // See getKey(byte[] keyID) for an explanation.
        return null;
    }
    
    /**
     * Retrieves a key by any of the user names on it.
     *
     * @exception IOException if there was an I/O error
     * @exception DecryptException if the key could not be decrypted
     */
    public final SecretKey getKey(String username, Passphrase passphrase)
    throws IOException, DecryptException
    {
        KeyID id = (KeyID)(users.get(username)); 
        if (id == null)
            return null;
        else
            return getKey(id, passphrase);
    }
        
    /**
     * <b>DON'T USE THIS</b> - it's only here to allow SecretKeyRing to fit
     * into this class hierarchy. <code>null</code> is always returned as a 
     * passphrase is needed to decrypt the key first.
     */
    // wouldn't it be better for this to throw an exception?
    public final PublicKey getKey(KeyID keyID)
    {
        return null;
    }
    
    /**
     * Retrieves a key by its ID.
     *
     * @exception DecryptException if the key could not be decrypted
     */
    public final SecretKey getKey(KeyID keyID, Passphrase passphrase)
    throws DecryptException
    {
        return getSecretKey(keyID, passphrase);
    }

    /**
     * Retrieves a key by any of the e-mail addresses on it.
     *
     * @exception DecryptException if the key could not be decrypted
     */
    public final SecretKey getKey(EmailAddress address, Passphrase passphrase)
    throws DecryptException
    {
        return getSecretKey(address, passphrase);
    }
    
    public final int getKeyTrust(KeyID keyId)
    {
        // If we have this secret key, we must trust it totally...
        if (entriesById.get(keyId) != null)
            return 3; // should use some symbolic constant
        else
            // otherwise we don't know.
            return -1;
    }

    protected final void init(PacketInputStream reader)
    throws IOException
    {
        KeyRingEntry entry;
        // Logically, you must trust completely all entries on a secret keyring
        while ((entry = reader.readKeyRingEntry(KeyRingTrust.SECRING)) != null)
            if (! entry.certificate().expired()) add(entry);
    }
}
