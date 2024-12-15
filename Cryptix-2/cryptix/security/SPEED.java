/*
 * Copyright (c) 1997 Systemics Ltd on behalf of the Cryptix development team.
 * All rights reserved.
 */

package cryptix.security;
import java.io.PrintStream;

/**
 * SPEED is a block cipher with variable key size, data block
 * size and number of rounds (in the style of RC5).
 * <P>
 * This implementation has the key schedule and all variables set
 * in class instantiation:
 * <UL>
 *   <LI>
 *     The key size is taken from the key passed to the constructor,
 *     and can be any even number
 *     of bytes from 6 to 32 (6, 8, 10, ...32).
 *   </LI>
 *   <LI>
 *     The length of the data block defaults to 8 bytes
 *     (c.f., IDEA and DES).
 *     It can be set by calling the overloaded constructor.
 *     It can be 8, 16, or 32 bytes.
 *   </LI>
 *   <LI>
 *     The number of rounds
 *     can be any number from 32 upwards, divisible by 4 (32, 36, ...).
 *     It is defaults to 64 (which gives 'adequate' security, see paper below)
 *     or set by calling the variable constructor.
 *   </LI>
 * </UL>
 * <P>
 * No linked libraries currently supported.
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <P>
 * <bold>Algorithm:</bold>
 *	<a href="mailto:yzheng@fcit.monash.edu.au">Yuliang Zheng</a>
 *	(public domain).  See
 *	<a href="http://pscit-www.fcit.monash.edu.au:/~yuliang/">Y. Zheng</a>
 *	"The SPEED Cipher,"
 *	<cite>Proceedings of Financial Cryptography 97</cite>,
 *	Springer-Verlag (forthcoming).
 *	FC97 held at Anguilla, BWI, 24-28 February 1997.
 *
 * <p>
 * @author Systemics Ltd
 */

public final class SPEED extends BlockCipher
{
	private static final String LIBRARY_NAME = "speed";

	///
	// Defaults are set to the same as IDEA, e.g. 16 key and 8 data bytes.
	// However, the real key length is taken from the key provided.
	///
	private int key_length = 16;	// in bytes
	private int num_rounds = 64;	// 'adequate' security, c.f. paper
	private int block_length = 8;	// in bytes

	///
	// Note length refers to bytes, size to bits ... for now at least.
	// Other names are taken from the C reference code.
	///
	private int key_size;		// is key_length * 8
	private int data_size;		// is block_length * 8
	private int s0, s1, s2;		// taken from sqrt(15) and key_length
	private int f_wd_len;
	private int h_wd_len;
	private int f_wd_mask;		// long ?? if data_size == 256
	private int h_wd_mask;
	private int v_shift;
	private int kb_size;
	private int[] round_key;	// the internal key buffer
	private int key_len_dbyte;


	/**
	 * Return data block length of this cipher.
	 * Default (8 bytes) is returned before instantiation,
	 * actual length used by object returned after instantiation.
	 * @return block length in bytes.
	 */
	public int blockLength() { return block_length; }
	/**
	 * Return the maximum block length.
	 * @return the maximum block length in bytes.
	 */
	public int blockLengthMax() { return 32; }
	/**
	 * Return the minimum block length.
	 * @return the minimum block length in bytes.
	 */
	public int blockLengthMin() { return 8; }
	/**
	 * Return the modulo block length.
	 * @return the modulo block length in bytes.
	 */
	public int blockLengthMod() { return 8; }

	/**
	 * Return key length of this cipher.
	 * Default is returned before instantiation, actual returned after.
	 * @return the key length in bytes.
	 */
	public int keyLength() { return key_length; }
	/**
	 * Return the maximum key length.
	 * @return the maximum key length.
	 */
	public int keyLengthMax() { return 32; }
	/**
	 * Return the minimum key length.
	 * @return the minimum key length.
	 */
	public int keyLengthMin() { return 6; }
	/**
	 * Return the modulo key length.
	 * @return the modulo key length.
	 */
	public int keyLengthMod() { return 2; }

