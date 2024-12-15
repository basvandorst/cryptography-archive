/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */


package cryptix.mime;
 
public abstract class Base64 
{ 
	/**
	 * a static array that maps 6 bit integers to a specific char
	 */
	protected final static char enc_table[] = 
	{ 
	//   0   1   2   3   4   5   6   7 
		'A','B','C','D','E','F','G','H', // 0 
		'I','J','K','L','M','N','O','P', // 1 
		'Q','R','S','T','U','V','W','X', // 2 
		'Y','Z','a','b','c','d','e','f', // 3 
		'g','h','i','j','k','l','m','n', // 4 
		'o','p','q','r','s','t','u','v', // 5 
		'w','x','y','z','0','1','2','3', // 6 
		'4','5','6','7','8','9','+','/'  // 7 
	}; 
	protected final static byte dec_table[] = 
	{ 
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	}; 
 
	public static String 
	encode( byte buf[] ) 
	{
		int i = 0; 
		StringBuffer buffer = new StringBuffer(); 
		int len = buf.length; 
		int delta =	len % 3; 
		byte a, b, c;
		for (int count = len / 3; count > 0; count--) 
		{
			a = buf[i++];
			b = buf[i++];
			c = buf[i++];
			buffer.append(enc_table[(a >>> 2) & 0x3F]); 
			buffer.append(enc_table[((a << 4) & 0x30) + ((b >>> 4) & 0xf)]); 
			buffer.append(enc_table[((b << 2) & 0x3c) + ((c >>> 6) & 0x3)]); 
			buffer.append(enc_table[c & 0x3F]); 

			if (i != 0 && i%57 == 0)
				buffer.append("\r\n");
		}

		if (delta == 1) 
		{
			a = buf[i++];
			buffer.append(enc_table[(a >>> 2) & 0x3F]); 
			buffer.append(enc_table[((a << 4) & 0x30)]); 
			buffer.append('='); 
			buffer.append('='); 
		}
		if (delta == 2) 
		{
			a = buf[i++];
			b = buf[i++];
			buffer.append(enc_table[(a >>> 2) & 0x3F]); 
			buffer.append(enc_table[((a << 4) & 0x30) + ((b >>> 4) & 0xf)]); 
			buffer.append(enc_table[((b << 2) & 0x3c)]); 
			buffer.append('='); 
		}

		buffer.append("\r\n");
		return buffer.toString(); 
	}

	public static String 
	encode( String s ) 
	{
                return encode( LegacyString.toByteArray( s ));
	}

        // Ignores trailing whitespace and newlines
	public static byte[]
	decode( byte buf[] ) 
	{
                int padCount = 0;
                int i, len = buf.length;
		int real_len = 0;

		for (i=len-1; i >= 0; --i)
                {
			if (buf[i] > ' ')
				real_len++;

                        if (buf[i] == 0x3D)
                                padCount++;
                }

		// Hmm - should this be a "bad format MIME" exception instead?
		if (real_len%4 != 0)
			throw new IllegalArgumentException("Length not a multiple of 4");

                int ret_len = (real_len/4)*3 - padCount;
		byte ret[] = new byte[ret_len];

		i = 0;
		byte[] t = new byte[4];
		int output_index = 0;
		int j = 0;
		t[0] = t[1] = t[2] = t[3] = 61;	// Ascii =
		while (i < len)
		{
			byte c =  buf[i++];
			if (c > ' ')
				t[j++] = c;

			if (j == 4)
			{
				output_index += decode(ret, output_index, t[0], t[1], t[2], t[3]);
				j = 0;
				t[0] = t[1] = t[2] = t[3] = 61;	// Ascii =
			}
		}
		if (j > 0)
			decode(ret, output_index, t[0], t[1], t[2], t[3]);

		return ret;
	}

	public static byte[]
	decode( String msg ) 
	{
            return decode( LegacyString.toByteArray( msg ));
	}

	// Returns the number of bytes converted
        private static int
	decode( byte ret[], int ret_off, byte a, byte b, byte c, byte d )
	{
		byte da = dec_table[a];
		byte db = dec_table[b];
		byte dc = dec_table[c];
		byte dd = dec_table[d];

		if (da == -1 || db == -1 || (dc == -1 && c != 0x3d) || (dd == -1 && d != 0x3d))
			throw new IllegalArgumentException("Invalid character ["+a+", "+b+", "+c+", "+d+"]");

		ret[ret_off++] = (byte)(da << 2 | db >>> 4);
		if (c == 0x3d)	// Ascii =
			return 1;
		ret[ret_off++] = (byte)(db << 4 | dc >>> 2);
		if (d == 0x3d)	// Ascii =
			return 2;
		ret[ret_off++] = (byte)(dc << 6 | dd);
                        return 3;
	}
}
