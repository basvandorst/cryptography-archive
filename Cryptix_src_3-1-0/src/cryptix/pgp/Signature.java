// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Signature.java,v $
// Revision 1.3  1999/06/30 23:02:01  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.2  1997/11/20 23:02:06  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.4  1997/03/10  Jill Baker
//   Replaced use of BigInteger.cmp() method with BigInteger.equals()
//   Added getType() method
//
// Revision 0.2.5.3  1997/03/04  Jill Baker
//   Explicit (byte) cast put in to keep JDK 1.0 compiler happy
//
// Revision 0.2.5.2  1997/03/04  Jill Baker
//   Restored the constructor (SecretKey, MD5)
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Added constants for signature types.
//   Moved method bigIntFromHash() to class HashFactory.
//   Added support for alternative public key and hash algorithms.
//   Added method getSignatureType()
//   Added the requirement for a RandomStream to the constructor.
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
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Date;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.MD5;
import cryptix.security.MessageHash;
import cryptix.security.rsa.SecretKey;
import cryptix.security.rsa.PublicKey;
import cryptix.util.core.Hex;

/**
 * DOCUMENT ME.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.3 $</b>
 * @author original author not stated
 * @author Jill Baker
 * @since  Cryptix 2.2 or earlier
 */
public final class Signature extends Packet
{
    private static final boolean DEBUG = false;

    // Signature types
    public static final int BINARY_IMAGE                   = 0x00;
    public static final int CANONICAL_TEXT                 = 0x01;
    public static final int KEY_CERT_GENERIC               = 0x10;
    public static final int KEY_CERT_PERSONA               = 0x11;
    public static final int KEY_CERT_CASUAL_ID             = 0x12;
    public static final int KEY_CERT_POSITIVE_ID           = 0x13;
    public static final int KEY_COMPROMISE_CERTIFICATE     = 0x20;
    public static final int USER_ID_REVOCATION_CERTIFICATE = 0x30;
    public static final int TIMESTAMP_CERTIFICATE          = 0x40;

    private BigInteger number;
    private byte extra[];
    private byte keyId[];
    private int  pkeAlgorithm  = Algorithm.RSA;
    private int  hashAlgorithm = Algorithm.MD5;
    private byte hashId[];

    public Signature(SecretKey key, MD5 md)
    {
        this(key, md, null);
    }

    public Signature(SecretKey key, MD5 md, RandomStream rand)
    {
        int unixtime = (int)(System.currentTimeMillis()/1000);
        extra = new byte[5];
        extra[0] = (byte) BINARY_IMAGE; // binary data.
        extra[1] = (byte) ((unixtime >>> 24) & 0xFF);
        extra[2] = (byte) ((unixtime >>> 16) & 0xFF);
        extra[3] = (byte) ((unixtime >>> 8)  & 0xFF);
        extra[4] = (byte) ( unixtime          & 0xFF);
        md.add(extra);
        byte hash[] = md.digest();
        BigInteger tmp = HashFactory.bigIntFromHash(key, hash, rand);
        number = key.decrypt(tmp); // signing is encrypting with secret therefore normal decryption.
        hashId = new byte[2];
        hashId[0] = hash[0];
        hashId[1] = hash[1];
        keyId = key.id();
    }

    public Signature(DataInput in, int length)
    throws IOException
    {
        super(in, length);
    }

    public void
    read(DataInput in, int length)
    throws IOException
    {
        in.readByte(); // version
        int extraLen = in.readByte() & 0xFF;

        if (extraLen != 5 && extraLen != 7)
            throw new FormatException("The length of the extra data field is incorrect.");

        in.readFully(extra = new byte[extraLen]);
        in.readFully(keyId = new byte[8]);

        pkeAlgorithm  = in.readByte();
        hashAlgorithm = in.readByte();

        in.readFully(hashId = new byte[2]);

        number = MPI.read(in);
    }

    public int
    write(DataOutput out)
    throws IOException
    {
        out.write(0x02);
        out.write(extra.length);
        out.write(extra);
        out.write(keyId);
        out.write(pkeAlgorithm); 
        out.write(hashAlgorithm);
        out.write(hashId);
        byte tmp[] = MPI.save(number);
        out.write(tmp);
        return extra.length + tmp.length + 14;
    }

    public String
    typeString()
    {
        switch (extra[0])
        {
            case BINARY_IMAGE:
                return "Binary image";
            case CANONICAL_TEXT:
                return "Text image";
            case KEY_CERT_GENERIC:
                return "Key cert";
            case KEY_CERT_PERSONA:
                return "Key certificate anon";
            case KEY_CERT_CASUAL_ID:
                return "Key certificate casual";
            case KEY_CERT_POSITIVE_ID:
                return "Key certificate heavy duty";
            case KEY_COMPROMISE_CERTIFICATE:
                return "Key compromise certificate";
            case USER_ID_REVOCATION_CERTIFICATE:
                return "User ID revocation certificate";
            case TIMESTAMP_CERTIFICATE:
                return "Time stamp certificate";
        }
        return "unknown type (" + extra[0] + ")";
    }

    public String
    toString()
    {
        return "Signature packet " + typeString() + " key id : " +
            Hex.toString(keyId);
    }

    private boolean check(PublicKey key, byte[] filehash)
    {
if (DEBUG) System.err.println("in  KeyId: " + Hex.toString(key.id()));
if (DEBUG) System.err.println("sig KeyId: " + Hex.toString(keyId));
if (DEBUG) System.err.println("Hash:      " + Hex.toString(filehash));

        if ((filehash[0] != hashId[0]) || (filehash[1] != hashId[1]))
            return false;

        byte[] id = key.id();
        int len = id.length;
        if (len == keyId.length)
        {
            int i;
            for (i = 0; i < len; i++)
                if (id[i] != keyId[i])
                    return false;
        }

        // hash id and key id both match now check the real Hashes.

        BigInteger hashFromPacket = key.encrypt(number);
        BigInteger hashFromFile = HashFactory.bigIntFromHash(key, filehash);

if (DEBUG) System.err.println("hash   " + hashFromFile);
if (DEBUG) System.err.println("number " + hashFromPacket);

        return hashFromPacket.equals(hashFromFile);
    }

    public boolean
    check(PublicKey key, MD5 md)
    {
        md.add(extra);
        return check(key, md.digest());
    }

    public boolean
    verify(PublicKey key, byte[] data)
    {
        MD5 md = new MD5();
        md.add(data);
        md.add(extra);
        return check(key, md.digest());
    }

    public byte[]
    checkAndGetHash(PublicKey key, MD5 md)
    {
        md.add(extra);
        byte[] hash = md.digest();
        if (!check(key, hash))
            return null;
        return hash;
    }

    // use with caution.
    public void
    addExtasToHash(MD5 md)
    {
        md.add(extra);
    }

    public int getSignatureType()
    {
        return extra[0];
    }

    public boolean
    check(PublicKey key, MessageHash completeHash)
    {
        return check(key, completeHash.toByteArray());
    }

    public byte[]
    keyId()
    {
        byte tmp[];
        int len = keyId.length;
        System.arraycopy(keyId, 0, tmp = new byte[len], 0, len);
        return tmp;
    }

    public KeyID
    getKeyId()
    {
        return new KeyID(keyId);
    }

    public int getType()
    {
        return Packet.SIGNATURE;
    }
}
