// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Message.java,v $
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
import cryptix.math.RandomStream;
import cryptix.security.rsa.PublicKey;

import cryptix.util.Streamable;

/**
 * This class represents a PGP message.
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
public class Message extends Streamable
{
    private PublicKeyEncrypted messageHdr;
    private ConvEncryptedData messageData;
    
    public Message(byte[] data, PublicKey publickey, byte[] key,
                    RandomStream rand)
    throws IOException
    {
        messageHdr = new PublicKeyEncrypted(publickey, key, rand);
        messageData = new ConvEncryptedData(data, key, rand);
    }

    public byte[]
    data(byte[] key)
    throws IOException, DecryptException
    {
        return messageData.data(key);
    }
    
    public void 
    read(DataInput in)
    throws IOException
    {
        throw new IOException("This is an outgoing-only data type.");
    }
    
    public void
    write(DataOutput out)
    throws IOException
    {
        PacketFactory.write(out, messageHdr);
        PacketFactory.write(out, messageData);
    }
}
