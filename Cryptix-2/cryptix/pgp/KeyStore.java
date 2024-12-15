package cryptix.pgp;

import java.io.IOException;
import cryptix.security.MD5;
import cryptix.security.rsa.PublicKey;

/**
 * Abstract base class for PGP keyrings and keyservers.<p> 
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 * @version 1.1, 29/11/1996
 * @author Ian Brown
 * @see systemics.pgp.PGPKeyRing systemics.pgp.KeyServer
 */
public abstract class KeyStore
{
	protected abstract PublicKey getKey( String username ) throws IOException;
	protected abstract PublicKey getKey( KeyID keyID ) throws IOException;

	/**
	 * Check the signature <code>sig</code> on <code>cert</code>
	 * and <code>username</code> using the keys in this keystore.
	 */
	public final boolean 
	checkSignature( Signature sig, Certificate cert, UserId name )
	throws IOException
	{
		return checkSignature( this, sig, cert, name );
	}
	
	/**
	 * Check the signature <code>sig</code> on <code>cert</code>
	 * and <code>username</code> using the keys in <code>store</code>.
	 */
	public final static boolean 
	checkSignature( KeyStore store, Signature sig, Certificate cert, UserId name )
	throws IOException
	{
		PublicKey key;
		if ( (key = store.getKey( sig.getKeyId() )) != null )
			return checkSignature( key, sig, cert, name );
		throw new IOException( "Key id " + sig.getKeyId() + " not found" );
	}
	
	/**
	 * Check the signature <code>sig</code> on <code>key</code>'s <code>cert</code>
	 * and <code>username</code> using the keys in <code>store</code>.
	 */
	public final static boolean 
	checkSignature( PublicKey key, Signature sig, Certificate cert, UserId name )
	throws IOException
	{
		MD5 md = new MD5();
		md.add( PacketFactory.save( cert ) );
		md.add( name.save() );
		return sig.check( key, md );
	}
}

