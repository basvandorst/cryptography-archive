/*
// $Log: check_signed.java,v $
// Revision 1.2  1999/06/30 23:06:44  edwin
// Changing package name and fixing references to moved and/or changed classes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.examples.pgp;

import java.util.Hashtable;
import java.io.*;

import cryptix.security.MD5;
import cryptix.security.MessageHash;
import cryptix.security.rsa.PublicKey;
import cryptix.util.mime.Base64;
import cryptix.mime.LegacyString;
import cryptix.pgp.CRC;
import cryptix.pgp.Packet;
import cryptix.pgp.Signature;
import cryptix.pgp.PacketFactory;

/**
 * DOCUMENT ME.
 */
final class check_signed
{
    private final static int WAITING        = 0;
    private final static int DATA_INFO      = 1;
    private final static int DATA_FIRST     = 2;
    private final static int DATA           = 3;
    private final static int SIGNATURE_INFO = 4;
    private final static int SIGNATURE      = 5;
    private final static int SIGNATURE_END  = 6;
    private final static int END            = 7;
    // \r \n 
    private byte[] PGPendOfLineData = { (byte)13, (byte)10 };

    private final String BEGIN_MULTI_LINE = "{";
    private final String END_MULTI_LINE   = "}";
    
    private Signature signature;
    private MessageHash pgpHash;

    private final static Base64 base64 = new Base64();

    public boolean checkSignature(PublicKey publickey)
    {
        if (signature != null)
            return signature.check(publickey, pgpHash);
        return false;
    }

    public byte[] getSignatureKeyId()
    {
        if (signature != null)
            return signature.getKeyId().toByteArray();
        return null;
    }
    
    public void load(InputStream raw_in) throws IOException
    {
        int state = WAITING;
        DataInputStream in = (raw_in instanceof DataInputStream)
                                 ? (DataInputStream) raw_in
                                 : new DataInputStream(raw_in);
        Hashtable h = null, data = new Hashtable();
        MD5 md = new MD5();
        String s, end = null, item = null;
        boolean firsttime = false, multiline = false;
        StringBuffer sb = null;
        String sig_buf = "";

        data:
        while ((s = in.readLine()) != null)
        {
            switch(state) 
            { 
            case WAITING: 
                if (s.startsWith("-----BEGIN ")) 
                { 
                    if (s.startsWith("-----BEGIN PGP SIGNED MESSAGE"))
                    {
                        state = DATA_INFO;
                        continue data;
                    }
                    throw new IOException("Bad begin : " + s); 
                }
                break; 
            case DATA_INFO: 
                if (s.length() == 0)
                    state = DATA_FIRST;
                break; 
            case DATA_FIRST: 
            case DATA:
                if (s.startsWith("-----END ")) 
                    throw new IOException("Bad end : " + s); 
                if (s.startsWith("-----BEGIN ")) 
                { 
                    if (s.startsWith("-----BEGIN PGP SIGNATURE"))
                    {
                        state = SIGNATURE_INFO;
                        continue data;
                    }
                    throw new IOException("unexpected begin : " + s); 
                }
                if ((s.length() > 2) && (s.charAt(0) == '-'))
                    s = s.substring(2); 
                if (state != DATA)
                    state = DATA; // first line.
                else
                    md.add(PGPendOfLineData); 
                md.add(LegacyString.toByteArray(s));
                
                if (!multiline)
                {
                    int i;
                    if ((s = s.trim()).length() == 0)
                        continue;
                    switch (s.charAt(0))
                    {
                    case '[':
                        if ((i = s.indexOf("]")) == - 1)
                            throw new IOException("Invalid section definition " + s);
                        if (data.put(s.substring(1, i) , h = new Hashtable()) != null)
                            throw new IOException("Section " + s + " defined twice");
                        continue data;
                    case ';': // comments
                    case '#':
                        continue data;
                    default:
                        if (h == null)
                            throw new IOException("Missing section definition");
                        if ((i = s.indexOf("=")) == - 1)
                            throw new IOException("Invalid section definition " + s);
                        item  = s.substring(0, i).trim();
                        s = s.substring(i + 1).trim();
                        if (!s.equals("*"))
                        {
                            h.put(item, s);
                            continue data;
                        }
                        firsttime = multiline = true;
                        sb = null;
                        continue data;
                    }
                }
                if (firsttime)
                {
                    if (!s.trim().equals(BEGIN_MULTI_LINE))
                        throw new IOException("Missing Block begin marker " + s);
                    firsttime = false;
                    continue data;
                }
                // multi line
                if (s.trim().equals(END_MULTI_LINE))
                {
                    if (h == null)
                        throw new IOException("Missing section definition");
                    h.put(item, sb.toString());
                        multiline = false;
                }
                if (sb == null)
                    sb = new StringBuffer();
                else
                    sb.append('\n');
                sb.append(s);
                break; 
            case SIGNATURE_INFO: 
                if (s.length() == 0) 
                    state = SIGNATURE; 
                break; 
            case SIGNATURE: 
                if (s.startsWith("=")) 
                {
                    state = SIGNATURE_END;

                    if (s.length() != 5)
                        throw new IOException("checksum length incorrect");

                    //
                    // Verify the checksum
                    // This is a hack - there should be cryptix.pgp.Armoury
                    //
                    byte[] asc_sum = LegacyString.toByteArray(s, 1, 4);
                    byte[] binsum = base64.decode(asc_sum);
                    int sum = (((int) binsum[0] & 0xFF) << 16) |
                              (((int) binsum[1] & 0xFF) <<  8) |
                               ((int) binsum[2] & 0xFF);

                    byte[] buffer = base64.decode(sig_buf);
                    int calc_sum = CRC.checksum(buffer);

                    if (sum != calc_sum)
                        throw new IOException("Incorrect checksum");

                    Packet pkt;
                    if ((pkt = PacketFactory.read(new DataInputStream(new ByteArrayInputStream(buffer)))) == null) 
                        throw new IOException("no signature");
                    if (!(pkt instanceof Signature))
                        throw new IOException("Packet is not a signature");
                    (signature = (Signature)pkt).addExtasToHash(md);
                    pgpHash = md.digestAsHash();
                    state = SIGNATURE_END;
                    break;
                } 
                sig_buf += s;
                break; 
            case SIGNATURE_END: 
                if (s.startsWith("-----END ")) 
                    break data;
                throw new IOException("End excected."  + s); 
            default: 
                throw new IOException("State machine failure."); 
            }
        } 
    }
}
