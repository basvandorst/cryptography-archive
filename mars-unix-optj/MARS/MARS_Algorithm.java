/*
 * Parts Copyright (c) 1997, 1998 Systemics Ltd on behalf of
 * the Cryptix Development Team. All rights reserved.
 */

package MARS;

import java.io.PrintWriter;
import java.security.InvalidKeyException;
import java.util.* ;

//...........................................................................
/**
 * MARS is a submission to AES from a team of researchers at IBM. <p>
 * MARS_Algorithm.java reference code - copyright(c) 1998 IBM.
 * All rights reserved.
 *
 * Portions of this code are <b>Copyright</b> &copy; 1997, 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix DevelopmentTeam</a>.
 * <br>All rights reserved.<p>
 *
 * <b>$Revision: $</b>
 * @author  Raif S. Naffah
 */
public final class MARS_Algorithm // implicit no-argument constructor
{
    // Debugging methods and variables
    //.......................................................................

    static final String NAME = "MARS_Algorithm";
    static final boolean IN  = true, OUT = false;

    static final boolean DEBUG   = MARS_Properties.GLOBAL_DEBUG;
    static final int debuglevel  = DEBUG ? MARS_Properties.getLevel(NAME) : 0;
    static final PrintWriter err = DEBUG ? MARS_Properties.getOutput() : null;

    static final boolean TRACE = MARS_Properties.isTraceable(NAME);

    static void debug (String s) { err.println(">>> "+NAME+": "+s); }
    static void trace (boolean in, String s) {
        if (TRACE) 
            err.println((in?"==> ":"<== ")+NAME+"."+s);
    }
    static void trace (String s) { if (TRACE) err.println("<=> "+NAME+"."+s); }


    // MARS constants and variables
    //.....................................................................

    private static int  NUM_MIX         = 8  ;
    private static int  NUM_ROUNDS      = 16 ;
    private static int  NUM_SETUP       = 7  ;
    private static int  W               = 32 ;
    private static int  NUM_DATA        = 4  ;
    private static int  MAX_KEY         = 40 ;
    private static int  EKEY_WORDS      = (2*(NUM_DATA+NUM_ROUNDS)); 

