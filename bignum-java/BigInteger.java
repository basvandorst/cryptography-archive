/*
 * @(#)BigInteger.java	1.1 95/08/31 Chuck McManis
 *
 * Copyright (c) 1994 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies. Please refer to the file "copyright.html"
 * for further important copyright and licensing information.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

package java.misc; 

import java.util.Enumeration;
import java.util.Vector;
import java.util.Random;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.PrintStream;


/**
 * The large integer (BigInteger) class is a variable precision 
 * integer subsystem used for working with typically very large numbers.
 * It implements the standard operations +,-,*, and / as well as mod 
 * (modulus) and rem (remainder). Further it provides a compound 
 * exponentiation and modulus function as exponentiation can generate
 * many, many bits quickly.
 *
 * In our implementation large integers are immutable and signed. 
 * They are created by static factory methods defined in this class. 
 *
 * @version 	1.1, 31 Aug 1995
 * @author 	Chuck McManis
 */
public class BigInteger extends Number {

    /** number of bits in this number */
    private int bits;

    /** data storage for the number */
    /* [0] - Least Significant word, [n] - Most Signifcant Word */
    private int data[];

    /** private statics to make ops with longs and ints efficient */
    private static BigInteger longTmp;
    private static BigInteger intTmp;

    /**
     * Private constructor constructs a large integer of the specified number of
     * bits rounded up to the nearest 32 bits.
     */
    private BigInteger(int nbits) {

	data = new int[(nbits + 31)/32];	// 32 bit ints
	bits = data.length * 32;
	for (int i = 0; i < data.length; i++) data[i] = 0;
    }

    /**
     * Private constructor that makes a new large integer out of an array of
     * ints.
     */
    private BigInteger(int numdata[]) {
	data = numdata;
	bits = data.length * 32;
    }

    /* *********************************
     * * F A C T O R Y   M E T H O D S *
     * *********************************
     *
     * This section defines the factory methods that return a large integer.
     */

    /*
     * Private array of large integer's containing small integers. This is a
     * hack, ideally we would a sparse array of all integers we
     * generate from large integers and use that to re-use existing large integers.
     */
    private static BigInteger smallIntegers[];

    /**
     * Manufacture a new large integer with the given integer value.
     * Note: Using the versions of add, sub, mul, and div that
     * take an integer parameter is more efficient than creating
     * a large integer just for that purpose.
     */
    public static BigInteger fromNumber(int value) {
	BigInteger result;
	if (smallIntegers == null)
	    smallIntegers = new BigInteger[100];
	
	if ((value >= 0) && (value < smallIntegers.length)) {
	    if (smallIntegers[value] == null) {
		result = new BigInteger(32);
		result.data[0] = value;
		smallIntegers[value] = result;
	    } else
		result = smallIntegers[value];
	} else {
	    result = new BigInteger(32);
	    result.data[0] = value;
	}
	return result;
    }

    /**
     * Manufacture a new large integer with the given long value.
     * Note: Using the versions of add, sub, mul, and div that
     * take an integer parameter is more efficient than creating
     * a large integer just for that purpose.
     */
    public static BigInteger fromNumber(long value) {
	BigInteger result;

	result = new BigInteger(64);
	result.data[0] = (int) ((value >>> 32) & 0xffffffff);
	result.data[1] = (int) (value & 0xffffffff);
	return result;
    }

    /**
     * Manufacture a new BigInteger from an array of bytes.
     * The first byte in the array is the most significant
     * byte. If its high order bit is true the number is negative.
     */
    public static BigInteger fromByteArray(byte numData[]) {
	BigInteger result = new BigInteger(numData.length * 8);
	ByteArrayInputStream in = new ByteArrayInputStream(numData);
	DataInputStream dis = new DataInputStream(in);
	int i = 0;
	
	if ((numData.length & 3) > 0) {
	    result.data[0] = ((numData[0] & 0x80) != 0) ? -1 : 0;
	    for (int j = 0; j < (numData.length & 3); j++)
		try {
		    result.data[0] = (result.data[0] << 8) |
					(dis.readByte() & 0xff);
		} catch (IOException e) { }
	    i++;
	}
	for ( ; i < result.data.length; i++) {
	    try {
	        result.data[i]  = dis.readInt();
	    } catch (IOException e) { } // never happens
	    
	}
	return result;
    }

    /**
     * Manufacture a new BigInteger from an array of integers.
     * The first integer in the array is presumed to be the
     * most significant 32 bits of the number and the last
     * integer is the least significant 32 bits. If the MSB
     * of the first integer is true, the resulting large integer will
     * be network.
     */
    public static BigInteger fromIntArray(int value[]) {
	BigInteger result = new BigInteger(value.length * 32);
	for (int i = 0; i < value.length; i++)
	    result.data[i] = value[i];
	return result;
    }

