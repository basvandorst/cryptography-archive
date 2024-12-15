// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Algorithm.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.0  1997/02/24  Jill Baker
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
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

/**
 * Constants describing the various algorithms we can use.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Jill Baker
 * @since  Cryptix 2.2 or earlier
 */
public interface Algorithm
{
    // No algorithm
    public static final int NONE = 0;

    // Compression algorithms
    public static final int ZIP = 1;
    public static final int GZIP = 2;

    // Hashing algorithms
    public static final int MD5 = 1;
    public static final int SHA = 2;

    // Conventional Key Algorithms
    public static final int IDEA = 1;
    public static final int BLOWFISH = 2;

    // Public Key algorithms
    public static final int RSA = 1;
    public static final int PKETEST = 2;  // I made this up for test purposes

    // Canonical encoding algorithms
    public static final int BINARY = 'b';
    public static final int LEGACY = 't';
    public static final int UTF8   = 'u';
}