    private static int S[] = {
        0x09d0c479, 0x28c8ffe0, 0x84aa6c39, 0x9dad7287, 
        0x7dff9be3, 0xd4268361, 0xc96da1d4, 0x7974cc93, 
        0x85d0582e, 0x2a4b5705, 0x1ca16a62, 0xc3bd279d, 
        0x0f1f25e5, 0x5160372f, 0xc695c1fb, 0x4d7ff1e4, 
        0xae5f6bf4, 0x0d72ee46, 0xff23de8a, 0xb1cf8e83, 
        0xf14902e2, 0x3e981e42, 0x8bf53eb6, 0x7f4bf8ac, 
        0x83631f83, 0x25970205, 0x76afe784, 0x3a7931d4, 
        0x4f846450, 0x5c64c3f6, 0x210a5f18, 0xc6986a26, 
        0x28f4e826, 0x3a60a81c, 0xd340a664, 0x7ea820c4, 
        0x526687c5, 0x7eddd12b, 0x32a11d1d, 0x9c9ef086, 
        0x80f6e831, 0xab6f04ad, 0x56fb9b53, 0x8b2e095c, 
        0xb68556ae, 0xd2250b0d, 0x294a7721, 0xe21fb253, 
        0xae136749, 0xe82aae86, 0x93365104, 0x99404a66, 
        0x78a784dc, 0xb69ba84b, 0x04046793, 0x23db5c1e, 
        0x46cae1d6, 0x2fe28134, 0x5a223942, 0x1863cd5b, 
        0xc190c6e3, 0x07dfb846, 0x6eb88816, 0x2d0dcc4a, 
        0xa4ccae59, 0x3798670d, 0xcbfa9493, 0x4f481d45, 
        0xeafc8ca8, 0xdb1129d6, 0xb0449e20, 0x0f5407fb, 
        0x6167d9a8, 0xd1f45763, 0x4daa96c3, 0x3bec5958, 
        0xababa014, 0xb6ccd201, 0x38d6279f, 0x02682215, 
        0x8f376cd5, 0x092c237e, 0xbfc56593, 0x32889d2c, 
        0x854b3e95, 0x05bb9b43, 0x7dcd5dcd, 0xa02e926c, 
        0xfae527e5, 0x36a1c330, 0x3412e1ae, 0xf257f462, 
        0x3c4f1d71, 0x30a2e809, 0x68e5f551, 0x9c61ba44, 
        0x5ded0ab8, 0x75ce09c8, 0x9654f93e, 0x698c0cca, 
        0x243cb3e4, 0x2b062b97, 0x0f3b8d9e, 0x00e050df, 
        0xfc5d6166, 0xe35f9288, 0xc079550d, 0x0591aee8, 
        0x8e531e74, 0x75fe3578, 0x2f6d829a, 0xf60b21ae, 
        0x95e8eb8d, 0x6699486b, 0x901d7d9b, 0xfd6d6e31, 
        0x1090acef, 0xe0670dd8, 0xdab2e692, 0xcd6d4365, 
        0xe5393514, 0x3af345f0, 0x6241fc4d, 0x460da3a3, 
        0x7bcf3729, 0x8bf1d1e0, 0x14aac070, 0x1587ed55, 
        0x3afd7d3e, 0xd2f29e01, 0x29a9d1f6, 0xefb10c53, 
        0xcf3b870f, 0xb414935c, 0x664465ed, 0x024acac7, 
        0x59a744c1, 0x1d2936a7, 0xdc580aa6, 0xcf574ca8, 
        0x040a7a10, 0x6cd81807, 0x8a98be4c, 0xaccea063, 
        0xc33e92b5, 0xd1e0e03d, 0xb322517e, 0x2092bd13, 
        0x386b2c4a, 0x52e8dd58, 0x58656dfb, 0x50820371, 
        0x41811896, 0xe337ef7e, 0xd39fb119, 0xc97f0df6, 
        0x68fea01b, 0xa150a6e5, 0x55258962, 0xeb6ff41b, 
        0xd7c9cd7a, 0xa619cd9e, 0xbcf09576, 0x2672c073, 
        0xf003fb3c, 0x4ab7a50b, 0x1484126a, 0x487ba9b1, 
        0xa64fc9c6, 0xf6957d49, 0x38b06a75, 0xdd805fcd, 
        0x63d094cf, 0xf51c999e, 0x1aa4d343, 0xb8495294, 
        0xce9f8e99, 0xbffcd770, 0xc7c275cc, 0x378453a7, 
        0x7b21be33, 0x397f41bd, 0x4e94d131, 0x92cc1f98, 
        0x5915ea51, 0x99f861b7, 0xc9980a88, 0x1d74fd5f, 
        0xb0a495f8, 0x614deed0, 0xb5778eea, 0x5941792d, 
        0xfa90c1f8, 0x33f824b4, 0xc4965372, 0x3ff6d550, 
        0x4ca5fec0, 0x8630e964, 0x5b3fbbd6, 0x7da26a48, 
        0xb203231a, 0x04297514, 0x2d639306, 0x2eb13149, 
        0x16a45272, 0x532459a0, 0x8e5f4872, 0xf966c7d9, 
        0x07128dc0, 0x0d44db62, 0xafc8d52d, 0x06316131, 
        0xd838e7ce, 0x1bc41d00, 0x3a2e8c0f, 0xea83837e, 
        0xb984737d, 0x13ba4891, 0xc4f8b949, 0xa6d6acb3, 
        0xa215cdce, 0x8359838b, 0x6bd1aa31, 0xf579dd52, 
        0x21b93f93, 0xf5176781, 0x187dfdde, 0xe94aeb76, 
        0x2b38fd54, 0x431de1da, 0xab394825, 0x9ad3048f, 
        0xdfea32aa, 0x659473e3, 0x623f7863, 0xf3346c59, 
        0xab3ab685, 0x3346a90b, 0x6b56443e, 0xc6de01f8, 
        0x8d421fc0, 0x9b0ed10c, 0x88f1a1e9, 0x54c1f029, 
        0x7dead57b, 0x8d7ba426, 0x4cf5178a, 0x551a7cca, 
        0x1a9a5f08, 0xfcd651b9, 0x25605182, 0xe11fc6c3, 
        0xb6fd9676, 0x337b3027, 0xb7c8eb14, 0x9e5fd030, 
        0x6b57e354, 0xad913cf7, 0x7e16688d, 0x58872a69, 
        0x2c2fc7df, 0xe389ccc6, 0x30738df1, 0x0824a734, 
        0xe1797a8b, 0xa4a8d57b, 0x5b5d193b, 0xc8a8309b, 
        0x73f9a978, 0x73398d32, 0x0f59573e, 0xe9df2b03, 
        0xe8a5b6c8, 0x848d0704, 0x98df93c2, 0x720a1dc3, 
        0x684f259a, 0x943ba848, 0xa6370152, 0x863b5ea3, 
        0xd17b978b, 0x6d9b58ef, 0x0a700dd4, 0xa73d36bf, 
        0x8e6a0829, 0x8695bc14, 0xe35b3447, 0x933ac568, 
        0x8894b022, 0x2f511c27, 0xddfbcc3c, 0x006662b6, 
        0x117c83fe, 0x4e12b414, 0xc2bca766, 0x3a2fec10, 
        0xf4562420, 0x55792e2a, 0x46f5d857, 0xceda25ce, 
        0xc3601d3b, 0x6c00ab46, 0xefac9c28, 0xb3c35047, 
        0x611dfee3, 0x257c3207, 0xfdd58482, 0x3b14d84f, 
        0x23becb64, 0xa075f3a3, 0x088f8ead, 0x07adf158, 
        0x7796943c, 0xfacabf3d, 0xc09730cd, 0xf7679969, 
        0xda44e9ed, 0x2c854c12, 0x35935fa3, 0x2f057d9f, 
        0x690624f8, 0x1cb0bafd, 0x7b0dbdc6, 0x810f23bb, 
        0xfa929a1a, 0x6d969a17, 0x6742979b, 0x74ac7d05, 
        0x010e65c4, 0x86a3d963, 0xf907b5a0, 0xd0042bd3, 
        0x158d7d03, 0x287a8255, 0xbba8366f, 0x096edc33, 
        0x21916a7b, 0x77b56b86, 0x951622f9, 0xa6c5e650, 
        0x8cea17d1, 0xcd8c62bc, 0xa3d63433, 0x358a68fd, 
        0x0f9b9d3c, 0xd6aa295b, 0xfe33384a, 0xc000738e, 
        0xcd67eb2f, 0xe2eb6dc2, 0x97338b02, 0x06c9f246, 
        0x419cf1ad, 0x2b83c045, 0x3723f18a, 0xcb5b3089, 
        0x160bead7, 0x5d494656, 0x35f8a74b, 0x1e4e6c9e, 
        0x000399bd, 0x67466880, 0xb4174831, 0xacf423b2, 
        0xca815ab3, 0x5a6395e7, 0x302a67c5, 0x8bdb446b, 
        0x108f8fa4, 0x10223eda, 0x92b8b48b, 0x7f38d0ee, 
        0xab2701d4, 0x0262d415, 0xaf224a30, 0xb3d88aba, 
        0xf8b2c3af, 0xdaf7ef70, 0xcc97d3b7, 0xe9614b6c, 
        0x2baebff4, 0x70f687cf, 0x386c9156, 0xce092ee5, 
        0x01e87da6, 0x6ce91e6a, 0xbb7bcc84, 0xc7922c20, 
        0x9d3b71fd, 0x060e41c6, 0xd7590f15, 0x4e03bb47, 
        0x183c198e, 0x63eeb240, 0x2ddbf49a, 0x6d5cba54, 
        0x923750af, 0xf9e14236, 0x7838162b, 0x59726c72, 
        0x81b66760, 0xbb2926c1, 0x48a0ce0d, 0xa6c0496d, 
        0xad43507b, 0x718d496a, 0x9df057af, 0x44b1bde6, 
        0x054356dc, 0xde7ced35, 0xd51a138b, 0x62088cc9, 
        0x35830311, 0xc96efca2, 0x686f86ec, 0x8e77cb68, 
        0x63e1d6b8, 0xc80f9778, 0x79c491fd, 0x1b4c67f2, 
        0x72698d7d, 0x5e368c31, 0xf7d95e2e, 0xa1d3493f, 
        0xdcd9433e, 0x896f1552, 0x4bc4ca7a, 0xa6d1baf4, 
        0xa5a96dcc, 0x0bef8b46, 0xa169fda7, 0x74df40b7, 
        0x4e208804, 0x9a756607, 0x038e87c8, 0x20211e44, 
        0x8b7ad4bf, 0xc6403f35, 0x1848e36d, 0x80bdb038, 
        0x1e62891c, 0x643d2107, 0xbf04d6f8, 0x21092c8c, 
        0xf644f389, 0x0778404e, 0x7b78adb8, 0xa2c52d53, 
        0x42157abe, 0xa2253e2e, 0x7bf3f4ae, 0x80f594f9, 
        0x953194e7, 0x77eb92ed, 0xb3816930, 0xda8d9336, 
        0xbf447469, 0xf26d9483, 0xee6faed5, 0x71371235, 
        0xde425f73, 0xb4e59f43, 0x7dbe2d4e, 0x2d37b185, 
        0x49dc9a63, 0x98c39d98, 0x1301c9a2, 0x389b1bbf, 
        0x0c18588d, 0xa421c1ba, 0x7aa3865c, 0x71e08558, 
        0x3c5cfcaa, 0x7d239ca4, 0x0297d9dd, 0xd7dc2830, 
        0x4b37802b, 0x7428ab54, 0xaeee0347, 0x4b3fbb85, 
        0x692f2f08, 0x134e578e, 0x36d9e0bf, 0xae8b5fcf, 
        0xedb93ecf, 0x2b27248e, 0x170eb1ef, 0x7dc57fd6, 
        0x1e760f16, 0xb1136601, 0x864e1b9b, 0xd7ea7319, 
        0x3ab871bd, 0xcfa4d76f, 0xe31bd782, 0x0dbeb469, 
        0xabb96061, 0x5370f85d, 0xffb07e37, 0xda30d0fb, 
        0xebc977b6, 0x0b98b40f, 0x3a4d0fe6, 0xdf4fc26b, 
        0x159cf22a, 0xc298d6e2, 0x2b78ef6a, 0x61a94ac0, 
        0xab561187, 0x14eea0f0, 0xdf0d4164, 0x19af70ee 
    };

