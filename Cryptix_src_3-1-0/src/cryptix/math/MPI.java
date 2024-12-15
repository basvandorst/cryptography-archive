// $Id: MPI.java,v 1.2 1997/11/20 22:00:57 hopwood Exp $
//
// $Log: MPI.java,v $
// Revision 1.2  1997/11/20 22:00:57  hopwood
// + Deprecated this class.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/06/22  David Hopwood
// + Used private MPI() {} convention.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
// + Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

import java.io.InputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * This class contains static methods used to read and write
 * cryptix.math.BigIntegers in MPI format.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 * @deprecated
 * @see cryptix.util.core.BI
 */
public class MPI
{
    private MPI() {} // static methods only

    public static BigInteger read(DataInput in)
    throws IOException
    {
        int bitlen = in.readShort() & 0xFFFF;
        byte[] buffer = new byte[(bitlen + 7) / 8];
        in.readFully(buffer);
        return new BigInteger(buffer);
    }
    
    public static void write(DataOutput out, BigInteger bigInt)
    throws IOException
    {
        out.writeShort(bigInt.bitLength());
        out.write(bigInt.toByteArray());
    }

    public static BigInteger load(byte[] buf)
    throws IOException
    {
        return load(buf, 0);
    }

    public static BigInteger load(byte[] buf, int offset)
    throws IOException
    {
        InputStream in = new ByteArrayInputStream(buf);
        if (offset > 0)
            in.skip(offset);
        return read(new DataInputStream(in));
    }

    public static byte[] save(BigInteger bigInt)
    throws IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        write(new DataOutputStream(out), bigInt);
        return out.toByteArray();
    }
}
