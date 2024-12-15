// $Id: ByteArray.java,v 1.2 1997/11/20 22:27:04 hopwood Exp $
//
// $Log: ByteArray.java,v $
// Revision 1.2  1997/11/20 22:27:04  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/06/21  David Hopwood
// + Added more documentation.
// + theSame(...) is public again (for compatibility with Cryptix 2.2),
//   but deprecated.
// + ArrayUtil.areEqual is used to compare array contents.
// + Removed (String) constructor added in 0.2.5.2, because it assumed
//   ISO-Latin-1. In general if an application assumes that strings
//   are ISO-Latin-1, methods of LegacyString should be called by the
//   application, not by Cryptix.
//
// Revision 0.2.5.2  1997/05/11  Jill Baker
// + Changed definition of hashCode() method. It is now a checksum.
// + theSame() is now protected.
// + toString() no longer uses deprecated method.
// + New constructor (String).
// + Deprecated toByteArray() because it had a silly name - now use
//   getData() instead.
//
// Revision 0.2.5.1  1997/02/24  Jill Baker
// + Changed visibility of data[] from private to protected. There is no
//   reason why subclasses should be prohibited from accessing its data.
//
// Revision 0.2.5.0  1997/02/24  Original author not stated
// + Start of history - no details recorded before this time.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util;

import cryptix.util.core.ArrayUtil;
import cryptix.mime.LegacyString;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

/**
 * A class that represents a byte array.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  Jill Baker
 * @since   Cryptix 2.2 or earlier
 */
public class ByteArray
{
    protected byte[] data;

    /**
     * Constructs a ByteArray from data in <i>buf</i>. If <i>copy</i> is true,
     * the data will be copied. Otherwise, <i>buf</i> will be aliased.
     * <p>
     * To construct a ByteArray from a String using ISO-Latin-1 encoding, use
     * <code>new ByteArray(LegacyString.toByteArray(str), false)</code>.
     */
    public ByteArray(byte[] buf, boolean copy)
    {
        if (copy)
        {
            int len = buf.length;
            System.arraycopy(buf, 0, data = new byte[len], 0, len);
        }
        else
            data = buf;
    }

    /**
     * Constructs a ByteArray from a copy of the data in <i>buf</i>.
     */
    public ByteArray(byte[] buf)
    {
        this(buf, true);
    }

    /**
     * Returns the length (in bytes) of this ByteArray's data.
     */
    public int length()
    {
        return data.length;
    }

/*
    public int hashCode()
    {
        switch(data.length)
        {
            case 0:
                return 0;
            case 1:
                return data[0];
            case 2:
                return (data[0] << 8) ^ data[1];
            case 3:
                return (data[0] << 16) ^ (data[1] << 8) ^ data[2];
            default:
                break;
        }
        return (data[0] << 24) ^ (data[1] << 16) ^ (data[2] << 8) ^ data[3];
    }
*/

    public int hashCode()
    {
        int h = 0;
        for (int i=0; i<data.length; i++) h += data[i];
        return h;
    }

    public boolean equals(Object obj)
    {
        if (obj == this)
            return true;
        if (obj instanceof ByteArray)
            return ArrayUtil.areEqual(this.data, ((ByteArray) obj).data);
        return false;
    }
    
    public String toString()
    {
        return LegacyString.toString(data);
    }

    /**
     * Returns a copy of this ByteArray's data.
     */
    public byte[] getData()
    {
        int len = data.length;
        byte[] tmp = new byte[len];
        System.arraycopy(data, 0, tmp, 0, len);
        return tmp;
    }

    /**
     * @deprecated Use <code>getData()</code> instead.
     */
    public byte[] toByteArray()
    {
        return getData();
    }

    /**
     * @deprecated Use <code>ArrayUtil.areEqual(a, b)</code> instead.
     */
    public static boolean theSame(byte[] a, byte[] b) {
        return ArrayUtil.areEqual(a, b);
    }
}