    static final int BLOCK_SIZE = 16; // bytes in a data-block
    private static final char[] HEX_DIGITS = {
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };

    // Static code - to intialise S-box and permutation tables if any
    //......................................................................

    static {
        long time = System.currentTimeMillis();

        if (DEBUG && debuglevel > 6) {
            System.out.println("Algorithm Name: "+MARS_Properties.FULL_NAME);
            System.out.println("Electronic Codebook (ECB) Mode");
            System.out.println();
        }
        //
        // for testing purposes set the S-boxes to be identity mappings
        //
       
        time = System.currentTimeMillis() - time;

        if (DEBUG && debuglevel > 8) {
            System.out.println("==========");
            System.out.println();
            System.out.println("Static Data");
            System.out.println();
            //
            // any other println() statements
            //
            System.out.println();
            System.out.println("Total initialization time: "+time+" ms.");
            System.out.println();
        }
    }


    // Basic API methods
    //......................................................................

    /**
     * Expand a user-supplied key material into a session key.
     *
     * @param key  The 128/192/256-bit user-key to use.
     * @return     This cipher's round keys.
     * @exception  InvalidKeyException  If the key is invalid.
     */
    public static synchronized Object makeKey (byte[] k)
        throws InvalidKeyException {
        if( k == null) 
            throw new InvalidKeyException("empty key given");
        if( !(k.length == 16 || k.length == 24 || k.length == 32) ) 
            throw new InvalidKeyException("bad key length");
       
        //  convert key bytes to key ints
        int[] K = new int[40];
        int i = 0 ;
        int j = 0 ;
        int offset = 0 ;
	int n = k.length/4;
	int w = 0;
        for(i = 0 ; i < n; i++) {
            K[i] = 0 ;
            for(j = 3; j >= 0; j--) 
                K[i] = (K[i] << 8)^(k[offset+j]&0xFF);
            offset += 4 ;
        }
    
        /* space for final key */
        int[] Key = new int[EKEY_WORDS] ;
        int[] buf = new int[7+EKEY_WORDS];

	wordKeySetup(K, n, buf, Key);

        return Key ;
    }