    /**
     * Manufacture a new large integer that is probably prime of approximately 
     * <i>bits</i> precision. The number will be between 3*2^(bits-2)
     * and 2^bits - 1. The initial number is generated randomly by
     * using bits supplied by the Random object passed. While using
     * the internal random bit source works fine, the prime numbers
     * thus chosen have a certain degree of predictability.
     */
    public static BigInteger prime(int bits, Random rndSrc) {
	BigInteger res;
	BigInteger t, u;
	BigInteger lowerBound;
	BigInteger upperBound;
	BigInteger orig;
	int	w, k = 0;
         
	if (rndSrc != null) 
	    res = random(bits, rndSrc);
	else
	    res = BigInteger.random(bits);

	/* force it to be odd (couldn't be prime if even) */
	res.data[res.data.length - 1] |= 1;

	/* compute t = 2^(bits - 2), u = 3 * 2^(bits - 2) */
	t = pow2(bits - 2);
	u = pow2(bits - 1);
	lowerBound = t.add(u);

	/* compute upperBound = 2^b - 1 */
	upperBound = (pow2(bits).sub(1));
	
	/* We assume we will find a prime eventually */
	orig = res;
	w = 2;
	while (! res.isPrime()) {
	    if (res.cmp(upperBound) <= 0) {
		res = orig;
		w = -2;
	    }
	    res = res.add(w);
	}
	return res;
    }

    /**
     * Manufacture a new large integer that is probably prime of approximately 
     * the specified bits precision. This version uses the 
     * java.util.Random object with a seed of the current time for
     * the initial guess at a prime.
     */
    public static BigInteger prime(int bits) {
	return (BigInteger.prime(bits, null));
    }
 
    /**
     * This helper method will read hex digits from the passed
     * string, collect them into an array of bytes, and then
     * generate a large integer from that array of bytes. It covers the
     * case where the number of hex digits is not even (implicit
     * four high order bits) the fromByteArray method covers
     * the case when the number of bytes is not a multiple of 4
     * (implicit 24, 16, or 8 high order bits). If the first
     * hex digit is > 7 the number is negative.
     */
    private static BigInteger fromHexString(String aNum) {
	ByteArrayOutputStream ou = new ByteArrayOutputStream();
	byte digits[];
	int	len;
	int 	dig = 0;
	int	i, x = 0;

	len = aNum.length();
	ou.write(0); // guarantee a positive number 
	// start at 2 to skip the "0x"
	for (i = 2; i < len; i++) {
	    int	 t = aNum.charAt(i);

	    if (t >= '0' && t <= '9')
		x = t - '0';
	    else if (t >= 'a' && t <= 'f')
		x = (t - 'a') + 10;
	    else if (t >= 'A' && t <= 'F')
		x = (t - 'A') + 10;
	    else
		break;
	    ou.write(x);
	}
	digits = ou.toByteArray();

	// digits now contains all of the processed hex digits
	ou.reset();
	i = 0;

	// Deal with the case where we have an odd number of digits
	if ((digits.length & 1) != 0) {
	    ou.write(digits[0]);
	    i++;
	}
	
	for (; i < digits.length; i+= 2) {
	    ou.write((digits[i] << 4) | (digits[i+1] & 0xf));
	}
	return (fromByteArray(ou.toByteArray()));
    }

    private static BigInteger fromOctalString(String aNum) {
	return null;
    }

    private static BigInteger fromDecimalString(String aNum) {
	return null;
    }

    /*
     * Since the fromString method may need to generate a negative
     * number, this method allows us to do that without allocating
     * an entirely new one for this special case.
     */
    private static void inSituNegation(BigInteger a) {

	// one's complement 
	for (int i = 0; i < a.data.length; i++) {
	    a.data[i] = ~a.data[i];
	}

	// increment result by 1 
	for (int i = a.data.length - 1 ; i >= 0; i--) {
	    int q = a.data[i];
	    a.data[i]++;
	    if (((q & 0x80000000) ^ (a.data[i] & 0x80000000)) == 0)
		break; // no overflow, we're done.
	}
    }

    /**
     * Manufacture a new large integer from a string of digits. If the first
     * two characters must be the string "0x" the rest are interpreted
     * as hexadecimal digits. In future versions of this class other
     * bases will be supported. 
     */
    public static BigInteger fromString(String aNum) throws 
	NumberFormatException {
	boolean unaryMinus = false;
	BigInteger	res;

	if (aNum.startsWith("-")) {
	    unaryMinus = true;
	    aNum = aNum.substring(1);
	}
	if (aNum.startsWith("0x")) {
	    res =  fromHexString(aNum);
	} else if (aNum.startsWith("0")) {
	    res = fromOctalString(aNum);
	} else if (aNum.charAt(0) >= '1' && aNum.charAt(0) <= '9') {
	    res = fromDecimalString(aNum);
	} else
	    throw new NumberFormatException();
	
	if (unaryMinus)
	    inSituNegation(res);
	return res;
    }

    private static BigInteger makeRandomNumber(int bits, DataInputStream dis) {
	int w[] = new int[(Math.abs(bits) + 31)/32+1];
	int q[];

	for (int i = 1; i < w.length; i++) {
	    try {
	        w[i] = dis.readInt();
	    } catch (IOException e) { }
	}

	if (bits < 0) {
	    // I want a negative number
	    if ((w[1] & 0x80000000) == 0) {
		w[0] = -1;
		return new BigInteger(w);
	    } else {
	        q = new int[w.length - 1];
	        for (int i = 0; i < q.length; i++) {
		    q[i] = w[i+1];
		    w[i+1] = 0;
	        }
	        return (new BigInteger(q));
	    }
	} else {
	    // I want a positive number.
	    if ((w[1] & 0x80000000) != 0) {
		w[0] = 0;
		return new BigInteger(w);
	    } else {
	        q = new int[w.length - 1];
	        for (int i = 0; i < q.length; i++) {
		    q[i] = w[i+1];
		    w[i+1] = 0;
	        }
	        return (new BigInteger(q));
	    }
	
	}
    }