	/**
	 * Return number of rounds of this cipher
	 * Default is returned before instantiation, actual returned after.
	 * @return the number of rounds.
	 */
	public int numRounds() { return num_rounds; }
	/**
	 * Return the maximum number of rounds.
	 * @return the maximum number of rounds.
	 */
	public int numRoundsMax() { return (int)(0x090000000 - 4); }
	/**
	 * Return the minimum number of rounds.
	 * @return the minimum number of rounds.
	 */
	public int numRoundsMin() { return 32; }
	/**
	 * Return the modulo number of rounds.
	 * @return the modulo number of rounds.
	 */
	public int numRoundsMod() { return 4; }




	//
	// These should be longs, but for now try ints,
	// since Java doesn't have unsigned, and int is the optimised
	// or best unit.
	// Note that as ints, they might give trouble with 256 bit
	// data blocks, as the full 32 bits are used.
	//
	private int ks[];
	private int dks[] = null;

	
	/**
	 * Create a default SPEED block cipher from a key in a byte array.
	 * The key length is taken from the key provided and should
	 * be one of 6, 8, ..., 32.
	 * Default block size (8) and number of rounds (64) are set
	 * to approximate IDEA or DES, allowing for drop-in replacement.
	 * @param userKey    the user key (length is variable).
	 */
	public SPEED( byte userKey[] ) 
	{
		this(userKey, 8, 64);
	}

	/**
	 * Create a SPEED block cipher from a key, setting data length and
	 * number of rounds.
	 * These are recommended settings for 'adequate' security.
	 * <pre>
	 *  +--------------------------------------------------+
	 *  |   blockLength  |   keyLength   |   numRounds     |
	 *  |==================================================|
	 *  |       8        |     >= 8      |     >= 64       |
	 *  |--------------------------------------------------|
	 *  |      16        |     >= 8      |     >= 48       |
	 *  |--------------------------------------------------|
	 *  |      32        |     >= 8      |     >= 48       |
	 *  +--------------------------------------------------+
	 * </pre>
	 * see paper above.
	 *
	 * @param userKey      the user key, length can be 6, 8, ... 32.
	 * @param blockLength  length of the data block in bytes, 8, 16, 32.
	 * @param numRounds    number of rounds to apply, 32, 36 ... %4==0;
	 */
	public SPEED( byte userKey[], int blockLength, int numRounds ) 
	{
		this.block_length = blockLength;
		num_rounds = numRounds;
		//
		// This algorithm has quite a complex setup, so for
		// now, ignore native linking.
		//
		set_constants(userKey.length);

//		System.out.println(" the key");
//		for (int i = 0 ; i < userKey.length ; i++)
//			System.out.print(" " + Integer.toString(userKey[i], 16));
		key_schedule(userKey);
	}

	///
	//	Work out all the various constant choices.
	///
	private void set_constants(int key_length)
	{
		this.key_length = key_length;
		key_size = key_length * 8;
		key_len_dbyte = key_length / 2;
		set_sqrt_15(key_size);

		data_size = block_length * 8;
		f_wd_len = data_size / 8;	// ???  SPEED word in bits
		h_wd_len = f_wd_len / 2;	// half a SPEED word in bits

		switch (data_size)
		{
		case 256:
			f_wd_mask = 0xFFFFFFFF;
			h_wd_mask = 0xFFFF;
			v_shift = 11;
			kb_size = 2 * num_rounds;
			break ;

		case 128:
			f_wd_mask = 0xFFFF;
			h_wd_mask = 0xFF;
			v_shift = 4;
			kb_size = num_rounds;
			break ;

		case 64:
			f_wd_mask = 0xFF;
			h_wd_mask = 0xF;
			v_shift = 1;
			kb_size = num_rounds / 2;
			break ;

		default:
			throw new CryptoError( "SPEED: " + data_size + " is bad data size (not 64/128/256)" );
		}
	}


