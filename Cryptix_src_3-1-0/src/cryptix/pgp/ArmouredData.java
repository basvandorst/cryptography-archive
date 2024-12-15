// This file is currently locked by jbaker@madge.com
//
// $Log: ArmouredData.java,v $
// Revision 1.2  1999/06/30 23:02:00  edwin
// Fixing references to moved and/or changed classes. It should compile now with cryptix 3.0.x
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import cryptix.mime.LegacyString;
import cryptix.util.mime.Base64;
import cryptix.util.mime.QuotedPrintable;
import cryptix.util.Streamable;
import cryptix.security.Cryptix;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;

/**
 * This class represents an ASCII-Armoured message.
 * <p>
 * Although this class can also represent a clearsigned message, it is
 * not the job of this class either to sign, or to verify the signature
 * - that is the job of the Signature class. Thus, using the ArmouredData
 * class alone, it is perfectly possible to create a clearsigned message
 * with a bad signature!
 * <p>
 * It is also not the job of this class to deal with character set
 * conversions or canonical form. The classes in the package
 * cryptix.pgp.charset can do that job much better.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.2 $</b>
 * @author Jill Baker
 * @author Ian Brown
 * @author Gary Howland
 * @since  Cryptix 2.2 or earlier
 * @see cryptix.pgp.Signature
 * @see cryptix.pgp.app.ArmourDecryptor
 */
public final class ArmouredData
{
    // For the "Comment:", etc., parts of the message...
    private Properties properties;

    // For the clear text of a clearsigned message
    private byte[] clearText;

    // The signature, or self-contained armoured packet(s)
    private PacketByteArray payload;
    
    // Mime (d)e(n)coders
    private static Base64 base64 = new Base64();
    protected static QuotedPrintable quotedPrintable = new QuotedPrintable();

    public ArmouredData(PacketByteArray payload)
    {
        this(new Properties(), payload);
    }

    public ArmouredData(Properties properties, PacketByteArray payload)
    {
        this.properties = properties;
        this.clearText = null;
        this.payload = payload;
        properties.put("version", Cryptix.getVersionString());
    }

    public ArmouredData(byte[] text, PacketByteArray signature)
    throws FormatException
    {
        this(new Properties(), text, signature);
    }

    public ArmouredData(Properties properties,
                         byte[] clearText, PacketByteArray signature)
    throws FormatException
    {
        this.properties = properties;
        this.clearText = clearText;
        this.payload = payload;
        properties.put("version", Cryptix.getVersionString());

        /* This is all the checking we do.
         * Full signature verification is left to another class.
         */
        if ((payload.numPackets() != 1) ||
            (payload.getPacketType(0) != Packet.SIGNATURE))
        {
            throw new FormatException(
                "Payload for clearsigned text is not a Signature packet");
        }
    }

    /**
     * The following constructors accept text that is already
     * armoured.
     *
     * @see #parse(String)
     * @see #toString()
     */
    public ArmouredData(String armouredText) throws FormatException
    {
        properties = new Properties();
        parse(armouredText);
    }

    public ArmouredData(byte[] armouredBytes) throws FormatException
    {
        this(LegacyString.toString(armouredBytes));
    }

    public Properties getProperties() { return properties; }
    public byte[] getClearText() { return clearText; }
    public PacketByteArray getPayload() { return payload; }



    // Static methods to armour and de-armour a message (EXcluding the
    // "-----BEGIN PGP MESSAGE-----" (etc.), version, etc. wrappers.
    // Variants of these functions were originally placed in the
    // now deprecated Armoury class.

    public static String armour(PacketByteArray pba)
    {
        byte[] mainBytes = pba.toByteArray();
        String mainEncodedString = base64.encode(mainBytes);

        int crcInt = CRC.checksum(mainBytes);
        byte crcBytes[] = new byte[3];
        crcBytes[0] = (byte)(crcInt >> 16);
        crcBytes[1] = (byte)(crcInt >>  8);
        crcBytes[2] = (byte)(crcInt     );
        String crcEncodedString = base64.encode(crcBytes);

        return mainEncodedString + "=" + crcEncodedString;
    }

