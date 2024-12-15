// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyGlob.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.1  1997/11/02  David Hopwood (for Zoran Rajic)
// + getUserName(KeyID) was returning KeyID.toString() which was,
//   well .. ugly. Reprogrammed to check if we use KeyClient (e.g.
//   WebClient) and if we do, then use KeyClient's getUserName(KeyID).
//
// Revision 0.1.0.0  1997/??/??  Ian Brown?
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Vector;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;
import cryptix.util.EmailAddress;

/**
 * Stores all of the key sources known to a program - 
 * all public and secret keyrings, and a Web master keyserver.
 * If setKeyServer is not used, the object will not use one.
 * <p>
 * Can be passed to objects which decrypt and verify PGP messages
 * such as ArmouredMessage.
 * <p>
 * NB. Keyrings are searched in the order in which they are added.
 * Therefore, <b>always add the most highly trusted keyrings first.</b>
 * <p>
 * The master keyserver is only consulted if a key is not available locally.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Ian Brown
 * @author  Jill Baker
 * @author  Zoran Rajic
 * @since   Cryptix 2.2
 */
public final class KeyGlob extends KeyStore
{
    private Vector publicKeyRings;
    private Vector secretKeyRings;
    private KeyClient keyClient;

    /**
     * Initializes an empty KeyGlob. Use <code>addKeyRing</code> and
     * <code>setKeyServer</code> to fill it up.
     */
    public KeyGlob()
    {
        publicKeyRings = new Vector();
        secretKeyRings = new Vector();
        keyClient = null;
    }

    /**
     * Adds <i>publicKeyRing</i> to the KeyGlob's collection.
     */
    public void
    addKeyRing(PublicKeyRing publicKeyRing)
    {
        publicKeyRings.addElement(publicKeyRing);
    }

    /**
     * Adds <i>secretKeyRing</i> to the KeyGlob's collection.
     * <p>
     * By allowing more than one secret keyring, users can keep a standard
     * secret key available, and have a high-security key stored on a floppy
     * disk which applications only access transparently through this class
     * when it's needed.
     */
    public void
    addKeyRing(SecretKeyRing secretKeyRing)
    {
        secretKeyRings.addElement(secretKeyRing);
    }

    /**
     * Gives KeyGlob a keyclient object that it can use to fetch keys remotely.
     * This will be searched for public keys if they are not available locally.
     *
     * @param client    an instantiated KeyClient subclass object
     * @exception IOException if there was an I/O error
     * @exception UnknownHostException if the public key server was unreachable
     */
    public void
    setKeyServer(KeyClient client)
    throws IOException, UnknownHostException
    {
        keyClient = client;
    }

    /**
     * Gets the trust value for a given key ID.
     *
     * @param keyID         the key ID you are interested in
     * @param publicRings   search public rings (true), or secret rings (false)
     * @return trust value, or -1 if key not present
     */
    public int getKeyTrust(KeyID keyID, boolean publicRings)
    {
        int trust;

        // Cycle through all public keyrings to look for key ID
        if (publicRings)
        {
            for (int i = 0; i < publicKeyRings.size(); i++)
            {
                PublicKeyRing ring = (PublicKeyRing) publicKeyRings.elementAt(i);
                trust = ring.getKeyTrust(keyID);
                if (trust != -1) return trust;
            }
        }
        else
        {
            for (int i = 0; i < secretKeyRings.size(); i++)
            {
                SecretKeyRing ring = (SecretKeyRing) secretKeyRings.elementAt(i);
                trust = ring.getKeyTrust(keyID);
                if (trust != -1) return trust;
            }
        }
        return -1;
    }

    /**
     * Tries to find a public key for <i>name</i>.
     *
     * @param name  the name of the user
     * @return their public key, or null if one can't be found
     * @exception IOException if there was an I/O error
     */
    public PublicKey
    getKey(String name)
    throws IOException
    {
        PublicKey publicKey = null;

        // Cycle through all public keyrings to look for name
        for (int i = 0; i < publicKeyRings.size(); i++)
        {
            publicKey = ((PublicKeyRing) publicKeyRings.elementAt(i)).getKey(name);
            // If we've found it, return immediately
            if (publicKey != null)
                return publicKey;
        }

        // If user has specified a keyserver...
        if (keyClient != null)
            // then try it for the key
            return keyClient.getKey(name);
        else
            return null;
    }