	///
	//	Three constants are taken from square root of 15,
	//	with 3 different ones for each different key length.
	//	Also checks validity of key length here.
	///
	private void set_sqrt_15(int size)
	{
		/* fractional part of sqrt(15), used in key scheduling */
		switch (size)
		{
		case 48:    s0 = 0xDF7B; s1 = 0xD629; s2 = 0xE9DB; return;
		case 64:    s0 = 0x362F; s1 = 0x5D00; s2 = 0xF20F; return;
		case 80:    s0 = 0xC3D1; s1 = 0x1FD2; s2 = 0x589B; return;
		case 96:    s0 = 0x4312; s1 = 0x91EB; s2 = 0x718E; return;
		case 112:   s0 = 0xBF2A; s1 = 0x1E7D; s2 = 0xB257; return;
		case 128:   s0 = 0x77A6; s1 = 0x1654; s2 = 0x6B2A; return;
		case 144:   s0 = 0x0D9B; s1 = 0xA9D3; s2 = 0x668F; return;
		case 160:   s0 = 0x19BE; s1 = 0xF855; s2 = 0x6D98; return;
		case 176:   s0 = 0x022D; s1 = 0xE4E2; s2 = 0xD017; return;
		case 192:   s0 = 0xEA2F; s1 = 0x7572; s2 = 0xC3B5; return;
		case 208:   s0 = 0x1086; s1 = 0x480C; s2 = 0x3AA6; return;
		case 224:   s0 = 0x9CA0; s1 = 0x98F7; s2 = 0xD0E4; return;
		case 240:   s0 = 0x253C; s1 = 0xC901; s2 = 0x55F3; return;
		case 256:   s0 = 0x9BF4; s1 = 0xF659; s2 = 0xD76C; return;

		default:
			throw new CryptoError( "SPEED: " + size + " is bad key length (not 48 .. 256 % 16)" );
		}
	}

	///
	//	Expand user key to full internal buffer.
	///
	private int[] kb;	// scheduling temp buffer, for dump of data
	private void key_schedule(byte userKey[])
	{
//		int i, j;
		kb = new int[ kb_size ];	// scheduling temp buffer

		round_key = new int[ num_rounds ];	// real internal key

		//
		// Copy the key into the double-byte temporary buffer
		//
		for(int i = 0; i < key_len_dbyte; i++)
		{
			kb[i] = userKey[ 2*i ] | userKey[ 2*i + 1 ] << 8;
//			System.out.print( " " +  Integer.toString(kb[i], 16));
		}

		//
		// Fill out the buffer from earlier parts of the key
		//
		for(int i = key_len_dbyte; i < kb_size; i++)
		{
			int t = (s2 & s1) ^ (s1 & s0) ^ (s0 & s2);
			t = (t << 5) | (t >>> 11);
			t += s2 + kb[ i % key_len_dbyte ];
			t &= 0xFFFF;
			s2 = s1;
			s1 = s0;
			s0 = kb[i] = t;
		}

		//
		// Transfer the double-byte temporary key into the real key
		//
		// System.out.println( "kb_size "+kb_size+" kb.l "+kb.length+" rk.l "+round_key.length);
		switch (data_size)
		{
		case 256:
			for (int i = 0; i < kb_size / 2; i++)
				round_key[i] = kb[2*i] | (kb[2*i + 1] << 16);
			break;

		case 128:
			for (int i = 0; i < kb_size; i++)
				round_key[i] = kb[i];
			break;

		case 64:
			for (int i = 0; i < kb_size; i++)
			{
				round_key[2*i]   = kb[i] & 0xFF;
				round_key[2*i+1] = (kb[i] >>> 8) & 0xFF;
			}
			break;

		default:
			throw new CryptoError( "SPEED: " + data_size + " illegal in key_schedule?");
		}
	}

