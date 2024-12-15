/**
 * This class stores a passphrase's MD5 hash, which is all is needed,
 * then attempts to wipe the plaintext from memory.
 * A determined attacker could still find this hash - so keep your computer
 * physically secure and only install trusted applications.
 * @author Ian Brown
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
package cryptix.pgp;

import cryptix.security.BlockCipher;
import cryptix.security.Blowfish;
import cryptix.security.IDEA;
import cryptix.security.MD5;
import java.io.DataInputStream;
import java.io.IOException;

public final class Passphrase
{
    private byte[] md5hash;  // Stores a hash of passphrase

    public Passphrase( String passphrase )
    {
        md5hash = MD5.hash( passphrase );

        // Clean up;
        passphrase = null;
        System.gc();
        System.gc(); // twice - an attempt to overcome Microsoft's
                     //  "stop and copy" garbage collection algorithm
    }

    /**
     * @deprecated
     * Obtaining a password from the user is part of the UI, and
     * should not be a part of this library. (At least, not in this file).
     */
    public Passphrase() throws IOException
    {
        // Get passphrase from user
        this( false );
    }

    /**
     * @deprecated
     * Obtaining a password from the user is part of the UI, and
     * should not be a part of this library. (At least, not in this file).
     */
    public Passphrase( boolean showPass ) throws IOException
    {
        System.out.println( "Passphrase: " );
        DataInputStream dis = new DataInputStream( System.in );
        String passphrase = dis.readLine();

        // Get its hash
        md5hash = MD5.hash( passphrase );

        // Clean up
        passphrase = null;
        dis = null;
        System.gc();
        System.gc();
    }

    /**
     * @deprecated
     * @see blockCipher( int )
     */
    public IDEA hash()
    {
        return new IDEA( md5hash );
    }

    public byte[] md5hash()
    {
        return md5hash ;
    }
}