    /**
     * Manufacture a new large integer <i>bits</i> bits of precision. This uses the 
     * internal random bit source. If the number of bits specified is a 
     * positive value, the resulting number will be positive. If it is 
     * negative the result will be negative.
     */
    public static BigInteger random(int bits) {
	DataInputStream dis = new DataInputStream(new RandomDataStream());
	return (makeRandomNumber(bits, dis));
    }

    /**
     * Manufacture a new large integer the specified number of bits precision. This 
     * uses a user supplied random bit source. If the number of bits specified
     * is positive, the resulting number will be positive. If it is negative 
     * the result will be negative.
     */
    public static BigInteger random(int bits, Random rnd) {
	DataInputStream dis = new DataInputStream(new RandomDataStream(rnd));
	return (makeRandomNumber(bits, dis));
    }

    /* ***********************************
     * * H E L P E R   F U N C T I O N S *
     * ***********************************
     *
     * These methods are used by the public methods.
     */

    /**
     * Clear out potentially dangerous information in a large integer. This 
     * effectively destroys a number so that it cannot be reconstituted
     */
    private void assignZero() {
	if (data != null) {
	    for (int i = 0; i < data.length; i++) data[i] = 0;
	}
	data = null;
    }

    /**  
     * Manufacture a large integer that is some power of 2
     */  
    private static BigInteger pow2(int pwr) {
	BigInteger res = new BigInteger(pwr + 2);
	int newBits;
	
	res.data[res.data.length - (pwr/32) - 1] = 1 << (pwr % 32);
	return res;
    }
 
    /* *********************************
     * * U T I L I T Y   M E T H O D S *
     * *********************************
     * 
     * These methods let you manipulate large integers into and out of streams
     * etc. They are distinct from the numerical methods that come
     * later.
     */

    /**
     * Write a large integer out to a stream. The number is written in canonical
     * form which is high order byte first followed by the rest of
     * of the bytes and then the lsb.
     */
    public void write(OutputStream outStream) throws IOException {
	DataOutputStream dos;

	if (outStream instanceof DataOutputStream)
	    dos = (DataOutputStream) outStream;
	else
	    dos = new DataOutputStream(outStream);

	for (int i = 0; i < data.length; i++) {
	    dos.writeInt(data[i]);
	}
    }

    /**
     * Manufacture a  large integer from an input stream. The first byte is the
     * the high order byte.
     */
    public static BigInteger read(InputStream inStream) {
	BigInteger	res;		// result
	byte		newData[];	// tmp buffer
	ByteArrayOutputStream ou;	// accumulator for data
	int		i = 0;		// counter
	boolean 	done = false;	// loop control

	ou = new ByteArrayOutputStream();
	while (! done) {
	    try {
		i = inStream.read(); // read a byte 
		done =  (i == -1);
	    } catch (IOException e) {
		done = true;
	    }
	    if (! done)
	        ou.write(i);
	}
	newData = ou.toByteArray();
	if (newData == null)	
	    return null;

	return fromByteArray(newData);
    }

    /**
     * Manufacture a large integer of the specified number of bits from the input stream. 
     * if the stream doesn't have enough data it throws an IOException.
     */
    public static BigInteger read(int nbits, InputStream inStream) {
	DataInputStream dis;
	BigInteger	res = new BigInteger(nbits);;
	Vector v = new Vector();
	int	i,j;
	boolean eof = false;

	if (inStream instanceof DataInputStream)
	    dis = (DataInputStream)inStream;
	else
	    dis = new DataInputStream(inStream);

	j = 0;
	for (i = 0; (i < res.data.length) && ! eof; i++) {
	    try {
		j = dis.readInt();
	    } catch (IOException e) {
		eof = true;
	    }
	    if (! eof)
	        v.addElement(new Integer(j));
	}
	if (eof || (v.size() == 0))
	    return null;

	i = 0;
	for (Enumeration e = v.elements(); e.hasMoreElements(); i++) {
	    res.data[i] = ((Integer)(e.nextElement())).intValue();
	}
	return (res);
    }

    /**
     * Return the large integer as an array of bytes. Byte zero is the msb.
     */
    public byte toByteArray()[] {
	ByteArrayOutputStream ou = new ByteArrayOutputStream();

	try {
	    write(ou);
	} catch (IOException e) { } // never happens
	return (ou.toByteArray());
    }


    /**
     * Return the number of bits of precision in this number.
     */
    public int numBits() {
	return (data.length * 32);
    }

    /** 
     * Return the number of octets needed to hold the bits used in 
     * this number. This can be used to compute the impact of writing
     * the number to a stream.
     */
    public int numBytes() {
	return (data.length * 4);
    }

    /* *********************************
     * * N U M E R I C   M E T H O D S *
     * *********************************
     *
     * These methods implement the numeric operations on BigIntegers
     */