	void dump()
	{
		if (0 == data_size)
		{
			System.out.println( "no data set yet" );
			return ;
		}

		System.out.println("KEY SCHEDULE");
		System.out.println(" data_size " + data_size );
		System.out.println(" kb_size " + kb_size );
		System.out.println(" kb.length " + kb.length );
		System.out.println(" f_wd_mask "
			+ Integer.toString(f_wd_mask, 16) );
		System.out.println(" h_wd_mask " + Integer.toString(h_wd_mask, 16) );
		System.out.println(" v_shift " + v_shift );

		System.out.println(" double byte buffer");
		for(int i = 0; i < key_len_dbyte; i++)
		{
			System.out.print( " " +  Integer.toString(kb[i], 16));
		}
		System.out.println();

		switch (data_size)
		{
		case 256:
			for (int i = 0; i < kb_size / 2; i++)
				System.out.print( " " + Integer.toString((0xFFFFFFFF & round_key[i]), 16) );
			break;

		case 128:
			for (int i = 0; i < kb_size; i++)
				System.out.print( " " + Integer.toString((0xFFFF & round_key[i]), 16) );
			break;

		case 64:
			for (int i = 0; i < kb_size * 2; i++)
			{
				System.out.print( " " + Integer.toString((0xFF & round_key[i]), 16) );
			}
			break;

		default:
			throw new CryptoError( "SPEED: " + data_size + " illegal in key_schedule?");
		}
		System.out.println();
	}

	private void to_internal(byte in[], int offset, int buf[])
	{
		//
		// Translate bytes into SPEED internal words.
		//
		switch (data_size)
		{
		case 256:
			for (int i = 0; i < 8; i++)
			{
				buf[i] =
				      ((in[offset + 4*i]        ) & 0xFF)
				    | ((in[offset + 4*i+1] <<  8) & 0xFF00)
				    | ((in[offset + 4*i+2] << 16) & 0xFF0000)
				    | ((in[offset + 4*i+3] << 24) & 0xFF000000);
			}
			break;

		case 128:
			// System.out.println( "TO 128" );
			for (int i = 0; i < 8; i++)
			{
				buf[i] =
					((in[offset + 2*i]        ) & 0xFF)
				      | ((in[offset + 2*i+1] <<  8) & 0xFF00);
				//System.out.print( " " +
				//Integer.toString((0xFFFFFFFF & buf[i]), 16) );
			}
			// System.out.println();
			// System.out.println( "-----" );
			break;

		case 64:
			for (int i = 0; i < 8; i++)
			{
				buf[i] = (in[offset + i] & 0xFF);
			}
			break;

		default:
			throw new CryptoError( "SPEED: " + data_size + " illegal in key_schedule?");
		}
	}

	//
	// Encrypt a block.
	// The in and out buffers can be the same.
	// @param in The data to be encrypted.
	// @param in_offset   The start of data within the in buffer.
	// @param out The encrypted data.
	// @param out_offset  The start of data within the out buffer.
	//
	protected void 
	blockEncrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		int[] big_in = new int[8];		// long ??
		int[] big_out = new int[8];		// long ??

