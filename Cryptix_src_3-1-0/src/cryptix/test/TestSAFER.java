// $Id: TestSAFER.java,v 1.6 1998/01/28 05:40:02 hopwood Exp $
//
// $Log: TestSAFER.java,v $
// Revision 1.6  1998/01/28 05:40:02  hopwood
// + Major update of test classes.
//
// Revision 1.5.1  1998/01/28  hopwood
// + Fixed conflicts with my version.
// + Updated expected number of passes.
// + Changed to use BaseTest API.
//
// Revision 1.5  1998/01/22 04:40:36  iang
// + Added some extra tests, only one is now missing from check.ref.
// + Added a few additional comments where tests didn't match check.ref.
// + Now System.exit(1) on exception, and throws exception on bad data.
//
// Revision 1.4  1997/11/22 07:05:41  raif
// *** empty log message ***
//
// Revision 1.3  1997/11/22 05:59:03  iang
// + core.util ==> util.core
//
// Revision 1.2  1997/11/10 07:31:33  raif
// + Tests new support of engineGet/SetParameter();
//
// Revision 1.2  1997/10/28 00:09:10  raif
// + Incorporated name changes done to SAFER
//
// Revision 1.1  1997/10/27 22:50:17  raif
// + Use new cryptix.util classes.
// + More compact.
//
// Revision 0.1.0.1  1997/07/??  R. Naffah
// + Added decryption tests in CFB and OFB modes.
//
// Revision 0.1.0.0  1997/07/??  R. Naffah
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */

package cryptix.test;

import cryptix.provider.key.RawSecretKey;
import cryptix.util.core.ArrayUtil;
import cryptix.util.core.Hex;
import cryptix.util.test.BaseTest;

import java.security.Cipher;
import java.security.FeedbackCipher;
import java.security.SecretKey;
import java.security.Parameterized;

/**
 * Tests the output of the SAFER cipher algorithm implementation against
 * certified pre-computed output for a given set of reference input.
 * <p>
 * Test data is taken from <code>check.ref</code> file in Richard De Moliner's
 * <a href="ftp://ftp.isi.ee.ethz.ch/pub/simpl/safer.V1.2.tar.Z">
 * SAFER toolkit V1.2</a>.  As this is the distribution of the
 * author of the algorithm, this is understood to be conformance data,
 * cautious label of Examples notwithstanding.
 * <p>
 * One test is not present, that of testing a text key
 * (more a test for the test program).
 * <p>
 * <b>$Revision: 1.6 $</b>
 * @author  Raif S. Naffah
 * @author  David Hopwood
 */
