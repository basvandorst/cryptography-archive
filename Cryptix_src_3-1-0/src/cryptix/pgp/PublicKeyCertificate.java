// This file is currently unlocked (change this line if you lock the file)
//
// $Log: PublicKeyCertificate.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
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

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import cryptix.math.BigInteger;
import cryptix.math.MPI;
import cryptix.security.rsa.PublicKey;

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
 * @since Cryptix 2.2 or earlier
 */
public final class PublicKeyCertificate extends Certificate
{
    private PublicKey publicKey;

    public PublicKeyCertificate(PublicKey publicKey)
    {
        this.publicKey = publicKey;
    }
    
    public PublicKeyCertificate(DataInput in, int length)
    throws IOException
    {
        super(in, length);
    }

    public void 
    read(DataInput in, int length)
    throws IOException
    {
        super.read(in, length);
        if (in.readByte() != 0x01) // RSA algorithm identifier.
            throw new FormatException("Incorrect algorithm identifier.");
        BigInteger n = MPI.read(in);
        BigInteger e = MPI.read(in);
        publicKey = new PublicKey(n, e);
    }

    public int
    write(DataOutput out)
    throws IOException
    {
        byte tmp[];
        int len = super.write(out) + 1;
        out.write(0x01);
        out.write(tmp = MPI.save(publicKey.n()));
        len += tmp.length;
        out.write(tmp = MPI.save(publicKey.e()));
        len += tmp.length;
        return len;
    }

    public PublicKey publicKey() { return publicKey; }

    // To be removed:
    /** @deprecated Use publicKey() instead. */
    public PublicKey publickey() { return publicKey; }

    public String
    toString()
    {
        return "Public Key Certificate: " + super.toString() + "\n" + publicKey.toString();
    }
}