		to_internal(in, in_offset, big_in);
		encrypt(big_in, big_out);
		from_internal(big_out, out, out_offset);
	}

	private void from_internal(int buf[], byte out[], int offset)
	{
		//
		// Translate internal words back into bytes.
		//
		switch (data_size)
		{
		case 256:
			for (int i = 0; i < 8; i++)
			{
				out[4*i   + offset] = (byte)( buf[i]         & 0xFF);
				out[4*i+1 + offset] = (byte)((buf[i] >>>  8) & 0xFF);
				out[4*i+2 + offset] = (byte)((buf[i] >>> 16) & 0xFF);
				out[4*i+3 + offset] = (byte)((buf[i] >>> 24) & 0xFF);
			}
			break;

		case 128:
			for (int i = 0; i < 8; i++)
			{
				out[2*i   + offset] = (byte)( buf[i]         & 0xFF);
				out[2*i+1 + offset] = (byte)((buf[i] >>>  8) & 0xFF);
			}
			break;

		case 64:
			for (int i = 0; i < 8; i++)
			{
				out[i + offset] = (byte)(buf[i] & 0xFF);
			}
			break;

		default:
			throw new CryptoError( "SPEED: " + data_size + " illegal in key_schedule?");
		}
	}


	//
	// Decrypt a block.
	// The in and out buffers can be the same.
	// @param in The data to be decrypted.
	// @param in_offset   The start of data within the in buffer.
	// @param out The decrypted data.
	// @param out_offset  The start of data within the out buffer.
	//
	protected void
	blockDecrypt( byte in[], int in_offset, byte out[], int out_offset )
	{
		//
		// Translate bytes into SPEED internal words.
		//
		int[] big_in  = new int[8];		// long ??
		int[] big_out = new int[8];		// long ??

		to_internal(in, in_offset, big_in);
		decrypt(big_in, big_out);
		from_internal(big_out, out, out_offset);
	}



	///
	//	Encryption and decryption
	///
	private void encrypt(int in[], int out[])
	{
		int	t0 = in[0], 
			t1 = in[1],
			t2 = in[2],
			t3 = in[3],
			t4 = in[4],
			t5 = in[5],
			t6 = in[6],
			t7 = in[7];

		int k = 0;   /* points to the first round key */
		int quarter_rounds = num_rounds / 4;

		//
		// In the following 4 passes, only the first assignment
		// changes, which is the nonlinear function.
		//
		/* Pass 1 uses FF1 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int temp = ((t6) & (t3)) ^ ((t5) & (t1)) ^
			           ((t4) & (t2)) ^ ((t1) & (t0)) ^ (t0);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;
			//
			//	rotate_data_right(t7, h_wd_len - 1)
			//	+ rotate_data_right(temp, vv)
			//	+ round_key[k++];
			//
			t7 &= f_wd_mask;		// not re-used
			int rot1 = (t7 >>> (h_wd_len - 1))
			           | (t7 << (f_wd_len - (h_wd_len - 1)));

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)
			           | (temp << (f_wd_len - vv));

			temp = rot1 + rot2 + round_key[k++];

			t7 = t6; t6 = t5; t5 = t4; t4 = t3;
			t3 = t2; t2 = t1; t1 = t0; t0 = temp & f_wd_mask;
		}
//		System.out.println( "PASS 1" );
//		System.out.println(
//			" " + Integer.toString((0xFFFFFFFF & t7), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t6), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t5), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t4), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t3), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t2), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t1), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t0), 16) +
//			" ");

		/* Pass 2 uses FF2 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int temp = ((t6) & (t4) & (t0)) ^ ((t4) & (t3) & (t0))
				^ ((t5) & (t2)) ^ ((t4) & (t3)) ^ ((t4) & (t1))
				^ ((t3) & (t0)) ^ (t1);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;
			//
			//	rotate_data_right(t7, h_wd_len - 1)
			//	+ rotate_data_right(temp, vv)
			//	+ round_key[k++];
			//
			t7 &= f_wd_mask;		// not re-used
			int rot1 = (t7 >>> (h_wd_len - 1))
			           | (t7 << (f_wd_len - (h_wd_len - 1)));

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)
			           | (temp << (f_wd_len - vv));

			temp = rot1 + rot2 + round_key[k++];

			t7 = t6; t6 = t5; t5 = t4; t4 = t3;
			t3 = t2; t2 = t1; t1 = t0; t0 = temp & f_wd_mask;
		}

		/* Pass 3 uses FF3 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int temp = ((t5) & (t4) & (t0)) ^ ((t6) & (t4))
				^ ((t5) & (t2)) ^ ((t3) & (t0))
				^ ((t1) & (t0)) ^ (t3);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;
			//
			//	rotate_data_right(t7, h_wd_len - 1)
			//	+ rotate_data_right(temp, vv)
			//	+ round_key[k++];
			//
			t7 &= f_wd_mask;		// not re-used
			int rot1 = (t7 >>> (h_wd_len - 1))
			           | (t7 << (f_wd_len - (h_wd_len - 1)));

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)
			           | (temp << (f_wd_len - vv));

			temp = rot1 + rot2 + round_key[k++];

			t7 = t6; t6 = t5; t5 = t4; t4 = t3;
			t3 = t2; t2 = t1; t1 = t0; t0 = temp & f_wd_mask;
		}

		/* Pass 4 uses FF4  */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int temp = ((t6) & (t4) & (t2) & (t0))
				^ ((t6) & (t5)) ^ ((t4) & (t3))
				^ ((t3) & (t2)) ^ ((t1) & (t0)) ^ (t2);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;
			//
			//	rotate_data_right(t7, h_wd_len - 1)
			//	+ rotate_data_right(temp, vv)
			//	+ round_key[k++];
			//
			t7 &= f_wd_mask;		// not re-used
			int rot1 = (t7 >>> (h_wd_len - 1))
			           | (t7 << (f_wd_len - (h_wd_len - 1)));

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)
			           | (temp << (f_wd_len - vv));

			temp = rot1 + rot2 + round_key[k++];

			t7 = t6; t6 = t5; t5 = t4; t4 = t3;
			t3 = t2; t2 = t1; t1 = t0; t0 = temp & f_wd_mask;
		}

		out[0] = t0; out[1] = t1; out[2] = t2; out[3] = t3;
		out[4] = t4; out[5] = t5; out[6] = t6; out[7] = t7;

	}

	///
	//	Encryption and decryption
	///
	private void decrypt(int in[], int out[])
	{
		int	t0 = in[0] & 0xFFFFFFFF, 
			t1 = in[1] & 0xFFFFFFFF,
			t2 = in[2] & 0xFFFFFFFF,
			t3 = in[3] & 0xFFFFFFFF,
			t4 = in[4] & 0xFFFFFFFF,
			t5 = in[5] & 0xFFFFFFFF,
			t6 = in[6] & 0xFFFFFFFF,
			t7 = in[7] & 0xFFFFFFFF;

		int k = num_rounds - 1;   /* points to the first round key */
		int quarter_rounds = num_rounds / 4;
//		System.out.println( "START " + k + " " + quarter_rounds );
//		System.out.println(
//			" " + Integer.toString((0xFFFFFFFF & t7), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t6), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t5), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t4), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t3), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t2), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t1), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t0), 16) +
//			" ");


		/* Inverse of Pass 4 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int new7 = t0; t0 = t1; t1 = t2; t2 = t3;
			t3       = t4; t4 = t5; t5 = t6; t6 = t7;

//		System.out.print( "PASS 4." + i );
			int temp = ((t6) & (t4) & (t2) & (t0))
				^ ((t6) & (t5)) ^ ((t4) & (t3))
				^ ((t3) & (t2)) ^ ((t1) & (t0)) ^ (t2);

//		System.out.print(" "+Integer.toString((0xFFFFFFFF & temp), 16) );
			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;

//		System.out.print(" "+Integer.toString((0xFFFFFFFF & vv), 16) );
			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)	// rotate right
			           | (temp << (f_wd_len - vv));

			new7 -= rot2 + round_key[k--];	// check --

			new7 &= f_wd_mask;
//		System.out.print(" "+Integer.toString((0xFFFFFFFF & new7), 16) );
			t7 = (new7 << (h_wd_len - 1))	// rotate left
			           | (new7 >>> (f_wd_len - (h_wd_len - 1)));
//		System.out.println();
//		System.out.println(
//			" " + Integer.toString((0xFFFFFFFF & t7), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t6), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t5), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t4), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t3), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t2), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t1), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t0), 16) +
//			" ");
		}
//		System.out.println( "PASS 4" );
//		System.out.println(
//			" " + Integer.toString((0xFFFFFFFF & t7), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t6), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t5), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t4), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t3), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t2), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t1), 16) +
//			" " + Integer.toString((0xFFFFFFFF & t0), 16) +
//			" ");

		/* Inverse of Pass 3 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int new7 = t0; t0 = t1; t1 = t2; t2 = t3;
			t3       = t4; t4 = t5; t5 = t6; t6 = t7;

			int temp = ((t5) & (t4) & (t0)) ^ ((t6) & (t4))
				^ ((t5) & (t2)) ^ ((t3) & (t0))
				^ ((t1) & (t0)) ^ (t3);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)	// rotate right
			           | (temp << (f_wd_len - vv));

			new7 -= rot2 + round_key[k--];	// check --

			new7 &= f_wd_mask;
			t7 = (new7 << (h_wd_len - 1))	// rotate left
			           | (new7 >>> (f_wd_len - (h_wd_len - 1)));
		}

		/* Inverse of Pass 2 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int new7 = t0; t0 = t1; t1 = t2; t2 = t3;
			t3       = t4; t4 = t5; t5 = t6; t6 = t7;

			int temp = ((t6) & (t4) & (t0)) ^ ((t4) & (t3) & (t0))
				^ ((t5) & (t2)) ^ ((t4) & (t3)) ^ ((t4) & (t1))
				^ ((t3) & (t0)) ^ (t1);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)	// rotate right
			           | (temp << (f_wd_len - vv));

			new7 -= rot2 + round_key[k--];	// check --

			new7 &= f_wd_mask;
			t7 = (new7 << (h_wd_len - 1))	// rotate left
			           | (new7 >>> (f_wd_len - (h_wd_len - 1)));
		}

		/* Inverse of Pass 1 */
		for (int i = 0; i < quarter_rounds; i++)
		{
			int new7 = t0; t0 = t1; t1 = t2; t2 = t3;
			t3       = t4; t4 = t5; t5 = t6; t6 = t7;

			int temp = ((t6) & (t3)) ^ ((t5) & (t1)) ^
			           ((t4) & (t2)) ^ ((t1) & (t0)) ^ (t0);

			int vv   = (((temp >>> h_wd_len) + temp) & h_wd_mask)
				   >>> v_shift;

			temp &= f_wd_mask;		// not re-used
			int rot2 = (temp >>> vv)	// rotate right
			           | (temp << (f_wd_len - vv));

			new7 -= rot2 + round_key[k--];	// check --

			new7 &= f_wd_mask;
			t7 = (new7 << (h_wd_len - 1))	// rotate left
			           | (new7 >>> (f_wd_len - (h_wd_len - 1)));
		}

		out[0] = t0; out[1] = t1; out[2] = t2; out[3] = t3;
		out[4] = t4; out[5] = t5; out[6] = t6; out[7] = t7;

	}
