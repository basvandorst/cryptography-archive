// $Id: SHA0.java,v 1.6 1998/01/11 23:18:12 iang Exp $
//
// $Log: SHA0.java,v $
// Revision 1.6  1998/01/11 23:18:12  iang
// made all the exceptions exit (1)
//
// Revision 1.5  1998/01/11 20:22:03  iang
// Hash???: added exit(1) on error, added test to SHA0.
// ???: added exit(1).
// MessageHash: needs to return digest name as SHA/SHA0/MD5, not the
// names listed as aliases in Cj3.0.   Added a hack to check and change.
//
// Revision 1.4  1997/12/19 05:41:39  hopwood
// + Committed changes below.
//
// Revision 1.3  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.2.1  1997/11/29  hopwood
// + Extend MessageDigest instead of BlockMessageDigest.
// + This class now supports only the old Cryptix 2.2 API, and is deprecated.
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
 * This class implements the Cryptix V2.2 API for the the SHA-0 message digest
 * algorithm (not to be confused with the revised SHA-1 algorithm). SHA-1
 * is preferred for new applications.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> NIST FIPS PUB 180,
 *        "Secure Hash Standard",
 *        U.S. Department of Commerce, May 1993.
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.6 $</b>
 * @author  Systemics Ltd
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 */
public class SHA0
extends MessageDigest
implements Cloneable
{

// SHA-0 constants and variables
//...........................................................................

    /** Length of the final hash (in bytes). */
    public static final int HASH_LENGTH = 20;

    /** Length of a block (the number of bytes hashed in every transform). */
    public static final int DATA_LENGTH = 64;


// Constructor
//............................................................................

    /**
     * This creates an SHA-0 message digest instance.
     */
    public SHA0() {
        super(getDigestImpl());
    }

    private static java.security.MessageDigest getDigestImpl() {
        try { return java.security.MessageDigest.getInstance("SHA-0", "Cryptix"); }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }


// Legacy methods
//...........................................................................

    /**
     * Returns the SHA-0 hash of a single string.
     *
     * @param str the string to hash.
     * @deprecated Use <code>MessageDigest.getInstance("SHA-0", "Cryptix").<a href="java.security.MessageDigest.html#digest(byte[])">digest</a>(cryptix.mime.LegacyString.toByteArray(str))</code> instead.
     */
    public static byte[] hash(String str) {
        try {
            return java.security.MessageDigest.getInstance("SHA-0",
                "Cryptix").digest(LegacyString.toByteArray(str));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the SHA-0 hash of a single byte array.
     *
     * @param msg the byte array to hash.
     * @deprecated Use <code>MessageDigest.getInstance("SHA-0", "Cryptix").<a href="java.security.MessageDigest.html#digest(byte[])">digest</a>(data)</code> instead.
     */
    public static byte[] hash(byte[] data) {
        try {
            return java.security.MessageDigest.getInstance("SHA-0",
                "Cryptix").digest(data);
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the MessageHash of a single string.
     *
     * @param msg the string to hash.
     * @deprecated Use <code><a href="cryptix.security.MessageHash.html#fromDigest(java.security.MessageDigest, byte[])">MessageHash.fromDigest</a>(MessageDigest.getInstance("SHA-0", "Cryptix"),
     *          cryptix.mime.LegacyString.toByteArray(str))</code> instead.
     */
    public static HashSHA0 hashAsMessageHash(String str) {
        try {
            return new HashSHA0(java.security.MessageDigest.getInstance("SHA-0",
                "Cryptix").digest(LegacyString.toByteArray(str)));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns the MessageHash of a single byte array.
     *
     * @param data the byte array to hash.
     * @deprecated Use <code><a href="cryptix.security.MessageHash.html#fromDigest(java.security.MessageDigest, byte[])">MessageHash.fromDigest</a>(MessageDigest.getInstance("SHA-0", "Cryptix"),
     *          data)</code> instead.
     */
    public static HashSHA0 hashAsMessageHash(byte[] data) {
        try {
            return new HashSHA0(java.security.MessageDigest.getInstance("SHA-0",
                "Cryptix").digest(data));
        }
        catch (Exception e) { throw new CryptoError(e.toString()); }
    }

    /**
     * Returns a HashSHA0 object whose value as a byte array is <i>hash</i>.
     *
     * @deprecated Use <code>new <a href="cryptix.security.MessageHash.html#MessageHash">MessageHash</a>("SHA-0", hash)</code> instead.
     */
    public static HashSHA0 CreateHash(byte[] hash) {
        return new HashSHA0(hash);
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
        // These come from cryptix.provider.md.SHA0.
        test(out, "",               "F96CEA198AD1DD5617AC084A3D92C6107708C0EF");
        test(out, "abc",            "0164B8A914CD2A5E74C4F7FF082C4D97F1EDF880");
        test(out, "message digest", "C1B0F222D150EBB9AA36A40CAFDC8BCBED830B14");
    }

    private static void
    test(PrintWriter out, String msg, String hashStr) throws Exception
    {
        java.security.MessageDigest digest =
            java.security.MessageDigest.getInstance("SHA-0", "Cryptix");
        digest.update(LegacyString.toByteArray(msg));
        String x = Hex.toString(digest.digest());
        out.println("Message:   \"" + msg + "\"");
        out.println("calc hash: " + x);
        out.println("real hash: " + hashStr);
        if (hashStr.equalsIgnoreCase(x))
            out.println("Good");
        else {
            out.println("************ SHA-0 FAILED ************");
            throw new Exception("SHA-0 (V2.2) Failed test <" + msg + ">");
        }
    }
}
