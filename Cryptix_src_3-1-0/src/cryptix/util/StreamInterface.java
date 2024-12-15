// $Id: StreamInterface.java,v 1.2 1997/11/20 22:27:05 hopwood Exp $
//
// $Log: StreamInterface.java,v $
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
import java.io.IOException;

/**
 * This interface allows objects to be read from or written to DataInput and
 * DataOutput objects. There are also binary array methods to allow other
 * types of storing, or for when it is necessary to know the length of the
 * data.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.2 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 */
public interface StreamInterface
{
    /**
     * Loads 'this' from a saved buffer.
     */
    void load(byte[] buffer) throws IOException;

    /**
     * Saves 'this' into a buffer.
     */
    byte[] save() throws IOException;
    
    /**
     * Reads 'this' from the DataInput object.
     */
    void read(DataInput in) throws IOException;

    /**
     * Writes 'this' to the DataOutput object.
     */
    void write(DataOutput out) throws IOException;
}
