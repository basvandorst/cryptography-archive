/**
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

public class InvalidChecksumException extends Exception
{
    InvalidChecksumException(String reason)
	{
		super(reason);
	}
}
