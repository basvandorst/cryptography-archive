// $Id: ByteArrayFactory.java,v 1.3 1997/11/20 22:27:05 hopwood Exp $
//
// $Log: ByteArrayFactory.java,v $
// Revision 1.3  1997/11/20 22:27:05  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.2  1997/11/04 19:33:31  raif
// *** empty log message ***
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.2  1997/08/27  David Hopwood
// + Use new debugging API.
//
// Revision 0.1.0.1  1997/06/21  David Hopwood
// + Added DEBUG flag.
// + Deprecated string conversion methods that now have equivalents in
//   cryptix.util.core.Hex.
//
// Revision 0.1.0.0  1997/06/21  Original author not stated
// + Start of history (Cryptix 2.2)
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util;

import cryptix.util.core.Debug;
import cryptix.util.core.Hex;

import java.io.PrintWriter;
import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

/**
 * Static methods for reading and writing message byte arrays.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 */
public class ByteArrayFactory
{
    private ByteArrayFactory() {} // static methods only

// Debugging methods and vars.
//...........................................................................

    private static final boolean DEBUG = Debug.GLOBAL_DEBUG;
    private static final int debuglevel = DEBUG ? Debug.getLevel("ByteArrayFactory") : 0;
    private static final PrintWriter err = DEBUG ? Debug.getOutput() : null;
    private static void debug(String s) { err.println("ByteArrayFactory: " + s); }


// Own methods
//...........................................................................

    public static byte[] load(byte[] buf)
    throws IOException
    {
        return load(buf, 0);
    }

    public static byte[] load(byte[] buf, int offset)
    throws IOException
    {
        DataInputStream in = new DataInputStream(new ByteArrayInputStream(buf));
        if (offset > 0)
            in.skip(offset);
        return read(in);
    }

    public static byte[] save(byte[] data)
    throws IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        write(new DataOutputStream(out), data);
        return out.toByteArray();
    }
    
    private static final int SIGN_MASK = 0xFF;
    private static final int LENGTH_MASK = 0x7F;
    private static final int CONTINUE_MASK = 0x80;
    private static final int SHIFT_SIZE = 7;
    private static final int MAX_SHIFT = 3;
    
    public static byte[] read(DataInput in)
    throws IOException
    {
        int data = in.readByte() & SIGN_MASK;
        if (DEBUG && debuglevel >= 4) debug("Data read:  0x" + Hex.byteToString(data));
        int length = data & LENGTH_MASK;
        if (DEBUG && debuglevel >= 4) debug("new length: " + length);
        while((data & CONTINUE_MASK) != 0)
        {
            data = in.readByte() & SIGN_MASK;
            if (DEBUG && debuglevel >= 4) debug("Data read:  0x" + Hex.byteToString(data));
            length <<= SHIFT_SIZE;
            length |= data & LENGTH_MASK;
            if (DEBUG && debuglevel >= 4) debug("new length: " + length);
        }
        if (DEBUG && debuglevel >= 3) debug("length: " + length);
        byte[] buffer = new byte[length];
        if (length > 0)
            in.readFully(buffer);
        return buffer;
    }

    public static void write(DataOutput out, ByteArray data)
    throws IOException
    {
        write(out, data.toByteArray());
    }

    public static void write(DataOutput out, byte[] data)
    throws IOException
    {
        int length = data.length;
if (DEBUG && debuglevel >= 3) debug("length: " + length);
        int shift = MAX_SHIFT;
        int mask = LENGTH_MASK << (SHIFT_SIZE * shift);
        while (((length & mask) == 0) && (shift > 0))
            mask = LENGTH_MASK << (SHIFT_SIZE * --shift);

        while(shift > 0)
        {
            out.write(((length & mask) >>> (SHIFT_SIZE * shift)) | CONTINUE_MASK);
            mask = LENGTH_MASK << (SHIFT_SIZE * --shift);
        }
        out.write(length & LENGTH_MASK);
        if (length > 0)
            out.write(data);
    }

    /**
     * @deprecated Use <code><a href="cryptix.util.core.Hex.html#toString(byte[])">cryptix.util.core.Hex.toString</a>(buf)</code> instead.
     */
    public static String toHexString(byte[] buf)
    {
        return Hex.toString(buf);
    }

    /**
     * @deprecated Use <code><a href="cryptix.util.core.Hex.html#toString(byte[], int, int)">cryptix.util.core.Hex.toString</a>(buf, offset, length)</code> instead.
     */
    public static String toHexString(byte[] buf, int offset, int length)
    {
        return Hex.toString(buf, offset, length);
    }

    /**
     * @deprecated Use <code><a href="cryptix.util.core.Hex.html#dumpString(byte[])">cryptix.util.core.Hex.dumpString</a>(buf)</code> instead.
     */
    public static String toHexdumpString(byte[] buf)
    {
        return Hex.dumpString(buf);
    }
    
    /**
     * @deprecated Use <code><a href="cryptix.util.core.Hex.html#dumpString(byte[], int, int)">cryptix.util.core.Hex.dumpString</a>(buf, offset, length)</code> instead.
     */
    public static String toHexdumpString(byte[] buf, int offset, int length)
    {
        return Hex.dumpString(buf, offset, length);
    }

    /**
     * @deprecated Use <code><a href="cryptix.util.core.Hex.html#fromString(java.lang.String)">cryptix.util.core.Hex.fromString</a>(str)</code> instead.
     */
    public static byte[] fromHexString(String str)
    {
        return Hex.fromString(str);
    }
}