    /* low level (word oriented) key setup */
    public static final void wordKeySetup(int[] K,int n,int[] buf,int[] Key){

        int i = 0, j = 0, t = 0, w=0;

        for(i = 0 ; i < Key.length; i++) 
            Key[i] = 0 ;

	/* initialize the temporary array with key data */
	for (i=0; i<n; i++)
	    buf[i] = K[i];
	buf[n] = n;
	for (i=n+1; i<15; i++)
	    buf[i] = 0;

	/* Four iterations, each one computing 10 words of the array */
	for (j=0; j<4; j++) {
	    /* Linear transformation */
	    w = buf[8] ^ buf[13]; 
	    buf[0] ^= ((w>>>29)|(w<<3)) ^ j;
	    w = buf[9] ^ buf[14]; 
	    buf[1] ^= ((w>>>29)|(w<<3)) ^ 4 ^ j;
	    for (i=2; i<7; i++) { 
		w = buf[i+8] ^ buf[i-2]; 
		buf[i] ^= ((w>>>29)|(w<<3)) ^ (i<<2) ^ j;
	    }
	    for (i=7; i<15; i++) { 
		w = buf[i-7] ^ buf[i-2]; 
		buf[i] ^= ((w>>>29)|(w<<3)) ^ (i<<2) ^ j;
	    }

	    /* Four stirring rounds */
	    for (t=0; t<4; t++){
		/* stir with full type-1 s-box rounds */
		w = buf[0] + S[ buf[14]&511 ];
		buf[0] = (w>>>23)|(w<<9);
		for (i=1 ; i<15; i++) {
		    w = buf[i] + S[ buf[i-1]&511 ];
		    buf[i] = (w>>>23)|(w<<9);
		}
	    }

	    /* copy subkeys to mars_ctx, with swapping around */
	     i = 10*j;
	     Key[i] = buf[0];
	     Key[i+1] = buf[4];
	     Key[i+2] = buf[8];
	     Key[i+3] = buf[12];
	     Key[i+4] = buf[1];
	     Key[i+5] = buf[5];
	     Key[i+6] = buf[9];
	     Key[i+7] = buf[13];
	     Key[i+8] = buf[2];
	     Key[i+9] = buf[6];
	}

        for(i = NUM_DATA + 1; i < (EKEY_WORDS-NUM_DATA); i +=2 ) 
            Key[i] = fix_subkey(Key[i],Key[i-1]);
    }