///////////////////////////////// T E S T /////////////////////////

/**
 * Entry point for self_test.
 */
public static final void
main(String argv[]) 
{
	try {
		self_test(System.out, argv);
	}
	catch(Throwable t)
	{
		t.printStackTrace();
	}
}


/**
 *  Runs algorithm through test data, including certification data
 *  provided in paper.
 */
public static void
self_test( PrintStream out, String argv[] )
throws Exception
{
  //
  //  Note that the paper uses certification data that is indexed
  //  from RIGHT to LEFT, i.e., 7, 6, 5, 4, 3, 2, 1, 0.
  //
  test(out,  64,			// certification 1
    "0000000000000000",
    "0000000000000000",
    "2E008019BC26856D");
  test(out, 128,
    "00000000000000000000000000000000",
    "00000000000000000000000000000000",
    "A44FBF29EDF6CBF8D7A2DFD57163B909");
  test(out, 128,			// certification 2
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    "6C13E4B9C3171571AB54D816915BC4E8");
  test(out,  48,
    "504F4E4D4C4B4A494847464544434241",
    "1F1E1D1C1B1A191817161514131211100F0E0D0C0B0A09080706050403020100",
    "90C5981EF6A3D21BC178CACDAD6BF39B2E51CDB70A6EE875A73BF5ED883E3692");
  test(out, 256,
    "0000000000000000000000000000000000000000000000000000000000000000",
    "0000000000000000000000000000000000000000000000000000000000000000",
    "6CD44D2B49BC6AA7E95FD1C4AF713A2C0AFA1701308D56298CDF27A02EB09BF5");
  test(out, 256,
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    "C8F3E864263FAF24222E38227BEBC022CF4A9A0ECE89FB81CA1B9BA3BA93D0C5");
  test(out, 256,			// certification 3
    "605F5E5D5C5B5A595857565554535251504F4E4D4C4B4A494847464544434241",
    "1F1E1D1C1B1A191817161514131211100F0E0D0C0B0A09080706050403020100",
    "3DE16CFA9A626847434E1574693FEC1B3FAA558A296B61D708B131CCBA311068");
}

