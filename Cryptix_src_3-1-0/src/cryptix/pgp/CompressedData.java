// This file is currently unlocked (change this line if you lock the file)
//
// $Log: CompressedData.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
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

import java.util.zip.Inflater;
import java.util.zip.InflaterInputStream;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;

import cryptix.util.Streamable;

/**
 * Class representing a compressed Packet.
 *
 * <hr><P>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author Jill Baker
 * @since  Cryptix 2.2 or earlier
 */
public final class CompressedData extends Packet implements Algorithm
{
    private int algorithm;
    private PacketByteArray data; /* stored uncompressed */

    public CompressedData(PacketByteArray data)
    {
        this.data = data;
        algorithm = ZIP;
    }

    public CompressedData(PacketByteArray data, int algorithm)
    throws FormatException
    {
        this.data = data;
        setAlgorithm(algorithm);
    }

    public CompressedData(DataInput in, int length)
        throws IOException
    {
        super(in, length);
    }

    public int getAlgorithm()
    {
        return algorithm;
    }

    public void setAlgorithm(int algorithm)
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
                "Unknown compression algorithm (" + algorithm + ")");
        }
    }

    public PacketByteArray getData()
    {
        return data;
    }

    public void 
    read(DataInput in, int length)
        throws IOException, FormatException
    {
        byte[] cda = null;

        setAlgorithm(in.readByte());
        length--;
        if (length >= 0)
        {
            cda = new byte[length-1];
            in.readFully(cda);
        }
        else
        {
            InputStream is = (InputStream) in;
            BufferedInputStream sin = new BufferedInputStream(is);
            ByteArrayOutputStream sout = new ByteArrayOutputStream();

            for (int b=sin.read(); b!=-1; b=sin.read()) sout.write(b);

            cda = sout.toByteArray();
        }

        data = new PacketByteArray(inflate(cda));
    }

    public int
    write(DataOutput out)
         throws IOException
    {
        byte[] cda = deflate(data.toByteArray());
        out.writeByte((byte)algorithm);
        out.write(cda);
        return 1 + cda.length;
    }

    public int getType()
    {
        return 8;
    }

    private byte[] inflate(byte[] compressedData)
    throws IOException
    {
        try {
            ByteArrayInputStream bs = new ByteArrayInputStream(compressedData);
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            InputStream in;
            switch (algorithm)
            {
                case ZIP:
                {
                    Inflater inf = new Inflater(true);
                    in = new InflaterInputStream(bs, inf);
                    break;
                }
                default:
                {
                    // shouldn't this throw an exception?
                    return compressedData;
                }
            }
            for (int c=in.read(); c != -1; c=in.read()) out.write(c);
            return out.toByteArray();
        } catch (NoClassDefFoundError e) {
            throw new IOException("Sorry - you need Java 1.1 to inflate compressed data");
        }
    }

    private byte[] deflate(byte[] uncompressedData)
    throws IOException
    {
        try {
            ByteArrayInputStream in = new ByteArrayInputStream(uncompressedData);
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            OutputStream out;
            switch (algorithm)
            {
                case ZIP:
                {
                    Deflater def = new Deflater(
                        Deflater.DEFAULT_COMPRESSION, true);
                    out = new DeflaterOutputStream(bout, def);
                    break;
                }
                default:
                {
                    // shouldn't this throw an exception?
                    return uncompressedData;
                }
            }

            for (int c=in.read(); c != -1; c=in.read()) out.write(c);
            out.flush();
            return bout.toByteArray();
        } catch (NoClassDefFoundError e) {
            throw new IOException("Sorry - you need Java 1.1 to inflate compressed data");
        }
    }
}
