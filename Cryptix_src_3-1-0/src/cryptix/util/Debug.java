// $Id: Debug.java,v 1.2 1997/11/20 22:27:04 hopwood Exp $
//
// $Log: Debug.java,v $
// Revision 1.2  1997/11/20 22:27:04  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.1  1997/06/21  David Hopwood
// + Deprecated this class (all methods have equivalents in
//   cryptix.util.core.Hex).
//
// Revision 0.1.0.0  1997/06/21  Original author not stated
// + Start of history (Cryptix 2.2).
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util;

import java.io.PrintStream;
import cryptix.util.core.Hex;

/**
 * All the methods of this class have equivalents that use methods from
 * cryptix.util.core.Hex.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  David Hopwood
 * @since   Cryptix 2.2
 * @deprecated
 * @see cryptix.util.core.Hex
 */
public class Debug
{
    private Debug() {} // static methods only

    /**
     * @deprecated Use <code>System.err.print(<a href="cryptix.util.core.Hex.html#dumpString(byte[], int, int)">cryptix.util.core.Hex.dumpString</a>(buf, 0, len))</code> instead.
     */
    public static void println(byte[] buf, int len) {
        System.err.print(Hex.dumpString(buf, 0, len));
    }

    /**
     * @deprecated Use <code>System.err.print(<a href="cryptix.util.core.Hex.html#dumpString(byte[], int, int)">cryptix.util.core.Hex.dumpString</a>(buf, ptr, len))</code> instead.
     */
    public static void println(byte[] buf, int len, int ptr) {
        System.err.print(Hex.dumpString(buf, ptr, len));
    }

    /**
     * @deprecated Use <code>out.print(<a href="cryptix.util.core.Hex.html#dumpString(byte[], int, int)">cryptix.util.core.Hex.dumpString</a>(buf, ptr, len))</code> instead.
     */
    public static void println(PrintStream out, byte[] buf, int len, int ptr)
    {
        out.print(Hex.dumpString(buf, ptr, len));
    }

    /**
     * @deprecated Use <code>System.err.print(<a href="cryptix.util.core.Hex.html#dumpString(byte[])">cryptix.util.core.Hex.dumpString</a>(buf))</code> instead.
     */
    public static void print(byte[] buf)
    {
        System.err.print(Hex.dumpString(buf));
    }
}
