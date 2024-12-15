/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.pgp_examples;

import java.io.*;
import cryptix.security.MD5;
import cryptix.security.rsa.*;
import cryptix.pgp.*;

public final class Test
{
	public static final void
	main( String argv[] )
	{
		if ( argv.length < 1 )
		{
			System.out.println( "Usage: java cryptix.pgp_examples.KeyRingTest keyring-name" );
			return;
		}
		try
		{
			checkKeyRing( new DataInputStream( new FileInputStream( argv[0] ) ) );
		}
		catch( Exception e )
		{
			e.printStackTrace( System.out );
		}
	}

	public static void
	checkKeyRing( DataInput in )
	throws IOException
	{
		Packet p;
		PublicKeyCertificate cert = (PublicKeyCertificate)PacketFactory.read( in );
		PacketFactory.read( in );
		UserId name = (UserId)PacketFactory.read( in );
		PacketFactory.read( in );
		Signature sig = (Signature)PacketFactory.read( in );
		PublicKey key = cert.publicKey();
		// N.B. this is the WRONG KEY BUT we know the hash should be
		System.out.println("REAL:4100673812DF82143FE66681CE26D764577" );
		// 0x4100673812DF82143FE66681CE26D764577
		MD5 md = new MD5();
		md.add( cert.save() );
		md.add( name.save() );
		sig.check( key, md );
		System.out.println("Hash should end\n43FE66681CE26D764577" );
	}
}
