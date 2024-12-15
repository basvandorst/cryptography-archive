// $Log: conv_encrypt.java,v $
// Revision 1.2  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
package cryptix.examples.pgp;

import java.io.*;

import cryptix.math.PseudoRandomStream;
import cryptix.security.MD5;
import cryptix.security.SHA;
import cryptix.pgp.*;
import cryptix.mime.LegacyString;

/**
 * DOCUMENT ME.
 * <p>
 * SECURITY: this should not be public, since it can write to "stuff.txt" and
 * "conv.pgp".
 */
final class conv_encrypt
{
    public static void main(String[] args)
    throws FileNotFoundException, IOException, FormatException
    {
        String messageText = "The quick brown fox jumps over the lazy dog";

        byte[] foo = { (byte)'x' };
        byte[] ideaKey = new MD5().digest(foo);

        String keySeed = "Some random data to seed PseudoRandomStream ^%$£*&£^$&!£*!%&&^%";
        PseudoRandomStream rs = new PseudoRandomStream(
            new SHA().digest(LegacyString.toByteArray(keySeed)));

        System.err.println("ideaKey length: " + ideaKey.length + " (should be 16)");

        LiteralData stuff = new LiteralData(LegacyString.toByteArray(messageText),
            "stuff.txt", 't');
        byte[] stuff_buf = PacketFactory.save(stuff);

        ConvEncryptedData CKE = new ConvEncryptedData(stuff_buf, ideaKey, rs);
        System.err.println("Writing file");

        FileOutputStream fos = new FileOutputStream("conv.pgp");
        DataOutputStream dos = new DataOutputStream(fos);
        PacketFactory.write(dos, CKE);
        fos.close();
    }
}
