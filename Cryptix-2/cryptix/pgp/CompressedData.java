/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import cryptix.util.Streamable;

/**
 * @author Jill Baker
 */
public final class CompressedData extends Packet implements Algorithm
{
    private int algorithm;
    private PacketByteArray data; /* stored uncompressed */

    public CompressedData( PacketByteArray data )
    {
        this.data = data;
        algorithm = ZIP;
    }

    public CompressedData( PacketByteArray data, int algorithm )
    throws FormatException
    {
        this.data = data;
        setAlgorithm( algorithm );
    }

	public CompressedData( DataInput in, int length )
		throws IOException
	{
        super( in, length );
	}

    public int getAlgorithm()
    {
        return algorithm;
    }

    public void setAlgorithm( int algorithm )
    throws FormatException
    {
        switch (algorithm)
        {
            case NONE:
            case ZIP:
            case GZIP:
                this.algorithm = algorithm;
                break;

            default:
                throw new FormatException(
                "Unknown compression algorithm (" + algorithm + ")" );
        }
    }

    public PacketByteArray getData()
    {
        return data;
    }

	public void 
	read( DataInput in, int length )
        throws IOException, FormatException
	{
        byte[] cda = null;

        setAlgorithm( in.readByte() );
        length--;
        if (length >= 0)
        {
            cda = new byte[length-1];
            in.readFully( cda );
        }
        else
        {
            InputStream is = (InputStream) in;
            BufferedInputStream sin = new BufferedInputStream( is );
            ByteArrayOutputStream sout = new ByteArrayOutputStream();

            for (int b=sin.read(); b!=-1; b=sin.read()) sout.write( b );

            cda = sout.toByteArray();
        }

        data = new PacketByteArray( inflate( cda ));
	}

	public int
	write( DataOutput out )
         throws IOException
	{
        byte[] cda = deflate( data.toByteArray() );
        out.writeByte( (byte)algorithm );
        out.write( cda );
        return 1 + cda.length;
	}

	public int getType()
	{
		return 8;
	}

    private byte[] inflate( byte[] compressedData )
    throws IOException
    {
        if (algorithm != 0) throw new IOException( "Sorry - you need Java 1.1 to inflate compressed data" );
        return compressedData;
    }

    private byte[] deflate( byte[] uncompressedData )
    throws IOException
    {
        if (algorithm != 0) throw new IOException( "Sorry - you need Java 1.1 to deflate compressed data" );
		return uncompressedData;
    }

}
