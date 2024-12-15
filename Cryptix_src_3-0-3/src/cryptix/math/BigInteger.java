// This file is currently unlocked (change this line if you lock the file)
//
// $Log: BigInteger.java,v $
// Revision 1.3  1997/11/20 19:13:04  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.2  1997/11/04 19:33:29  raif
// *** empty log message ***
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.2  1997/06/22  David Hopwood
// + Changed to use methods of cryptix.util.Hex for conversion to/from
//   hex strings.
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
// + Moved this file here from old namespace.
// + Added method equals(BigInteger).
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

import cryptix.util.core.Hex;

import java.io.PrintWriter;

/**
 * A class implementing arbitrary-length integers.
 * <samp>java.math.BigInteger</samp> is preferred for new code.
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  unattributed
 * @since   Cryptix 2.2
 */
public class BigInteger extends BigNum
{
    public static final BigInteger zero = new BigInteger();
    public static final BigInteger one = new BigInteger(1);
    
    static
    {
        zero.zero();
    }
    
    public BigInteger()
    {
        super();
    }

    public BigInteger(int from)
    {
        super();
        assign(from);
    }

    public BigInteger(BigInteger from)
    {
        super();
        copy(this, from);
    }

    public BigInteger(String hexString)
    {
        super();
        fromString(hexString);
    }

    public BigInteger(byte[] buffer)
    {
        super();
        fromByteArray(buffer);
    }

    public int bitLength() { return bitLength(this); }

    public Object clone()
    {
        return new BigInteger(this);
    }
    
    public void copy(Object src)
    {
        copy(this, (BigInteger) src);
    }

    public BigInteger assign(BigInteger n)
    {
        copy(this, n);
        return this;
    }

    public BigInteger assign(int n)
    {
        assign(this, n);
        return this;
    }
    
    public void fromString(String inHex)
    {
        if (inHex.length() == 0) {
            zero(this);
        } else {
            fromBinary(Hex.fromString(inHex));
        }
    }
    
    public String toString()
    {
        byte[] buffer = toByteArray();
        String s = Hex.toString(buffer);

        // remove leading 0's.
        int pos = 0, len = s.length();
        while ((pos < len) && (s.charAt(pos) == '0'))
            pos++;

        if (pos == len) {
            return "0";
        } else {
            return s.substring(pos);
        }
    }
    
    protected void finalize()
    {
        super.finalize();
    }

    public BigInteger modExp(BigInteger power, BigInteger modulo)
    {
        modExp(this, this, power, modulo);
        return this;
    }

    public BigInteger inverseModN(BigInteger a, BigInteger n)
    {
        inverseModN(this, a, n);
        return this;
    }

    public BigInteger mod(BigInteger a, BigInteger b)
    {
        mod(this, a, b);
        return this;
    }

    public BigInteger mul(BigInteger a, BigInteger b)
    {
        mul(this, a, b);
        return this;
    }

    public BigInteger div(BigInteger a, BigInteger b)
    {
        div(this, a, b);
        return this;
    }

    public BigInteger add(BigInteger a, BigInteger b)
    {
        add(this, a, b);
        return this;
    }

    public BigInteger add(int a)
    {
        add(this, a);
        return this;
    }

    public BigInteger sub(BigInteger a, BigInteger b)
    {
        sub(this, a, b);
        return this;
    }

    public BigInteger gcd(BigNum a, BigNum b)
    {
        gcd(this, a, b);
        return this;
    }

    public int cmp(BigInteger a)
    {
        return cmp(this, a);
    }

    /**
     * Tests whether another object is equal to this one.
     * <p>
     * Note: since BigIntegers are mutable, it was a mistake to override
     * equals. Also, <code>equals</code> and <code>hashCode</code> should
     * always be overridden together.
     *
     * @param obj object to test
     * @return true if the object is equal, otherwise false.
     */
    public boolean equals(Object o)
    {
        if (!(o instanceof BigInteger)) return false;
        return (cmp((BigInteger) o) == 0);
    }
    
    public BigInteger inc()
    {
        inc(this);
        return this;
    }

    public BigInteger dec()
    {
        dec(this);
        return this;
    }

    public BigInteger shiftLeft(int n)
    {
        shiftLeft(this, this, (short) n);
        return this;
    }

    public BigInteger shiftRight(int n)
    {
        shiftRight(this, this, (short) n);
        return this;
    }

    public static BigInteger zero()
    {
        BigInteger r = new BigInteger();
        zero(r);
        return r;
    }