    /**
     * Return a large integer that is the negative equivalent of this integer.
     */
    public BigInteger negate() {
	BigInteger result = this.complement();
	int t[];
	int	i;

	for (i = result.data.length - 1 ; i >= 0; i--) {
	    int q = result.data[i];
	    result.data[i]++;
	    if (((q & 0x80000000) ^ (result.data[i] & 0x80000000)) == 0)
		return result; // no overflow, we're done.
	}

	/*
	 * We've made the most negative thing positive.
	 */
	t = new int[result.data.length + 1];
	System.arraycopy(result.data, 0, t, 1, result.data.length);
	t[0] = 0;
	result.data = t;
	return result;
    }

    /**
     * Return the one's complement of this integer.
     */
    public BigInteger complement() {
	int	newData[];

	newData = new int[data.length];
	for (int i = 0; i < newData.length; i++) {
	    newData[i] = ~data[i];
	}
	return (new BigInteger(newData));
    }


    /*
     * Native implementation of multiprecision add.
     */
    private native BigInteger nativeAdd(BigInteger add_value);

    /**
     * Add a BigInteger to this number and return the sum as a new number. 
     * <pre>
     * computes: res = this + aValue
     * </pre>
     */
    public BigInteger add(BigInteger aValue) {
	BigInteger res;
	int	foo[], j;

	res = nativeAdd(aValue);
	return (res);
    }

    /**
     * Add an integer constant to this number. This is more efficient than
     * doing a <code>this.add(BigInteger.fromNumber(integer))</code> as it
     * uses an internal static temporary.
     * <pre>
     * computes: res = this + constant
     * </pre>
     */
    public synchronized BigInteger add(int value) {
	if (intTmp == null) 
	    intTmp = new BigInteger(32);
	intTmp.data[0] = value;
	return add(intTmp);
    }

    /**
     * Add a long integer constant to this number. This is more efficient 
     * than doing a <code>this.add(BigInteger.fromNumber(integer))</code> 
     * as it uses an internal static temporary.
     * <pre>
     * computes: res = this + constant
     * </pre>
     */
    public synchronized BigInteger add(long value) {
	if (longTmp == null)
	    longTmp = new BigInteger(64);
	longTmp.data[0] = (int)(value >>> 32);
	longTmp.data[1] = (int)(value & 0xffffffff);
	return add(longTmp);
    }

    /*
     * Native implementation of multiprecision subtraction
     */
    native BigInteger nativeSub(BigInteger aValue);

    /**
     * Subtract a value from this number and return the difference.
     * <pre>
     * computes: res = this - aValue
     * </pre>
     */
    public BigInteger sub(BigInteger aValue) {
	BigInteger res;

	res = nativeSub(aValue);
	return (res);
    }

    /**
     * Subtract an integer constant from this number.
     * <pre>
     * computes: res = this - constant
     * </pre>
     */
    public synchronized BigInteger sub(int value) {
	if (intTmp == null)
	    intTmp = new BigInteger(32);
	intTmp.data[0] = value;
	return sub(intTmp);
    }

    /**
     * Subtract a long integer constant from this number.
     * <pre>
     * computes: res = this - constant
     * </pre>
     */
    public synchronized BigInteger sub(long value) {
	if (longTmp == null)
	    longTmp = new BigInteger(64);
	longTmp.data[0] = (int)(value >>> 32);
	longTmp.data[1] = (int)(value & 0xffffffff);
	return sub(longTmp);
    }

    /*
     * Native implementation of multiply.
     */
    native BigInteger nativeMul(BigInteger aValue);

    /**
     * Multiply the this number by a value and return the result.
     * <pre>
     * computes: res = this * aValue
     * </pre>
     */
    public BigInteger mul(BigInteger aValue) {
	BigInteger res;

	if (isZero() || aValue.isZero()) {
	    return (BigInteger.fromNumber(0));
	}

	res = nativeMul(aValue);
	return (res);
    }

    /**
     * Multiply by an integer constant.
     * <pre>
     * computes: res = this * C
     * </pre>
     */
    public synchronized BigInteger mul(int aValue) {
	if (intTmp == null)
	    intTmp = new BigInteger(32);
	intTmp.data[0] = aValue;
	return mul(intTmp);
    }

    /**
     * Multiply by a long integer constant.
     * <pre>
     * computes: res = this * C
     * </pre>
     */
    public synchronized BigInteger mul(long value) {
	if (longTmp == null)
	    longTmp = new BigInteger(64);
	longTmp.data[0] = (int)(value >>> 32);
	longTmp.data[1] = (int)(value & 0xffffffff);

	return mul(longTmp);
    }

    /*
     * Native implementation of divide.
     */
    native BigInteger nativeDiv(BigInteger dividend); 

    /*
     * Native implementation of the mod function
     */
    native BigInteger nativeMod(BigInteger modulus);

    /*
     * Native implementation of the remainder function
     */
    native BigInteger nativeRem(BigInteger modulus);

    /**
     * Divide this BigInteger by a value return the result.
     * <pre>
     * computes: res = this / aValue
     * </pre>
     */
    public BigInteger div(BigInteger aValue) throws ArithmeticException {

	if (aValue.isZero()) {
	    throw new ArithmeticException("BigInteger: divide by zero.");
	}

	BigInteger res = nativeDiv(aValue);
	return (res);
    }

