// This file is currently unlocked (change this line if you lock the file)
//
// $Log: HashSHA.java,v $
// Revision 1.4  1998/01/11 20:22:02  iang
// Hash???: added exit(1) on error, added test to SHA0.
// ???: added exit(1).
// MessageHash: needs to return digest name as SHA/SHA0/MD5, not the
// names listed as aliases in Cj3.0.   Added a hack to check and change.
//
// Revision 1.3  1997/11/28 03:11:24  iang
// added a main as tests to the algorithm classes
// changed glue code to getInstance()
//
// Revision 1.2  1997/11/21 05:30:19  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/07/26  David Hopwood
// + Deprecated this class.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 */

package cryptix.security;

import cryptix.CryptixException;

/**
 * This class represents the output of a SHA message digest.
 *
 * <p>Copyright (c) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * <p><b>$Revision: 1.4 $</b>
 * @deprecated Use MessageHash instead.
 * @see MessageHash
 */
public final class HashSHA extends MessageHash {
    /**
     * Constructs a HashSHA from an SHA message digest.
     *
     * @param md    a SHA MessageDigest.
     * @deprecated Use <code>MessageHash.fromDigest(md)</code>.
     */
    public HashSHA(SHA md) {
        super("SHA-1", md.digest());
    }

    /**
     * Constructs a HashSHA from a byte array that must be the the
     * correct length.
     *
     * @param hash  a byte array representing a SHA-1 hash.
     * @deprecated Use <code>new MessageHash("SHA-1", hash)</code> (note that
     *          this does not check the hash length).
     */
    public HashSHA(byte[] hash) {
        super("SHA-1", checkLength(hash));
    }

    private static byte[] checkLength(byte[] hash) {
        if (hash.length != SHA.HASH_LENGTH)
            throw new CryptixException("SHA hash is wrong length");
        return hash;
    }
///////////////////////////////////////// TEST ////////////////


    /**
     * Entry point for self test.
     */
    public static void
    main(String argv[])
        throws java.io.IOException
    {
	String str;
        HashSHA hash = SHA.hashAsMessageHash("abc");
	str = hash.toString();
        System.out.println("Hash is <<<" + hash.toString() + ">>>");
	if (!str.equals("SHA:A9993E364706816ABA3E25717850C26C9CD0D89D"))
	{
	    System.out.println("not App A"); 
	    System.exit(1);
	}
    }

}