class TestSAFER
extends BaseTest
{

// Variables and constants
//...........................................................................

    private static byte[] input = new byte[16 * 18];
    private static final byte[] anIV = {
        (byte)0x74, (byte)0x53, (byte)0x6E, (byte)0xBD,
        (byte)0xC2, (byte)0x11, (byte)0x48, (byte)0x4A
    };
    private SecretKey aKey;


// Test methods
//...........................................................................

    public static void main(String[] args) {
        new TestSAFER().commandline(args);
    }

    protected void engineTest() throws Exception {
        setExpectedPasses(17);

        byte[] b = {
            (byte)0x42, (byte)0x43, (byte)0x1B, (byte)0xA4,
            (byte)0x0D, (byte)0x29, (byte)0x1F, (byte)0x81,
            (byte)0xD6, (byte)0x60, (byte)0x83, (byte)0xC6,
            (byte)0x05, (byte)0xD3, (byte)0xA4, (byte)0xD6
        };

        aKey = new RawSecretKey("SAFER", b);
        for (int i = 0; i < 2; i++) {
            for (int j = 1; j < 9; j++)
                input[16 * i + j + 7] = (byte)j;
        }
        for (int i = 0; i < 256; i++) input[32 + i] = (byte)i;
                
        out.println("input data\n" + Hex.dumpString(input));
            
        // There is a small amount of test data in SAFER_SK.TXT
        // (see the above test kit).  This data has internal
        // round data included for debugging purposes.
        // These data sets are included in the below tests.
        // Look at the second block of the common input data.
 
        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
        test8();
        test8B();
        test8C();
        // This is using a text key - how to interpret extra chars?
        //    safer -e -ecb -k AaBcDeFgHiJkLmNoPqRsTuVwXyZz0123456789
        // test8D();
        test8E();
        test9();
        test10();
        test11();
    }

    private void test1() throws Exception {
        int[] data = {
      3,  40,   8, 201,  14, 231, 171, 127, 125,  40,   3, 134,  51, 185,  46, 180,
      3,  40,   8, 201,  14, 231, 171, 127, 125,  40,   3, 134,  51, 185,  46, 180,
    145, 193, 218, 242, 149, 199,  15, 163,  25, 207,  49, 154,  65,  22, 246,   7,
    222, 135, 141,  58, 225, 131,  75,  71,  89, 235, 249,  44, 127, 227, 181,  69,
    159,  12,  78,  64, 221, 251,  20,  32, 119, 184,  78, 136,  10,   2, 106, 193,
    143,  74, 196, 156,  26, 228, 156, 155, 251, 162, 156, 195, 133, 136, 151, 164,
      7, 189,  49, 125,  68,  77, 106, 185,  12, 206, 130, 131,  69, 124, 179,   4,
      5,  19, 191,  12, 223, 108, 112,  79, 171,  62, 108, 206, 116, 117,  67, 115,
     96, 119,  81,   7, 167,  42,  84,  64, 251, 182, 224,  42,   5, 204,  31, 169,
    157, 122, 237,  33, 196, 183, 241,  50, 125, 221,  74,  87, 198, 143, 128, 131,
    240, 168,  59, 136,  63, 211, 116, 117,  93,  13, 180,  15, 216,  25,  39, 243,
    204,  51,  23,  62, 218, 100, 194, 254, 138,  60, 153,  12, 141, 112,  67, 189,
     52,  19, 188,  96, 221, 105, 239,  51, 136, 244, 210, 250,  24, 224, 181, 103,
     74,  22, 174, 213,  84, 134,  59, 146, 195, 129, 144, 255, 209, 179,  88, 219,
    130, 133,  36, 133,  12,  70, 104,  64,  18, 157,  66, 244,  86, 129, 171, 231,
     70,  15, 199, 114,  63,   5, 106, 141, 217, 241,  87, 204, 106, 178,  64, 190,
     91,  58, 131,  40, 121, 246,   9, 129, 172,   3, 144, 148,  86, 233,  70, 137,
    234, 154, 150,  87,  27, 126, 194, 213,  97,   0,  42, 166,  24,  28,  36,  46
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 0000000000000000 -r 6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-64");
        ((Parameterized) alg).setParameter("rounds", new Integer(6));
        byte[] b = new byte[8];
        RawSecretKey key = new RawSecretKey("SAFER", b);
        
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test2() throws Exception {
        int[] data = {
   90, 178, 127, 114,  20, 163,  58, 225,    53, 216,  27, 187, 244,  86, 143, 221,
   90, 178, 127, 114,  20, 163,  58, 225,    53, 216,  27, 187, 244,  86, 143, 221,
  176,  77, 205,  34, 113, 253,  45, 100,   117,  35, 111,  82, 245,  22, 192, 247,
  113, 229, 207, 127,   8,  58,  89, 197,    53, 111, 112,  44, 199, 250, 129,  97,
   22,  45, 118, 143,  36,  19, 175,  42,    17,  98, 225, 143, 245,  22,  92, 104,
   30,  78,  11, 197, 233, 245, 156, 211,   252, 128,  59, 102,  39, 106,  52, 201,
   36, 150,  88,  51,  99, 193, 250, 199,   101, 188, 204, 108, 195, 158,   4,  16,
   64,  14,  12,   4,  69,  77,  87, 174,  179, 168, 218, 211,  98,  49, 250, 255,
  108, 197, 119, 252, 139, 188, 188,  69,  111, 126, 135,  26, 214,  89,  66, 223,
   89,  23, 144, 219,  71, 139,  62,  28,   168,  72,  23,  84, 206, 185, 152, 189,
  224,   0,   3, 206,  49,  73,  65, 167,   192, 241,  81,  40,  12, 142, 229,  74,
    4, 186,  59, 173, 112, 219, 152,  96,     6,  40,  84, 232, 152, 173,  91,  48,
   65,  34,  12,  56,  92, 127, 196,  80,   104,  37, 133,  26,  77, 151, 103, 250,
   83,  94, 214, 100, 211, 196, 211,  86,   176, 148,  84,  52,  16,  98, 181, 167,
  171, 165, 176, 131,  34, 234, 253, 245,   243, 229, 190, 104, 135,  12, 178, 255,
   88,  83,  26, 197,  13,  87, 161,  25,   152,  13, 132, 120, 232, 132, 144, 232,
  238, 120,  80, 141, 244,  81, 238, 133,   178, 195, 161,  51, 196, 174, 204,  11,
   79,  48,  86,  90, 145, 237, 144, 173,   117, 144, 116,  74, 226, 113, 221,  90
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 0102030405060708 -r 6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-64");
        ((Parameterized) alg).setParameter("rounds", new Integer(6));
        byte[] b = new byte[8];
        for (int i = 1; i < 9; i++)
            b[i - 1] = (byte)i;
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test3() throws Exception {
        int[] data = {
    3,  92,  58,  98, 214, 251,  36,  18,   200, 242, 156, 221, 135, 120,  62, 217,
    3,  92,  58,  98, 214, 251,  36,  18,   200, 242, 156, 221, 135, 120,  62, 217,
    2, 200, 228,  89, 250, 111, 128, 147,   203, 125, 189,  26,  90,  50,   9, 146,
  149,  78, 101, 142, 114, 229, 131,   2,    62, 174,  98, 110, 169, 197,  54,  63,
   89,  85,  87, 223, 143, 246, 139,  69,   175,  47,  36, 136,  12, 214,  13,  29,
   27,  37,  48, 227, 234, 251,  65, 118,   189, 249, 125, 187,  61,  95,  43, 165,
  192,  19,  53, 236, 129, 201, 111, 186,   128, 238, 168, 161, 255, 185, 159,  60,
   39, 215,  88, 181,  24, 210, 205, 138,    73, 233, 133, 129, 204, 192, 157,  89,
   11, 204, 197,  13, 254,  57, 201,  86,     1, 104,  36,  93,  89, 167, 209,  28,
   93, 108, 251,  66,  28, 190,  50, 250,   173,  10, 172, 104,  44, 220,  12,  66,
  227, 252, 245,  29, 253,  71, 160,  74,   246, 140, 143,  29, 178,  92, 125, 196,
   43,  64, 156, 229, 193,  21, 103, 211,   217, 196, 211,  32, 127, 150,  31,  19,
  116, 226, 149, 154, 103,  37, 206,  37,    94,  65,  62,  89, 112,  69, 212, 173,
  227,  86, 217,  69,  24, 157, 171,  91,    84, 105, 146, 134, 221,  99, 119, 167,
    3,  38, 221, 132, 166, 194, 145, 244,   136,  58,  22, 218, 103,  85, 190, 178,
   60, 157, 226, 234, 174,  78, 151, 117,   238, 127, 201, 155, 244,   3,  63,  66,
   47, 145, 200, 239, 161, 113,  76, 233,    78, 142,  46, 172,  80,   1,   4,   9,
  183,  25, 176, 250, 175, 216, 123,  45,    90, 108,  67, 187, 147,  57, 235,  25
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 0807060504030201 -r 6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-64");
        ((Parameterized) alg).setParameter("rounds", new Integer(6));
        byte[] b = new byte[8];
        for (int i = 0; i < 8; i++)
            b[i] = (byte)(8 - i);
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }

    private void test4() throws Exception {
        int[] data = {
  193, 162, 137, 119,  71, 111,   4, 122,   221,  53, 132, 163,  31, 251,  91, 187,
  193, 162, 137, 119,  71, 111,   4, 122,   221,  53, 132, 163,  31, 251,  91, 187,
  200,   7, 221, 209, 159, 188, 204, 148,   108, 182, 178,  60,  21,   6, 172,   3,
  135,  38,  14,  54, 116,  19, 125, 236,    12,  65, 190, 138, 183,  64,  84,  94,
  211,  57, 211,  91, 134,  90, 189, 173,   198,  52,  54,  78, 218, 211,  10,  64,
  160, 250,  24, 252,  43,  62, 176,  51,   229,  82, 255, 241, 155, 211,  23, 185,
  144,  30, 255,  20,  82, 191,  37, 197,   241,  45,  53, 149, 201, 159,  67,  72,
   56, 230,  77, 191, 110,  15, 137, 110,   125, 143,   1,  74, 144,  36, 128, 254,
   42,  66,  86, 239, 123, 207, 118, 129,    79,  15, 246, 165,  44,  92, 253,  96,
  229, 105, 211, 191, 109, 129, 236, 199,   134, 174, 205, 197, 236, 183, 160, 119,
   39,  48,  34, 113,  23,  99, 138, 156,   241,  88,  87, 154, 155, 138, 208,  73,
   76, 235, 164, 163,  78, 161,  71,  16,    35,  35,  55,  87,   0, 148, 112, 232,
   98, 175,  26, 239, 137,  77,  16, 106,    26, 246,  43,  38, 237,  84, 201,  35,
  213, 220, 132, 203,  53,  48, 202, 119,   192,  74, 167, 216,   1, 111, 138,  10,
   25,  68, 167, 158, 226, 226, 255,  64,   179, 148,  31,   8,  78, 243, 131, 146,
  144,  89, 114, 186, 228,  43,  99, 173,   198, 192, 243,   9,  44, 153,  27, 235,
    6,  33, 118,   4, 173, 100,  48, 242,    92,  98, 216, 213, 149, 225, 238, 128,
  198,  93, 197, 243, 203, 210, 227, 203,   191, 225, 181, 137, 184,  73, 136,  66
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 08070605040302010807060504030201 -r 12\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
        ((Parameterized) alg).setParameter("rounds", new Integer(12));

        // Our implementation, automatically duplicates an 8-byte
        // key. The following for loop code is functionally equivalent
        // to that used in test3() above.        
        byte[] b = new byte[16];
        for (int i = 0; i < 8; i++)
            b[i] = b[8 + i] = (byte)(8 - i);
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test5() throws Exception {
        int[] data = {
   18, 137,   3,  44, 179, 185,  93, 230,   127, 241,  49,  31, 136, 105, 240,  21,
   18, 137,   3,  44, 179, 185,  93, 230,   127, 241,  49,  31, 136, 105, 240,  21,
   90,  66, 146,  57, 213,  20, 160,  29,    97,  17, 200, 117, 225, 187,  79, 170,
  248, 157,   7, 200, 220, 237,  22, 209,    88, 114,  63, 233,  77, 121,  72,  88,
  210,  88, 247, 213, 135, 182,  73, 127,   253,  46, 224,  20,  23, 240, 166, 236,
  105,  19, 251, 246, 255, 225,  70,  32,   198,  78,  25,  12,  13,  10, 181, 136,
  111, 165, 163, 110, 119, 225, 189,  19,   141, 195,  52, 246, 182,  99,  86, 150,
  115, 139, 227,  14,  40, 161,   6, 252,   156, 253,  33, 133, 167, 121, 148, 124,
   17,  53,  17, 194,  46, 121,  54, 223,   158, 235,  45,  23, 192,  88,  20,  55,
  189,  10,   9,  37, 150, 229,  58, 181,   154,  45,   0, 118,  16, 195, 222,   6,
  118, 246, 201,  66, 234, 100,  64,  66,   188,  83,  41, 128, 222, 163, 141,  21,
   70, 176, 202, 249, 121,   7,  61, 102,   205,  76,  17, 232,   2,  17, 235,  10,
  252, 150, 232,  84, 114, 108, 208, 236,   170, 110,  90, 179,   5,  33, 242, 223,
   23, 229, 149, 166, 172, 184, 101, 219,    38, 244, 232,  18,  24, 130,  67,  39,
   67, 230,   3,  18,  82, 130, 164, 238,   195,  12, 225,  83,  31, 113,  52,  79,
  200,  93,  22, 110,  35,   5, 185, 123,    91, 245, 245,  54, 146, 154, 172, 124,
  222,  91,  96, 112, 125,  17, 206, 170,    18, 128, 119,  30,  11, 223,  41, 238,
  246, 179, 146,  99,  52, 210,  95, 140,   158, 173, 224,  60, 208, 158, 118, 190
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 01020304050607080807060504030201 -r 12\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
        ((Parameterized) alg).setParameter("rounds", new Integer(12));
        byte[] b = new byte[16];
        for (int i = 0; i < 8; i++)
            b[i] = b[15 - i] = (byte)(i + 1);
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test6() throws Exception {
        int[] data = {
  251, 144, 115, 211,  68,  87,  59,  12,    21,  27, 255,   2, 173,  17, 191,  45,
  251, 144, 115, 211,  68,  87,  59,  12,    21,  27, 255,   2, 173,  17, 191,  45,
  229, 117,  85, 247,  74, 153, 163,  85,   206, 201, 191,  54, 204, 202,  90,  44,
   14, 245,  31, 156,  84, 150, 204, 139,   229,  23, 225,  47, 154, 235,  35, 238,
  112, 249, 114,  21, 162, 199, 168, 244,   120, 175, 114, 205,  14, 104, 178,  57,
   93,  59, 252,  73,  34, 161, 109, 125,    72,   0,  40,  87, 137,  72, 206, 118,
  239,  96, 232, 157, 143, 171, 180,  46,   112, 194,  16, 200, 111, 135, 159,  41,
  144, 137, 177, 229,  60,  50, 245,  33,   248, 206,  90, 198, 166,   6,  27, 197,
  125, 101, 158,  44, 132, 176, 113,  88,   173, 224,  16, 183,   6, 201, 162, 225,
  154, 190,  44, 133, 190,  45, 118,  20,   235, 196, 169, 198, 194,  92, 242,  21,
  171, 165, 233,  90, 101, 139, 244,  71,    60,  15,  47,  40,   1,  64, 110,   2,
  200,   4, 112, 190, 166,  64, 150, 228,     3, 181,  54, 127, 143, 171,  32, 251,
  213, 179, 250, 249, 101, 249, 212,  76,   245, 128, 115, 148,  90, 208, 164,  63,
   55,  89,  10, 141,  56, 240, 101,  33,    30,  95, 252, 243, 141, 126,  56,  48,
    4, 156, 148, 205, 121, 113,  55,  66,   205, 139,  22,  85,  61,  21, 156,  24,
   42, 119, 129, 122, 163,  88, 186,  39,    45, 126,  27,   6, 241, 142, 162,  99,
  110, 166, 153, 205, 149, 254, 171,  33,    22,   9, 109, 173, 137,  91,  33,  80,
  171, 133, 102,  45, 255, 215,   6, 173,    74, 138, 120, 243,  70,  12,  82,  99
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 0000000000000001 -s -r 6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "SK-64");
        ((Parameterized) alg).setParameter("rounds", new Integer(6));
        byte[] b = new byte[8];
        b[7] = 0x01;
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test7() throws Exception {
        int[] data = {
  233, 169, 154, 148,  75,  45, 142,  87,    95, 206, 155, 162,   5, 132,  56, 199,
  233, 169, 154, 148,  75,  45, 142,  87,    95, 206, 155, 162,   5, 132,  56, 199,
  214,  50, 195, 106, 117, 166, 122,  92,   202,   4, 216,  87,   7, 223, 170,  73,
  239, 104, 126,  77, 226, 113, 115, 185,   215,  50, 152, 187,  52, 171,   2,  40,
  118, 110, 129, 126, 164, 124,  17,  85,   166, 157, 153, 102,  90, 153, 200, 181,
  187, 164,  17,  94,  83, 113,   2, 198,   244,  30, 189,  16,  46,  78, 220,  81,
   30, 161,  31, 115, 123, 181,  76,  50,    97, 220, 154, 210,  80, 130, 205,  14,
   10,  53, 212,  47,  81,  17, 140, 146,   161,  12, 200,  63,  30, 168,  72, 156,
  150, 110, 208, 196, 183,  97, 199, 143,   222, 126, 117, 146,   9, 117, 194, 157,
   54,  39, 223,  55, 138, 168, 147, 186,    83, 140, 115, 208,  37, 135,  74, 118,
   84, 231,  43, 162, 215,  68, 197, 102,    87, 245,  93,  15, 126, 182, 248, 254,
  248, 243, 173, 225, 128, 150, 240,  15,   118, 218,  46,  45, 123,  73, 235, 239,
  160, 248, 224,  97,  22, 245, 178,   3,   208,  69,  47, 191, 162, 228,  56,  91,
  230, 126, 112,  90,  86, 183,   8, 111,    71, 200, 228,  99, 129,  93, 146,  28,
   30,  98, 245, 232,  81,  32, 102,   5,     9, 125, 187, 182, 230,  41,  29, 142,
  241,  20, 113, 121,  35, 186, 240,  65,   182, 188, 127, 113,  95, 112, 142,  75,
  108, 153, 144, 201,  95,   1,  58,   9,   142, 129, 195, 241, 242, 216, 189, 136,
  106,   8, 133, 209, 126, 138,   9,   6,   171, 176, 206,  27, 154,   8,   2, 227
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 0102030405060708 -s -r 6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "SK-64");
        ((Parameterized) alg).setParameter("rounds", new Integer(6));
        byte[] b = new byte[8];
        for (int i = 0; i < 8; i++)
            b[i] = (byte)(i + 1);
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }
    
    private void test8() throws Exception {
        int[] data = {
  249, 176, 171,  27, 220,  97, 189, 230,    65,  76,  84,  90, 182, 153,  74, 247,
  249, 176, 171,  27, 220,  97, 189, 230,    65,  76,  84,  90, 182, 153,  74, 247,
   46, 146, 221,  48, 220,  60, 180,  23,   126, 253, 126, 141, 249, 122, 226, 165,
  167,  55, 252,  75,  43,  44,  32, 108,   245, 218,  93,  55, 108,  36,  89, 124,
  224, 133, 135, 250, 141,  48,  29, 210,    19, 171, 202,  56,  13, 241, 111,  42,
  152, 144, 163,  18, 147,  83, 195,  34,     0, 232, 179, 239, 235,  84, 142, 245,
  191, 254,  57, 202,  59,  68, 135, 208,   239, 233, 213, 101, 207, 170,  27, 143,
  220, 121,  54, 138, 190,  38, 116, 209,    21, 216,  16, 131, 193, 245,  80, 235,
   22, 190, 171,  51, 230,  85,  77, 139,    96, 228,  66, 240,   3, 245,  93, 200,
   72, 128,  35, 100, 251, 106,  45, 244,    26, 248, 224, 195, 223,  40, 122, 156,
  191, 188, 191, 143, 160, 162,  43, 153,    13, 117,   4, 196, 145, 105, 132, 253,
  158, 170,  77, 241, 224, 239, 244,  69,    76, 193,  72,  56,  57, 158,  83,  45,
  192, 246, 218,  38, 125, 229,   8, 117,    70,  69,  63,   2, 162,  32, 183,  54,
  223, 182, 226,  88,  82, 232,  35,  61,   168,  28, 135, 156, 128, 131,  61, 148,
  233, 182, 210, 131, 190, 235,  68, 124,   190, 196,  43, 154, 237, 244,  18,  45,
    2, 253, 241, 158,  59, 231, 160, 155,   169, 212, 113, 243, 149,  27,  57, 194,
  173, 110, 113, 131,  48,  19,  34,  87,   250, 153,  35, 119,  30, 188, 187,  54,
   76,  87, 204, 211, 240, 253,  28, 145,    36,  80,  23,  12, 155, 214, 209, 129
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 00000000000000010000000000000001 -s -r 10\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "SK-128"); // this is the default mode
        ((Parameterized) alg).setParameter("rounds", new Integer(10));
        // we'll rely on the fact that our implementation doubles an 8-byte key
        byte[] b = new byte[8];
        b[7] = 0x01;
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }

    // Following the sequence in <code>check.ref</code> here we were missing these sets:
    //    safer -e -ecb -kx 01020304050607080000000000000000 -s -r 10
    //    safer -e -ecb -kx 00000000000000000102030405060708 -s -r 10
    //
    private void test8B() throws Exception {
        int[] data = {
  104, 156,  77,  53,  69, 225, 184, 209,   255, 120,  17, 228, 179, 167,  46, 113,
  104, 156,  77,  53,  69, 225, 184, 209,   255, 120,  17, 228, 179, 167,  46, 113,
   33,  61,  56, 177,   4, 208,  50, 242,   238,   4,  78, 221, 252,  76, 168,  10,
   22, 134, 153,  52,  41, 133, 112,  72,   181,  42,  55,  74, 190,  66, 151, 146,
   66,  68,   7, 201,   2, 129, 170,  21,    14,  95, 190, 213,  76, 127, 109, 170,
  249, 206, 170, 180, 195,  20, 104, 238,    91, 113, 245,  77,  53,   2, 223, 241,
  155,  24,  66, 130,   4, 126, 162,  41,   170, 158,  37, 243, 177, 237, 105, 112,
  220,  14,  61,  40, 203, 182,  95,  68,    50, 255, 202, 143, 248,  91,  14, 228,
  240, 219, 224, 117,  84, 139, 108,  61,   177, 102, 218,   0, 240, 245,  99,  77,
  212, 190,  68, 114, 138, 164,  75,  82,   172, 215,  24, 124,  16, 132, 222, 200,
   46, 189, 146, 205, 132,  30, 113,  49,   104, 137,  97, 109, 214,  46,  58,  80,
   22,  26,  14,  65, 234, 138, 119,  54,    82,  39,  65, 175, 248,   1,  77, 239,
   65,  36, 107, 101, 241, 220, 106, 250,   105, 122, 127,  42, 189, 106,  53,  16,
  187, 116, 162, 228, 204, 228, 245,  44,     1,  43,  53, 173,   0, 164,  56, 120,
   70,  56,  66,  57, 248,  93,  49,  31,   104, 116,  72, 177, 245, 232,   3, 232,
  230, 175,  79, 155, 111, 193,  49, 121,   220, 165, 203,  52, 252,  86, 207,  47,
  174,  48, 118,  70, 109,  66, 204, 143,    54, 169,  90, 239, 124,  66,  54,  86,
   22, 176, 155, 176,  70, 221,  48,  27,   172,  61,  99, 141,  93, 145,  48,  30,
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 01020304050607080000000000000000 -s -r 10\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
//        ((Parameterized) alg).setParameter("variant", "SK-128"); // this is the default variant
//        ((Parameterized) alg).setParameter("rounds", new Integer(10)); // this is also the default
        byte[] b = new byte[16];
        for (int i = 0; i < 8; i++) { b[i] = (byte)(i + 1);  b[i + 8] = (byte)0; }
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }

    private void test8C() throws Exception {
        int[] data = {
    1, 194, 254,  49,   5, 168,  29,  16,    73, 201, 157, 152, 165, 188,  89,   8,
    1, 194, 254,  49,   5, 168,  29,  16,    73, 201, 157, 152, 165, 188,  89,   8,
  142,  84,  64,   7, 168, 177,  52, 112,   147, 150, 146, 127, 150,   8, 125,  55,
  236,   6, 197, 172,  69, 115, 158, 128,    74,  84,  34, 162,  82, 158,  27, 176,
   90, 191,  47,  85,  40,  97, 159, 237,   228, 194, 116,   4,   4,  35, 156,  62,
   49,  80, 122,  19,  20, 124, 173,  88,   191, 103, 205, 164, 106, 165, 178, 133,
   79,  83, 120, 127, 124, 254,  28, 244,   173, 182, 157, 212, 240, 120, 247,  90,
  224, 185, 108, 101, 183,  79,  83,  86,   222, 190, 208, 162, 228, 186,   9,  55,
  136, 213,  53,   6,  47, 224, 109, 160,   219,  86,  60, 145, 172, 116,  48, 107,
  176,  32,   6,  89,  39, 152, 171, 143,    17, 161, 187,  95, 105, 135, 164,   1,
  211,  29, 246,  81,  22,  51, 105,  79,    58,  56,  54, 193, 174,  28, 140, 184,
  244,  40, 192,  18, 185,  19,  13, 176,   160, 217,  42,  37, 241, 100,  49,  80,
   92, 189, 119, 176,  54,  38, 254,  59,    50, 146,  73, 211, 147, 199,  33, 182,
   36, 245, 251, 101, 134, 120, 241, 150,   114, 250, 222,  38,  78, 201, 243, 140,
  199, 196, 185,  46, 222,  24,  66, 229,   232, 251,  77, 219, 221,  32, 101,  22,
   19, 191, 242, 109,  41, 207, 125, 223,   103, 135,  55, 167, 240, 202, 224,  23,
   39, 208,   7,  63, 206, 207, 109,  91,    99,  43,  38,  87,   0, 115,  25,   0,
   93,  30,  36, 147, 239, 236,  50, 109,   120,  18, 104, 132, 241,  85,  45, 194,
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -kx 00000000000000000102030405060708 -s -r 10\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
//        ((Parameterized) alg).setParameter("variant", "SK-128"); // this is the default variant
//        ((Parameterized) alg).setParameter("rounds", new Integer(10)); // this is also the default
        byte[] b = new byte[16];
        for (int i = 0; i < 8; i++) { b[i + 8] = (byte)(i + 1);  b[i] = (byte)0; }
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }

    // The next two tests appear to be "options" only, they should probably be included here.
    //
    // This is using a text key - how to interpret extra chars?
    //    safer -e -ecb -k AaBcDeFgHiJkLmNoPqRsTuVwXyZz0123456789
    //
    private void test8D() throws Exception {
        int[] data = {
  205,  12,  46, 193, 207, 240, 215,  91,    13, 210,  52,   6,  71, 172,   3, 254,
  205,  12,  46, 193, 207, 240, 215,  91,    13, 210,  52,   6,  71, 172,   3, 254,
   96,  29,  39,  40,  55, 146, 237, 123,   163, 222, 184, 162, 239,  94,  48, 227,
  228, 222, 185, 200,   2, 157, 249, 164,    68,  39, 224, 119,  89,  33,  96,   7,
  242, 117, 142,  64,  57, 232, 204, 149,   190, 247, 201, 153,  52,  49,   3,  63,
  155, 237,  13,  46, 120, 238, 140, 219,   122,  30, 255,  85,  78,  61,  10,  48,
  168, 115,  66, 152, 102,  60, 154, 227,   107,  14, 152,  53, 133, 152,  68, 139,
   83, 200,  77,  30, 227,  67,  72, 211,   164, 177,  85, 204, 155, 172, 146,  35,
    8, 193, 190,  95, 239,  98, 140,  95,   233,  65,  18, 184, 202, 230, 205,  91,
  255, 104, 241,   3, 226, 173, 125,   6,   244, 176,  44,  67, 248, 195,  43, 191,
  124,  41, 157,  69, 145,  16, 177, 172,   169, 234, 191, 227,  52, 188, 217, 124,
  141, 132,  66, 147, 148, 208, 139,  53,    74,  34, 190, 164, 218, 124, 123, 229,
  208, 105, 140,  21,  81,  68,  50,  58,   125, 159, 156,  47, 200, 186, 151,  23,
  193, 147, 212, 182,  33, 100, 119, 187,    84, 210, 239, 222, 193, 230, 145,  53,
  240, 212, 114, 130, 109, 253, 150, 142,   230, 219, 160, 207,  73, 147, 129,  21,
   54, 163, 136, 104,  15, 147,  61, 237,    73,  70, 215,  57,  52, 209, 173, 244,
   14,   9,  56,  52,  95,  76,  40,  70,   148,  21, 166,  15,  66, 204,  46, 196,
   68, 243, 154,  48,  91, 149, 142,  20,   118,  65,   1, 124,  83,  86, 111,  82,
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -e -ecb -k AaBcDeFgHiJkLmNoPqRsTuVwXyZz0123456789\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
            // default rounds is 10 for K-128
        byte[] b = new byte[16];
//how to interpret key?
//for (int i = 0; i < 8; i++) { b[i + 8] = (byte)(i + 1);  b[i] = (byte)0; }
        RawSecretKey key = new RawSecretKey("SAFER", b);
            
        alg.initEncrypt(key);
        compareIt(alg.crypt(input), output);
    }

    // This is just combining the options.
    //    safer -ecbkx 42431BA40D291F81D66083C605D3A4D6
    //
    private void test8E() throws Exception {
        int[] data = {
  249, 118, 152, 143, 221,  25, 156, 132,    38,  89, 129, 249, 212, 217,  39,  66,
  249, 118, 152, 143, 221,  25, 156, 132,    38,  89, 129, 249, 212, 217,  39,  66,
  212, 160,  72, 109, 162, 244,  13,   6,   237, 228, 143, 223, 216, 238, 188, 151,
  149, 236, 141, 238, 223, 191,  16, 133,   208, 169, 126,  87, 136,  68,  18,  19,
  235, 112, 249,  81,  19,  74, 153,  74,   133, 255, 187, 201,  93,  49,   1, 135,
  116, 231, 223, 137, 117, 184, 130, 120,   141, 191, 176, 154,  87,  91,  77,  77,
  191,  70, 198, 162,  67, 234,  95,  57,    12,  74, 134,   3, 179,  67,  40,   1,
  138,  85,  70, 107, 230, 140, 199, 224,    16,   6,  76, 150, 237, 122,  76, 178,
  137, 186,  54,  55,  35,  38, 189, 191,   104, 123,  16,  46, 237,  93, 192, 191,
  127, 251, 160,   7,  80, 103,  86, 226,    82, 145,  61,  28,  69,  49,  59, 239,
  153,  40,  25, 164, 175, 163, 103,  54,   157, 181,  57, 211, 173,  10, 254, 209,
  131,  21, 240, 174, 155, 170, 193, 252,    27,  46, 156, 207, 204,  38,  69,  60,
   84, 241, 118, 129, 107, 181,  76,  70,   166, 233, 136,  85, 193, 118, 154, 207,
   19, 129, 201, 152,  80, 235, 240, 180,   220, 151, 183, 196, 193,  98, 104, 193,
  111, 215,  58, 139, 250,  52, 147,  22,   121, 231,  59, 251, 121,   1,  29, 148,
  246, 143, 211, 211, 199, 202, 195,  13,    53, 225, 142,   6,  75,  43, 227,  74,
  151,  96, 118, 187, 166,  56,   3,  15,   164, 176,  21, 134,  77, 114, 247,  69,
  159, 143, 156, 109, 116, 149,  75, 109,   138,  83,  97,  21,  95,  62,  45,  41,
    };

        out.println("\nSAFER in ECB mode: " +
            "safer -ecbkx 42431BA40D291F81D66083C605D3A4D6\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
            // default rounds is 10 for K-128
//      String kk = "42431BA40D291F81D66083C605D3A4D6";
//      RawSecretKey key = new RawSecretKey("SAFER", Hex.fromString(kk));

        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
    }


//............................... MODE TESTS ................................
//
// Mode tests are not strictly necessary.
// The next test in <code>check.ref</code> is test #11, further down.
// It is a (CBC) mode test:
//    safer -cbckx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A
//...........................................................................

    private void test9() throws Exception {
        int[] data = {
  251, 221, 148,   3, 222,  99, 246, 105,     9, 253, 180,  32,  22,  75,  11, 213,
   56, 232, 236, 153, 153,  92, 253,  97,    39,  54,  14,  69,  23,  95, 212,  37,
  161,   9,  26, 224, 245, 215,  57, 210,    53, 131,  63, 111,  94, 178,  65, 235,
  183,  88, 244,  42,  86,   6, 154, 213,   111,  59, 179,  11,   4,   4,  39, 199,
    8, 190, 114, 105, 117, 168, 126, 106,   226, 247,  84, 100, 219, 249,  30,  18,
  250,  82, 221,  11, 124, 218, 156,  27,   108,  72,  43, 222, 132, 148,  37, 177,
  185, 197, 166,  74,  41,  88, 247, 169,    14, 163,  26, 178,  79,  15,  49, 112,
  129, 171,  54, 147, 224,  78, 181,  50,   221, 191, 166, 224, 235,  73, 160, 113,
  170, 172, 188,  19,  25, 154, 114, 144,   158,  64, 185,  30, 243, 120, 184, 193,
  128, 159, 136, 128,  49, 166,  81,  75,   166, 120,  95, 116, 225,  37, 228,  62,
  245,  30,  89, 115,  23, 205, 247, 137,   197, 194, 211,  91, 115, 241, 191,  93,
  113, 126, 232, 106, 118,  95,  62, 206,    69, 102,  42, 176, 155, 102, 166,  27,
   82,   6, 174,  34, 144, 203,  93, 255,    57, 197,   9,  64, 138, 225, 204, 151,
  214,   0, 123, 225,  85, 191, 233,  18,   176, 217, 192, 128, 162, 213,  16, 212,
  101,  97, 100, 135, 194, 186,  79, 105,   216,  70, 152,  40, 134, 217,  76, 115,
   65,  89, 202,  13, 123, 181,  94,  35,    38, 253, 153, 104, 137, 244, 208, 102,
   94, 142,  58, 219, 141, 132, 166, 136,   174, 247,  99,  31,  50,  74,  68,  79,
  106,  89,  51,  71,  70,   6,  84,  69,   139, 229,  78, 232, 188, 118,  26,  17
    };

        out.println("\nSAFER in CFB mode: " +
            "safer -cfbkx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER/CFB", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
        ((FeedbackCipher) alg).setInitializationVector(anIV);
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);

        out.println("\nDecrypting input and checking it against output:\n");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }
    
    private void test10() throws Exception {
        int[] data = {
  251, 221, 148,   3, 222,  99, 246, 105,     9, 253, 180,  32,  22,  75,  11, 213,
  162,  53, 126,  83, 219,  33,  48, 193,    63, 143,  14, 194,  20,  93, 226,  67,
   66,  17, 114,  26, 204, 190, 221, 173,   181,  32, 143, 213,  75,  56, 106, 155,
   19,  68, 122, 230, 144,  69,  36, 218,   207,  25,  28,  90, 228, 113,   4, 170,
  201,  65, 206,  75, 122,  23,  55, 252,    77, 162,   4, 254, 172,  25, 129, 238,
  230,  17,  91, 175,  92, 252, 132, 126,   252, 174, 150, 132, 148, 118,  43, 206,
   36,  16, 155,  58, 130,  46, 173,  74,    88, 230, 128, 171, 154, 183,  67, 235,
   75,  70, 180, 110, 170, 220, 206, 193,   237, 221, 214,  64, 157, 118, 200,   3,
   31, 200,   1, 251,   5,  54, 135,  22,    51, 245, 132, 202, 244, 202,  40,  39,
   98, 110, 244, 198,  96,  90,  76, 174,    53, 128, 217, 224,  53,  69, 118,  51,
  230, 117,  36, 211, 166,  41, 143, 183,   195, 155,  46,  49, 203,  54,  88,  70,
  216, 185, 212, 220, 122,  45,  93,  43,   167,  67, 131,  82,   7, 104,  18,  90,
  238,  97, 204, 240,  42, 153, 157, 105,   194,  77,  87, 247,  54, 210, 142,  56,
   31, 191,  59, 182,  26, 200,  38,  91,    76,  22, 100, 209,   2, 135,  77, 156,
  231, 128,  81, 241,  44, 218,   8, 227,    50, 236, 152, 128,  25, 206,  82,  62,
  105,  57,  67, 153,  42,  57, 105, 198,   135,  68,  17,  54,  73, 216, 136,   9,
   91,  85, 103, 146, 107,  37, 224,   1,   238, 149,  31, 133, 147, 138, 233,  61,
  187, 120, 162,   6, 230,  35,  39,  75,   169, 142, 185, 209, 195, 141,  66, 198
    };

        out.println("\nSAFER in OFB mode: " +
            "safer -ofbkx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A\n");
        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER/OFB", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
        ((FeedbackCipher) alg).setInitializationVector(anIV);
        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);
        
        out.println("\nDecrypting input and checking it against output:\n");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }
    
    private void test11() throws Exception {
        int[] data = {
  251, 221, 148,   3, 222,  99, 246, 105,   226,  42,  71,  84,  68,  15,  77, 253,
  159, 242, 171, 246, 112,  85,   1,  17,   201, 140,  36,  25, 241, 217, 207,  44,
   80, 203, 156, 123,   6, 117, 108,  33,   253, 118, 156,  50, 215,  83, 192, 211,
  192,  96, 132, 205, 118,  92, 206, 254,   202, 230, 171, 236,  51, 135, 233, 194,
   53,  57,  72,  89,   0, 227, 222,  39,   185,  94, 100, 159,  46, 224, 186, 134,
  232,  36,  54, 169,  81, 223, 150, 161,   105,  97, 100, 184,  37, 205, 111, 144,
   43, 156, 135, 129,  56,  57,   2, 115,    59, 120, 171,  26, 204,  97, 254,  53,
  164,  17, 135,  22,  23, 121,  49, 144,   173,  38, 181,  61, 227, 132, 145,  40,
   75,  69,  65, 164,  65,  64,  25,   9,   199,  38, 132, 165, 135,  97, 129, 144,
    4, 179, 112,  60, 182, 250, 222, 252,   248,  23,  59, 228, 191,   0,  56, 237,
  163,  20,  30, 104, 216,  45, 209, 254,   107, 188, 169,  97, 173,  41, 205, 249,
  164, 128, 120, 201, 215, 188,  35, 103,   108, 202, 174, 140, 138,  95, 241,  71,
   83, 199,  70, 162,  73, 226,  87,  84,    43,  17, 178, 203,  21, 250, 128, 159,
  137, 190, 174,  11, 215,  22, 155,  34,    34, 169,  13,  95, 208, 118, 231,  89,
   88, 179,   4,  29, 176,  17,  64, 235,    58,  88, 114,  65,  57,  96, 211, 163,
  188,  18, 100,   4,  91,  45, 233,  80,   217,  90,  45,  93, 144,  82,  60, 203,
  224,  81, 113, 231, 159, 185, 171, 128,   109, 224, 154, 141,  33,  66, 255,  44,
  112,  41, 125, 205, 154, 174, 226, 225,   173, 243, 107, 172, 214, 234,  22, 186
    };

        out.println("\nSAFER in CBC mode: " +
            "safer -cbckx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A\n");

        byte[] output = new byte[data.length];
        for (int i = 0; i < data.length; i++)
            output[i] = (byte)data[i];

        Cipher alg = Cipher.getInstance("SAFER/CBC", "Cryptix");
        ((Parameterized) alg).setParameter("variant", "K-128");
        ((FeedbackCipher) alg).setInitializationVector(anIV);
        alg.initEncrypt(aKey);
        byte[] x = alg.crypt(input);
        compareIt(x, output);
        
        out.println("\nNow decrypting this and checking it against input:\n");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(x), input);
    }

    // These (skipped) tests from <code>check.ref</code> are Hash Functions
    // tandem Davies-Meyer scheme:
    //        safer -h -tan -kx 42431BA40D291F81D66083C605D3A4D6
    // abreast Davies-Meyer scheme:
    //        safer -h -abr -kx 42431BA40D291F81D66083C605D3A4D6

    private void compareIt (byte[] o1, byte[] o2) {
        out.print("  computed output:" + Hex.dumpString(o1));
        boolean ok = ArrayUtil.areEqual(o1, o2);
        if (!ok)
            out.print("\n certified output:" + Hex.dumpString(o2));
        passIf(ok, " *** SAFER OUTPUT");
    }
}
