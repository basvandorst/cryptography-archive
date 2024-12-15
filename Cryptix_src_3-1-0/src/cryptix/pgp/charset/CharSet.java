// This file is currently unlocked (change this line if you lock the file)
//
// $Log: CharSet.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.3  1997/02/28  Jill Baker
//   Complete rewrite
//
// Revision 0.2.5.2  1997/02/26  Jill Baker
//   Complete rewrite.
//
// Revision 0.2.5.1  1997/02/24  Jill Baker
//   Original version
//
//   If this file is unlocked, or locked by someone other than you, then you
//   may download this file for incorporation into your build, however any
//   changes you make to your copy of this file will be considered volatile,
//   and will not be incorporated into the master build which is maintained
//   at this web site.
//
//   If you wish to make permanent changes to this file, you must wait until
//   it becomes unlocked (if it is not unlocked already), and then lock it
//   yourself. Whenever the file is locked by you, you may upload new master
//   versions to the master site. Make sure you update the history information
//   above. When your changes are complete, you should relinquish the lock to
//   make it available for other developers.
// $Endlog$
package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * This class contains static methods to convert between PGP's
 * internal character set, and any charset whose name is recognised
 * by old-PGP.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Jill Baker
 */
public class CharSet
{
    private CharSet() {} // static methods only

    /**
     * Convert a byte[] buffer from PGP's internal form to the specified
     * charset.
     */
    public static byte[] encode(byte[] in, String name)
    throws FormatException
    {
        String lname = name.toLowerCase();

        if (lname.equals("ascii"))
            return Ascii.encode(in);

        else if (lname.equals("alt_codes"))
            return Cp866.encode(in);

        else if (lname.equals("cp850"))
            return Cp850.encode(in);

        else if (lname.equals("cp852"))
            return Cp852.encode(in);

        else if (lname.equals("cp860"))
            return Cp860.encode(in);

        else if (lname.equals("cp866"))
            return Cp866.encode(in);

        else if (lname.equals("keybcs"))
            return Keybcs.encode(in);

        else if (lname.equals("ko18"))
            return in;

        else if (lname.equals("latin1"))
            return in;

        else if (lname.equals("mac"))
            return Mac.encode(in);

        else if (lname.equals("next"))
            return Next.encode(in);

        else if (lname.equals("noconv"))
            return in;

        else
            throw new FormatException("Charset '" + name + "' not recognised");
    }

    /**
     * Convert a byte[] buffer from the specified charset
     * to PGP's internal form.
     */
    public static byte[] decode(byte[] in, String name)
    throws FormatException
    {
        String lname = name.toLowerCase();

        if (lname.equals("ascii"))
            return Ascii.decode(in);

        else if (lname.equals("alt_codes"))
            return Cp866.decode(in);

        else if (lname.equals("cp850"))
            return Cp850.decode(in);

        else if (lname.equals("cp852"))
            return Cp852.decode(in);

        else if (lname.equals("cp860"))
            return Cp860.decode(in);

        else if (lname.equals("cp866"))
            return Cp866.decode(in);

        else if (lname.equals("keybcs"))
            return Keybcs.decode(in);

        else if (lname.equals("ko18"))
            return in;

        else if (lname.equals("latin1"))
            return in;

        else if (lname.equals("mac"))
            return Mac.decode(in);

        else if (lname.equals("next"))
            return Next.decode(in);

        else if (lname.equals("noconv"))
            return in;

        else
            throw new FormatException("Charset '" + name + "' not recognised");
    }
}

