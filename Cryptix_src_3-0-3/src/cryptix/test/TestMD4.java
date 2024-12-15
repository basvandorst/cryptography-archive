// $Id: TestMD4.java,v 1.4 1998/01/05 03:43:29 iang Exp $
//
// $Log: TestMD4.java,v $
// Revision 1.4  1998/01/05 03:43:29  iang
// updated references to show where the conformance data comes from.
//
// Revision 1.3  1997/11/22 07:05:41  raif
// *** empty log message ***
//
// Revision 1.2  1997/11/22 05:59:03  iang
// core.util ==> util.core
//
// Revision 1.1  1997/11/07 05:53:26  raif
// *** empty log message ***
//
// Revision 1.2  1997/10/28 00:09:10  raif
// *** empty log message ***
//
// Revision 1.1  1997/10/27 22:50:17  raif
// + use new cryptix.util classes.
// + more compact.
//
// 1.0.0           June 1997     R. Naffah
// + original version.
//
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */
package cryptix.test;

import cryptix.util.core.Hex;

import java.security.MessageDigest;

/**
 * Tests the output of the MD4 message digest algorithm implementation
 * against certified pre-computed output for a given set of reference input.
 * <p>
 * Note there are seven sets of conformance data for MD4 listed
 * in Appendix A.5 of the RFC, as indicated.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Ronald L. Rivest,
 *        "<a href="http://www.roxen.com/rfc/rfc1320.html">
 *        The MD4 Message-Digest Algorithm</a>",
 *        IETF RFC-1320 (informational).
 * </ol>
 *
 * <p><b>$Revision: 1.4 $</b>
 * @author  Raif S. Naffah
 */
public final class TestMD4
{
    public static final void main (String[] args) { TestMD4 x = new TestMD4(); }

    
    private TestMD4 () {
        String[][] data = {
            //    data, md
            //  ......................
            {"",               "31D6CFE0D16AE931B73C59D7E0C089C0"}, // A.5 1
            {"a",              "BDE52CB31DE33E46245E05FBDBD6FB24"}, // A.5 2
            {"abc",            "A448017AAF21D8525FC10AE87AA6729D"}, // A.5 3
            {"message digest", "D9130A8164549FE818874806E1C7014B"}, // A.5 4
            {"abcdefghijklmnopqrstuvwxyz",
                               "D79E1C308AA5BBCDEEA8ED63DF412DA9"}, // A.5 5
            {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
                               "043F8582F241DB351CE627E153E7F0E4"}, // A.5 6
            {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
                               "E33B4DDC9C38F2199C3E7B164FCC0536"}  // A.5 7
	};

        System.out.println("*** MD4:\n");
        try {
            MessageDigest alg = MessageDigest.getInstance("MD4", "Cryptix");

            for (int i = 0; i < data.length; i++) {
                String a = Hex.toString(alg.digest(data[i][0].getBytes()));
                System.out.println("  test vector: " + data[i][0]);
                System.out.println("  computed md: " + a);
                System.out.println(" certified md: " + data[i][1]);
                System.out.println(a.equals(data[i][1]) ? " *** MD4 GOOD" : " *** MD4 FAILED");
            }
        } catch (Throwable t) { t.printStackTrace(); }
    }
}