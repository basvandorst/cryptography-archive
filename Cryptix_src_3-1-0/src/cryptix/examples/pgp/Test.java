/*
// $Log: Test.java,v $
// Revision 1.2  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.examples.pgp;

import java.io.*;
import cryptix.security.MD5;
import cryptix.security.rsa.*;
import cryptix.pgp.*;

final class Test
{
    public static void main(String[] args)
    {
        if (args.length < 1)
        {
            System.out.println("Usage: java cryptix.pgp_examples.Test keyring-name");
            return;
        }
        try
        {
            checkKeyRing(new DataInputStream(new FileInputStream(args[0])));
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }
    }

    public static void checkKeyRing(DataInput in)
    throws IOException
    {
        Packet p;
        PublicKeyCertificate cert = (PublicKeyCertificate)
            PacketFactory.read(in);
        PacketFactory.read(in);
        UserId name = (UserId) PacketFactory.read(in);
        PacketFactory.read(in);
        Signature sig = (Signature) PacketFactory.read(in);
        PublicKey key = cert.publicKey();

        // N.B. this is the WRONG KEY BUT we know the hash should be
        System.out.println("REAL:4100673812DF82143FE66681CE26D764577");
        // 0x4100673812DF82143FE66681CE26D764577
        MD5 md = new MD5();
        md.add(cert.save());
        md.add(name.save());
        sig.check(key, md);
        System.out.println("Hash should end\n43FE66681CE26D764577");
    }
}
