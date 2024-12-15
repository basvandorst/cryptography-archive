// $Id: TestAll.java,v 1.6 1998/01/12 04:10:39 hopwood Exp $
//
// $Log: TestAll.java,v $
// Revision 1.6  1998/01/12 04:10:39  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// Revision 1.5  1997/12/07 06:42:21  hopwood
// + Added TestElGamal, various other changes.
//
// Revision 1.4  1997/11/29 17:47:08  hopwood
// + Minor changes to test API.
// + Added better support for BaseTest subclasses in TestAll (doesn't work
//   yet).
//
// Revision 1.3  1997/11/29 11:51:42  raif
// *** empty log message ***
//
// 1997.11.29: RSN
// + Changed the call statement to invoke a constructor with
//   no argument. Now works with some Test* classes which, in
//   their turn invoke local test*() methods from their constructor;
//   ie. TestIDEA, TestSAFER, etc...
//
// Revision 1.2  1997/11/29 05:12:22  hopwood
// + Changes to use new test API (BaseTest).
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.0  1997/09/14  David Hopwood
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.test;

/**
 * Class for running all tests.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.6 $</b>
 * @author  David Hopwood
 * @since   Cryptix 2.2.2
 */
class TestAll
extends BaseTest {

// Constants
//................................................................................

    private static String[] tests = {
        "IJCE",
        "3LFSR", "Base64Stream", "BR",
        "RSA", "ElGamal", "RPK",
        "Blowfish", "CAST5", "DES", "DES_EDE3", "IDEA",
          "LOKI91", "RC2", "RC4", "SAFER", "SPEED", "Square",
        "HAVAL", "MD2", "MD4", "MD5", "RIPEMD128", "RIPEMD160", "SHA0", "SHA1",
    };


// Test methods
//................................................................................

    public static void main(String[] args) {
        new TestAll().commandline(args);
    }

    protected void engineTest() throws Exception {
        Class[] args = {};
        Object[] initargs = {};
        for (int i = 0; i < tests.length; i++) {
            out.println("---------------------------------------------------------------------------");
            String classname = "cryptix.test.Test" + tests[i];
            try {
                Object obj = Class.forName(classname).getConstructor(args)
                    .newInstance(initargs);
                if (obj instanceof BaseTest) {
                    BaseTest test = (BaseTest) obj;
                    test.setOutput(out);
                    test.test();
                    passes += test.passes;
                    failures += test.failures;
                }
            }
            catch (Exception e) {
                fail(">>> Exception (" + classname + ")");
                e.printStackTrace();
            }
        }
    }
}