    /**
     * An internal function to create a buffer big enough in which
     * to store a number.
     * @param bitLen The number of <b>bits</b> in the number.
     * @return a buffer in which to store a number.
     */
    protected static byte[] newBuffer(int bitLen)
    {
        return new byte[(bitLen + 7) / 8];
    }

    /**
     * Convert a number into a byte array.
     * @return a byte array.
     */
    public byte[] toByteArray()
    {
        byte[] buffer = newBuffer(bitLength());
        intoBinary(buffer);
        return buffer;
    }

    public void fromByteArray(byte[] buf)
    {
        fromBinary(buf);
    }

    /**
     * Convert this cryptix.math.BigInteger to a java.math.BigInteger.
     *
     * @return the java.math.BigInteger
     */
    public java.math.BigInteger toJavaMathBigInteger() {
        // inefficient, but it seems to work.
        return new java.math.BigInteger(toString(), 16);
    }

    /**
     * Convert from a java.math.BigInteger to a cryptix.math.BigInteger.
     *
     * @param from  the java.math.BigInteger
     * @return      the cryptix.math.BigInteger
     */
    public static cryptix.math.BigInteger fromJavaMathBigInteger(java.math.BigInteger from) {
        // inefficient, but it seems to work.
        return new cryptix.math.BigInteger(from.toString(16));
    }

//
// Test methods
//...........................................................................

    public static void main(String[] args) {
        try {
            self_test(new PrintWriter(System.out, true));
        } catch (Throwable t) { t.printStackTrace(); }
    }

    private static final String USD_HASH = "2d0a96ecc3bef46309b2f753a804f9d9";
    private static final String TMP_STR1 = "d0a96ecc3bef46309b2f753a804f9d9";
    private static final String TMP_STR2 = "0d0a96ecc3bef46309b2f753a804f9d9";
    private static final String TMP_STR3 = "1d0a96ecc3bef46309b2f753a804f9d9";
    private static final String TMP_STR4 = "1d0a96ecc3bef46309b2f753a804f9d";

