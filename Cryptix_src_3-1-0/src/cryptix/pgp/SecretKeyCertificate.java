// This file is currently unlocked (change this line if you lock the file)
//
// $Log: SecretKeyCertificate.java,v $
// Revision 1.3  1999/07/12 20:27:13  edwin
// Basic support for reading (not writing) unencrypted secret keys.
//
// Revision 1.2  1999/06/30 23:02:01  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix development team.  All rights reserved.
 */

//
// This module still has IDEA hard coded all over the place
// This must be fixed
//
// This source file originally included the non-public classes
// EncryptedBigInt and EncryptedSecretKey - apparently Metrowerks'
// compiler doesn't like that, even though it's explicitly allowed
// by the Java language spec.
//

package cryptix.pgp;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.math.RandomStream;
import cryptix.security.StreamCipher;
import cryptix.security.MD5;
import cryptix.security.IDEA;
import cryptix.security.rsa.SecretKey;
import cryptix.util.Streamable;

/**
 * DOCUMENT ME.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.3 $</b>
 * @author unattributed
 * @since Cryptix 2.2 or earlier
 */
public final class SecretKeyCertificate extends Certificate 
{
    private EncryptedSecretKey key;

    public SecretKeyCertificate(SecretKey key0, Passphrase passphrase, RandomStream rand)
        throws IOException
    {
        key = new EncryptedSecretKey(key0, passphrase, rand);
    }

    public SecretKeyCertificate(DataInput in, int length)
        throws IOException
    {
        super(in, length);
    }

    public void
    read(DataInput in, int length)
        throws IOException
    {
        super.read(in, length);
        
        if (in.readByte() != 0x01)
            throw new FormatException("Bad algorithm byte (not RSA).");

        key = new EncryptedSecretKey(in);
    }

    public int
    write(DataOutput out)
        throws IOException
    {
        int len = super.write(out) + 1;
        out.write(0x01);
        byte tmp[];
        out.write(tmp = key.save());
        len += tmp.length;
        return len;
    }

    public SecretKey
    getSecretKey(Passphrase passphrase)
        throws DecryptException
    {
        if (key.isEncrypted()) {
            CFB cipher = new CFB(new IDEA(passphrase.md5hash()));
            return key.secretKey(cipher);
        } else {
            return key.secretKey(null);
        }
    }

    /**
     * Gets the key ID.
     *
     * @return the ID of the key in the certificate.
     */
    public final KeyID
    id()
    {
        return key.id();
    }

    public String
    toString()
    {
        return "Secret key Cert " + super.toString();
    }

    public int getType()
    {
        return Packet.SECRET_KEY;
    }
}
