package cryptix.pgp;

import cryptix.util.ByteArray;
import cryptix.util.ByteArrayFactory;

/**
 * A wrapper around the byte[] key id. 
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @author Ian Brown
 * @version 1.0
 */
public final class KeyID extends ByteArray
{
	public KeyID( byte[] id )
	{
		// Could perhaps check that id[] is the right length here?
		// Although future PGP versions may use different KeyID lengths...
		super( id );
	}

	public String
	toString()
	{
		return "id: 0x" + ByteArrayFactory.toHexString( toByteArray() );
	}
}
