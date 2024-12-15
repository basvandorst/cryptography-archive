// This file is currently unlocked (change this line if you lock the file)
//
// $Log: ConvEncryptedData.java,v $
// Revision 1.2  1999/06/30 23:02:00  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/03/10  Jill Baker
//   Added getType() method
//   imports changed to new namespace
//
// Revision 0.2.5.1  1997/03/01  Jill Baker
//   Added the getBuffer() method.
//
// Revision 0.2.5.0  1997/02/24  Original Author Not Stated
//   No history before this date
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

import java.io.ByteArrayInputStream;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.FileOutputStream;

import cryptix.math.RandomStream;
import cryptix.security.BlockCipher;
import cryptix.security.IDEA;

/**
 * Class representing an encrypted Packet.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public final class ConvEncryptedData extends Packet
{
    protected byte[] buffer = null;

    public ConvEncryptedData(DataInput in, int length)
    throws IOException
    {
        read(in, length);
    }
    
    public ConvEncryptedData(byte[] data, byte[] key, RandomStream rand)
    throws IOException
    {
        buffer = new byte[data.length + 10];
        IDEA block_cipher = new IDEA(key);
        CFB cipher = new CFB(block_cipher);
        byte[] pre = getKeyCheck(rand);

        int blen = block_cipher.blockLength();

        cipher.encrypt(pre, 0, buffer, 0, 10);
        cipher.decrypt(buffer, 2, new byte[blen], 0, blen); // Set the IV
        cipher.encrypt(data, 0, buffer, 10, data.length);
    }
    
    public byte[] getBuffer() { return buffer; }

    private byte[] getKeyCheck(RandomStream rand)
    {
        byte[] tmp = new byte[10];
        for (int i = 7; i >= 0; i--)
        {
            byte fill;
            do {
                fill = (byte) rand.read();
            } while (fill == 0);
            tmp[i] = fill;
        }
        tmp[8] = tmp[6];
        tmp[9] = tmp[7];
        return tmp;
    }
    
    public void read(DataInput in, int length)
    throws IOException
    {
        if (length <= 11)
            throw new FormatException("Length too short");

        buffer = new byte[length];
        in.readFully(buffer);
    }

    public int write(DataOutput out)
    throws IOException
    {
        out.write(buffer);
        return buffer.length;
    }

    public byte[] data(byte[] key)
    throws DecryptException
    {
        IDEA block_cipher = new IDEA(key);
        CFB cipher = new CFB(block_cipher);
        int blen = block_cipher.blockLength();

        byte[] returnBuf = new byte[buffer.length - 10];
        byte[] iv = new byte[10];

        System.arraycopy(buffer, 0, iv, 0, 10);
        System.arraycopy(buffer, 10, returnBuf, 0, returnBuf.length);

        // Set the IV and verify check digits
        cipher.decrypt(buffer, 0, iv, 0, iv.length);
        if ((iv[6] != iv[8]) || (iv[7] != iv[9]))
            throw new DecryptException("Invalid decryption key");

        cipher.decrypt(buffer, 10, returnBuf, 0, returnBuf.length);
        return returnBuf;
    }

    public int getType()
    {
        return Packet.CONV_ENCRYPTED;
    }
}