    /**
     * Encrypt exactly one block of plaintext.
     *
     * @param in          The plaintext.
     * @param inOffset    Index of in from which to start considering data.
     * @param sessionKey  The session key to use for encryption.
     * @return The ciphertext generated from a plaintext using the session key.
     */
    public static byte[] blockEncrypt(byte[] in, int inOffset, Object 
        sessionKey){

        int[] K = (int[]) sessionKey ;
        int[] out = new int[NUM_DATA];                 
        byte[] result = new byte[4*NUM_DATA];  
	
        int i = 0 ;
        int j = 0 ;

        /* copy input to word[] */
        for(i = 0 ; i < 4; i++) {
            out[i] = 0 ;
            for(j = 3; j >= 0 ; j--) 
                out[i] = (out[i] << 8)^(in[inOffset+j]&0xFF);
            inOffset += 4;
        }

        /* call to low level (word oriented) routine */
        wordEncrypt(out,K);

        /* copy output back to byte[] */
        for(i = 0; i < 4; i++) {
            for(j = 0; j < 4; j++) {
                result[(i << 2) + j] = (byte) ( out[i] & 0xFF ) ;
                out[i] >>>= 8 ;
            }
        }
 
        return result;
    }
	    
    /* low level (word oriented) encryption */
    public static final void wordEncrypt(int[] out, int[] K) {

        int i = 0 ;
        int j = 0 ;
        int a, b, c, d;

        /* subkey add */
        a = out[0] + K[0]; 
        b = out[1] + K[1]; 
        c = out[2] + K[2]; 
        d = out[3] + K[3];

	/* eight rounds of forward mixing */
	int tmp1, tmp2, tmp3, r;
	for (i = 0; i < 8; i++) {
	    tmp3 = ((a>>>24)|(a<<8)); 
	    a = (b ^ S[a&255]) + S[((a>>>8)&255)+256]; 
	    b = c + S[(tmp3>>>24)&255]; 
	    c = d ^ S[(tmp3&255)+256];
	    d = tmp3; 

            if ((i==0)|(i==4)) 
                d = d + c;
            else if ((i==1)|(i==5))
                d = d + a;
	}

	/* sixteen "core" rounds */
	j = 4;
 	for (i = 0; i < 16; i++) {
  	    int sav_d = d;

	    // Compute the E-function
 	    tmp2 = a + K[j];
 	    d = (a>>>19) | (a<<13);
 	    tmp3 = d*K[j+1];
 	    tmp3 = (tmp3<<5) | (r = tmp3>>>27);
 	    tmp1 = S[tmp2&511] ^ tmp3;
	    tmp2 = ((tmp2>>>(32-r)) | (tmp2<<r));
 	    tmp3 = (tmp3<<5) | (r = tmp3>>>27);
 	    tmp1 ^= tmp3;
	    if (i < 8) {    // first 8 rounds in forward mode
                tmp1 =  ((tmp1>>>(32-r))|(tmp1<<r));
	        a = b + tmp1;
	        b = c + tmp2; 
	        c = sav_d ^ tmp3;
	    }
	    else {           // last 8 rounds in backwards mode
	        a = b ^ tmp3;
	        b = c + tmp2; 
	        tmp1 =  ((tmp1>>>(32-r))|(tmp1<<r));
	        c = sav_d + tmp1;
	    }
	    j += 2;
	}

	/* eight rounds of backwards mixing */
	for (i = 0; i < 8; i++) {
            if ((i==2)|(i==6))
                a -= d;
            else if ((i==3)|(i==7))
                a -= b;
  
  	    tmp1 = ((a>>>8)|(a<<24));     // imix_subround(out,0,1,2,3);
	    a = b ^ S[(a&255)+256];
	    b = c - S[(tmp1>>>16)&255];
	    c = (d - S[((tmp1>>>8)&255)+256]) ^ S[tmp1&255];
	    d = tmp1;
	}
     
        out[0] = a - K[2*NUM_ROUNDS+4];
        out[1] = b - K[2*NUM_ROUNDS+5];
        out[2] = c - K[2*NUM_ROUNDS+6];
        out[3] = d - K[2*NUM_ROUNDS+7];
    }

