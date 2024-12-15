/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */
package cryptix.pgp_examples;

import java.io.*;
import cryptix.pgp.*;
import cryptix.security.*;
import cryptix.security.rsa.*;

public final class FileView
{
	private static final void
	usage()
	{
		System.out.println( "Usage: java FileView [-s secring-filename] pgp-filename passphrase" );
	}

	public static final void
	main( String argv[] )
	{
		String filename = null;
		String secringname = "secring.pgp";
		String passphrase = null;
		switch ( argv.length )
		{
		case 4:
			if ( ("-?".equals( argv[0] )) || (!("-s".equals( argv[0] ))) )
			{
				usage();
				return;
			}
			secringname = argv[1];
			filename = argv[2];
			passphrase = argv[3];
			break;
		case 2:
			if ( ("-?".equals( filename = argv[0] )) )
			{
				usage();
				return;
			}
			passphrase = argv[1];
			break;
		default:
			usage();
			return;
		}
		try
		{
			SecretKey key = null;
			try
			{
				InputStream sec_in;
				Packet p = PacketFactory.read( new DataInputStream( sec_in = new FileInputStream( secringname ) ) );
				sec_in.close();
				if ( !( p instanceof SecretKeyCertificate ) )
				{
					System.out.println( "File : " + secringname + " Not a pgp secret key." );
					System.exit( -1 );
				}
				key = ((SecretKeyCertificate)p).getSecretKey( new Passphrase( passphrase ) );
			}
			catch( FileNotFoundException fnfe )
			{
				System.out.println( "File : " + secringname + " Not found." );
				System.exit( -1 );
			}
			catch( DecryptException fnfe )
			{
				System.out.println( "Incorrect passphrase" );
				System.exit( -1 );
			}
			InputStream in;
			System.out.println( "- contents of " + filename + " -" );
			viewPackets( 0, key, new DataInputStream( in = new FileInputStream( filename ) ) );
			in.close();
		}
		catch( FileNotFoundException fnfe )
		{
			System.out.println( "File : " + filename + " Not found." );
		}
		catch( Exception e )
		{
			e.printStackTrace( System.out );
		}
	}

	private static final void
	viewPackets( int level, cryptix.security.rsa.SecretKey key, DataInputStream in )
		throws IOException, DecryptException
	{
		int i = 1;
		Packet pkt;
		PublicKeyEncrypted hdr = null;
		Signature sig = null;

		while( ( pkt = PacketFactory.read( in ) ) != null )
		{
			println( level, "*- packet " + i++ + " : " + pkt.getClass().getName() );
			if ( pkt instanceof PublicKeyEncrypted )
				hdr = (PublicKeyEncrypted)pkt;
			println( level, pkt.toString() );
			if ( pkt instanceof ConvEncryptedData )
			{
				try
				{
					if ( hdr != null )
					{
						byte ideakey[] = hdr.getKey( key );
						byte data[] = ((ConvEncryptedData)pkt).data( ideakey );
						viewPackets( level + 1, key, new DataInputStream( new ByteArrayInputStream( data ) ) );
			  		}
					else
						println( level, "no idea key found" );
				}
				catch ( IOException ioe )
				{
					println( level, "Unable to recure ..." );
					println( level, ioe.getClass().getName() + ":" + ioe.getMessage() );
				}
				catch ( Error e )
				{
			e.printStackTrace( System.out );
					println( level, "unable to get Idea key ..." );
					println( level, e.getClass().getName() + ":" + e.getMessage() );
				}
			}
			else if ( pkt instanceof LiteralData)
			{
				LiteralData p = (LiteralData)pkt;
				byte buf[];
				println( level, "original file: " + p.filename() + " mode:" + p.mode() );
				println( level, " date created: " + p.timestamp() );
				println( level, "data;" );
				println( level + 1, new String( buf = p.data(), 0 ) );
				if ( sig != null )
				{
					try
					{
						MD5 md = new MD5();
						if ( p.mode()  != 'b' )
						{
							println( level, "text file converting for signature" );
							texthashlinesinto( md, buf );
						}
						else
						{
							println( level, "Binary file" );
							md.add( buf );
						}

						if ( sig.check( key, md ) )
							println( level, "signed by me and O.K." );
						else
							println( level, "NOT signed by me" );
					}
					catch ( IOException ioe )
					{
						println( level, "Unable to check signature ..." );
						println( level, ioe.getClass().getName() + ":" + ioe.getMessage() );
					}
				}
				else
					println( level, "NO signature" );
			}
			else if ( pkt instanceof Signature)
			{
				sig = (Signature)pkt;
				println( level, "keyid:" + hexString( sig.getKeyId() ) );
			}
		}
		println( level, "- Done -" );
	}
	
	private static final int LINE_LENGTH = 76;

	private static final void
	println( int level, String msg )
	{
		DataInputStream in = new DataInputStream( new StringBufferInputStream( msg ) );
		int max = LINE_LENGTH - level;
		try
		{
			while ( ( msg = in.readLine() ) != null )
			{
				while ( msg.length() > max )
				{
					printString( level, msg.substring( 0, max ) );
					msg = msg.substring( max );
				}
				printString( level, msg );
			}
		}
		catch ( IOException ioe )
		{
		}
	}

	private static final void
	printString( int level, String msg )
	{
		System.out.print( level );
		System.out.print( '-' );
		while ( level-- > 0 )
			System.out.print( ' ' );
		System.out.println( msg );
	}
	
	private static final void
	texthashlinesinto( MD5 md, byte buf[] )
	throws IOException
	{
		DataInputStream in = new DataInputStream( new ByteArrayInputStream( buf ) );
		String line;
		while( (line = in.readLine()) != null )
		{
			if ( line.startsWith( "- " ) );
				line = line.substring( 2 );
			md.add( line );
			md.add( "\r\n" );
		}
	}
	
	private static final char HEX_DIGIT[] = 
	{ 
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	
	private static final String
	hexString( KeyID id )
	{
		int len;
		byte[] buf = id.toByteArray();
		StringBuffer sb = new StringBuffer( (len = buf.length) * 2 + 2 );
		for ( int i = 0; len > 0; len-- )
		{
			int j = buf[i++];
			sb.append( HEX_DIGIT[(j>>4) & 0xF] ).append( HEX_DIGIT[j & 0xF] );
		}
		return sb.toString();
	}
}
