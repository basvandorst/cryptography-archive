/*
// $Log: EncryptedBigInt.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package cryptix.pgp;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataOutput;

import cryptix.math.BigInteger;
import cryptix.security.StreamCipher;
import cryptix.util.Streamable;

/**
 * An encrypted BigInteger class (used by SecretKeyCertificate).
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since Cryptix 2.2 or earlier
 */
final class EncryptedBigInt extends Streamable
{
    private byte[] buf_;
    private int bitLen_;
    private int checksum_;

    EncryptedBigInt(DataInput in)
    throws IOException
    {
        super(in);
    }

    EncryptedBigInt(BigInteger num, StreamCipher cipher)
    {
        buf_ = num.toByteArray();
        bitLen_ = num.bitLength();
        int len = buf_.length;
        int sum = 0;
        for (int i = len - 1; i >= 0; i--)
            sum += (buf_[i] & 0xFF);
        checksum_ = sum & 0xFFFF;
        cipher.encrypt(buf_);
    }

    BigInteger decrypt(StreamCipher cipher)
    {
        int len = buf_.length;
        byte tmp[] = new byte[len];
        cipher.decrypt(buf_, tmp);
        
        int sum = 0;
        for (int i = len - 1; i >= 0; i--)
            sum += (tmp[i] & 0xFF);

        checksum_ = sum & 0xFFFF;
    
        return new BigInteger(tmp);
    }

    public void read(DataInput in)
    throws IOException
    {
        bitLen_ = in.readShort();
        buf_ = new byte[(bitLen_ + 7) / 8];
        in.readFully(buf_);
    }

    public void write(DataOutput out)
    throws IOException
    {
        out.writeShort(bitLen_);
        out.write(buf_);
    }

    public int checksum()
    {
        return ((bitLen_ & 0xFF) + ((bitLen_ >> 8) & 0xFF) + checksum_) & 0xFFFF;
    }
}
