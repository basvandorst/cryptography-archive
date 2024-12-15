/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.security;

import java.io.PrintStream;

/**
 * This class implements the MD5 message digest.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public final class MD5 extends MessageDigest
{
	private static final String LIBRARY_NAME = "md5";
	private static final String DIGEST_NAME = "MD5";

	private static boolean native_link_ok = false;
	private static boolean native_lib_loaded = false;
	private static String native_link_err = "Class not loaded";

	static 
	{
		// load the DLL or shared library that contains the native code
		// implementation of the MD5 message digest algorithm.
		try
		{
			System.loadLibrary( LIBRARY_NAME );
			native_lib_loaded = true;
			try
			{
				//
				//	Should really do a bit more testing than this ...
				//
				if (md5_test() == 0)
				{
					native_link_ok = true;
					native_link_err = null;
				}
				else
				{
					native_link_err = "Self test failed";
				}
			}
			catch ( UnsatisfiedLinkError ule )
			{
				native_link_err = "Errors linking to " + LIBRARY_NAME + " native library";
			}
		}
		catch ( UnsatisfiedLinkError ule )
		{
			native_link_err = "The " + LIBRARY_NAME + " native library was not found";
		}
	}

	public final static boolean
	hasFileLibraryLoaded()
	{
		return native_lib_loaded;
	}

	public final static boolean
	isLibraryCorrect()
	{
		return native_link_ok;
	}

	public final static String
	getLinkErrorString()
	{
		return native_link_err;
	}
	
	/**
	 * Length of the final hash (in bytes).
	 */
	public static final int HASH_LENGTH = 16;
	public static final int DATA_LENGTH = 64;


	//
	// Only required for java implementation
	//
	protected int data[];
	protected int digest[];
	protected byte tmp[];



	/**
	 * Return length of the hash (in bytes).
	 * @see #HASH_LENGTH
	 * @return   The length of the hash.
	 */
	public final int hash_length()
	{
		return HASH_LENGTH;
	}

	/**
	 * Return length of the data (in bytes) hashed in every transform.
	 * @return   The length of the data block.
	 */
	public final int data_length()
	{
		return DATA_LENGTH;
	}

	/**
	 * Return name of this hash function.
	 * @return   The name of the hash function.
	 */
	public String name()
	{
		return "MD5";
	}


	/**
	 * The public constructor.
	 * @throws UnsatisfiedLinkError if the library is not of the correct version
	 */
	public MD5()
	{
		if (native_link_ok) native_init(); else java_init();
		reset();
	}

	private void java_init()
	{
		digest = new int[HASH_LENGTH/4];
		data = new int[DATA_LENGTH/4];
		tmp = new byte[DATA_LENGTH];
	}

	/**
	 * Initialise (reset) the message digest.
	 */
	public void md_reset()
	{
		if (native_link_ok) native_reset(); else java_reset();
	}

	private void java_reset()
	{
		digest[0] = 0x67452301;
		digest[1] = 0xefcdab89;
		digest[2] = 0x98badcfe;
		digest[3] = 0x10325476;
	}

	/**
	 * Add data to the message digest
	 * @param data    The data to be added.
	 * @param offset  The start of the data in the array.
	 * @param length  The amount of data to add.
	 */
	protected void md_transform()
	{
		if (native_link_ok) native_transform();
					else java_transform();
	}

	protected void java_transform()
	{
		byte2int(data, 0, buf(), 0, DATA_LENGTH/4);
		transform(data);
	}

	/**
	 * Returns the digest of the data added and resets the digest.
	 * @return    the digest of all the data added to the message digest as a byte array.
	 */
	protected byte[] md_digest()
	{
		return (native_link_ok) ? native_digest() : java_digest();
	}

	private byte[] java_digest()
	{
		int pos = buf_off();
		if (pos != 0) System.arraycopy(buf(), 0, tmp, 0, pos);

		tmp[pos++] = -128;

		if (pos > (DATA_LENGTH-8))
		{
			while (pos < DATA_LENGTH) tmp[pos++] = 0;
			byte2int(data, 0, tmp, 0, DATA_LENGTH/4);
			transform(data);
			pos = 0;
		}
		while (pos < (DATA_LENGTH-8)) tmp[pos++] = 0;

		byte2int(data, 0, tmp, 0, (DATA_LENGTH/4)-2);

		// Little endian
		data[14] = (int)bitcount();
		data[15] = (int)(bitcount()>>>32);

		transform(data);


		byte buf[] = new byte[HASH_LENGTH];

		// Little endian
		int off = 0;
		for (int i=0; i<HASH_LENGTH/4; ++i) {
			int d = digest[i];
			buf[off++] = (byte)d;
			buf[off++] = (byte)(d>>>8);
			buf[off++] = (byte)(d>>>16);
			buf[off++] = (byte)(d>>>24);
		}
		return buf;
	}


    /**
	 * Returns the digest of the data added and resets the digest.
	 * @return the digest of all the data added to the message digest as an object.
	 */
	public MessageHash digestAsHash()
	{
		return new HashMD5( this );
	}

	/**
	 * Returns the hash of a single string.
	 * @param msg the string to hash.
	 * @return the hash of the string.
	 */
	public static byte[]
	hash( String msg )
	{
		return hash( msg, new MD5() );
	}

	/**
	 * Returns the hash of a single byte array.
	 * @param msg the byte array to hash.
	 * @return the hash of the string.
	 */
	public static byte[]
	hash( byte msg[] )
	{
		return hash( msg, new MD5() );
	}

	/**
	 * Returns the MessageHash of a single string.
	 * @param msg the string to hash.
	 * @return the MessageHash of the string.
	 */
	public static HashMD5
	hashAsMessageHash( String msg )
	{
		return new HashMD5( hash( msg, new MD5() ) );
	}

	/**
	 * Returns the MessageHash of a single byte array.
	 * @param msg the byte array to hash.
	 * @return the MessageHash of the byte array.
	 */
	public static HashMD5
	hashAsMessageHash( byte msg[] )
	{
		return new HashMD5( hash( msg, new MD5() ) );
	}

	/**
	 * Returns the hash of a single byte array.
	 * @param msg the byte array to hash.
	 * @return the hash of the string.
	 */
	public static HashMD5
	CreateHash( byte hash[] )
	{
		return new HashMD5( hash );
	}






	//
	// MD5 transform routines
	//

	static protected int F(int x,int y,int z) { return (z ^ (x & (y^z))); }
	static protected int G(int x,int y,int z) { return (y ^ (z & (x^y))); }
	static protected int H(int x,int y,int z) { return (x ^ y ^ z); }
	static protected int I(int x,int y,int z) { return (y  ^  (x | ~z)); }


	static protected int FF(int a,int b,int c,int d,int k,int s,int t)
	{
			a += k+t+F(b,c,d);
			a = (a << s | a >>> -s);
			return a+b;
	}

	static protected int GG(int a,int b,int c,int d,int k,int s,int t)
	{
			a += k+t+G(b,c,d);
			a = (a << s | a >>> -s);
			return a+b;
	}

	static protected int HH(int a,int b,int c,int d,int k,int s,int t)
	{
			a += k+t+H(b,c,d);
			a = (a << s | a >>> -s);
			return a+b;
	}

	static protected int II(int a,int b,int c,int d,int k,int s,int t)
	{
			a += k+t+I(b,c,d);
			a = (a << s | a >>> -s);
			return a+b;
	}


	protected void transform (int M[])
	{
		int a,b,c,d;

		a = digest[0];
		b = digest[1];
		c = digest[2];
		d = digest[3];

		a = FF(a,b,c,d,M[ 0], 7,0xd76aa478);
		d = FF(d,a,b,c,M[ 1],12,0xe8c7b756);
		c = FF(c,d,a,b,M[ 2],17,0x242070db);
		b = FF(b,c,d,a,M[ 3],22,0xc1bdceee);
		a = FF(a,b,c,d,M[ 4], 7,0xf57c0faf);
		d = FF(d,a,b,c,M[ 5],12,0x4787c62a);
		c = FF(c,d,a,b,M[ 6],17,0xa8304613);
		b = FF(b,c,d,a,M[ 7],22,0xfd469501);
		a = FF(a,b,c,d,M[ 8], 7,0x698098d8);
		d = FF(d,a,b,c,M[ 9],12,0x8b44f7af);
		c = FF(c,d,a,b,M[10],17,0xffff5bb1);
		b = FF(b,c,d,a,M[11],22,0x895cd7be);
		a = FF(a,b,c,d,M[12], 7,0x6b901122);
		d = FF(d,a,b,c,M[13],12,0xfd987193);
		c = FF(c,d,a,b,M[14],17,0xa679438e);
		b = FF(b,c,d,a,M[15],22,0x49b40821);

		a = GG(a,b,c,d,M[ 1], 5,0xf61e2562);
		d = GG(d,a,b,c,M[ 6], 9,0xc040b340);
		c = GG(c,d,a,b,M[11],14,0x265e5a51);
		b = GG(b,c,d,a,M[ 0],20,0xe9b6c7aa);
		a = GG(a,b,c,d,M[ 5], 5,0xd62f105d);
		d = GG(d,a,b,c,M[10], 9,0x02441453);
		c = GG(c,d,a,b,M[15],14,0xd8a1e681);
		b = GG(b,c,d,a,M[ 4],20,0xe7d3fbc8);
		a = GG(a,b,c,d,M[ 9], 5,0x21e1cde6);
		d = GG(d,a,b,c,M[14], 9,0xc33707d6);
		c = GG(c,d,a,b,M[ 3],14,0xf4d50d87);
		b = GG(b,c,d,a,M[ 8],20,0x455a14ed);
		a = GG(a,b,c,d,M[13], 5,0xa9e3e905);
		d = GG(d,a,b,c,M[ 2], 9,0xfcefa3f8);
		c = GG(c,d,a,b,M[ 7],14,0x676f02d9);
		b = GG(b,c,d,a,M[12],20,0x8d2a4c8a);

		a = HH(a,b,c,d,M[ 5], 4,0xfffa3942);
		d = HH(d,a,b,c,M[ 8],11,0x8771f681);
		c = HH(c,d,a,b,M[11],16,0x6d9d6122);
		b = HH(b,c,d,a,M[14],23,0xfde5380c);
		a = HH(a,b,c,d,M[ 1], 4,0xa4beea44);
		d = HH(d,a,b,c,M[ 4],11,0x4bdecfa9);
		c = HH(c,d,a,b,M[ 7],16,0xf6bb4b60);
		b = HH(b,c,d,a,M[10],23,0xbebfbc70);
		a = HH(a,b,c,d,M[13], 4,0x289b7ec6);
		d = HH(d,a,b,c,M[ 0],11,0xeaa127fa);
		c = HH(c,d,a,b,M[ 3],16,0xd4ef3085);
		b = HH(b,c,d,a,M[ 6],23,0x04881d05);
		a = HH(a,b,c,d,M[ 9], 4,0xd9d4d039);
		d = HH(d,a,b,c,M[12],11,0xe6db99e5);
		c = HH(c,d,a,b,M[15],16,0x1fa27cf8);
		b = HH(b,c,d,a,M[ 2],23,0xc4ac5665);

		a = II(a,b,c,d,M[ 0], 6,0xf4292244);
		d = II(d,a,b,c,M[ 7],10,0x432aff97);
		c = II(c,d,a,b,M[14],15,0xab9423a7);
		b = II(b,c,d,a,M[ 5],21,0xfc93a039);
		a = II(a,b,c,d,M[12], 6,0x655b59c3);
		d = II(d,a,b,c,M[ 3],10,0x8f0ccc92);
		c = II(c,d,a,b,M[10],15,0xffeff47d);
		b = II(b,c,d,a,M[ 1],21,0x85845dd1);
		a = II(a,b,c,d,M[ 8], 6,0x6fa87e4f);
		d = II(d,a,b,c,M[15],10,0xfe2ce6e0);
		c = II(c,d,a,b,M[ 6],15,0xa3014314);
		b = II(b,c,d,a,M[13],21,0x4e0811a1);
		a = II(a,b,c,d,M[ 4], 6,0xf7537e82);
		d = II(d,a,b,c,M[11],10,0xbd3af235);
		c = II(c,d,a,b,M[ 2],15,0x2ad7d2bb);
		b = II(b,c,d,a,M[ 9],21,0xeb86d391);

		digest[0] += a;
		digest[1] += b;
		digest[2] += c;
		digest[3] += d;
	}







	//
	// The native functions that implement MD5
	//

	/**
	 * This is the amount of data required by the native code.
	 */
	private static final int INT_BUFFER_LENGTH = 88;

	/**
	 * The contextBuffer required by the native code.
	 */
    private byte contextBuf[]; /* MD5 internal data buffer */

	private void native_init()
	{
		contextBuf = new byte[INT_BUFFER_LENGTH];
	}

	private void native_reset()
	{
		md5_init();
	}

	private void native_transform()
	{
		md5_transform(buf(), 0, data_length());
	}

	private byte[] native_digest()
	{
		byte buf[] = new byte[HASH_LENGTH];
		md5_finish(buf);
		return buf;
	}

	/**
	 * Resets the Context buffer to initial values.
	 */
	private synchronized native void md5_init();

	/**
	 * Adds to the hash.
	 * @param data    The data to be added.
	 * @param offset  The start of the data within the array.
	 * @param length  The amount of data to add.
	 */
	private synchronized native void md5_transform(byte data[], int offset, int length);

	/**
	 * Fills the buffer with the digested output and resets the digest.
	 * @param output   The buffer where the digest is to be stored.
	 */
	private synchronized native void md5_finish( byte output[] );
	private synchronized native static int md5_test();






	public static final void byte2int(int dst[], int dst_off, byte src[], int src_off, int len)
	{
		while (len-- > 0)
		{
			// Little endian
			dst[dst_off++] = (src[src_off++] & 0xFF) | ((src[src_off++] << 8) & 0xFF00)
					| ((src[src_off++] << 16) & 0xFF0000) | ((src[src_off++] << 24));
		}
	}




    public static final void
    main(String argv[]) 
		throws java.io.IOException
    {
		if (argv.length > 0)
		{
			java.io.FileInputStream fis = new java.io.FileInputStream(argv[0]);
			MD5 md = new MD5();
			byte[] buf = new byte[1];
			int i;
			while ((i = fis.read()) >= 0)
			{
				buf[0] = (byte)i;
				md.add(buf);
			}

			System.out.println(md.digestAsHash().toString());
		}
		else
		{
			try {
				self_test(System.out, argv);
			}
			catch(Throwable t)
			{
				t.printStackTrace();
			}
		}
	}

	public static void
	self_test( PrintStream out, String argv[] )
	throws Exception
	{
	 	test(out, "", "d41d8cd98f00b204e9800998ecf8427e" );
		test(out, "a", "0cc175b9c0f1b6a831c399e269772661" );
		test(out, "aa", "4124bc0a9335c27f086f24ba207a4912" );
		test(out, "aaa", "47bce5c74f589f4867dbd57e9ca9f808" );
		test(out, "bbb", "08f8e0260c64418510cefb2b06eee5cd" );
		test(out, "ccc", "9df62e693988eb4e1e1444ece0578579" );
		test(out, "abc", "900150983cd24fb0d6963f7d28e17f72" );
		test(out, "abcdefg", "7ac66c0f148de9519b8bd264312c4d64" );
		test(out, "abcdefghijk", "92b9cccc0b98c3a0b8d0df25a421c0e3" );
		test(out, "message digest", "f96b697d7cb7938d525a2f31aaf161d0" );
		test(out, "abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b" );
		test(out, 
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			"d174ab98d277d9f5a5611c2c9f419d9f"
			);
		test(out, 
			"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
			"57edf4a22be3c955ac49da2e2107b67a"
		);
	}

	private static void
	test( PrintStream out, String msg, String hashStr )
	{
		hashStr = hashStr.toUpperCase();
		MD5 md5 = new MD5();
		md5.add( msg );
		String x = toString( md5.digest() );
		out.println( "Message " + msg );
		out.println( "calculated: " + x );
		out.println( "expected:   " + hashStr );
		if ( hashStr.equals( x ) )
			out.println( "Good" );
		else
			out.println( "************* MD5 FAILED **************" );
	}

	private static byte[]
	fromString( String inHex )
	{
		int len=inHex.length();
		int pos =0;
		byte buffer[] = new byte [( ( len + 1 ) / 2 )];
		if ( ( len % 2 ) == 1 )
		{
			buffer[0]=(byte)asciiToHex(inHex.charAt(0));
			pos=1;
			len--;
		}

		for(int ptr = pos; len > 0; len -= 2 )
			buffer[pos++] = (byte)( 
					( asciiToHex( inHex.charAt( ptr++ ) ) << 4 ) |
					( asciiToHex( inHex.charAt( ptr++ ) ) )
					);
		return buffer;
	}

	private static final String
	toString( byte buffer[] )
	{
		StringBuffer returnBuffer = new StringBuffer();
		for ( int pos = 0, len = buffer.length; pos < len; pos++ )
			returnBuffer.append( hexToAscii( ( buffer[pos] >>> 4 ) & 0x0F ) )
						.append( hexToAscii( buffer[pos] & 0x0F ) );
		return returnBuffer.toString();
	}

	private static final int
	asciiToHex(char c)
	{
		if ( ( c >= 'a' ) && ( c <= 'f' ) )
			return ( c - 'a' + 10 );
		if ( ( c >= 'A' ) && ( c <= 'F' ) )
			return ( c - 'A' + 10 );
		if ( ( c >= '0' ) && ( c <= '9' ) )
			return ( c - '0' );
		throw new Error("ascii to hex failed");
	}

	private static char
	hexToAscii(int h)
	{
		if ( ( h >= 10 ) && ( h <= 15 ) )
			return (char)( 'A' + ( h - 10 ) );
		if ( ( h >= 0 ) && ( h <= 9 ) )
			return (char)( '0' + h );
		throw new Error("hex to ascii failed");
	}
}
