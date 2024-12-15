// $Log: sTest.java,v $
// Revision 1.2  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
package cryptix.examples.pgp;

import java.io.*;
import cryptix.security.rsa.*;

import cryptix.pgp.*;

/**
 * Encrypts and signs a string (messageText) and writes the ciphertext to es-test.asc
 * Demonstrates the ArmouredMessage, Passphrase and SecretKeyRing classes.
 * Run this in your PGP directory, or provide the full path to your keyrings.
 * <p><b>$Revision: 1.2 $</b>
 * @author Ian Brown
 */
public final class sTest
{
    public static final void main(String argv[]) throws FileNotFoundException, IOException, DecryptException, FormatException
    {
    SecretKeyRing secretKeyRing;
        cryptix.security.rsa.SecretKey myKey;
    ArmouredMessage message;
    FileOutputStream fos;

    String myAddress = "Edwin Woudt testing Cryptix PGP <edwin@cryptix.org>";
    String messageText = "Put your test message here." + Armoury.LINEFEED
        + "It can be as long as you like.";

     if (argv.length < 1)
    {
        System.out.println("Usage: java sTest passphrase");
        return;
    }

    System.out.println("Writing a clearsigned version of '" + messageText + " to s-test.asc.");

    // Get key
     secretKeyRing = new SecretKeyRing("secring.pgp");
     myKey = secretKeyRing.getKey(myAddress, new Passphrase(argv[0]));
    if (myKey == null)
    {
        System.err.println("Couldn't find a key for " + myAddress);
        System.exit(1);
    }

    // Message will be clearsigned
    message = new ArmouredMessage(messageText, myKey, null);

    // Write clearsigned message to a file
    fos = new FileOutputStream("s-test.asc");
    new PrintStream(fos).print(message.ciphertext());
    fos.close();
    }
}
