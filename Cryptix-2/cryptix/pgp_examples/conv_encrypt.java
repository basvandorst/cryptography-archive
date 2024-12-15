package cryptix.pgp_examples;

import java.io.*;

import cryptix.math.PseudoRandomStream;
import cryptix.security.MD5;
import cryptix.security.SHA;

import cryptix.pgp.*;

public final class conv_encrypt
{
	public static final void
	main( String argv[] )
		throws FileNotFoundException, IOException, FormatException
    {
        String messageText = "The quick brown fox jumps over the lazy dog";

		byte ideaKey[] = MD5.hash("x");

        String keySeed = "Some random data to seed PseudoRandomStream ^%$£*&£^$&!£*!%&&^%";
        PseudoRandomStream rs = new PseudoRandomStream( SHA.hash( keySeed ) );


        System.err.println( "ideaKey length: " + ideaKey.length + " (should be 16)" );


		LiteralData stuff = new LiteralData(byteEncode( messageText ), "stuff.txt", 't' );
		byte stuff_buf[] = PacketFactory.save(stuff);

        ConvEncryptedData CKE = new ConvEncryptedData(stuff_buf , ideaKey, rs);
        System.err.println( "Writing file" );

        FileOutputStream fos = new FileOutputStream( "conv.pgp" );
        DataOutputStream dos = new DataOutputStream( fos );
        PacketFactory.write( dos, CKE );
        fos.close();
    }

    private static byte[] byteEncode( String msg )
    {
        /**
         * These PGP classes work on byte arrays, not strings.
         * This function converts from one to the other.
         *@returns A byte array representing msg
         */

        int len = msg.length();
        byte buf[];
        msg.getBytes( 0, len, buf = new byte[len], 0 );
        return buf;
    }
}
