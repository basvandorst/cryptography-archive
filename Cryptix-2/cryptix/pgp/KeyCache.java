package cryptix.pgp;

import java.io.IOException;
import cryptix.security.rsa.PublicKey;
import cryptix.security.rsa.SecretKey;

/**
 * This class currently does nothing other than provide a particular
 * type which will be used by the next versions of the remote key-fetching
 * classes to store retrieved keys.<p>
 * The next version will:<p>
 * <ul><li>Automatically sets expiry date of added keys to 30 days</li>
 * <li>Be able to write itself out to disk and restore itself at next session</li></ul>
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * @version 1.0, 31/12/1996
 * @author Ian Brown
 */
public final class KeyCache extends PublicKeyRing
{
	public KeyCache() {}
}