    /**
     * Decrypt exactly one block of ciphertext.
     *
     * @param in          The ciphertext.
     * @param inOffset    Index of in from which to start considering data.
     * @param sessionKey  The session key to use for decryption.
     * @return The plaintext generated from a ciphertext using the session key.
     */
    public static byte[] blockDecrypt(byte[] in, int inOffset, Object 
        sessionKey){
        
        int[] K = (int[]) sessionKey ; 
        int[] out = new int[NUM_DATA];
        byte[] result = new byte[4*NUM_DATA];  

        int i = 0 ;
        int j = 0 ;

        /* convert to word[] */
        for(i = 0 ; i < 4; i++) {
            out[3-i] = 0 ;
            for(j = 3; j >= 0 ; j--) 
                out[3-i] = (out[3-i] << 8)^(in[inOffset+j]&0xFF);
            inOffset += 4;
        }

        /* call low level (word oriented) routine */
        wordDecrypt(out,K);

        for(i = 0; i < 4; i++) {
            for(j = 0; j < 4; j++) {
                result[(i << 2) + j] = (byte) ( out[3-i] & 0xFF ) ;
                out[3-i] >>>= 8 ;
            }
        }
        return result;
    }

    public static final void wordDecrypt(int[] out, int[] K){

        int a, b, c, d;
        int i = 0 ;
        int j = 0 ;

        a = out[0] + K[2*NUM_ROUNDS+7];
        b = out[1] + K[2*NUM_ROUNDS+6];
        c = out[2] + K[2*NUM_ROUNDS+5];
        d = out[3] + K[2*NUM_ROUNDS+4];

	int tmp1, tmp2, tmp3, r;
	for (i = 0; i < 8; i++) {
	    tmp3 = ((a>>>24)|(a<<8));    // mix_subround(out,0,1,2,3); 
	    a = (b ^ S[a&255]) + S[((a>>>8)&255)+256]; 
	    b = c + S[(tmp3>>>24)&255]; 
	    c = d ^ S[(tmp3&255)+256];
	    d = tmp3; 

            if ((i==0)|(i==4)) 
                d = d + c;
            else if ((i==1)|(i==5))
                d = d + a;
	}

 	j = 35;
 	for (i = 0; i < 16; i++) {
	    int sav_d = d;

	    // Compute the E-function
 	    tmp3 = a*K[j];
 	    d = (a>>>13) | (a<<19);
 	    tmp3 = (r = tmp3>>>27) | (tmp3<<5);
 	    tmp2 = d + K[j-1];
 	    tmp1 = S[tmp2&511] ^ tmp3;
	    tmp2 = ((tmp2<<r) | (tmp2>>>(32-r)));
 	    tmp3 = (r = tmp3>>>27) | (tmp3<<5);
 	    tmp1 ^= tmp3;
	    if (i < 8) {     // first 8 rounds in forward mode
	        tmp1 = ((tmp1>>>(32-r)) | (tmp1<<r));
	        a = b - tmp1;
	        b = c - tmp2; 
	        c = sav_d ^ tmp3;
	    }
	    else {           // last 8 rounds in backwards mode
	        a = b ^ tmp3;
	        b = c - tmp2; 
	        tmp1 = ((tmp1>>>(32-r))|(tmp1<<r));
	        c = sav_d - tmp1;
	    }
	    j -= 2;
	}
   
	for (i = 0; i < 8; i++) {
            if ((i==2)|(i==6))
                a -= d;
            else if ((i==3)|(i==7))
                a -= b;

	    tmp1 = ((a>>>8)|(a<<24));     // imix_subround(out,0,1,2,3);
	    a = b ^ S[(a&255)+256];
	    b = c - S[(tmp1>>>16)&255];
	    c = (d - S[((tmp1>>>8)&255)+256]) ^ S[tmp1&255];
	    d = tmp1;
	}

        /* subtract off subkeys */
        out[0] = a - K[3];
        out[1] = b - K[2];
        out[2] = c - K[1];
        out[3] = d - K[0];
    }

