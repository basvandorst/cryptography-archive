// This file is currently unlocked (change this line if you lock the file)
//
// $Log: RandomStream.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

import java.io.InputStream;

/**
 * An abstract class whose subclasses generate potentially random bytes.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public abstract class RandomStream extends InputStream
{
    public int read()
    {
        return nextByte() & 0xFF;
    }

    public int read(byte[] b)
    {
        return read(b, 0, b.length);
    }

    public int read(byte[] b, int off, int len)
    {
        for (int i = off ; i < len ; i++)
            b[i] = nextByte();
        return len - off;
    }

    public long skip(long n)
    {
        for (long i = n; i > 0; i--)
            nextByte();
        return n;
    }

    protected abstract void seed(byte[] seed);

    // Wouldn't it be more efficient to allow generating more than one byte
    // at once?
    protected abstract byte nextByte();
}
