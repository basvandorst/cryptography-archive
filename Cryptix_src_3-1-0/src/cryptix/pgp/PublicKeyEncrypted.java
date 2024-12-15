// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PublicKeyEncrypted.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/03/10  Jill Baker
//   Added method getType()
//   Changed imports to new namespace
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   bigIntFromHash function has been moved into class HashFactory.
//   Added method getEncryptionKey().
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
import java.io.EOFException;
import java.util.Vector;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;

/**
 * DOCUMENT ME.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author original author not stated
 * @since  Cryptix 2.2 or earlier
 */
public final class PublicKeyEncrypted extends Packet
{
    private BigInteger enckey;
    private KeyID keyId;
    
    public PublicKeyEncrypted(PublicKey publicKey, byte[] ideaKey,
                               RandomStream rand)
    {
        keyId = new KeyID(publicKey.id());
        enckey = HashFactory.bigIntFromHash(publicKey, ideaKey, rand);
    }
    
    public PublicKeyEncrypted(DataInput in, int length)
    throws IOException
    {
        super(in, length);
    }

    public void 
    read(DataInput in, int length)
        throws IOException
    {
        in.readByte(); // version.
        byte[] keyIdBytes = new byte[8];
        in.readFully(keyIdBytes); // key id;
        keyId = new KeyID(keyIdBytes);
        if (in.readByte() != 0x01) // RSA algorythm identifier.
            throw new FormatException("Incorrect algorithm identifier.");
        try
        {
            enckey = MPI.read(in);
        }
        catch (EOFException e)
        {
            throw new IOException("MPI too long.");
        }
    }

    public int
    write(DataOutput out)
        throws IOException
    {
        out.write(0x02);
        out.write(keyId.toByteArray());        
        out.write(0x01);
        byte[] tmp = MPI.save(enckey);
        out.write(tmp);
        return tmp.length + 10;
    }
    
    /**
     * Returns the ID of the secret key needed to decrypt this packet.
     * Can be used with
     * <code>SecretKeyRing.getKey(KeyID keyID, Passphrase passphrase)</code>.
     */
    public KeyID
    secretKeyID()
    {
        return keyId;
    }

    public BigInteger getEncryptionKey()
    {
        return enckey;
    }

    public byte[]
    getKey(SecretKey key)
    {
        byte[] buf = key.decrypt(enckey).toByteArray();

        int sum = 0;
        for (int i = buf.length - 18; i < buf.length-2; i++)
            sum += buf[i] & 0xFF;
        int check = ((buf[buf.length-2] & 0xFF) << 8) + (buf[buf.length-1] & 0xFF);
        if (sum != check)
            throw new Error("sum mismatch sum:" + sum + " check:" + check);
        byte[] ideaKey = new byte[16];
        System.arraycopy(buf, buf.length - 18, ideaKey, 0, 16);
        return ideaKey;
    }
    
    public String
    toString()
    {
        return "Public key encrypted packet - number: " + enckey.toString();
    }

    public int getType()
    {
        return Packet.PUBLIC_KEY_ENCRYPTED;
    }
}
