// This file is currently unlocked (change this line if you lock the file)
//
// $Log: UserIdCertificate.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Added method getTrustValue().
//   Changed imports to new namespace
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

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;

import cryptix.security.rsa.PublicKey;

/**
 * Stores all the information about a user in a keyring - 
 * their key certificate, usernames, trust packet(s) and any signatures.
 * <p>
 * Each user ID can contain any number of signatures and corresponding
 * trust packets. This class is used by KeyRingEntry to deal with this.
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
public final class UserIdCertificate
{
    private UserId         name;
    private KeyRingTrust   usertrust;
    private Signature[]    sigs;
    private KeyRingTrust[] sigtrust;

    UserIdCertificate(UserId userName, KeyRingTrust utrust,
                       Vector signatures, Vector trusts)
    throws FormatException
    {
        name = userName;
        usertrust = utrust;
        initSigs(signatures, trusts);
    }

    UserIdCertificate(PacketInputStream reader)
    throws IOException
    {
        restore(reader);
    }

    private final void
    initSigs(Vector signatures, Vector trusts)
    throws FormatException
    {
        if ((signatures != null) && (trusts != null))
        {
            int len;
            if ((len = trusts.size()) != signatures.size())
                throw new FormatException(
                    "Number of trust packets incorrect for number of signatures");

            if (len > 0)
            {
                signatures.copyInto(sigs = new Signature[len]);
                trusts.copyInto(sigtrust = new KeyRingTrust[len]);
                return;
            }
        }
        sigs = new Signature[0];
        sigtrust = new KeyRingTrust[0];
        return;
    }

    public String
    userName()
    {
        return name.toString();
    }

    public UserId
    userId()
    {
        return name;
    }

    public Signature[]
    signatures()
    {
        int len = sigs.length;
        Signature[] ret;
        System.arraycopy(sigs, 0, ret = new Signature[len], 0, len);
        return ret;
    }

    boolean
    checkSignatures(KeyStore store, Certificate cert)
    {
        int len = sigs.length;
        for (int i = 0; i < len; i++)
        {
            try
            {
                PublicKey key = store.getKey(sigs[i].getKeyId());
                if (key != null)
                {
                    if (!KeyStore.checkSignature(key, sigs[i], cert, name))
                        return false;
                }
                else
                    System.err.println("Key id " + sigs[i].getKeyId() + " not found (Signature ignored)");
            }
            catch(IOException e)
            {
                System.err.println("IOException Signature ignored");
            }
        }
        return true;
    }

    public String
    toString()
    {
        StringBuffer sb = new StringBuffer();
        sb.append("Username:").append(name).append('\n')
            .append(usertrust).append('\n');
        int len = sigs.length;
        for (int i = 0; i < len; i++)
            sb.append(sigs[i]).append('\n').append(sigtrust[i]).append('\n');
        return sb.toString();
    }

    void
    restore(PacketInputStream reader)
    throws IOException
    {
        Packet p;
        if (!((p = reader.readPacket()) instanceof UserId))
            throw new IOException("UserIdCetrificate must start with an UserId packet");
        name = (UserId)p;
        if (!((p = reader.readPacket()) instanceof KeyRingTrust))
            throw new IOException("KeyRingTrust expected not " + p.getClass().getName());
        usertrust = (KeyRingTrust)p;
        Vector signatures = new Vector(), trusts = new Vector();
        while ((p = reader.readPacket()) instanceof Signature) 
        {
            if (p == null)
                break;
            signatures.addElement(p);
            if (!((p = reader.readPacket()) instanceof KeyRingTrust))
                throw new IOException("KeyRingTrust expected not " + p.getClass().getName());
            trusts.addElement(p);
        }
        reader.pushback(p);
        initSigs(signatures, trusts);
    }
    
    void
    write(DataOutput out)
    throws IOException
    {
        PacketFactory.write(out, name);
        PacketFactory.write(out, usertrust); // Overall trust for UserId
        int len = sigs.length;
        for (int i = 0; i < len; i++)
        {
            PacketFactory.write(out, sigs[i]);
            PacketFactory.write(out, sigtrust[i]);
        }
    }

    int
    getTrustValue()
    {
        // where does the 3 come from?
        return usertrust.trustValue() & 3;
    }
}
