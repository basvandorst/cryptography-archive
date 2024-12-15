// $Id: MD5.java,v 1.4 1997/12/19 16:56:31 iang Exp $
//
// $Log: MD5.java,v $
// Revision 1.4  1997/12/19 16:56:31  iang
// added exit + exception for dist scripts
//
// Revision 1.3  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.2.1  1997/11/29  hopwood
// + Extend MessageDigest instead of BlockMessageDigest.
// + This class now supports only the old Cryptix 2.2 API, and is deprecated.
// + Added references in documentation.
//
// Revision 1.2  1997/11/21 05:30:19  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.security;

import java.io.PrintWriter;

import cryptix.util.core.Hex;
import cryptix.mime.LegacyString;

/**
 * This class implements the Cryptix V2.2 API for the the MD5 message digest
 * algorithm. New code should use the JCA implementation of MD5, which can be
 * obtained by calling <code>MessageDigest.getInstance("MD5")</code>, for example.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Bruce Schneier,
 *        "Section 18.5 MD5,"
 *        <cite>Applied Cryptography, 2nd edition</cite>,
 *        John Wiley &amp; Sons, 1996
 *        <p>
 *   <li> RFC 1321 [need complete reference]
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.4 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 */
public class MD5
extends MessageDigest
implements Cloneable
{

// MD5 constants and variables
//...........................................................................

    /** Length of the final hash (in bytes). */
    public static final int HASH_LENGTH = 16;

    /** Length of a block (the number of bytes hashed in every transform). */
    public static final int DATA_LENGTH = 64;


// Constructor
//............................................................................

    /**
     * This creates an MD5 message digest instance.
     */
    public MD5() {
        super(getDigestImpl());
    }

    private static java.security.MessageDigest getDigestImpl() {
        try { return java.security.MessageDigest.getInstance("MD5", "Cryptix"); }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }


// Legacy methods
//...........................................................................

    /**
     * Returns the MD5 hash of a single string.
     *
     * @param str the string to hash.
     * @deprecated Use <code>MessageDigest.getInstance("MD5", "Cryptix").<a href="java.security.MessageDigest.html#digest(byte[])">digest</a>(cryptix.mime.LegacyString.toByteArray(str))</code> instead.
     */
    public static byte[] hash(String str) {
        try {
            return java.security.MessageDigest.getInstance("MD5",
                "Cryptix").digest(LegacyString.toByteArray(str));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the MD5 hash of a single byte array.
     *
     * @param msg the byte array to hash.
     * @deprecated Use <code>MessageDigest.getInstance("MD5", "Cryptix").<a href="java.security.MessageDigest.html#digest(byte[])">digest</a>(data)</code> instead.
     */
    public static byte[] hash(byte[] data) {
        try {
            return java.security.MessageDigest.getInstance("MD5",
                "Cryptix").digest(data);
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the MessageHash of a single string.
     *
     * @param msg the string to hash.
     * @deprecated Use <code><a href="cryptix.security.MessageHash.html#fromDigest(java.security.MessageDigest, byte[])">MessageHash.fromDigest</a>(MessageDigest.getInstance("MD5", "Cryptix"),
     *          cryptix.mime.LegacyString.toByteArray(str))</code> instead.
     */
    public static HashMD5 hashAsMessageHash(String str) {
        try {
            return new HashMD5(java.security.MessageDigest.getInstance("MD5",
                "Cryptix").digest(LegacyString.toByteArray(str)));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the MessageHash of a single byte array.
     *
     * @param data the byte array to hash.
     * @deprecated Use <code><a href="cryptix.security.MessageHash.html#fromDigest(java.security.MessageDigest, byte[])">MessageHash.fromDigest</a>(MessageDigest.getInstance("MD5", "Cryptix"),
     *          data)</code> instead.
     */
    public static HashMD5 hashAsMessageHash(byte[] data) {
        try {
            return new HashMD5(java.security.MessageDigest.getInstance("MD5",
                "Cryptix").digest(data));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns a HashMD5 object whose value as a byte array is <i>hash</i>.
     *
     * @deprecated Use <code>new <a href="cryptix.security.MessageHash.html#MessageHash">MessageHash</a>("MD5", hash)</code> instead.
     */
    public static HashMD5 CreateHash(byte[] hash) {
        return new HashMD5(hash);
    }


// Test methods
//...........................................................................
//
// Don't expand this code please without thinking about it,
// much better to write a separate program.
//

    public static void main(String[] args)
    {
        try { self_test(new PrintWriter(System.out, true)); }
        catch (Exception e) { e.printStackTrace(); System.exit(1); }
    }

    public static void self_test(PrintWriter out) throws Exception
    {
        test(out, "",    "D41D8CD98F00B204E9800998ECF8427E");
        test(out, "a",   "0CC175B9C0F1B6A831C399E269772661");
        test(out, "aa",  "4124BC0A9335C27F086F24BA207A4912");
        test(out, "aaa", "47BCE5C74F589F4867DBD57E9CA9F808");
        test(out, "bbb", "08F8E0260C64418510CEFB2B06EEE5CD");
        test(out, "ccc", "9dF62E693988EB4E1E1444ECE0578579");
        test(out, "abc", "900150983CD24FB0D6963F7D28E17F72");
        test(out, "abcdefg", "7AC66C0F148DE9519B8BD264312C4D64");
        test(out, "abcdefghijk", "92B9CCCC0B98C3A0B8D0DF25A421C0E3");
        test(out, "message digest", "F96B697D7CB7938D525A2F31AAF161D0");
        test(out, "abcdefghijklmnopqrstuvwxyz", "C3FCD3D76192E4007DFB496CCA67E13B");
        test(out, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
                   "D174AB98D277D9F5A5611C2C9F419D9F"
        );
        test(out, "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
                   "57EDF4A22BE3C955AC49DA2E2107B67A"
        );
    }

    private static void
    test(PrintWriter out, String msg, String hashStr) throws Exception
    {
        java.security.MessageDigest digest =
            java.security.MessageDigest.getInstance("MD5", "Cryptix");
        digest.update(LegacyString.toByteArray(msg));
        String x = Hex.toString(digest.digest());
        out.println("Message:   \"" + msg + "\"");
        out.println("calc hash: " + x);
        out.println("real hash: " + hashStr);
        if (hashStr.equalsIgnoreCase(x))
            out.println("Good");
        else {
            out.println("************ MD5 FAILED ************");
	    throw new Exception("MD5 (V2.2) Failed test <" + msg + ">");
	}
    }
}
