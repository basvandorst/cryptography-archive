// $Log: conv_decrypt.java,v $
// Revision 1.2  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
package cryptix.examples.pgp;

import java.io.*;
import cryptix.security.MD5;
import cryptix.pgp.*;

/**
 * DOCUMENT ME.
 * <p>
 * SECURITY: this should not be public, since it can read from "conv.pgp".
 */
final class conv_decrypt
{
    public static void main(String[] args)
    throws FileNotFoundException, IOException, FormatException
    {
        byte[] foo = { (byte)'x' };
        byte[] key = new MD5().digest(foo);

        FileInputStream fis = new FileInputStream("conv.pgp");
        DataInputStream dis = new DataInputStream(fis);

        Packet pkt = PacketFactory.read(dis);
        if (pkt == null)
        {
            System.out.println("Could not read packet");
            return;
        }

        if (pkt instanceof ConvEncryptedData)
        {
            byte[] data;
            try {
                byte[] ld_data = ((ConvEncryptedData) pkt).data(key);
                Packet ld = PacketFactory.load(ld_data);
                if (ld instanceof LiteralData)
                {
                    data = ((LiteralData) ld).data();
                }
                else
                {
                    System.out.println("Invalid format packet");
                    return;
                }
            }
            catch (DecryptException e)
            {
                System.out.println("Invalid passphrase");
                return;
            }
            System.out.write(data, 0 ,data.length);
            System.out.println();
        }
    }
}
