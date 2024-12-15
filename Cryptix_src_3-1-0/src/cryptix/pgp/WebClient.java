// This file is currently unlocked (change this line if you lock the file)
//
// $Log: WebClient.java,v $
// Revision 1.2  1999/06/30 23:03:07  edwin
// The abstract method getUserName(KeyID keyID) was not implemented. I've put in a placeholder.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
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
 * DOCUMENT ME.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author  Ian Brown
 */
public final class WebClient extends KeyClient
{
    private String pathToPage;
    
    public WebClient(String name, int port, String path )
    throws IOException, UnknownHostException
    {
        super(name, port);
        pathToPage = path;
    }

    /**
     * Called by <code>KeyClient.getKey()</code> to fetch a key using a
     * specific protocol.
     * Should NOT be called directly - this would bypass checks and caching.
     */
    protected PublicKey fetch(String userName)
    throws IOException 
    {
        boolean fullKeyReceived = false;
        DataInputStream serverIn = new DataInputStream(
            (new URL("http", serverName, port, pathToPage +
            "?op=get&exact=on&search=" + userName)).openStream());
        String line = null;
        String keyBlock = "";
        
        // Skip HTML formatting and information about key
        do
        {
            line = serverIn.readLine();
            if (line == null) return null; // No key returned
        } while (!line.equals("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
        keyBlock += line + Armoury.LINEFEED;    
        
        // Get rest of key
        while ((line = serverIn.readLine()) != null)
        {
            if (line.equals("-----END PGP PUBLIC KEY BLOCK-----"))
            {
                keyBlock += line;
                fullKeyReceived = true;
                break; // End of key
            }
            else keyBlock += line + Armoury.LINEFEED;
        }
        try
        {
            if (! fullKeyReceived) return null; // Problem retrieving key
            else return new KeyRingEntry(new PacketInputStream(new ByteArrayInputStream(Armoury.strip(keyBlock))), KeyRingTrust.UNDEF).publicKey();
        }

        catch(InvalidChecksumException ice)
        {
            // We can't throw one as KeyStore (the ultimate parent class)
            // doesn't allow it.
            throw new IOException("Message's checksum is incorrect.");
        }
    }

    /**
     * WebClient doesn't need an address as it queries a
     * master keyserver.
     */
    protected PublicKey fetch(EmailAddress address, KeyID keyID)
    throws IOException
    {
        return fetch(keyID.toString());
    }
    
    protected PublicKey fetch(EmailAddress address)
    throws IOException
    {
        return fetch(address.toString());
    }
    
    protected PublicKey fetch(KeyID keyID)
    throws IOException
    {
        return fetch(null, keyID);
    }
    
    // fix this
    public String getUserName(KeyID keyID)
    {
        System.out.println("WebClient.getUserName is unsupported!");
        return null;
    }
}
