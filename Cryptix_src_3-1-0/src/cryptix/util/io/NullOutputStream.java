// This file is currently unlocked (change this line if you lock the file)
//
// $Log: NullOutputStream.java,v $
// Revision 1.1.1.1  1997/11/20 21:06:50  hopwood
// + Moved these classes here from cryptix.io.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:56  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.3.1  1997/08/02  David Hopwood
// + Added documentation.
//
// Revision 0.2.3.0  1997/04/16  Jill Baker
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995-1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util.io;

import java.io.OutputStream;
import java.io.IOException;

/**
 * A null OutputStream, that ignores any data sent to it.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  Jill Baker
 * @since   Cryptix 2.2
 */
public class NullOutputStream extends OutputStream
{
    /**
     * Creates a NullOutputStream, that ignores its output.
     */
    public NullOutputStream() {}

    public void close() throws IOException {}

    public void flush() throws IOException {}

    public void write(byte[] data) throws IOException {}

    public void write(byte[] data, int n, int m) throws IOException {}

    public void write(int ch) throws IOException {}
}
