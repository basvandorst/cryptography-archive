// This file is currently unlocked (change this line if you lock the file)
//
// $Log: HashSHA0.java,v $
// Revision 1.3  1998/01/11 20:22:02  iang
// Hash???: added exit(1) on error, added test to SHA0.
// ???: added exit(1).
// MessageHash: needs to return digest name as SHA/SHA0/MD5, not the
// names listed as aliases in Cj3.0.   Added a hack to check and change.
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
 * <p><b>$Revision: 1.3 $</b>
 * @deprecated Use MessageHash instead.
 * @see MessageHash
 */
public final class HashSHA0 extends MessageHash {
    /**
     * Constructs a HashSHA0 from an SHA0 message digest.
     *
     * @param md    a SHA0 MessageDigest.
     * @deprecated Use <code>MessageHash.fromDigest(md)</code>.
     */
    public HashSHA0(SHA0 md) {
        super("SHA-0", md.digest());
    }

    /**
     * Constructs a HashSHA0 from a byte array that must be the the
     * correct length.
     *
     * @param hash  a byte array representing a SHA-0 hash.
     * @deprecated Use <code>new MessageHash("SHA-0", hash)</code> (note that
     *          this does not check the hash length).
     */
    public HashSHA0(byte[] hash) {
        super("SHA-0", checkLength(hash));
    }

    private static byte[] checkLength(byte[] hash) {
        if (hash.length != SHA0.HASH_LENGTH)
            throw new CryptixException("SHA0 hash is wrong length");
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
        HashSHA0 hash = SHA0.hashAsMessageHash("abc");
	str = hash.toString();
        System.out.println("Hash is <<<" + hash.toString() + ">>>");
	if (!str.equals("SHA0:0164B8A914CD2A5E74C4F7FF082C4D97F1EDF880"))
	{
	    System.out.println("not App A"); 
	    System.exit(1);
	}
    }


}
