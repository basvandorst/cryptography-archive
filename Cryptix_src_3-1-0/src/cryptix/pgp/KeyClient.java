// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyClient.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.4  1997/11/01  David Hopwood (for Zoran Rajic)
// + getKey(KeyID keyID) was always returning null, so if you were using
//   WebClient, it would not automatically get the keys from the KeyServer.
//   Now it returns fetch(null, keyID).
// + getUserName(KeyID) made abstract and defined in WebClient.
//
//   [DJH: I'm not convinced that these two methods are the Right Thing,
//   because IDs are not collision-resistant (32 bits is too small to
//   prevent even accidental collisions). Is there any way to do a lookup
//   by fingerprint instead?]
//
// Revision 0.2.5.3  1997/05/07  Ian Brown
// + Added functions to return whole KeyRingEntries as well as PublicKeys.
//
//   [DJH: Where are these? I don't see them.]
//
// Revision 0.2.5.2  1997/03/29  Ian Brown
// + Removed caching for the moment - it doesn't work so was only clutter.
// + Added doc comments.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Imports changed to new namespace.
//
// Revision 0.2.5.0  1996/11/19  Ian Brown
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.*;
import java.net.*;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * Superclass for the classes that fetch keys from keyservers.
 * <p>
 * Next version will cache keys, within and between sessions.
 * <p>
 * WebClient is the only class which currently extends this. Others
 * can be easily written - they simply need to implement the <code>fetch()</code>
 * function.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Ian Brown
 * @author  Zoran Rajic
 * @since   Cryptix 2.2
 */
public abstract class KeyClient extends KeyStore
{
    protected String serverName;
    protected int port;    
    
    /**
     * Set up variables for server connection.
     */
    protected KeyClient(String name, int port)
    throws IOException, UnknownHostException
    {
        serverName = name;
        this.port = port;
    }

    public PublicKey getKey(String userName) throws IOException
    {
        return fetch(userName);
    }

    public PublicKey getKey(EmailAddress address) throws IOException
    {
        return fetch(address);
    }

    public PublicKey getKey(EmailAddress address, KeyID keyID) throws IOException
    {
        return fetch(address, keyID);
    }

    public PublicKey getKey(KeyID keyID) throws IOException
    {
        return fetch(null, keyID);
    }

    /**
     * <code>fetch()</code> is overridden by child classes to provide the 'meat'
     * of these classes.
     */
    protected abstract PublicKey fetch(String userName) throws IOException;
    protected abstract PublicKey fetch(EmailAddress address, KeyID keyID) throws IOException;
    protected abstract PublicKey fetch(EmailAddress address) throws IOException;
    public abstract String getUserName(KeyID keyID);
}
