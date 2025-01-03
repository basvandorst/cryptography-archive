/*
// $Log: KeyView.java,v $
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
import cryptix.pgp.*;

public final class KeyView
{
    public static final void
    main(String argv[])
    {
        if (argv.length < 1)
        {
            System.out.println("Usage: java KeyView keyring-name");
            return;
        }
        try
        {
            InputStream in;
            System.out.println("- contents of " + argv[0] + " -");
            viewPackets(new DataInputStream(in = new FileInputStream(argv[0])));
            in.close();
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println("File : " + argv[0] + " Not found.");
        }
        catch(Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private static final void
    viewPackets(DataInputStream in)
    throws IOException
    {
        int i = 1;
        Packet pkt;
        while((pkt = PacketFactory.read(in)) != null)
        {
            System.out.println("*- packet " + i++ + " : " + pkt.getClass().getName());
            System.out.println(pkt);
        }
        System.out.println("- Done -");
    }
}
