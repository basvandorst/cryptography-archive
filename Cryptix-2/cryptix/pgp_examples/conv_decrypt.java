package cryptix.pgp_examples;

import java.io.*;
import cryptix.security.MD5;
import cryptix.pgp.*;

public final class conv_decrypt
{
	public static final void
	main( String argv[] )
		throws FileNotFoundException, IOException, FormatException
    {
		byte key[] = MD5.hash("x");

        FileInputStream fis = new FileInputStream( "conv.pgp" );
        DataInputStream dis = new DataInputStream( fis );

		Packet pkt = PacketFactory.read(dis);
		if (pkt == null)
		{
			System.out.println("Could not read packet");
			return;
		}

		if (pkt instanceof ConvEncryptedData)
		{
			byte data[];
			try {
				byte ld_data[] = ((ConvEncryptedData)pkt).data(key);
				Packet ld = PacketFactory.load(ld_data);
				if (ld instanceof LiteralData)
				{
					data = ((LiteralData)ld).data();
				}
				else
				{
					System.out.println("Invalid format packet");
					return;
				}
			}
			catch (DecryptException e)
			{
				System.out.println("Invalid passphrase");
				return;
			}
			System.out.write(data, 0 ,data.length);
			System.out.println();
		}
    }
}
