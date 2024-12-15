// This file is currently unlocked
//
// $Log: Armoury.java,v $
// Revision 1.2  1999/06/30 23:02:00  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/04/12  Ian Brown
//   Cryptix version string read from cryptix.security.Cryptix
//
// Revision 0.2.5.0  1997/04/10  Ian Brown
//   Source control info and doccomments added
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
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix development team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.*;

import cryptix.util.mime.Base64;
import cryptix.mime.LegacyString;
import cryptix.security.Cryptix;

/**
 * Static methods to convert data to and from the 64-bit encoding, with
 * a 24-bit checksum, that PGP uses as 'transport armour'.
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author unattributed
 * @since  2.2
 * @deprecated New code should use ArmouredData in preference to this class.
 * @see ArmouredData
 */
public final class Armoury
{
    private Armoury() {} // static methods only
    
    private static Base64 base64 = new Base64();

    /** The string (\r\n) PGP uses as a 'canonical' linefeed. */
    public static final String LINEFEED = "\r\n";

    /**
     * Armour and return the data in <i>buf[]</i>
     * <strong>without</strong> the PGP framing lines.
     */
    public static String messageWithChecksum(byte[] buf)
    {
        byte[] checksum = new byte[3];
        int csum = CRC.checksum(buf);
        checksum[0] = (byte) (csum >>> 16);
        checksum[1] = (byte) (csum >>> 8);
        checksum[2] = (byte) (csum);

        return base64.encode(buf) + "=" + base64.encode(checksum);
    }

    /**
     * Armour and write the packet of type <i>name</i> in the byte array
     * <i>buf</i> to <i>out</i>.
     */
    public static void message(PrintStream out, String name, byte[] buf)
    {
        out.println("-----BEGIN PGP " + name + "-----");
        out.println("Version: " + Cryptix.getVersionString());
        out.println();
        out.print(messageWithChecksum(buf));
        out.println("-----END PGP " + name + "-----");
    }

    /**
     * Armour and return the packet of type <i>name</i> in the byte array
     * <i>buf</i>.
     */
    public static String message(String name, byte[] buf)
    {
        String framed =
            "-----BEGIN PGP " + name + "-----" + LINEFEED +
            "Version: " + Cryptix.getVersionString() + LINEFEED + LINEFEED +
            messageWithChecksum(buf) +
            "-----END PGP " + name + "-----" + LINEFEED;
        return framed;
    }

    /**
     * Armour and write the Packet <i>pkt</i> to <i>out</i>.
     *
     * @exception IOException if there was an I/O error
     */
    public static void message(PrintStream out, Packet pkt)
    throws IOException
    {
        message(out, name(pkt), pkt.save());
    }

    /**
     * 'Strips' armour off PGP binary data in the String <i>buf</i>
     * and checks its 24-bit checksum.
     *
     * @return the decoded data as a byte array.
     * @exception FormatException if the message was incorrectly formatted
     * @exception IOException if there was an I/O error
     * @exception InvalidChecksumException if the checksum was invalid
     */
    public static byte[] strip(String buf)
    throws FormatException, IOException, InvalidChecksumException
    {
        String checksum, armour;
        int pos;
        // Find start of ASCII armour
        pos = buf.indexOf("-----BEGIN PGP") ;

        if (pos == -1) throw new FormatException("Couldn't find PGP part in message.");

        // Now get rid of version / comments
        pos = buf.indexOf(LINEFEED + LINEFEED, pos);

        // Get message up to checksum
        armour = buf.substring(pos + 4, (pos = buf.indexOf(LINEFEED + "=")));
        checksum = buf.substring(pos + 2, pos + 7);
        // We now have message in armour and its checksum
        int givenChecksum = decodeChecksum(checksum);
        int realChecksum = CRC.checksum(base64.decode(armour));
        // Do the two checksums match?
        if (givenChecksum != realChecksum) throw new InvalidChecksumException("Message checksum wrong.");

        // OK - checksum correct, so return decoded armour
        return base64.decode(ArmouredMessage.byteEncode(armour));
    }

    /**
     * Returns the name PGP uses in its framing lines for this type
     * of packet.
     */
    public static String name(Packet pkt)
    {
        if (pkt instanceof Signature)
            return "SIGNATURE";
        else if (pkt instanceof SecretKeyCertificate)
            return "SECRET KEY BLOCK";
        else if (pkt instanceof PublicKeyCertificate)
            return "PUBLIC KEY BLOCK";

        throw new IllegalArgumentException("PGP Packet type "+pkt.getClass().getName()+" not valid for this method");
    }

    /**
     * Read from the DataInputStream <i>dis</i> until a PGP section appears.
     */
    public static void readUntilBegin(DataInputStream dis)
    throws IOException
    {
        String line;
        // Read from stream until a PGP section appears
        while (! ((line = dis.readLine()).startsWith("-----BEGIN PGP")))
            /* loop */;

        // Now get rid of version / comments
        while (! ((line = dis.readLine()).equals("")))
            /* loop */;
    }

    /**
     * Decode the checksum contained in the String <i>s</i>.
     */
    public static int decodeChecksum(String s)
    //throws InvalidChecksumException
    {
        if (s.length() != 5)
        {
        }

        byte[] asc_sum = LegacyString.toByteArray(s, 1, 4);
        byte[] binsum = base64.decode(asc_sum);
        int sum = (((int) binsum[0] & 0xFF) << 16) |
                  (((int) binsum[1] & 0xFF) <<  8) |
                   ((int) binsum[2] & 0xFF);
        return sum;
    }
}