private static void
test( PrintStream out, int num_rnds, String keyStr, String plainStr, String cipherStr )
{
	byte key[] = fromRevString( keyStr );
	byte plain[] = fromRevString( plainStr );
	byte cipher[] = fromRevString( cipherStr );

	SPEED cryptor = new SPEED( key, plain.length, num_rnds );
	byte encP[] = new byte[plain.length];
	byte decC[] = new byte[plain.length];

	cryptor.encrypt( plain, encP );
	String a,b;
	out.println( "    key:" + toRevString( key ) );
	out.println( "  plain:" + toRevString( plain ) );
	out.println( "    enc:" + ( a = toRevString( encP ) ) );
	b = toRevString( cipher);
	if ( a.equals( b ) )
		out.print( "encryption good; " );
	else
	{
		out.println( "   calc:" + b );
		out.println( " ********* SPEED ENC FAILED ********* " );
		cryptor.dump();
	}

	cryptor.decrypt( encP, decC );
	a = toRevString( decC );
	b = toRevString( plain );
	if ( a.equals( b ) )
		out.println( "decryption good" );
	else
	{
		out.println();
		out.println( "    enc:" + toRevString( encP ) );
		out.println( "    dec:" + ( a = toRevString( decC ) ) );
		out.println( "   calc:" + ( b = toRevString( plain ) ) );
		out.println( " ********* SPEED DEC FAILED ********* " );
		cryptor.dump();
	}
}

