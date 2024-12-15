// This file is currently unlocked (change this line if you lock the file)
//
// $Log: SHAOutputStream.java,v $
// Revision 1.2  1997/12/07 06:40:26  hopwood
// + Major overhaul of cryptix.security.*.
//
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Systemics
//   Original version
//
// $Endlog$
/*      
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */
 
package cryptix.security;

/**
 * This class is an Output stream that hashes the data sent to it using
 * the SHA hashing algorithm.
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * <p><b>$Revision: 1.2 $</b>
 * @author Systemics Ltd
 * @see MessageDigestOutputStream
 */
public final class SHAOutputStream extends MessageDigestOutputStream
{
    /**
     * Create a new digest output stream that uses SHA.
     */
    public SHAOutputStream()
    {
        super(new SHA());
    }
}
