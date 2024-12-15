/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.pgp_examples;

import java.io.*;
import cryptix.security.rsa.*;
import cryptix.pgp.*;

public final class KeyRingTestToo
{
	public static final void
	main( String argv[] )
	{
		if ( argv.length < 1 )
		{
			System.out.println( "Usage: java cryptix.pgp_examples.KeyRingTestToo keyring-name" );
			return;
		}
		try
		{
			new KeyRingTest( argv[0] ).list( System.out );
		}
		catch( FileNotFoundException fnfe )
		{
			System.out.println( "File : " + argv[0] + " Not found." );
		}
		catch( Exception e )
		{
			e.printStackTrace( System.out );
		}
	}
}