    public static PacketByteArray unarmour(String s) throws FormatException
    {
        int crcPos = s.indexOf("\r\n=");

        String mainEncodedString = s.substring(0, crcPos);
        byte[] mainBytes = base64.decode(mainEncodedString);

        String crcEncodedString = s.substring(crcPos + 3, crcPos + 7);
        byte[] crcBytes = base64.decode(crcEncodedString);

        int crcInt = ((crcBytes[0] & 0xFF) << 16)
                   | ((crcBytes[1] & 0xFF) <<  8)
                   | ((crcBytes[2] & 0xFF)     );

        int calculatedCRC = CRC.checksum(mainBytes);
        if (crcInt != calculatedCRC)
        {
            throw new FormatException("Armoured Data has incorrect CRC");
        }

        return new PacketByteArray(mainBytes);
    }


    // Methods to read and write the properties

    private String propertiesToString()
    {
        StringBuffer s = new StringBuffer(256);
        Enumeration e = properties.propertyNames();
        while (e.hasMoreElements())
        {
            String name  = (String) e.nextElement();
            if (name.length() > 0)
            {
                String value = properties.getProperty(name);
                s.append(name);
                s.append(": ");

                int vPos = s.length();
                s.append(value);
                s.setCharAt(vPos, Character.toUpperCase(value.charAt(0)));
                s.append("\r\n");
            }
        }
        s.append("\r\n");
        return new String(s);
    }

    private void stringToProperties(String s)
    throws FormatException
    {
        StringTokenizer st = new StringTokenizer(s, "\r\n");
        while (st.hasMoreTokens())
        {
            String line = st.nextToken();
            int pos = line.indexOf(": ");
            if (pos == -1)
            {
                throw new FormatException("Header \"" + line
                    + "\" does not conform to PGP standard \"Name: Value\"");
            }
            String name  = line.substring(0, pos);
            if (name.length() > 0)
            {
                String value = line.substring(pos + 2);
                properties.put(name.toLowerCase(), value);
            }
        }
    }


    /**
     * Unarmour an armoured message.
     */
    public void parse(String armouredMessage) throws FormatException
    {
        int startPos, msgPos, propPos;
        String propString;

        // Undo Quoted Printable format if neccessary.
        while ((armouredMessage.indexOf('=') != -1)
            && quotedPrintable.isProbablyEncoded(armouredMessage))
        {
            armouredMessage = quotedPrintable.decode(armouredMessage).toString();
        }

        if ((startPos = armouredMessage.indexOf("-----BEGIN PGP SIGNED MESSAGE-----\r\n")) != -1)
        {
            // Get clear text section
            startPos += 38;
            int sigPos = armouredMessage.indexOf("\r\n-----BEGIN PGP SIGNATURE-----\r\n", startPos);
            String text = armouredMessage.substring(startPos, sigPos);

            text = DashProtected.decode(text);
            clearText = LegacyString.toByteArray(text);
            propPos = sigPos + 33;

        }
        else if ((startPos = armouredMessage.indexOf("-----BEGIN PGP MESSAGE-----\r\n")) != -1)
        {
            propPos = startPos + 29;
        }
        else
        {
            throw new FormatException("Armoured Data without recognised -----BEGIN");
        }

        // Now deal with properties
        msgPos = armouredMessage.indexOf("\r\n\r\n", propPos);
        propString = armouredMessage.substring(propPos, msgPos);
        stringToProperties(propString);

        // Get signature
        payload = unarmour(armouredMessage.substring(msgPos + 4));
    }

    /**
     * Armour an unarmoured message.
     */
    public String toString()
    {
        StringBuffer sb = new StringBuffer(8192);
        if (clearText != null)
        {
            String text = LegacyString.toString(clearText);
            text = DashProtected.encode(text);

            sb.append("-----BEGIN PGP SIGNED MESSAGE-----\r\n\r\n");
            sb.append(text);
            sb.append("-----BEGIN PGP SIGNATURE-----\r\n");
            sb.append(propertiesToString());
            sb.append(armour(payload));
            sb.append("-----END PGP SIGNATURE-----\r\n");
        }
        else
        {
            sb.append("-----BEGIN PGP MESSAGE-----\r\n");
            sb.append(propertiesToString());
            sb.append(armour(payload));
            sb.append("-----END PGP MESSAGE-----\r\n");
        }
        return new String(sb);
    }
}
