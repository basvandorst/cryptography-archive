// This file is currently unlocked (change this line if you lock the file)
//
// $Log: SignedData.java,v $
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

import cryptix.security.rsa.PublicKey;
import cryptix.mime.LegacyString;

/**
 * Represents a signature, together with the data that has been signed.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public final class SignedData
{
    public byte[] data;
    public Signature signature;

    public ByteArrayInputStream dataAsByteArrayInputStream()
    {
        return new ByteArrayInputStream(data);
    }

    public String dataAsString()
    {
        return LegacyString.toString(data);
    }

    public SignedData(byte[] data, Signature sig)
    {
        this.data = data;
        this.signature = sig;
    }

    public KeyID getKeyId()
    {
        return signature.getKeyId();
    }

    public byte[] keyId()
    {
        return signature.keyId();
    }

    public boolean verify(PublicKey key)
    {
        return signature.verify(key, data);
    }
}