    public static void self_test(PrintWriter out)
    throws Exception
    {
        BigInteger i = new BigInteger(0x7fffffff);
        out.println("Displaying 0x7fffffff: " + i.toString() + " / " + i.bitLength());

        i.inc();
        out.println("Adding one: " + i.toString() + " / " + i.bitLength());
        i.inc();
        out.println("Adding one: " + i.toString() + " / " + i.bitLength());

        BigInteger a = new BigInteger(0x7fffffff);
        BigInteger b = new BigInteger(0x7fffffff);
        BigInteger r = new BigInteger();

        add(r, a, b);
        out.println("add: " + r.toString() + " / " + r.bitLength());
        add(r, r, a);
        out.println("add: " + r.toString() + " / " + r.bitLength());
        sub(r, r, a);
        out.println("sub: " + r.toString() + " / " + r.bitLength());

        a.fromString("ffffffffffffffff");
        b.fromString("ffffffffffffffff");
        add(r, a, b);
        out.println(a.toString() + " + " + b.toString() +  " = " + r.toString() + " / " + r.bitLength());

        a.copy(r);
        sub(r, a, b);
        out.println(a.toString() + " - " + b.toString() +  " = " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x3);
        b = new BigInteger(0x3);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x17);
        b = new BigInteger(0x17);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7f);
        b = new BigInteger(0x7f);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7ff);
        b = new BigInteger(0x7ff);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7fff);
        b = new BigInteger(0x7fff);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7ffff);
        b = new BigInteger(0x7ffff);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7fffff);
        b = new BigInteger(0x7fffff);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(r);
        b = new BigInteger(r);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x7fffffff);
        b = new BigInteger(0x7fffffff);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(r);
        b = new BigInteger(r);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a.copy(r);
        mul(r, a, a);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a.copy(r);
        mul(r, a, a);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x8000);
        b = new BigInteger(0x8000);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x80000);
        b = new BigInteger(0x80000);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(0x800000);
        b = new BigInteger(0x800000);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(r);
        b = new BigInteger(r);
        mul(r, a, b);
        out.println("mul: " + r.toString() + " / " + r.bitLength());

        r = new BigInteger(1);
        r.shiftLeft(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(2);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(4);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(61);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(62);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(63);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(64);
        out.println("shift: " + r.toString() + " / " + r.bitLength());

        r.shiftRight(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(2);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(4);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(61);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(62);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(63);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(64);
        out.println("shift: " + r.toString() + " / " + r.bitLength());

        a.fromString("2d0a96ecc3bef46309b2f753a804f9d9");
        b.fromString("1d0a96ecc3bef46309b2f753a804f9d");

        mul(r, a, b);
        out.println(a.toString() + " * " + b.toString() +  " = " + r.toString() + " / " + r.bitLength());

        BigInteger dv = new BigInteger();
        BigInteger rem = new BigInteger();

        r.copy(a);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftLeft(124);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());
        r.shiftRight(1);
        out.println("shift: " + r.toString() + " / " + r.bitLength());

        a.fromString("2d0a96ecc3bef46309b2f753a804f9d9");
        b.fromString("1d0a96ecc3bef46309b2f753a804f9d");
        mul(r, a, b);
        out.println(a.toString() + " * " + b.toString() +  " = " + r.toString() + " / " + r.bitLength());

        div(dv, rem, r, a);
        out.println("div = " + dv.toString() + " / " + dv.bitLength());
        out.println("rem = " + rem.toString() + " / " + rem.bitLength());

        div(dv, rem, r, b);
        out.println("div = " + dv.toString() + " / " + dv.bitLength());
        out.println("rem = " + rem.toString() + " / " + rem.bitLength());

        out.println("shift: " + r.toString() + " / " + r.bitLength());

        out.println("stuff");
        dv.fromString("1d0a96ecc3bef46309b2f753a804f9d");
        dv.shiftLeft(126);
        out.println("cmp = " + cmp(r,dv));
        dv.shiftRight(1);
        out.println("cmp = " + cmp(r,dv));
        out.println("stuff");

        BigInteger z = BigInteger.zero;

        r = new BigInteger(0);
        dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());

        r = new BigInteger(10);
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());
        dec(r); dec(r); dec(r);
        out.println("dec (-1): " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(76543);
        b = new BigInteger(2);
        div(r, null, a, b);
        out.println("div: " + r.toString() + " / " + r.bitLength());
        out.println("cmp: " + r.cmp(z));

        out.println("ucmp: " + ucmp(a, b));
        out.println("a len: " + a.bitLength());
        out.println("b len: " + b.bitLength());

        BigInteger ds = new BigInteger();

        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 1);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 1);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 1);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 2);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 5);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 5);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());

        out.println("b: " + b.toString() + " / " + b.bitLength());
        shiftLeft(ds, b, 15);
        out.println("ds: " + ds.toString() + " / " + ds.bitLength());
        out.println("cmp: " + r.cmp(ds));

// for (; i>= 0; --i)
// {       
//     if (cmp(r, ds) >= 0)
//     sub(r, r, ds);
//     shiftRightOnce(ds, ds);
// }

        a = new BigInteger(76543);
        b = new BigInteger(2);
        mod(r, a, b);
        out.println("mod: " + r.toString() + " / " + r.bitLength());
        out.println("cmp: " + r.cmp(z));

        a = new BigInteger(34567);
        b = new BigInteger(13);
        mod(r, a, b);
        out.println("mod: " + r.toString() + " / " + r.bitLength());
        out.println("cmp: " + r.cmp(z));

        out.println("cmp: " + r.cmp(z));
        out.println("cmp: " + r.cmp(BigInteger.zero));
        out.println("cmp: " + r.cmp(z));
        out.println("cmp: " + z.cmp(r));
        
        display(out, r);
        display(out, z);

        r = BigInteger.zero;
        out.println("zero: " + r.toString() + " / " + r.bitLength());
        out.println("zero: " + BigInteger.zero.toString() + " / " + BigInteger.zero.bitLength());

        a = new BigInteger(15);
        b = new BigInteger(27);
        gcd(r, a, b);
        out.println("gcd: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(13);
        b = new BigInteger(500);
        gcd(r, a, b);
        out.println("gcd: " + r.toString() + " / " + r.bitLength());

        a = new BigInteger(79);
        b = new BigInteger(3220);
        inverseModN(r, a, b);
        out.println("inv: " + r.toString() + " / " + r.bitLength());

        test(out, USD_HASH);
        test(out, TMP_STR1);
        test(out, TMP_STR2);
        test(out, TMP_STR3);
        test(out, TMP_STR4);
    }

    private static void test(PrintWriter out, String testStr)
    {
        BigInteger i = new BigInteger();
        i.fromString(testStr);
        out.println("Test: " + testStr);
        out.println("  big int  " + i.toString());
        i.fromString(i.toString());
        out.println("  restored " + i.toString());
    }
}