    /**
     * Divide this BigInteger by an integer constant.
     * <pre>
     * computes: res = this / C
     * </pre>
     */
    public synchronized BigInteger div(int value) {
	if (intTmp == null)
	    intTmp = new BigInteger(32);
	intTmp.data[0] = value;
	return div(intTmp);
    }

    /**
     * Divide this BigInteger by a long integer constant.
     * <pre>
     * computes: res = this / C
     * </pre>
     */
    public synchronized BigInteger div(long value) {
	if (longTmp == null) 
	    longTmp = new BigInteger(64);
	longTmp.data[0] = (int)(value >>> 32);
	longTmp.data[1] = (int)(value & 0xffffffff);
	return div(longTmp);
    }

    /**
     * Compute the modulus of the division between this
     * BigInteger and the passed value.
     * <pre>
     * computes: res = this mod aValue
     * </pre>
     */
    public BigInteger mod(BigInteger aValue) {

	if (aValue.isZero()) {
	    return BigInteger.fromNumber(0);
	}

	BigInteger res = nativeMod(aValue);
	return (res);
    }

    /**
     * Compute the modulus of the division between this
     * BigInteger and an integer constant.
     * <pre>
     * computes: res = this mod C
     * </pre>
     */
    public synchronized BigInteger mod(int value) {
	if (intTmp == null)
	    intTmp = new BigInteger(32);
	intTmp.data[0] = value;
	return mod(intTmp);
    }

    /**
     * Compute the modulus of the division between this
     * BigInteger and a long integer constant.
     * <pre>
     * computes: res = this mod C
     * </pre>
     */
    public synchronized BigInteger mod(long value) {
	if (longTmp == null)
	    longTmp = new BigInteger(64);
	longTmp.data[0] = (int)(value >>> 32);
	longTmp.data[1] = (int)(value & 0xffffffff);
	return mod(longTmp);
    }

    /**
     * Compute the remainder of the division between this
     * BigInteger and the value passed.
     * <pre>
     * compute res = this % aValue
     * </pre>
     */
    public BigInteger rem(BigInteger aValue) {
	if (aValue.isZero()) {
	    return BigInteger.fromNumber(0);
	}
	return nativeRem(aValue);
    }

    /*
     * Compound operations involving the modulus operator.
     */
    native BigInteger nativeExpMod(BigInteger anExp, BigInteger aModulus);

    /**
     * Compound operation of both exponentiation and a modulus function
     * that takes advantage of the fact that the modulus will keep the
     * total number of bits in the result to be equal to the number of
     * bits in the modulus.
     * <pre>
     * computes: result = (this ^ e) mod m
     * </pre>
     */
    public BigInteger expMod(BigInteger anExp, BigInteger aModulus) {
	BigInteger res = new BigInteger(aModulus.bits);
	boolean negcheck;

	if (anExp.isNeg()) {
	    System.out.println("expMod:: exponent is negative.");
	}
	negcheck = aModulus.isNeg();

	res = nativeExpMod(anExp, aModulus);
	return (res);
    }

    native BigInteger nativeModInv(BigInteger aValue);


    /**
     * Another compound operation.
     * <pre>
     * computes; result = 1/this mod m
     * </pre>
     */
    public BigInteger modInv(BigInteger aValue) {

	BigInteger res = nativeModInv(aValue);
	return (res);
    }

    /**
     * Compute the greatest common divisor between this BigInteger and
     * the BigInteger in 'aNumber.'
     */
    public BigInteger GCD(BigInteger aNumber) {
	return (BigInteger.GCD(this, aNumber));
    }

    /**
     * Compute the greatest common denominator of the two integers and
     * returns a new BigInteger with the result.
     */
    public static BigInteger GCD(BigInteger a, BigInteger b) {

	BigInteger u = a;
	BigInteger v = b;
        BigInteger t;
	while (!v.isZero()) {
	    t = u.mod(v);
	    u = v;
	    v = t;
	}
	return u;
    }

    /**
     * Return the absolute value of this BigInteger.
     */
    public BigInteger abs() {
	if (! isNeg())
	    return this;
	return negate();
    }

    /**
     * Return the smaller of two BigIntegers, this or that.
     */
    public BigInteger min(BigInteger that) {
	return ((cmp(that) > 0) ? this : that);
    }

    /**
     * Return the larger of two BigIntegers, this or that.
     */
    public BigInteger max(BigInteger that) {
	return ((cmp(that) < 0) ? this : that);
    }

    /**
     * Returns true if this BigInteger contains a negative value.
     */
    public boolean isNeg() {
	return (data[0] < 0);
    }


    /**
     * Return true if this BigInteger is equal to zero.
     */
    public boolean isZero() {
	return ((data == null) || ((data.length == 1) && (data[0] == 0)));
    }

