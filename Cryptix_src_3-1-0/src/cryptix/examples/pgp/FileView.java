/*
// $Log: FileView.java,v $
// Revision 1.3  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.2  1997/12/07 07:01:28  hopwood
// + Can't remember what the change was - must have been something trivial.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.examples.pgp;

import java.io.*;

import cryptix.pgp.*;
import cryptix.security.*;
import cryptix.security.rsa.*;
import cryptix.util.core.Hex;
import cryptix.mime.LegacyString;

/**
 * DOCUMENT ME.
 * <p>
 * SECURITY: this should not be public, since it can read from arbitrary
 * files.
 */
final class FileView
{
    private static void usage()
    {
        System.out.println("Usage: java cryptix.pgp_examples.FileView [-s secring-filename] pgp-filename passphrase");
    }

    public static void main(String[] args)
    {
        String filename = null;
        String secringname = "secring.pgp";
        String passphrase = null;
        switch (args.length)
        {
        case 4:
            if (("-?".equals(args[0])) || (!("-s".equals(args[0]))))
            {
                usage();
                return;
            }
            secringname = args[1];
            filename = args[2];
            passphrase = args[3];
            break;
        case 2:
            if (("-?".equals(filename = args[0])))
            {
                usage();
                return;
            }
            passphrase = args[1];
            break;
        default:
            usage();
            return;
        }
        try
        {
            SecretKey key = null;
            try
            {
                InputStream sec_in;
                Packet p = PacketFactory.read(new DataInputStream(sec_in = new FileInputStream(secringname)));
                sec_in.close();
                if (!(p instanceof SecretKeyCertificate))
                {
                    System.out.println("File : " + secringname + " Not a pgp secret key.");
                    System.exit(-1);
                }
                key = ((SecretKeyCertificate)p).getSecretKey(new Passphrase(passphrase));
            }
            catch(FileNotFoundException fnfe)
            {
                System.out.println("File : " + secringname + " Not found.");
                System.exit(-1);
            }
            catch(DecryptException fnfe)
            {
                System.out.println("Incorrect passphrase");
                System.exit(-1);
            }
            InputStream in;
            System.out.println("- contents of " + filename + " -");
            viewPackets(0, key, new DataInputStream(in = new FileInputStream(filename)));
            in.close();
        }
        catch(FileNotFoundException fnfe)
        {
            System.out.println("File : " + filename + " Not found.");
        }
        catch(Exception e)
        {
            e.printStackTrace(System.out);
        }
    }

    private static void viewPackets(int level, cryptix.security.rsa.SecretKey key,
                                     DataInputStream in)
    throws IOException, DecryptException
    {
        int i = 1;
        Packet pkt;
        PublicKeyEncrypted hdr = null;
        Signature sig = null;

        while((pkt = PacketFactory.read(in)) != null)
        {
            println(level, "*- packet " + i++ + " : " + pkt.getClass().getName());
            if (pkt instanceof PublicKeyEncrypted)
                hdr = (PublicKeyEncrypted)pkt;
            println(level, pkt.toString());
            if (pkt instanceof ConvEncryptedData)
            {
                try
                {
                    if (hdr != null)
                    {
                        byte ideakey[] = hdr.getKey(key);
                        byte data[] = ((ConvEncryptedData)pkt).data(ideakey);
                        viewPackets(level + 1, key, new DataInputStream(new ByteArrayInputStream(data)));
                      }
                    else
                        println(level, "no IDEA key found");
                }
                catch (IOException ioe)
                {
                    println(level, "Unable to recure ...");
                    println(level, ioe.getClass().getName() + ":" + ioe.getMessage());
                }
                catch (Error e)
                {
                    e.printStackTrace(System.out);
                    println(level, "unable to get IDEA key ...");
                    println(level, e.getClass().getName() + ":" + e.getMessage());
                }
            }
            else if (pkt instanceof LiteralData)
            {
                LiteralData p = (LiteralData)pkt;
                byte buf[];
                println(level, "original file: " + p.filename() + " mode:" + p.mode());
                println(level, " date created: " + p.timestamp());
                println(level, "data;");
                println(level + 1, LegacyString.toString(buf = p.data()));
                if (sig != null)
                {
                    try
                    {
                        MD5 md = new MD5();
                        if (p.mode()  != 'b')
                        {
                            println(level, "text file converting for signature");
                            texthashlinesinto(md, buf);
                        }
                        else
                        {
                            println(level, "Binary file");
                            md.add(buf);
                        }

                        if (sig.check(key, md))
                            println(level, "signed by me and O.K.");
                        else
                            println(level, "NOT signed by me");
                    }
                    catch (IOException ioe)
                    {
                        println(level, "Unable to check signature ...");
                        println(level, ioe.getClass().getName() + ":" + ioe.getMessage());
                    }
                }
                else
                    println(level, "NO signature");
            }
            else if (pkt instanceof Signature)
            {
                sig = (Signature)pkt;
                println(level, "keyid:" + hexString(sig.getKeyId()));
            }
        }
        println(level, "- Done -");
    }
    
    private static final int LINE_LENGTH = 76;

    private static void println(int level, String msg)
    {
        DataInputStream in = new DataInputStream(new StringBufferInputStream(msg));
        int max = LINE_LENGTH - level;
        try
        {
            while ((msg = in.readLine()) != null)
            {
                while (msg.length() > max)
                {
                    printString(level, msg.substring(0, max));
                    msg = msg.substring(max);
                }
                printString(level, msg);
            }
        }
        catch (IOException ioe)
        {
        }
    }

    private static void printString(int level, String msg)
    {
        System.out.print(level);
        System.out.print('-');
        while (level-- > 0)
            System.out.print(' ');
        System.out.println(msg);
    }
    
    private static void texthashlinesinto(MD5 md, byte[] buf)
    throws IOException
    {
        DataInputStream in = new DataInputStream(new ByteArrayInputStream(buf));
        String line;
        while((line = in.readLine()) != null)
        {
            if (line.startsWith("- "));
                line = line.substring(2);
            md.add(LegacyString.toByteArray(line + "\r\n"));
        }
    }
    
    private static String hexString(KeyID id)
    {
        return Hex.toString(id.toByteArray());
    }
}