    /**
     * Tries to find a public key for <i>id</i>.
     *
     * @param id    the key ID of the user (the low 8 bits of their key pair's
     *          public modulus)
     * @return their public key, or null if one can't be found
     * @exception IOException if there was an I/O error
     */
    public PublicKey
    getKey (KeyID id)
    throws IOException
    {
        PublicKey publicKey = null;

        // Cycle through all public keyrings to look for id
        for (int i = 0; i < publicKeyRings.size(); i++)
        {
            publicKey = ((PublicKeyRing) publicKeyRings.elementAt(i)).getKey(id);
            // If we've found it, return immediately
            if (publicKey != null)
                return publicKey;
        }

        // If user has specified a keyserver...
        if (keyClient != null)
            // then try it for the key
            return keyClient.getKey(id);
        else
            return null;
    }

    /**
     * Tries to find a public key for <i>address</i>.
     *
     * @param address   the e-mail address of the user
     * @return their public key, or null if one can't be found
     * @exception IOException if there was an I/O error
     */
    public PublicKey
    getKey (EmailAddress address)
    throws IOException
    {
        PublicKey publicKey = null;

        // Cycle through all public keyrings to look for address
        for (int i = 0; i < publicKeyRings.size(); i++)
        {
            publicKey = ((PublicKeyRing) publicKeyRings.elementAt(i)).getKey(address);
            // If we've found it, return immediately
            if (publicKey != null)
                return publicKey;
        }
        
        // If user has specified a keyserver...
        if (keyClient != null)
            // then try it for the key
            return keyClient.getKey(address);
        else
            return null;
    }

    /**
     * Tries to find a secret key for <i>name</i>, and decrypt it with
     * <i>passphrase</i>.
     *
     * @param name          the e-mail address of the user
     * @param passphrase    the passphrase protecting their secret key
     * @return their secret key, or null if one can't be found
     * @exception IOException if there was an I/O error
     * @exception DecryptException if the secret key could not be decrypted
     */
    public SecretKey
    getSecretKey(String name, Passphrase passphrase)
    throws IOException, DecryptException
    {
        SecretKey secretKey = null;
        // Cycle through all secret keyrings to look for name
        for (int i = 0; i < secretKeyRings.size(); i++)
        {
            secretKey = ((SecretKeyRing) secretKeyRings.elementAt(i)).getKey(name, passphrase);
            // If we've found it, return immediately
            if (secretKey != null)
                return secretKey;
        }
        // It's not in any of them
        return null;
    }

    /**
     * Tries to find a secret key for <i>id</i>, and decrypt it with
     * <i>passphrase</i>.
     *
     * @param id            the key ID of the user's key
     * @param passphrase    the passphrase protecting their secret key
     * @return their secret key, or null if one can't be found
     * @exception IOException if there was an I/O error
     * @exception DecryptException if the secret key could not be decrypted
     */
    public SecretKey
    getSecretKey (KeyID id, Passphrase passphrase)
    throws IOException, DecryptException
    {
        SecretKey secretKey = null;
        // Cycle through all secret keyrings to look for id
        for (int i = 0; i < secretKeyRings.size(); i++)
        {
            secretKey = ((SecretKeyRing) secretKeyRings.elementAt(i)).getKey(id, passphrase);
            // If we've found it, return immediately
            if (secretKey != null)
                return secretKey;
        }
        // It's not in any of them
        return null;
    }

    /**
     * Returns the name of a user whose key has ID <i>keyID</i>.
     * <p>
     * Only checks local keyrings.
     */
    public String
    getUserName(KeyID keyID)
    {
        String name = null;
        // Cycle through all public keyrings to look for name
        for (int i = 0; i < publicKeyRings.size(); i++)
        {
            name = ((PublicKeyRing) publicKeyRings.elementAt(i)).getUserName(keyID);
            // If we've found it, return immediately
            if (name != null)
                return name;
        }

        // We could look it up from the keyserver, but that would be overkill
        // so just follow PGP's example and give the key ID as a string
        return keyID.toString();
    }    
}