    /**
     * compare the passed number to this number and return -1, 0, 1
     * based on its value. 
     * <pre>
     * computes:
     *		(aNumber > this) ---> 1
     *		(aNumber = this) ---> 0
     *		(aNumber < this) ---> -1
     * </pre>
     */
    public int cmp(BigInteger aNumber) {
	int i, j;

	i = data.length;
	j = aNumber.data.length;
	
	if (isNeg()) {
	    if (! aNumber.isNeg()) // this negative is less than that positive
		return 1;
	    if (i > j)		// more bits means more negative
		return 1;
	    if (i < j)		// fewer bits, means less negative
		return -1;
	    // at this point they are both negative and both the same # of bits
	    for (int k = 0; k < data.length; k++) {
	        int this_data = data[k];
	        int aNumber_data = aNumber.data[k];
	        if (this_data != aNumber_data) {
		    if ((this_data ^ aNumber_data) < 0) 
		        // opposite signs.  The one that looks like a negative 
		        // number is smaller.
		        return (aNumber_data < 0) ? -1 : 1;
		    else 
		        return (aNumber_data > this_data) ? -1 : 1;
		}
	    }
	} else {
	    if (aNumber.isNeg()) // this positive > that negative
		return -1;
	    if (i > j)		// more bits means more positive
		return -1;
	    if (i < j)		// fewer bits, means less positive
		return 1;
	    // at this point they are both positive and both the same # of bits
	    for (int k = 0; k < data.length; k++) {
	        int this_data = data[k];
	        int aNumber_data = aNumber.data[k];
	        if (this_data != aNumber_data) {
		    if ((this_data ^ aNumber_data) < 0) 
		        // opposite signs.  The one that looks like a negative 
		        // number is larger.
		        return (aNumber_data < 0) ? 1 : -1;
		    else 
		        return (aNumber_data > this_data) ? 1 : -1;
		}
	    }
	}
	return (0);
    }

    /**
     * Returns true if this BigInteger is equal to the BigInteger in 'aValue'.
     */
    public boolean isEqual(BigInteger aValue) {
	return (cmp(aValue) == 0);
    }

    /**
     * Returns true if this BigInteger is equal to the integer 'aValue'.
     */
    public boolean isEqual(int aValue) {
	return (this.intValue() == aValue);
    }

    /**
     * Returns true if this BigInteger is equal to long 'aValue'.
     */
    public boolean isEqual(long aValue) {
	return (this.longValue() == aValue);
    }

    /**
     * Returns true if BigInteger a is even.
     */
    public static boolean isEven(BigInteger a) {
	return ((a.data[a.data.length - 1] & 1) == 0);
    }

    /**
     * Returns true if this BigInteger is even.
     */
    public boolean isEven() {
	return (BigInteger.isEven(this));
    }

    /* *****************************************
     * * M E T H O D S   F R O M   N U M B E R *
     * *****************************************
     * 
     * This section contains implementations of the abstract methods 
     * that the Number class requires.
     */

    /**
     * Returns the value of this BigInteger as an int if it can.
     * @exception BigIntegerOverflowException is thrown if it is too large.
     */
    public int intValue() {
	if (data.length > 1) {
	    for (int i = 0 ; i < data.length - 1; i++)
	   	if (! ((data[i] == 0) || (data[i] == -1)))
		    return (isNeg() ? Integer.MIN_VALUE : Integer.MAX_VALUE);
	    return (data[data.length - 1]);
	}
	return (data[0]);
    }

    /**
     * Returns the value of this BigInteger as a long if it can.
     * @exception BigIntegerOverflowException is thrown if it is too large.
     */
    public long longValue() {
	long	result;

	if (data.length > 2) {
	    for (int i = 0 ; i < data.length - 2; i++) 
	   	if (! ((data[i] == 0) || (data[i] == -1)))
		    return (isNeg() ? Long.MIN_VALUE :  Long.MAX_VALUE);
	}
	switch (data.length) {
	    case 1: return ((long) data[0]);
	    case 2: return ((((long)data[0]) << 32L) + (long)data[1]);
	   default: return Long.MAX_VALUE;
	}
    }

    /**
     * Returns the value of this BigInteger as a float. In the conversion
     * precision is often lost.
     */
    public float floatValue() {
	int	a;
	float	t, e;
	// XXX CHECK THIS

	t = 0;
	e = 1;
	for (a = 0; a < data.length; a++) {
	    t = t + ((float)((data[a]) & 0xffff) * e);
	    e = e * 65536.0f;
	    t = t + ((float)((data[a] >> 16) & 0xffff) * e);
	    e = e * 65536.0f;
	}
	if (data[data.length - 1] < 0) {
	    t = t * -1.0f;
	}
	return (t);
    }

    /**
     * Returns the value of this BigInteger as a double. In the conversion
     * precision may be lost.
     */
    public double doubleValue() {
	int	a;
	double	t, e;
	// XXX CHECK THIS

	t = 0;
	e = 1;
	for (a = data.length - 1; a >= 0; a--) {
	    t = t + ((double)((data[a]) & 0xffff) * e);
	    e = e * 65536.0;
	    t = t + ((double)((data[a] >> 16) & 0xffff) * e);
	    e = e * 65536.0;
	}
	if (data[data.length - 1] < 0) {
	    t = t * -1.0;
	}
	return (t);
    }

    final private static BigInteger printMagic = BigInteger.fromNumber(1000000000);

