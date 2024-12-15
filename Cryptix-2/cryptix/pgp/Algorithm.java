/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.pgp;

/**
 * Constants describing the various algorithms we can use.
 * @author Jill Baker
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
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
