/*
// $Log: KeyGen.java,v $
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
import java.util.Date;

import cryptix.math.*;
import cryptix.security.*;
import cryptix.security.rsa.*;
import cryptix.pgp.*;
import cryptix.mime.LegacyString;

/**
 * DOCUMENT ME.
 * <p>
 * SECURITY: this should not be public, since it writes to "pubring.pgp" and
 * "secring.pgp".
 */
class KeyGen implements RSAKeyGenObserver
{
    private static final int MIN_KEY_LENGTH = 384;
    private static final int MAX_KEY_LENGTH = 6000;
    
    protected DataInputStream in;
    protected PrintStream out;
    
    public KeyGen(InputStream in, PrintStream out)
    {
        this.in = (in instanceof DataInputStream) ? (DataInputStream) in
                                                  : new DataInputStream(in);
        this.out = out;
    }

    public KeyGen()
    {
        this(System.in, System.out);
    }

    public static void main(String[] args)
    {
        new KeyGen().begin();
    }

    protected final void begin()
    {
        try
        {
            String name = getUserName();
            int len = getKeyLength();
            RandomStream rs;
            SecretKey gsk = getKey(len, rs = getRandomStream(len));
            System.out.println("Key generated.");
            String passphrase = getPassphrase();
            saveNewKey(gsk, name, passphrase, rs);
        }
        catch(Exception e)
        {
            System.out.println("Key gen aborted.");
            e.printStackTrace();
        }
    }

    protected SecretKey getKey(int length, RandomStream rs)
    {
        return new RSAKeyGen(rs).createKey(length, this);
    }

    protected String getUserName()
    throws IOException
    {
        out.println();
        out.print("Please enter your user name? ");
        out.flush();
        in.readLine(); //why ??
        String name = in.readLine();
        out.print("using name  ");
        out.println(name);
        return name;
    }

    protected int getKeyLength()
    throws IOException
    {
        while (true)
        {
            int len;
            out.print("Please enter key size (512, 2048 etc).");
            out.flush();
            try
            {
                len = Integer.parseInt(in.readLine().trim());
            }
            catch (NumberFormatException nfe)
            {
                out.println("Bad format number try again.");
                continue;
            }
            if (len < MIN_KEY_LENGTH)
            {
                out.println("Key bit length too short. (<" + MIN_KEY_LENGTH + ")");
                continue;
            }
            if (len > MAX_KEY_LENGTH)
            {
                out.println("Key bit length too long. (>" + MAX_KEY_LENGTH + ")");
                continue;
            }
            return len;
        }
    }
    
    /**
     * This is NOT VERY SECURE as the seed is rather simplistic.
     */
    protected RandomStream getRandomStream(int keyLength)
    throws IOException
    {
        int min = ((keyLength / 6) + 10);
        ByteArrayOutputStream seedBuffer = new ByteArrayOutputStream(8192);
        PrintStream seedout = new PrintStream(seedBuffer);
        seedout.print(new Date());
        seedout.println("Please enter some random data");
        out.print("You will have to enter a minimum of ");
        out.print(min);
        out.println(" characters;");
        seedout.print(System.currentTimeMillis());
        SHA sha = new SHA();
        MD5 md5 = new MD5();
        while (min > 0)
        {
            String data = in.readLine();
            sha.add(LegacyString.toByteArray(
                Long.toString(System.currentTimeMillis())));
            sha.add(LegacyString.toByteArray(data));
            min -= data.length();
            seedout.print(min);
            seedout.print(data.length());
            seedout.print(System.currentTimeMillis());
            seedout.print(data);
            out.print("characters to go ");
            out.println(min < 0 ? 0 : min);
            md5.add(LegacyString.toByteArray(data));
            md5.add(LegacyString.toByteArray(
                Long.toString(System.currentTimeMillis(), 16)));
        }
        try
        {
            seedout.write(sha.digest());
            seedout.write(md5.digest());
        }
        catch (IOException ioe)
        {
            ioe.printStackTrace(seedout);
        }

        return new CSRandomStream(SHA.hash(seedBuffer.toByteArray()));
    }
    
    /**
     * WARNING this prints the passphrase out on to the screen.
     */
    protected String getPassphrase() throws IOException
    {
        String passphrase = "";
        while(true)
        {
            out.print(" please enter passphrase ");
            out.flush();
            String pass1 = in.readLine();
            out.print(" please re-enter passphrase ");
            out.flush();
            passphrase = in.readLine();
            if (pass1.equals(passphrase))
                return passphrase;
            out.println("Passphrases do not match.");
            out.println("Please try again.");
        }
    }
    
    private final void saveNewKey(SecretKey gsk, String name, String passphrase,
                                   RandomStream rs)
    throws IOException
    {    
        FileOutputStream pubring;
        FileOutputStream secring;

        UserId username = new UserId(name);

        DataOutputStream pub = new DataOutputStream(
            pubring = new FileOutputStream("pubring.pgp"));

        PublicKeyCertificate pkc = new PublicKeyCertificate(gsk); // sk isa pk.
        PacketFactory.write(pub, pkc);
        PacketFactory.write(pub, new KeyRingTrust((byte) 0x87));
        PacketFactory.write(pub, username);
        PacketFactory.write(pub, new KeyRingTrust((byte) 0x03));
        pubring.close();

        DataOutputStream sec = new DataOutputStream(
            secring = new FileOutputStream("secring.pgp"));

        SecretKeyCertificate skc = new SecretKeyCertificate(
            gsk, new Passphrase(passphrase) , rs);
        PacketFactory.write(sec, skc);
        PacketFactory.write(sec, username);
        secring.close();
    }
    
    public void handleEvent(int evnt)
    {
        switch (evnt)
        {
        case HAS_SMALL_FACTORS:
            out.print("-");
            out.flush();
            break;

        case NOT_PRIME:
            out.print("+");
            out.flush();
            break;

        case IS_PRIME:
            out.print("*");
            out.flush();
            break;

        case FINDING_P:
            out.println();
            out.print("p:");
            out.flush();
            break;

        case FINDING_Q:
            out.println();
            out.print("q:");
            out.flush();
            break;

        case KEY_RESOLVING:
            out.println("Key resolving begins");
            break;

        case NO_SMALL_FACTORS:
        case KEY_GEN_DONE:
            break;

        default:
            out.println("Unexpected event : " + evnt);
            break;
        }
    }
}
