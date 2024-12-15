// $Id: TestRIPEMD160.java,v 1.4 1998/01/05 03:43:29 iang Exp $
//
// $Log: TestRIPEMD160.java,v $
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
// + Original version.
//
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */
package cryptix.test;

import cryptix.provider.md.RIPEMD160;
import cryptix.util.core.Hex;

/**
 * Tests the output of the RIPEMD-160 message digest algorithm implementation
 * against certified pre-computed output for a given set of reference input.
 * <p>
 * Note there are nine sets of conformance data for RIPEMD* listed
 * in the Table of the Reference below.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Hans Dobbertin, Antoon Bosselaers, and Bart Preneel,
 *        "<a href="http://www.esat.kuleuven.ac.be/~bosselae/ripemd160.html">
 *        The hash function RIPEMD-160</a>",
 * </ol>
 * <p><b>$Revision: 1.4 $</b>
 * @author  Raif S. Naffah
 */
public final class TestRIPEMD160
{
    public static final void main (String[] args) { TestRIPEMD160 x = new TestRIPEMD160(); }


    public TestRIPEMD160 () {
        System.out.println("*** RIPEMD-160:\n");
        String[][] data = {
            //    data, md
            {"",
                "9C1185A5C5E9FC54612808977EE8F548B2258D31"},
            {"a",
                "0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE"},
            {"abc",
                "8EB208F7E05D987A9B044A8E98C6B087F15A0BFC"},
            {"message digest",
                "5D0689EF49D2FAE572B881B123A85FFA21595F36"},
            {"abcdefghijklmnopqrstuvwxyz",
                "F71C27109C692C1B56BBDCEB5B9D2865B3708DBC"},
            {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                "12A053384A9C0C88E405A06C27DCF49ADA62EB2B"},
            {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
                "B0E20B6E3116640286ED3A87A5713079B21F5189"},
            {"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
                "9B752E45573D4B39F4DBD3323CAB82BF63326BFB"},
            {"aaaaaaaaa...a (1 million times)",
                "52783243C1697BDBE16D37F97F68F08325DC1528"}};
        byte[] x;
        String a;
        try {
            RIPEMD160 alg = new RIPEMD160();
            for (int i = 0; i < data.length; i++) {
                if (i != 8)
                    x = alg.digest(data[i][0].getBytes());
                else {
                    // test for the million times 'a' requires special handling
                    // in the process test engineUpdate(byte) method implementation
                    for (int j = 0; j < 1000; j++)
                        for (int k = 0; k < 1000; k++)
                            alg.update((byte)'a');
                    x = alg.digest();
                }
                a = Hex.toString(x);
                System.out.println("Data: '" + data[i][0] + "'");
                System.out.println("  computed md: " + a);
                System.out.println(" certified md: " + data[i][1]);
                System.out.print(" ********* RIPEMD-160 ");
                System.out.print(a.equals(data[i][1]) ? "GOOD" : "FAILED");
                System.out.println(" *********\n...");
            }
        } catch (Throwable t) { t.printStackTrace(); }
    }
}