    /** A basic symmetric encryption/decryption test. */ 
    public static boolean self_test() { return self_test(BLOCK_SIZE); }

    // MARS subfunctions
    //.....................................................................

    public static final int fix_subkey(int k, int r) {
        /* the mask words come from S[265]..S[268] */
        int m1, m2;
        int i;

        i = k & 3;            /* store the least two bits of k */
        k |= 3;               /* and then mask them away       */

        /* we look for 9 consequtive 1's in m1 */
        m1 = (~k) ^ (k<<1);   /* for i > 1, m1_i = 1 iff k_i = k_{i-1} */
        m2 = m1 & (m1 << 1);  /* m2_i = AND (m1_i, m1_{i-1})   */
        m2 &= m2 << 2;        /* m2_i = AND (m1_i...m1_{i-3})  */
        m2 &= m2 << 4;        /* m2_i = AND (m1_i...m1_{i-7})  */
        m2 &= m1 << 8;        /* m2_i = AND (m1_i...m1_{i-8})  */
        m2 &= 0xfffffe00;     /* mask out the low 9 bits of m2 */
    
        /* if m2 is zero, k was good, so return */ 
        if (m2 == 0) {
            //System.out.println("no fix required\n");
            return k;
        }
        //else System.out.print("about to fix\t");

        /* need to fix k: we copy each 1 in m2 to the nine bits to its right */
        m1 = m2 | (m2 >>> 1); /* m1_i = AND (m2_i, m2_{i+1})   */
        m1 |= m1 >>> 2;       /* m1_i = AND (m2_i...m2_{i+3})  */
        m1 |= m2 >>> 4;       /* m1_i = AND (m2_i...m2_{i+4})  */
        m1 |= m1 >>> 5;       /* m1_i = AND (m2_i...m2_{i+9})  */
        /* m1_i = 1 iff k_i belongs to a sequence of ten 0's or ten 1's */

        /* we turn off the two lowest bits of M, and also every bit 
         * M_i such that k_i is not equal to both k_{i-1} and k_{i+1} 
         */
        m1 &= ((~k)^(k<<1)) & ((~k)^(k>>>1)) & 0x7ffffffc; 

        /* and finally pick a pattern, rotate it, 
         * and xor it into k under the control of the mask m1
         */
         k ^= LROTATE(S[265+i], r) & m1;

         //System.out.println("fixed value = "+k);

         return k;
    }

    public static final int LROTATE(int x, int y ) 
    {
      return ( (x << (y & 31)) | (x >>> (W-(y & 31))) );
    }

    public static final int RROTATE(int x, int y ) 
    {
      return ( (x  >>> (y & 31)) | (x << (W-(y & 31))) );
    }
    

    //-----------------------------------------------------------------------


    /** @return The length in bytes of the Algorithm input block. */
    public static int blockSize() { return BLOCK_SIZE; }