    /**
     * Returns the value of this BigInteger as a hexadecimal string.
     */
    public String toHexString() {
	StringBuffer x = new StringBuffer();

	for (int i = 0; i < data.length; i++) {
	    for (int j = 7; j >= 0; j--) {
		char digit;
		int  val;
		val = (data[i] >>> (j * 4)) & 0xf;
		digit = (char) ((val > 9) ? 'A' + (val - 10) : '0' + val);
		x.append(digit);
	    }
	}
	return (x.toString());
    }

    /**
     * Returns the value of this BigInteger as a decimal string.
     */
    public String toString() {
	BigInteger me = this;
	BigInteger quotient, remainder;
	// each 32bit data can produce slightly more than one 10^9 digit
	int values[] = new int[data.length * 2]; 
	// repeatedly divide by 10^9 and save the remainders, until nothing
	// is left.  Make sure we do the loop at least once.
	int i = 0;
	do {
	    quotient = me.div(printMagic);
	    remainder = me.rem(printMagic);
	    values[i++] = Math.abs(remainder.intValue());
	    me = quotient;
	} while (!me.isZero());

	// hold the digits, as we generate them.
	StringBuffer buf = new StringBuffer(data.length * 10);

	// get the remainder from final divide.  Don't zero pad them
	if (this.isNeg()) {
	    buf.append('-');
	}
	buf.append(values[--i]); 
	while (--i >= 0) {
	    // get all the other remainders.  Zero pad to 9 digits
	    String s = String.valueOf(values[i]);
	    for (int j = s.length(); j < 9; j++) 
		buf.append("0");
	    buf.append(s);
	}
	return buf.toString();
    }

    /**
     * Returns the number of binary 0's at the end of the string, i.e. the
     * maximum power of two by which this number can be evenly divided.
     *
     * Return "-1" on 0.  Is there some better value?
     */


    private int maxPowerTwo() {
	int words = data.length;
	int result = 0;

	for (int word = words-1; word >= 0; word--, result += 32) {
	    int value = data[word];
	    if (value != 0) {
		for (int bit = 0; ; bit++) {
		    if ((value & (1 << bit)) != 0) 
			return (result + bit);
		}
	    }
	}
	return -1;
    }

    /** 
     * Shift this large integer right by the indicated number of bits, and return the
     * result as a new large integer. The shift is logical or arithmetic, depending 
     * on isArithmetic <p>
     *
     * Since the precision of the integer varies with the number it is
     * representing, the isArithmetic flag simply insures that if the number
     * was negative before shifting, it is still negative after shifting.
     */
    public BigInteger rightShift(int shift, boolean isArithmetic) {
	int offset = 0;
	int words = data.length;
	int shift_words = shift >>> 5;
	int shift_bits = shift & 0x1f;
	BigInteger result;

	int carry = (isArithmetic && data[0] < 0) ? -1 : 0;

	// Check to see if shifting will open up a "free" word
 
	if (data.length > 1) {
	    // Check to see if MSB is all sign bit ...
	    if ((data[0] == 0) || (isArithmetic && (data[0] == -1)))
		offset = 1;
	}

	result = new BigInteger((data.length - shift_words - offset) * 32);
	// On arithmetic shifts, the sign might propogate into the 
	// significant bits of the first word of the result.
	carry = (shift_bits > 0) ? (carry << (32-shift_bits)) : 0;

	for (int i = offset; i < words - shift_words; i++) {
	    result.data[i-offset] = (data[i] >>> shift_bits) + carry;
	    if (shift_bits > 0)
		carry = data[i] << (32-shift_bits); // else, carry always 0;
	}

        return result;
    }

    /**
     * Shift this large integer left by the indicated number of bits, and return the
     * result.
     */

    public BigInteger leftShift(int shift) {
	int offset = 0;
	int words = data.length;
	int shift_words = shift >> 5;
	int shift_bits = shift & 0x1F;
	int shift_mask;
	int carry_mask = (0x80000000 >> (shift_bits - 1));
	int newData[];

	int carry = data[0] >> (32 - shift_bits);

	/*
	 * Don't let the left shift change the "sign" of the number
	 */
	int top = carry_mask & data[0]; // these bits will be lost

	int q, r;
	q = data[0] >> (32 - shift_bits);
	r = data[0] << shift_bits;
	if (q == 0) {
	    if ((r & 0x80000000) != 0) {
	 	// add zeros byte
		newData = new int[data.length + shift_words + 1];
		newData[0] = 0;
		offset = 1;
	    } else {
		newData = new int[data.length + shift_words];
		offset = 0;
	    }
	} else if (q == -1) {
	    if ((r & 0x80000000) == 0) {
		// add -1 byte
		newData = new int[data.length + shift_words + 1];
		newData[0] = -1;
		offset = 1;
	    } else {
		newData = new int[data.length + shift_words];
		offset = 0;
	    }
	} else {
	    newData = new int[data.length + shift_words + 1];
	    newData[0] = q;
	    offset = 1;
	}

	// The low words are always zero
	for (int i = data.length; i < newData.length; i++) 
	    newData[i] = 0;

	carry = 0;
	for (int i = 0 ; i < data.length;  i++) {
	    if (shift_bits > 0) {
	        carry = (i == (data.length - 1)) ? 0 : 
				data[i+1] >>> (32 - shift_bits);
	    }
	    newData[i+offset] = (data[i] << shift_bits) | carry;
	}
	return new BigInteger(newData);
    }


