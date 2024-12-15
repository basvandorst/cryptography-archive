/*
// $Log: KeyRingTestToo.java,v $
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

public final class KeyRingTestToo
{
    public static final void
    main(String argv[])
    {
        if (argv.length < 1)
        {
            System.out.println("Usage: java cryptix.pgp_examples.KeyRingTestToo keyring-name");
            return;
        }
        try
        {
            new KeyRingTest(argv[0]).list(System.out);
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
