// $Id: TestException.java,v 1.2 1997/12/16 08:52:08 iang Exp $
//
// $Log: TestException.java,v $
// Revision 1.2  1997/12/16 08:52:08  iang
// added exit code 3 - no test written yet
//
// Revision 1.1  1997/12/15 02:40:54  hopwood
// + Committed changes below.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.test;

/**
 * This exception is thrown when a test fails. If the test is running directly from
 * the command line, the <code>getErrorCode()</code> method returns the error code
 * that should be passed to <code>System.exit</code>.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  David Hopwood
 * @since   Cryptix 3.0.2
 */
class TestException extends Exception {

    static final int COMPLETE_FAILURE = 1;
    static final int ILLEGAL_ARGUMENTS = 2;
    static final int NO_TESTS_AVAILABLE = 3;      // some tests not written yet
    static final int PARTIAL_FAILURE = 4;

    private int errorcode;

    /** Returns the error code that should be passed to <code>System.exit</code>. */
    public int getErrorCode() { return errorcode; }

    /**
     * Constructs a TestException with the specified detail message and error code.
     *
     * @param  reason   the reason why the exception was thrown.
     * @param  code     the error code.
     */
    TestException(String reason, int code) {
        super(reason);
        errorcode = code;
    }
}