    /** A basic symmetric encryption/decryption test for a given key size. */
    private static final boolean self_test (int keysize) {
        boolean ok = false;
        try {
            byte[] kb = new byte[keysize];
            byte[] pt = new byte[BLOCK_SIZE];
            int i;

            for (i = 0; i < keysize; i++)
                kb[i] = (byte) i;
            for (i = 0; i < BLOCK_SIZE; i++)
                pt[i] = (byte) i;

            if (DEBUG && debuglevel > 6) {
                System.out.println("==========");
                System.out.println();
                System.out.println("KEYSIZE="+(8*keysize));
                System.out.println("KEY="+toString(kb));
                System.out.println();
            }
            Object key = makeKey(kb);

            if (DEBUG && debuglevel > 6) {
                System.out.println(
                    "Intermediate Ciphertext Values (Encryption)");
                System.out.println();
            }

            byte[] ct =  blockEncrypt(pt, 0, key);
	    
            if (DEBUG && debuglevel > 6) {
                System.out.println(
                    "Intermediate Plaintext Values (Decryption)");
                System.out.println();
            }
            byte[] cpt = blockDecrypt(ct, 0, key);

            ok = areEqual(pt, cpt);
            if (!ok)
                throw new RuntimeException("Symmetric operation failed");
        } catch (Exception x) {
            if (DEBUG && debuglevel > 0) {
                debug("Exception encountered during self-test: " 
                    + x.getMessage());
                x.printStackTrace();
            }
        }
        if (DEBUG && debuglevel > 0) 
            debug("Self-test OK? " + ok);
        if (DEBUG) 
            trace(OUT, "self_test()");
        return ok;
    }


    //utility static methods (from cryptix.util.core ArrayUtil and Hex classes)
    //.....................................................................
    
    /** @return True iff the arrays have identical contents. */
    private static boolean areEqual (byte[] a, byte[] b) {
        int aLength = a.length;
        if (aLength != b.length)
            return false;
        for (int i = 0; i < aLength; i++)
            if (a[i] != b[i])
                return false;
        return true;
    }

    /**
     * Returns a string of hexadecimal digits from a byte array. Each
     * byte is converted to 2 hex symbols.
     */
    private static String toString (byte[] ba) {
        return toString(ba, 0, ba.length);
    }
    private static String toString (byte[] ba, int offset, int length) {
        char[] buf = new char[length * 2];
        for (int i = offset, j = 0, k; i < offset+length; ) {
            k = ba[i++];
            buf[j++] = HEX_DIGITS[(k >>> 4) & 0x0F];
            buf[j++] = HEX_DIGITS[ k        & 0x0F];
        }
        return new String(buf);
    }


    // main(): use to run basic tests 
    //....................................................................

    public static void main (String[] args) {
        self_test(16);
        self_test(24);
        self_test(32); 

        /* low level timing test */
        try {
            byte[] kb = new byte[16];
            int[] pt = new int[NUM_DATA];
            int i;
            for (i = 0; i < 16; i++)
                kb[i] = (byte) i;
            for (i = 0; i < NUM_DATA; i++)
                pt[i] = i;

            int[] key = (int[]) makeKey(kb);
            long time = System.currentTimeMillis();
            for (i=0;i<200000;i++)
                wordEncrypt(pt, key);
            time = System.currentTimeMillis() - time;
            System.out.println("time to encrypt 200000 blocks: "+time+" ms.");
            System.out.println("     "+(25600.0/time)+"Mbit/sec");

	    time = System.currentTimeMillis();
            for (i=0;i<200000;i++)
                wordDecrypt(pt, key);
            time = System.currentTimeMillis() - time;
            System.out.println("time to decrypt 200000 blocks: "+time+" ms.");
            System.out.println("     "+(25600.0/time)+"Mbit/sec");

	    int[] buf = new int[7+EKEY_WORDS];
	    int[] K = { 0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10 };
	    time = System.currentTimeMillis();
            for (i=0;i<20000;i++)
                wordKeySetup(K, 4, buf, key);
            time = System.currentTimeMillis() - time;
            System.out.println("time to expand 20000 keys: "+time+" ms.");
            System.out.println("     "+(2560.0/time)+"Mbit/sec");

        } catch (Exception x) {
            System.out.println("self test timing test had exception "
                    + x.getMessage());
        }
    }
}
