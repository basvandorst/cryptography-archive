/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package java.math;

/**
 * this is a psudeo random number generator that will create
 * psudeo random 20 byte arrays.
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * @version     1.00, 01/07/1995
 * @author      Systemics Ltd. (Michael Wynn)
 */
public final class PRNG
{
	// N.B. the >> operator is not used because it sign shifts 
	// where as the >>> operator zero shifts.
	private final static int mask0 = 0x80000001;
	private final static int mask1 = 0;
	private final static int mask2 = 0;
	private final static int mask3 = 0;
	private final static int mask4 = 0x80000000;

	int i0, i1, i2, i3, i4;

	/**
	 * Creates a psudeo random number generator from a default seed
	 */
	public PRNG()
	{
		i0 = i1 = i2 = i3 = 0;
		i4 = 0x1000000;
	}

	/**
	 * Creates a psudeo random number generator from a seed
	 * @param b the seed for this generator
	 * @exception MathError if the seed is less that 20 bytes.
	 */
	public PRNG( byte b[] )
	{
		if ( b.length < 20 )
			throw new MathError( "The seed byte array must be at least twenty bytes." );
		i4=b[3]|(b[2]<<8)|(b[1]<<16)|(b[0]<<24);
		i3=b[7]|(b[6]<<8)|(b[5]<<16)|(b[4]<<24);
		i2=b[11]|(b[10]<<8)|(b[9]<<16)|(b[8]<<24);
		i1=b[15]|(b[14]<<8)|(b[13]<<16)|(b[12]<<24);
		i0=b[19]|(b[18]<<8)|(b[17]<<16)|(b[16]<<24);
		if ( ( i0 | i1 | i2 | i3 | i4 ) == 0 )
			i4=0x1000000; // must not start with all bits zero.
	}
	
	/**
	 * Creates a psudeo random number generator from a seed
	 * @return a twenty byte array representing the current state of the generator
	 */
	public synchronized byte[]
	toByteArray()
	{
		byte b[]=new byte[20];
		b[3]=(byte)i4; b[2]=(byte)(i4>>>8); b[1]=(byte)(i4>>>16); b[0]=(byte)(i4>>>24);
		b[7]=(byte)i3; b[6]=(byte)(i3>>>8); b[5]=(byte)(i3>>>16); b[4]=(byte)(i3>>>24);
		b[11]=(byte)i2; b[10]=(byte)(i2>>>8); b[9]=(byte)(i2>>>16); b[8]=(byte)(i2>>>24);
		b[15]=(byte)i1; b[14]=(byte)(i1>>>8); b[13]=(byte)(i1>>>16); b[12]=(byte)(i1>>>24);
		b[19]=(byte)i0; b[18]=(byte)(i0>>>8); b[17]=(byte)(i0>>>16); b[16]=(byte)(i0>>>24);
		return b;
	}
	
	/**
	 * moves the internal state on one cycle.
	 */
	public synchronized void
	clock()
	{
		if ((i0 & 0x00000001)==0)
		{
			i0 = ( ( i0 >>> 1 ) | ( ( ( i1 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i1 = ( ( i1 >>> 1 ) | ( ( ( i2 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i2 = ( ( i2 >>> 1 ) | ( ( ( i3 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i3 = ( ( i3 >>> 1 ) | ( ( ( i4 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i4 >>>= 1;
		}
		else
		{
			i0 ^= mask0;
			i1 ^= mask1;
			i2 ^= mask2;
			i3 ^= mask3;
			i4 ^= mask4;
			
			i0 = ( ( i0 >>> 1 ) | ( ( ( i1 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i1 = ( ( i1 >>> 1 ) | ( ( ( i2 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i2 = ( ( i2 >>> 1 ) | ( ( ( i3 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i3 = ( ( i3 >>> 1 ) | ( ( ( i4 & 0x00000001 )==0 ) ? 0 : 0x80000000 ) );
			i4 = ( ( i4 >>> 1 ) | 0x80000000 );
		}
	}
}

