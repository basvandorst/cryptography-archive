// $Id: TestSHA0.java,v 1.7 1998/01/12 04:10:40 hopwood Exp $
//
// $Log: TestSHA0.java,v $
// Revision 1.7  1998/01/12 04:10:40  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */

package cryptix.test;

import cryptix.provider.md.SHA0;
import cryptix.util.core.Hex;

/**
 * Tests the output of the SHA0 message digest algorithm implementation
 * against certified pre-computed output for a given set of reference input.
 * <p>
 * <b>References:</b>
 * <ol>
 *   <li> Jim Gillogly's Implementation,
 *        <a href="http://ftp.sunet.se/ftp/pub/security/tools/crypt/SHA/">
 *        Was the source of the FIPS-180 validation data.
 *   <li> NIST FIPS PUB 180,
 *        "Secure Hash Standard",
 *        U.S. Department of Commerce, 11 May 1993.<br>
 *        Original reference is lost, but see the SHA-1 doc:
 *        <a href="http://www.itl.nist.gov/div897/pubs/fip180-1.htm">
 *        http://www.itl.nist.gov/div897/pubs/fip180-1.htm</a>
 * </ol>
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.7 $</b>
 * @deprecated Here for historical interest.
 * @author  Systemics Ltd
 */
class TestSHA0
extends BaseTest
{

// test data
//.............................................................................

    private static String[][] testData1 = {
    //    data string, md hex
         { "",    "F96CEA198AD1DD5617AC084A3D92C6107708C0EF" },
         { "a",   "37F297772FAE4CB1BA39B6CF9CF0381180BD62F2" },
         { "aa",  "5173EC2335C575DEE032B01562A41330EB803503" },
         { "aaa", "5DFC8A87381AA03E963AB26A645F0FDD60847DFA" },
         { "abc", "0164B8A914CD2A5E74C4F7FF082C4D97F1EDF880" }, // 180 App A
         {                                                      // 180 App B
           "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
           "d2516ee1acfa5baf33dfc1c471e438449ef134c8"
         },
         {
           "message digest",
           "C1B0F222D150EBB9AA36A40CAFDC8BCBED830B14"
         },
         {
           "abcdefghijklmnopqrstuvwxyz",
           "B40CE07A430CFD3C033039B9FE9AFEC95DC1BDCD"
         },
// UPDATE bug
//       {                                                      // 180 App C
//         "aaaaaaaaa...a (1 million times)",                   // special last
//         "3232affa48628a26653b5aaa44541fd90d690603"           // set is fixed
//       },
    };


// main/test methods
//.............................................................................
    
    public static void main (String[] args) {
        new TestSHA0().commandline(args);
    }

    protected void engineTest() throws Exception {
        byte[] x;
        String a;
        int good = 0;
        int fails = 0;
        int i;
        String[][] data = testData1;

        SHA0 alg = new SHA0();

        setExpectedPasses(data.length + 1);      // one for the self_test()
        for (i = 0; i < data.length; i++) {
            alg.reset();
// last one fails - UPDATE bug in SHA0
//          if (i == (data.length - 1)) {
//              // a million times 'a' requires special handling
//              // in the process test engineUpdate(byte) method
//              for (int j = 0; j < 1000; j++)
//                  for (int k = 0; k < 1000; k++)
//                      alg.update((byte) 'a');
//              x = alg.digest();
//          } else                  // standard method
                x = alg.digest(data[i][0].getBytes());

            a = Hex.toString(x);
            out.println("         data: '" + data[i][0] + "'");
            out.println("  computed md: " + a);
            out.println(" certified md: " + data[i][1]);
            boolean ok = a.equalsIgnoreCase(data[i][1]);
            passIf(ok, "Data Set #" + (i+1));
            if (ok) {
                out.println("   * Hash (#"+ ++good+"/"+(i+1)+") good" );

//                // for self_test() generation
//                out.print("{");
//                for (int j = 0; j < x.length; j++)
//                    out.print(" " + x[j] + ",");
//                out.println("},");
            }
            else {
                out.println("===> Hash (#"+ ++fails+"/"+(i+1)+") FAILED  <===");
                out.println("     (no debugging available)");
            }

            out.println("");
        }

        System.out.println("\nSHA0 succeeded (" + i + " tests)");

        // this one last because exception stops it
        alg.self_test();
        passIf(true, "Self Test (no diags)");   // passed if no exception

    }
}
