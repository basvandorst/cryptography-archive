/*
// $Log: KeyRingTest.java,v $
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
import cryptix.security.rsa.*;
import cryptix.pgp.*;

public final class KeyRingTest extends KeyRing
{
    KeyRingTest(String name)
    throws IOException
    {
        super(name);
    }

    protected PublicKey
    getKey(String username)
    {
        return getPublicKey((KeyID)(users.get(username)));
    }

    protected PublicKey
    getKey(KeyID keyID)
    {
        return getPublicKey(keyID);
    }

        public void init(PacketInputStream ps){}

    public static final void
    main(String argv[])
    {
        if (argv.length < 1)
        {
            System.out.println("Usage: java cryptix.pgp_examples.KeyRingTest keyring-name");
            return;
        }
        try
        {
            new KeyRingTest(argv[0]).dump(System.out);
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
}
