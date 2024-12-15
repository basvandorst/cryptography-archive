// This file is currently unlocked (change this line if you lock the file)
//
// $Log: KeyRingEntry.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/05/07  Ian Brown
//   added function to retrieve a secret key contained in an entry
//   and bitLength() function
//
// Revision 0.2.5.2  1997/04/03  Ian Brown
//   added doc comments
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

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;

import cryptix.security.MD5;
import cryptix.security.rsa.PublicKey;

/**
 * Stores all the information about a user in a keyring - 
 * their key certificate, usernames, trust packet(s) and any signatures.
 * <p>
 * This is used by the KeyRing classes to read in keys, and
 * the KeyClient classes to convert retrieved key blocks.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Ian Brown
 * @since   Cryptix 2.2 or earlier
 */
public final class KeyRingEntry
{
    private Certificate cert;
    private KeyRingTrust trust; 
    private Vector userNameCerts;    

    /**
     * Creates a new KeyRingEntry with these parameters.
     */
    public KeyRingEntry(Certificate certificate, KeyRingTrust certificateTrust)
    {
        cert  = certificate;
        trust = certificateTrust;
        userNameCerts = new Vector();
    }
    
    /**
     * Creates a new KeyRingEntry with these parameters.
     */
    public KeyRingEntry(Certificate certificate, KeyRingTrust certificateTrust, Vector userCerts)
    {
        cert  = certificate;
        trust = certificateTrust;
        userNameCerts = userCerts;
    }

    /**
     * Reads in a KeyRingEntry from <i>reader</i>.
     * <p>
     * If the entry does not contain any trust information (i.e. is from
     * a secret keyring or a keyserver) assign the entry <i>defaultTrust</i>.
     */
    public KeyRingEntry(PacketInputStream reader, KeyRingTrust defaultTrust )
    throws IOException
    {
        userNameCerts = new Vector();
        restore(reader, defaultTrust);
    }

    /**
     * Adds these names, trust packets and signatures to this entry.
     */
    public void addNameAndSignatures(UserId userName, KeyRingTrust utrust, Vector signatures, Vector trusts)
    throws FormatException
    {
        userNameCerts.addElement(new UserIdCertificate(userName, utrust, signatures, trusts));
    }

    /**
     * How well is the key trusted overall?
     */
    public int 
    trustValue()
    {
        return trust.trustValue();
    }
    
    /**
     * Returns the certificate contained in this entry.
     */
    public Certificate
    certificate()
    {
        return cert;
    }
    
    /**
     * Returns the public key contained in this entry.
     * <p>
     * If the entry contains a secret key, <code>null</code> is returned.
     */    
    public PublicKey
    publicKey()
    {
        if (cert instanceof PublicKeyCertificate)
            return ((PublicKeyCertificate)cert).publicKey();
        else return null;
    }

    /**
     * Returns an array containing all the user ID certificates contained
     * in this entry.
     */
    public UserIdCertificate[]
    getUserIdCertificates()
    {
        int len;
        UserIdCertificate[] ret;
        userNameCerts.copyInto(ret = new UserIdCertificate[len = userNameCerts.size()]); 
        return ret;
    }

    /**
     * Keys may be associated with more than one name and e-mail address,
     * but PGP assumes the first user ID certificate after the key is
     * the most important one. This method returns that name.
     */
    public String
    primaryName()
    {
        return ((UserIdCertificate)userNameCerts.elementAt(0)).userName();
    }

    /**
     * Converts the entry to a String which contains the certificate,
     * user names and their associated trust and signature packets.
     */
    public String
    toString()
    {
        StringBuffer sb = new StringBuffer();
        sb.append("Cert:").append(cert.getClass().getName()).append('\n')
            .append(trust).append('\n');
        for(Enumeration e = userNameCerts.elements(); e.hasMoreElements();)
            sb.append(e.nextElement()).append('\n');
        return sb.toString();
    }

    /**
     * Checks the signatures on this entry using the keys in <i>store</i>.
     *
     * @param store where to retrieve public keys from
     */
    public boolean
    checkSignatures(KeyStore store)
    {
        for(Enumeration e = userNameCerts.elements(); e.hasMoreElements();)
            if (!((UserIdCertificate)e.nextElement()).checkSignatures(store, cert))
                return false;
        return true;
    }

    private void
    restore(PacketInputStream reader, KeyRingTrust defaultTrust)
    throws IOException
    {
        Packet p;
        UserIdCertificate usercert;

        // Need to catch incomplete Entries, when in runs out of data
        p = reader.readPacket();
        if (p == null) throw new IOException(
            "Tried to read a KeyRingEntry from an empty stream");
            // Reached the end of the stream
        else if (! (p instanceof Certificate)) throw new IOException(
            "KeyRingEntry must start with a Certificate.");

        else cert = (Certificate)p;
        p = reader.readPacket();
        if (! (p instanceof KeyRingTrust))
        {
            // We may be reading a secret keyring, or a keyserver-fetched key entry
            // which contains no trust information
            trust = defaultTrust;
            if (p != null) reader.pushback(p);
        }
        else trust = (KeyRingTrust)p;
        while ((usercert = reader.readUserIdCertificate(defaultTrust)) != null)
            userNameCerts.addElement(usercert);
    }

    /**
     * Writes out this entry as a series of PGP packets to <i>out</i>.
     */
    public void
    write(DataOutput out)
    throws IOException
    {
        // Is used in the KeyRing.write() function

        // Write out the certificate with its trust packet
        PacketFactory.write(out, cert);
        PacketFactory.write(out, trust);

        // Write out each userNameCert
        for (int i = 0;  i < userNameCerts.size(); i++)
        {
            ((UserIdCertificate)(userNameCerts.elementAt(i))).write(out);
        }
    }    
}