    /* some potential factors for the number */
    final static int some_primes[] = { 3, 5, 7, 11, 13, 17 };

    /**
     * Returns true if the number in this BigInteger is believed to be
     * prime.
     *
     * This method uses two techniques to check for primality, the
     * first is to try dividing the number by some small primes.
     * If any of these are successful, obviously the number isn't prime.
     */
    private boolean isPrime() {
	BigInteger x, y = null;
	try {
	    for (int i = 0; i < some_primes.length; i++) {
		y = mod(some_primes[i]);
		if (y.isZero()) {
		    return false; /* this is a factor of a */
		}
	    }
	    // Perform fifty probabilist tests on this number
	    for (int i = 100; i < 150; i++) {
		x = BigInteger.fromNumber(i);
		if (!onePrimeTest(x))
		    return false;
	    }
	    return true;
	} finally {
	    // don't leave any trails behind
	    if (y != null) y.assignZero();
	}
    }

    /* Perform a single probabilistic prime test on this number.  We calculate
     * (random ^ (n - 1)) (mod n) by setting n-1 = q * 2 ^ power2, and then
     * calculating (random ^ q), (random ^ 2q), .... (random ^ (n-1)).
     *
     * If "this" is prime then:
     *     (random ^ (n-1)) == 1 (mod n) by Fermat's little theorem
     *     The only square roots of 1 (mod n) are 1 and this - 1.  So if
     *        we hit "1" without having hit "this - 1" on the previous step, 
     *        then "this" can't be a prime.
     * Note that we only need to calculate up to (random ^ (n-1)/2), since  if
     * we haven't yet hit "this - 1", then the number can't be prime:  If we get
     * "1" on the next step, we fail the square root test;  If we don't get
     * "1" on the next step, we fail Fermat's little theorem.
     */
    private boolean onePrimeTest(BigInteger rndValue) {
	BigInteger n_minus_1 = sub(1);
	BigInteger one = BigInteger.fromNumber(1);
	BigInteger two = BigInteger.fromNumber(2);
	int power2 = n_minus_1.maxPowerTwo();
	BigInteger q = n_minus_1.rightShift(power2, false);
	BigInteger result = rndValue.expMod(q, this); // random ^ q (mod this)
	try {
	    if (result.isEqual(one) || result.isEqual(n_minus_1))
		return true;
	    for (int i = 0; i < power2 - 1; i++) {
		// we do this slightly awkwardly in order not to leave a trail
		BigInteger temp  = result.expMod(two, this);
		result.assignZero();
		result = temp;
		if (result.isEqual(one)) 
		    // We've got 1, but we didn't have -1 on previous iteration
		    return false;
		if (result.isEqual(n_minus_1))
		    // we'll hit 1 on the next iteration.  Fermat is happy
		    return true;
	    }
	    return false;
	} finally {
	    // make sure we don't leave a mess
	    n_minus_1.assignZero();
	    q.assignZero();
	    result.assignZero();
	    power2 = 0;
	}
    }


    /**
     * The finalize methods will clear out the value in the number
     * prior to the number being collected. This is extra insurance
     * that BigIntegers with "sensitive" information will not lie
     * around on the heap.
     */
    public void finalize() {
	assignZero();
	data = null;
    }

//    This version of GCD is better when we're taking the GCD of numbers of
//    comparable size.  However for RSA, we're almost always taking the GCD
//    of a really big number with a really small number.
//
//    public static BigInteger GCD(BigInteger a, BigInteger b) {
//	BigInteger u, v, t;
//	int bits;
//
//	// Reduce a and b by powers of two until both are odd
//	int a_power = a.maxPowerTwo();
//	int b_power = b.maxPowerTwo();
//	int max_bits = Math.max(a.bits, b.bits);
//	BigInteger temp = new BigInteger(max_bits);
//
//	a = a.rightShift(new BigInteger(max_bits), a_power, false);
//	b = b.rightShift(new BigInteger(max_bits), b_power, false);
//
//	try {
//	    for (;;) {
//		// both a and b must be odd at this point
//		switch(a.cmp(b)) {
//		case 0:
//		    // a == b;  result = a << min(a_power, b_power)
//		    int min_power = Math.min(a_power, b_power);
//		    if (min_power != 0) 
//			a.leftShift(b, min_power);
//		    return b;
//		case 1:		// a < b
//		    // a < b.  gcd(a, b) = gcd(a, b-a).  Since b-a is even, 
//		    // but a is odd, we can cast out powers of two from b-a
//		    // without affecting the result.
//		    b.nativeSub(temp, a); // temp = b - a
//		    temp.rightShift(b, temp.maxPowerTwo(), false);
//		    break;
//		case -1:	// b < a
//		    // b < a.  gcd(a, b) = gcd(a-b, b).  Since a-b is even, 
//		    // but b is odd, we can cast out powers of two from a-b
//		    // without affecting the result.
//		    a.nativeSub(temp, b); // temp = a - b
//		    temp.rightShift(a, temp.maxPowerTwo(), false);
//		} // switch statement		
//	    } // for statement
//	} finally {
//	    // don't clear b.  It's the result.
//	    a.assignZero();
//	    temp.assignZero();
//	}
//    }

}
