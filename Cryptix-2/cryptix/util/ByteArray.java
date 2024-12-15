/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.util;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

/**
 * A class that represents a byte array
 */
public class ByteArray
{
        protected byte data[];

        public ByteArray( byte buf[], boolean copy )
	{
		if ( copy )
		{
			int len = buf.length;
			System.arraycopy( buf, 0, data = new byte[len], 0, len );
		}
		else
			data = buf;
	
	}
	
	public ByteArray( byte buf[] )
	{
		this( buf, true );
	}
	
	
	public int
	length()
	{
		return data.length;
	}
	
	public int
	hashCode()
	{
		switch( data.length )
		{
		case 0:
			return 0;
		case 1:
			return data[0];
		case 2:
			return ( data[0] << 8 ) ^ data[1];
		case 3:
			return ( data[0] << 16 ) ^ ( data[1] << 8 ) ^ data[2];
		default:
			break;
		}
		return ( data[0] << 24 ) ^ ( data[1] << 16 ) ^ ( data[2] << 8 ) ^ data[3];
	}
	
	public boolean
	equals( Object obj )
	{
		if ( obj == this )
			return true;
		if ( obj instanceof ByteArray )
			return theSame( this.data, ( (ByteArray)obj ).data );
		return false;
	}
	
	public String
	toString()
	{
		return new String( data, 0 );
	}
	
	public byte[]
	toByteArray()
	{
		byte tmp[];
		int len = data.length;
		System.arraycopy( data, 0, tmp = new byte[len], 0, len );
		return tmp;
	}
	
	public static boolean 
	theSame( byte data1[], byte data2[] )
	{
		int len1, len2;
		if ( ( len1 = data1.length ) == ( len2 = data2.length ) )
		{
			while( --len1 > 0 )
				if ( data1[len1] != data2[len1] )
					return false;
			return true;
		}
		return false;
	}
}
