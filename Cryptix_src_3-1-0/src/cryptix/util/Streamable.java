// $Id: Streamable.java,v 1.2 1997/11/20 22:27:05 hopwood Exp $
//
// $Log: Streamable.java,v $
// Revision 1.2  1997/11/20 22:27:05  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.1  1997/06/21  David Hopwood
// + Cosmetic changes.
//
// Revision 0.1.0.0  1997/06/21  Original author unknown
// + Start of history (Cryptix 2.2).
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * This is a Streamable abstract base class that provides the load and
 * save methods from StreamInterface. Only the read and write methods
 * need defining for use.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 *
 * @see cryptix.util.StreamInterface
 */
public abstract class Streamable implements StreamInterface
{
    private static final int DEFAULT_BUFFER_SIZE = 2048;
    
    public Streamable()
    {
    }

    public Streamable(byte[] buf)
    throws IOException
    {
        load(buf);
    }

    public Streamable(DataInput in)
    throws IOException
    {
        read(in);
    }
    
    public final void load(byte[] buf)
    throws IOException
    {
        read(new DataInputStream(new ByteArrayInputStream(buf)));
    }

    public final byte[] save()
    throws IOException
    {
        ByteArrayOutputStream out = new ByteArrayOutputStream(
            DEFAULT_BUFFER_SIZE);
        write(new DataOutputStream(out));
        return out.toByteArray();
    }
}