//
//  Note that the paper uses certification data that is indexed
//  from RIGHT to LEFT, i.e., 7, 6, 5, 4, 3, 2, 1, 0.
//  To keep the data consistent with the paper, we reverse it in and out.
//  It is not expected that normal use would do this, however.
//
private static byte[]
fromRevString( String inHex )
{
	int len=inHex.length();
	int pos =0;
	byte buffer[] = new byte [( ( len + 1 ) / 2 )];
	if ( ( len % 2 ) == 1 )		// untested against certification
	{
		buffer[0]=(byte)asciiToHex(inHex.charAt( --len ));
		pos=1;
	}

	for(int ptr = pos; len > 0; )
		buffer[pos++] = (byte)( 
			( asciiToHex( inHex.charAt( --len ) ) ) |
			( asciiToHex( inHex.charAt( --len ) ) << 4)
				);
	return buffer;
}

private static final String
toRevString( byte buffer[] )
{
	char[] str = new char[buffer.length * 2];
	for (int i = 0; i < buffer.length; i++)
	{
		byte b = buffer[ buffer.length - i - 1 ];
		str[ i*2 ]     = hexToAscii( b & 0x0F );
		str[ i*2 + 1 ] = hexToAscii( (b >>> 4) & 0x0F );
	}
	return new String (str);
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
