// $Id: BaseTest.java,v 1.8 1998/01/13 13:51:05 iang Exp $
//
// $Log: BaseTest.java,v $
// Revision 1.8  1998/01/13 13:51:05  iang
// Made expectedPasses==1 so that pre-initialisation exceptions are caught
// and given exit(1).  Problem arose with TestRSA doing preparation.
// Also commented out the noisy bits of the not-yet-written
// debug-ON cycle.  Should really be protected with a property (?) but the
// 'finally' part made that non-trivial as that did the exceptions.
//
// Revision 1.7  1998/01/12 04:10:39  hopwood
// + Made engineTest() protected.
// + Cosmetics.
//
// Revision 1.6.1  1998/01/12  hopwood
// + Made all methods protected. The reflection hack added in 1.5.1 is no
//   longer needed.
//
// Revision 1.6  1997/12/22 15:07:47  raif
// *** empty log message ***
//
// Revision 1.5.1  1997/12/23  raif
// + Use reflection API to invoke engineTest. Doing so
//   eliminates the need to modify existing code.
// + other changes to allow use by Maker.
//
// Revision 1.5  1997/12/21 19:09:05  iang
// commented out the changes to passes variables, exit codes always fail=1
//
// Revision 1.4  1997/12/15 02:40:54  hopwood
// + Committed changes below.
//
// Revision 1.3.1  1997/12/15  hopwood
// + Use IanG's conventions for return codes.
//
// Revision 1.3  1997/11/29 17:47:08  hopwood
// + Minor changes to test API.
// + Added better support for BaseTest subclasses in TestAll (doesn't work
//   yet).
//
// Revision 1.2  1997/11/29 05:12:22  hopwood
// + Changes to use new test API (BaseTest).
//
// Revision 1.1.1.1.2  1997/11/23  hopwood
// + Fleshed out the API a little more, added 'expectedPasses'.
//
// Revision 1.1.1.1.1  1997/11/21  hopwood
// + Added the beginnings of a test API (fail, pass, done, engineTest etc.)
//   At the moment these methods are all package-private, in case some tests
//   are insecure.
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

import java.io.PrintWriter;

/**
 * This abstract class acts as a base for all Cryptix test classes. Its purpose
 * is to provide a framework that will allow us to easily add new testing features
 * (for example, repeating tests that have failed at a higher debugging level, or
 * testing as an applet).
 * <p>
 * Test classes should be non-public wherever possible, in order to reduce the risk
 * of security bugs in test code being exploited.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.8 $</b>
 * @author  David Hopwood
 * @since   Cryptix 2.2.2
 */
public abstract class BaseTest
{
    /** The PrintWriter to which output is to be sent. */
    protected PrintWriter out;

    protected int failures;
    protected int passes;
    protected int expectedPasses;

    protected BaseTest() {}

    protected void commandline(String[] args) {
        out = new PrintWriter(System.out, true);
        try {
            test();
        } catch (TestException e) {
            out.println(e.getMessage());
            System.exit(e.getErrorCode());
        }
    }

    protected void setOutput(PrintWriter output) { out = output; }

    protected void test() throws TestException {
        try {
	    //
	    //  Expect this test to do something ==> set expectedPasses
	    //  to be greater than zero.
	    //  This should catch errors before engineTest() initialisation.
	    //  Probably should be done with a boolean, and trapped explicitly.
	    //
            failures = passes = 0;
            expectedPasses = 1;

            out.println("Start tests...\n");
            engineTest();
            out.println("\nDone tests...");
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            report();

            if (failures > 0 || passes < expectedPasses) {
                try {
// iang  - breaks exits:       failures = passes = expectedPasses = 0;

                    out.println("(Debugging tests not written)");
//                    out.println("\nRepeating with debugging enabled...\n");
//                    out.println("(not implemented)");
//                    engineTest();
//                    out.println("\nDone tests with debugging enabled...");

                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
//                    report();
                    if (passes == 0)
                        throw new TestException(getClass().getName() + " failed completely",
                            TestException.COMPLETE_FAILURE);

                    throw new TestException(getClass().getName() + " failed partially",
                        TestException.PARTIAL_FAILURE);
                }
            }
        }
    }

    protected void fail (String msg) {
        failures++;
        out.println(msg + " - failed.\n");
    }

    protected void pass (String msg) {
        passes++;
        out.println(msg + " - passed.\n");
    }

    protected void passIf (boolean pass, String msg) {
        if (pass)
            pass(msg);
        else
            fail(msg);
    }

    protected void setExpectedPasses (int n) {
        if (n < 0) throw new IllegalArgumentException("n < 0");
        expectedPasses = n;
    }

    protected void report() {
        out.println("===========================================================================");
        out.println("Number of passes:   " + passes);
        out.println("Number of failures: " + failures);
        out.println("Expected passes:    " +
            (expectedPasses > 0 ? Integer.toString(expectedPasses) : "unknown"));
    }

    protected abstract void engineTest() throws Exception;
}
