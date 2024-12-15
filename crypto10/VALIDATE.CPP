#include "md5.h"
#include "shs.h"
#include "idea.h"
#include "des.h"
#include "rc4.h"
#include "rc5.h"
#include "blowfish.h"
#include "diamond.h"
#include "wake.h"
#include "blumshub.h"

#include <iostream.h>
#include <iomanip.h>

#include "rsa.h"
#include "rsarefcl.h"
#include "elgamal.h"
#include "dsa.h"
#include "dh.h"
#include "zeroknow.h"
#include "asn.h"
#include "rng.h"

#include <string.h>
#include <fstream.h>

#include "validate.h"

#include "files.h"
#include "hex.h"

boolean ValidateAll()
{
    boolean pass=TestEndianess();

    pass=ZKValidate() && pass;
    pass=MD5Validate() && pass;
    pass=SHSValidate() && pass;
    pass=DESValidate() && pass;
    pass=IDEAValidate() && pass;
    pass=RC4Validate() && pass;
    pass=RC5Validate() && pass;
    pass=BlowfishValidate() && pass;
    pass=DiamondValidate() && pass;
    pass=BBSValidate() && pass;
    pass=DHValidate() && pass;
    pass=RSAValidate() && pass;
    pass=ElGamalValidate() && pass;
    pass=DSAValidate() && pass;

    if (pass)
        cout << "\nAll tests passed!\n";
    else
        cout << "\nOops!  Not all tests passed.\n";

    return pass;
}

boolean TestEndianess()
{
    int pass;

    cout << "\nTesting Endianess...\n\n";

    if (*(word32 *)"\x01\x00\x00\x00" == 1L)
    {
#ifdef LITTLE_ENDIAN
        cout << "PASSED:  ";
        pass = TRUE;
#else
        cout << "FAILED:  (check config.h)  ";
        pass = FALSE;
#endif
        cout << "Your machine is little endian.\n";
    }
    else if (*(word32 *)"\x01\x00\x00\x00" == 0x01000000L)
    {
#ifndef LITTLE_ENDIAN
        cout << "PASSED:  ";
        pass = TRUE;
#else
        cout << "FAILED:  (check config.h)  ";
        pass = FALSE;
#endif
        cout << "Your machine is big endian.\n";
    }
    else
    {
        cout << "FAILED:  Your machine is neither big endian nor little endian.\n";
        pass = FALSE;
    }
    return pass;
}

boolean MD5Validate()
{
  byte TestVals[][100]=
  {
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
  };

  byte output[][MD5::DIGESTSIZE+1]={
"\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e",
"\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61",
"\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72",
"\xf9\x6b\x69\x7d\x7c\xb7\x93\x8d\x52\x5a\x2f\x31\xaa\xf1\x61\xd0",
"\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b",
"\xd1\x74\xab\x98\xd2\x77\xd9\xf5\xa5\x61\x1c\x2c\x9f\x41\x9d\x9f",
"\x57\xed\xf4\xa2\x2b\xe3\xc9\x55\xac\x49\xda\x2e\x21\x07\xb6\x7a"};

    MD5 md5;
    byte digest[MD5::DIGESTSIZE];
    boolean pass=TRUE, fail;

    cout << "\nMD5 validation suite running...\n\n";

    for (int i=0;i<7;i++)
    {
        md5.Update(TestVals[i], strlen((char *)TestVals[i]));
        md5.Final(digest);
        fail = memcmp(digest, output[i], MD5::DIGESTSIZE);
        pass = pass && !fail;
        cout << (fail ? "FAILED   " : "PASSED   ");
        for (int j=0;j<MD5::DIGESTSIZE;j++)
            cout << setw(2) << setfill('0') << hex << (int)digest[j];
        cout << "   \"" << TestVals[i] << '\"' << endl;
    }

    return pass;
}

boolean SHSValidate()
{

#ifdef NEW_SHS
byte shsTestResults[][SHS::DIGESTSIZE+1] = {
    "\xA9\x99\x3E\x36\x47\x06\x81\x6A\xBA\x3E\x25\x71\x78\x50\xC2\x6C\x9C\xD0\xD8\x9D",
    "\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1",
    "\x34\xAA\x97\x3C\xD4\xC4\xDA\xA4\xF6\x1E\xEB\x2B\xDB\xAD\x27\x31\x65\x34\x01\x6F"};
#else
byte shsTestResults[][SHS::DIGESTSIZE+1] = {
    "\x01\x64\xB8\xA9\x14\xCD\x2A\x5E\x74\xC4\xF7\xFF\x08\x2C\x4D\x97\xF1\xED\xF8\x80",
    "\xD2\x51\x6E\xE1\xAC\xFA\x5B\xAF\x33\xDF\xC1\xC4\x71\xE4\x38\x44\x9E\xF1\x34\xC8",
    "\x32\x32\xAF\xFA\x48\x62\x8A\x26\x65\x3B\x5A\xAA\x44\x54\x1F\xD9\x0D\x69\x06\x03"};
#endif /* NEW_SHS */

    byte digest[SHS::DIGESTSIZE];
    SHS shs;
    boolean pass=TRUE;
    int j;

    cout << "\nSHS validation suite running...\n\n";

    cout << "Test 1: ";
    shs.Update(( byte * ) "abc", 3 );
    shs.Final(digest);
    if (!memcmp(digest, shsTestResults[0], SHS::DIGESTSIZE))
        cout << "PASSED    ";
    else
    {
        cout << "FAILED    ";
        pass = FALSE;
    }
    for (j=0;j<SHS::DIGESTSIZE;j++)
        cout << setw(2) << setfill('0') << hex << (int)digest[j];
    cout << endl;

    cout << "Test 2: ";
    shs.Update(( byte * ) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56 );
    shs.Final(digest);
    if (!memcmp(digest, shsTestResults[1], SHS::DIGESTSIZE))
        cout << "PASSED    ";
    else
    {
        cout << "FAILED    ";
        pass = FALSE;
    }
    for (j=0;j<SHS::DIGESTSIZE;j++)
        cout << setw(2) << setfill('0') << hex << (int)digest[j];
    cout << endl;

    cout << "Test 3: ";
    for (int i = 0; i < 15625; i++ )
        shs.Update(( byte * ) "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 64 );
    shs.Final(digest);
    if (!memcmp(digest, shsTestResults[2], SHS::DIGESTSIZE))
        cout << "PASSED    ";
    else
    {
        cout << "FAILED    ";
        pass = FALSE;
    }
    for (j=0;j<SHS::DIGESTSIZE;j++)
        cout << setw(2) << setfill('0') << hex << (int)digest[j];
    cout << endl;

    return pass;
}

boolean DESValidate()
{
    cout << "\nDES validation suite running...\n\n";

    FileSource valdata("descert.dat", TRUE, new HexDecoder);
    HexEncoder output(new FileSink(cout));
    byte plain[8], cipher[8], out[8], outplain[8];
    byte key[8];
    boolean pass=TRUE, fail;

    while (valdata.MaxRetrieveable() >= 24)
    {
        valdata.Get(key, 8);
        valdata.Get(plain, 8);
        valdata.Get(cipher, 8);

        BlockTransformation *des = new DESEncryption(key);
        des->ProcessBlock(plain, out);
        delete des;
        fail=memcmp(out, cipher, 8);

        des = new DESDecryption(key);
        des->ProcessBlock(out, outplain);
        delete des;
        fail=fail || memcmp(outplain, plain, 8);

        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        output.Put(key, 8);
        cout << "    ";
        output.Put(outplain, 8);
        cout << "    ";
        output.Put(out, 8);
        cout << endl;
    }
    return pass;
}

boolean IDEAValidate()
{
    cout << "\nIDEA validation suite running...\n\n";

    FileSource valdata("ideaval.dat", TRUE, new HexDecoder);
    HexEncoder output(new FileSink(cout));
    byte plain[8], cipher[8], out[8], outplain[8];
    byte key[16];
    boolean pass=TRUE, fail;

    while (valdata.MaxRetrieveable() >= 32)
    {
        valdata.Get(key, 16);
        valdata.Get(plain, 8);
        valdata.Get(cipher, 8);

        BlockTransformation *idea = new IDEAEncryption(key);
        idea->ProcessBlock(plain, out);
        delete idea;
        fail=memcmp(out, cipher, 8);

        idea = new IDEADecryption(key);
        idea->ProcessBlock(out, outplain);
        delete idea;
        fail=fail || memcmp(outplain, plain, 8);

        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        output.Put(key, 16);
        cout << "  ";
        output.Put(outplain, 8);
        cout << "  ";
        output.Put(out, 8);
        cout << endl;
    }
    return pass;
}

boolean RC4Validate()
{
unsigned char Key0[] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
unsigned char Input0[]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
unsigned char Output0[] = {0x75,0xb7,0x87,0x80,0x99,0xe0,0xc5,0x96};

unsigned char Key1[]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
unsigned char Input1[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char Output1[]={0x74,0x94,0xc2,0xe7,0x10,0x4b,0x08,0x79};

unsigned char Key2[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char Input2[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char Output2[]={0xde,0x18,0x89,0x41,0xa3,0x37,0x5d,0x3a};

unsigned char Key3[]={0xef,0x01,0x23,0x45};
unsigned char Input3[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char Output3[]={0xd6,0xa1,0x41,0xa7,0xec,0x3c,0x38,0xdf,0xbd,0x61};

unsigned char Key4[]={ 0x01,0x23,0x45,0x67,0x89,0xab, 0xcd,0xef };
unsigned char Input4[] =
{0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01};
unsigned char Output4[]= {
0x75,0x95,0xc3,0xe6,0x11,0x4a,0x09,0x78,0x0c,0x4a,0xd4,
0x52,0x33,0x8e,0x1f,0xfd,0x9a,0x1b,0xe9,0x49,0x8f,
0x81,0x3d,0x76,0x53,0x34,0x49,0xb6,0x77,0x8d,0xca,
0xd8,0xc7,0x8a,0x8d,0x2b,0xa9,0xac,0x66,0x08,0x5d,
0x0e,0x53,0xd5,0x9c,0x26,0xc2,0xd1,0xc4,0x90,0xc1,
0xeb,0xbe,0x0c,0xe6,0x6d,0x1b,0x6b,0x1b,0x13,0xb6,
0xb9,0x19,0xb8,0x47,0xc2,0x5a,0x91,0x44,0x7a,0x95,
0xe7,0x5e,0x4e,0xf1,0x67,0x79,0xcd,0xe8,0xbf,0x0a,
0x95,0x85,0x0e,0x32,0xaf,0x96,0x89,0x44,0x4f,0xd3,
0x77,0x10,0x8f,0x98,0xfd,0xcb,0xd4,0xe7,0x26,0x56,
0x75,0x00,0x99,0x0b,0xcc,0x7e,0x0c,0xa3,0xc4,0xaa,
0xa3,0x04,0xa3,0x87,0xd2,0x0f,0x3b,0x8f,0xbb,0xcd,
0x42,0xa1,0xbd,0x31,0x1d,0x7a,0x43,0x03,0xdd,0xa5,
0xab,0x07,0x88,0x96,0xae,0x80,0xc1,0x8b,0x0a,0xf6,
0x6d,0xff,0x31,0x96,0x16,0xeb,0x78,0x4e,0x49,0x5a,
0xd2,0xce,0x90,0xd7,0xf7,0x72,0xa8,0x17,0x47,0xb6,
0x5f,0x62,0x09,0x3b,0x1e,0x0d,0xb9,0xe5,0xba,0x53,
0x2f,0xaf,0xec,0x47,0x50,0x83,0x23,0xe6,0x71,0x32,
0x7d,0xf9,0x44,0x44,0x32,0xcb,0x73,0x67,0xce,0xc8,
0x2f,0x5d,0x44,0xc0,0xd0,0x0b,0x67,0xd6,0x50,0xa0,
0x75,0xcd,0x4b,0x70,0xde,0xdd,0x77,0xeb,0x9b,0x10,
0x23,0x1b,0x6b,0x5b,0x74,0x13,0x47,0x39,0x6d,0x62,
0x89,0x74,0x21,0xd4,0x3d,0xf9,0xb4,0x2e,0x44,0x6e,
0x35,0x8e,0x9c,0x11,0xa9,0xb2,0x18,0x4e,0xcb,0xef,
0x0c,0xd8,0xe7,0xa8,0x77,0xef,0x96,0x8f,0x13,0x90,
0xec,0x9b,0x3d,0x35,0xa5,0x58,0x5c,0xb0,0x09,0x29,
0x0e,0x2f,0xcd,0xe7,0xb5,0xec,0x66,0xd9,0x08,0x4b,
0xe4,0x40,0x55,0xa6,0x19,0xd9,0xdd,0x7f,0xc3,0x16,
0x6f,0x94,0x87,0xf7,0xcb,0x27,0x29,0x12,0x42,0x64,
0x45,0x99,0x85,0x14,0xc1,0x5d,0x53,0xa1,0x8c,0x86,
0x4c,0xe3,0xa2,0xb7,0x55,0x57,0x93,0x98,0x81,0x26,
0x52,0x0e,0xac,0xf2,0xe3,0x06,0x6e,0x23,0x0c,0x91,
0xbe,0xe4,0xdd,0x53,0x04,0xf5,0xfd,0x04,0x05,0xb3,
0x5b,0xd9,0x9c,0x73,0x13,0x5d,0x3d,0x9b,0xc3,0x35,
0xee,0x04,0x9e,0xf6,0x9b,0x38,0x67,0xbf,0x2d,0x7b,
0xd1,0xea,0xa5,0x95,0xd8,0xbf,0xc0,0x06,0x6f,0xf8,
0xd3,0x15,0x09,0xeb,0x0c,0x6c,0xaa,0x00,0x6c,0x80,
0x7a,0x62,0x3e,0xf8,0x4c,0x3d,0x33,0xc1,0x95,0xd2,
0x3e,0xe3,0x20,0xc4,0x0d,0xe0,0x55,0x81,0x57,0xc8,
0x22,0xd4,0xb8,0xc5,0x69,0xd8,0x49,0xae,0xd5,0x9d,
0x4e,0x0f,0xd7,0xf3,0x79,0x58,0x6b,0x4b,0x7f,0xf6,
0x84,0xed,0x6a,0x18,0x9f,0x74,0x86,0xd4,0x9b,0x9c,
0x4b,0xad,0x9b,0xa2,0x4b,0x96,0xab,0xf9,0x24,0x37,
0x2c,0x8a,0x8f,0xff,0xb1,0x0d,0x55,0x35,0x49,0x00,
0xa7,0x7a,0x3d,0xb5,0xf2,0x05,0xe1,0xb9,0x9f,0xcd,
0x86,0x60,0x86,0x3a,0x15,0x9a,0xd4,0xab,0xe4,0x0f,
0xa4,0x89,0x34,0x16,0x3d,0xdd,0xe5,0x42,0xa6,0x58,
0x55,0x40,0xfd,0x68,0x3c,0xbf,0xd8,0xc0,0x0f,0x12,
0x12,0x9a,0x28,0x4d,0xea,0xcc,0x4c,0xde,0xfe,0x58,
0xbe,0x71,0x37,0x54,0x1c,0x04,0x71,0x26,0xc8,0xd4,
0x9e,0x27,0x55,0xab,0x18,0x1a,0xb7,0xe9,0x40,0xb0,
0xc0};

    RC4 *rc4;
    boolean pass=TRUE, fail;
    int i;

    cout << "\nRC4 validation suite running...\n\n";

    rc4 = new RC4(Key0, sizeof(Key0));
    rc4->ProcessString(Input0, sizeof(Input0));
    fail = memcmp(Input0, Output0, sizeof(Input0));
    cout << (fail ? "FAILED" : "PASSED") << "    Test 0" << endl;
    pass = pass && !fail;
    fail=0;
    delete rc4;

    rc4 = new RC4(Key1, sizeof(Key1));
    rc4->ProcessString(Key1, Input1, sizeof(Key1));
    fail = memcmp(Output1, Key1, sizeof(Key1));
    cout << (fail ? "FAILED" : "PASSED") << "    Test 1" << endl;
    pass = pass && !fail;
    fail=0;
    delete rc4;

    rc4 = new RC4(Key2, sizeof(Key2));
    for (i=0;i<sizeof(Input2);i++)
        if (rc4->ProcessByte(Input2[i]) != Output2[i])
            fail=1;
    cout << (fail ? "FAILED" : "PASSED") << "    Test 2" << endl;
    pass = pass && !fail;
    fail=0;
    delete rc4;

    rc4 = new RC4(Key3, sizeof(Key3));
    for (i=0;i<sizeof(Input3);i++)
        if (rc4->ProcessByte(Input3[i]) != Output3[i])
            fail=1;
    cout << (fail ? "FAILED" : "PASSED") << "    Test 3" << endl;
    pass = pass && !fail;
    fail=0;
    delete rc4;

    rc4 = new RC4(Key4, sizeof(Key4));
    for (i=0;i<sizeof(Input4);i++)
        if (rc4->ProcessByte(Input4[i]) != Output4[i])
            fail=1;
    cout << (fail ? "FAILED" : "PASSED") << "    Test 4" << endl;
    pass = pass && !fail;
    fail=0;
    delete rc4;

    return pass;
}

boolean RC5Validate()
{
    cout << "\nRC5 validation suite running...\n\n";

    FileSource valdata("rc5val.dat", TRUE, new HexDecoder);
    HexEncoder output(new FileSink(cout));
    byte plain[8], cipher[8], out[8], outplain[8];
    byte key[16];
    boolean pass=TRUE, fail;

    while (valdata.MaxRetrieveable() >= 32)
    {
        valdata.Get(key, 16);
        valdata.Get(plain, 8);
        valdata.Get(cipher, 8);

        BlockTransformation *rc5 = new RC5Encryption(key);
        rc5->ProcessBlock(plain, out);
        delete rc5;
        fail=memcmp(out, cipher, 8);

        rc5 = new RC5Decryption(key);
        rc5->ProcessBlock(out, outplain);
        delete rc5;
        fail=fail || memcmp(outplain, plain, 8);

        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        output.Put(key, 16);
        cout << "  ";
        output.Put(outplain, 8);
        cout << "  ";
        output.Put(out, 8);
        cout << endl;
    }
    return pass;
}

boolean BlowfishValidate()
{
    cout << "\nBlowfish validation suite running...\n\n";

    HexEncoder output(new FileSink(cout));
    char *key[]={"abcdefghijklmnopqrstuvwxyz", "Who is John Galt?"};
    byte *plain[]={(byte *)"BLOWFISH", (byte *)"\xfe\xdc\xba\x98\x76\x54\x32\x10"};
    byte *cipher[]={(byte *)"\x32\x4e\xd0\xfe\xf4\x13\xa2\x03", (byte *)"\xcc\x91\x73\x2b\x80\x22\xf6\x84"};
    byte out[8], outplain[8];
    boolean pass=TRUE, fail;

    for (int i=0; i<2; i++)
    {
        BlockTransformation *bf = new BlowfishEncryption((byte *)key[i], strlen(key[i]));
        bf->ProcessBlock(plain[i], out);
        delete bf;
        fail = memcmp(out, cipher[i], 8);

        bf = new BlowfishDecryption((byte *)key[i], strlen(key[i]));
        bf->ProcessBlock(cipher[i], outplain);
        delete bf;
        fail = fail || memcmp(outplain, plain[i], 8);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << '\"' << key[i] << '\"';
        for (int j=0; j<(30-strlen(key[i])); j++)
            cout << ' ';
        output.Put(outplain, 8);
        cout << "  ";
        output.Put(out, 8);
        cout << endl;
    }
    return pass;
}

boolean DiamondValidate()
{
    cout << "\nDiamond validation suite running...\n\n";

    FileSource valdata("diamond.dat", TRUE, new HexDecoder);
    HexEncoder output(new FileSink(cout));
    byte key[32], plain[16], cipher[16], out[16], outplain[16];
    byte blocksize, rounds, keysize;
    boolean pass=TRUE, fail;

    while (valdata.MaxRetrieveable() >= 1)
    {
        valdata.Get(blocksize);
        valdata.Get(rounds);
        valdata.Get(keysize);
        valdata.Get(key, keysize);
        valdata.Get(plain, blocksize);
        valdata.Get(cipher, blocksize);

        BlockTransformation *diamond;
        if (blocksize==16)
            diamond = new DiamondEncryption(key, keysize, rounds);
        else
            diamond = new DiamondLiteEncryption(key, keysize, rounds);

        diamond->ProcessBlock(plain, out);
        delete diamond;
        fail=memcmp(out, cipher, blocksize);

        if (blocksize==16)
            diamond = new DiamondDecryption(key, keysize, rounds);
        else
            diamond = new DiamondLiteDecryption(key, keysize, rounds);

        diamond->ProcessBlock(out, outplain);
        delete diamond;
        fail=fail || memcmp(outplain, plain, blocksize);

        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        output.Put(key, keysize);
        cout << "\n          ";
        output.Put(outplain, blocksize);
        cout << "  ";
        output.Put(out, blocksize);
        cout << endl;
    }
    return pass;
}

boolean BBSValidate()
{
    cout << "\nBlumBlumShub validation suite running...\n\n";

    bignum p("212004934506826557583707108431463840565872545889679278744389317666981496005411448865750399674653351");
    bignum q("100677295735404212434355574418077394581488455772477016953458064183204108039226017738610663984508231");
    bignum seed("63239752671357255800299643604761065219897634268887145610573595874544114193025997412441121667211431");
    BlumBlumShub bbs(p, q, seed);
    boolean pass = TRUE, fail;

    const byte output1[] = {
        0x49,0xEA,0x2C,0xFD,0xB0,0x10,0x64,0xA0,0xBB,0xB9,
        0x2A,0xF1,0x01,0xDA,0xC1,0x8A,0x94,0xF7,0xB7,0xCE};
    const byte output2[] = {
        0x74,0x45,0x48,0xAE,0xAC,0xB7,0x0E,0xDF,0xAF,0xD7,
        0xD5,0x0E,0x8E,0x29,0x83,0x75,0x6B,0x27,0x46,0xA1};

    byte buf[20];

    bbs.GetBlock(buf, 20);
    fail = memcmp(output1, buf, 20);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    for (int j=0;j<20;j++)
        cout << setw(2) << setfill('0') << hex << (int)buf[j];
    cout << endl;

    bbs.Seek(10);
    bbs.GetBlock(buf, 10);
    fail = memcmp(output1+10, buf, 10);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    for (j=0;j<10;j++)
        cout << setw(2) << setfill('0') << hex << (int)buf[j];
    cout << endl;

    bbs.Seek(1234567);
    bbs.GetBlock(buf, 20);
    fail = memcmp(output2, buf, 20);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    for (j=0;j<20;j++)
        cout << setw(2) << setfill('0') << hex << (int)buf[j];
    cout << endl;

    return pass;
}

boolean DHValidate()
{
    cout << "\nDH validation suite running...\n\n";

    DH dh1(FileSource("dhparams.dat", TRUE, new HexDecoder()));
#ifdef USE_RSAREF
    RSAREF_DH dh2(FileSource("dhparams.dat", TRUE, new HexDecoder()));
#else
    DH dh2(FileSource("dhparams.dat", TRUE, new HexDecoder()));
#endif

    SecByteBlock pub1(dh1.OutputLength()), pub2(dh2.OutputLength());
    SecByteBlock key1(dh1.OutputLength()), key2(dh2.OutputLength());

    LC_RNG rng(5234);
    dh1.Setup(rng, pub1);
    dh2.Setup(rng, pub2);

    dh1.Agree(pub2, key1);
    dh2.Agree(pub1, key2);

    if (memcmp(key1, key2, dh1.OutputLength()))
    {
        cout << "FAILED    keys not equal" << endl;
        return FALSE;
    }
    else
    {
        cout << "PASSED    keys agreed" << endl;
        return TRUE;
    }
}

boolean RSAValidate()
{
    cout << "\nRSA validation suite running...\n\n";

    byte *plain = (byte *)
        "\x30\x20\x30\x0c\x06\x08\x2a\x86\x48\x86\xf7\x0d\x02\x02\x05\x00\x04"
        "\x10\x1d\x32\xde\x00\x9f\x9c\x56\xea\x46\x36\xd3\x9a\xaf\xfd\xae\xa1";
    byte *privCipher = (byte *)
        "\x05\xfa\x6a\x81\x2f\xc7\xdf\x8b\xf4\xf2\x54\x25\x09\xe0\x3e\x84"
        "\x6e\x11\xb9\xc6\x20\xbe\x20\x09\xef\xb4\x40\xef\xbc\xc6\x69\x21"
        "\x69\x94\xac\x04\xf3\x41\xb5\x7d\x05\x20\x2d\x42\x8f\xb2\xa2\x7b"
        "\x5c\x77\xdf\xd9\xb1\x5b\xfc\x3d\x55\x93\x53\x50\x34\x10\xc1\xe1";
    byte out[100], outPlain[100];
    unsigned int outLen;
    LC_RNG rng(765);
    boolean pass = TRUE, fail;

#ifdef THROW_EXCEPTIONS
    try
#endif
    {
        FileSource keys("rsakey.dat", TRUE, new HexDecoder);
        RSAPrivateKey rsaPriv(keys);
        RSAPublicKey rsaPub(rsaPriv);

        rsaPriv.Encrypt(rng, plain, 34, out);
        fail = memcmp(privCipher, out, 64);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSA private key encryption\n";

        outLen = rsaPub.Decrypt(privCipher, outPlain);
        fail = (outLen!=34) || memcmp(plain, outPlain, 34);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSA public key decryption\n";

        rsaPub.Encrypt(rng, plain, 34, out);
        memset(outPlain, 0, 34);
        outLen = rsaPriv.Decrypt(out, outPlain);
        fail = (outLen!=34) || memcmp(plain, outPlain, 34);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSA public key encryption and private key decryption\n";
    }
#ifdef THROW_EXCEPTIONS
    catch (BERDecodeErr)
    {
        cout << "FAILED    Error decoding RSA key\n";
        pass = FALSE;
    }
#endif

#ifdef USE_RSAREF

#ifdef THROW_EXCEPTIONS
    try
#endif
    {
        FileSource keys("rsakey.dat", TRUE, new HexDecoder);
        RSAREFPrivateKey rsaPriv(keys);
        RSAREFPublicKey rsaPub(rsaPriv);

        rsaPriv.Encrypt(rng, plain, 34, out);
        fail = memcmp(privCipher, out, 64);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSAREF private key encryption\n";

        outLen = rsaPub.Decrypt(privCipher, outPlain);
        fail = (outLen!=34) || memcmp(plain, outPlain, 34);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSAREF public key decryption\n";

        rsaPub.Encrypt(rng, plain, 34, out);
        memset(outPlain, 0, 34);
        outLen = rsaPriv.Decrypt(out, outPlain);
        fail = (outLen!=34) || memcmp(plain, outPlain, 34);
        pass = pass && !fail;

        cout << (fail ? "FAILED    " : "PASSED    ");
        cout << "RSAREF public key encryption and private key decryption\n";
    }
#ifdef THROW_EXCEPTIONS
    catch (BERDecodeErr)
    {
        cout << "FAILED    Error decoding RSAREF key\n";
        pass = FALSE;
    }
#endif

#endif  // USE_RSAREF

    return pass;
}

boolean ElGamalValidate()
{
    cout << "\nElGamal validation suite running...\n\n";

    ElGamalPrivateKey priv(FileSource("elgamal.dat", TRUE, new HexDecoder));
    ElGamalPublicKey pub(priv);
    LC_RNG rng(9374);
    const byte *message = (byte *)"test message";
    const int messageLen = 12;
    byte buffer[256];
    boolean pass = TRUE, fail;

    priv.Sign(rng, message, messageLen, buffer);
    fail = !pub.Verify(message, messageLen, buffer);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "signature and verification\n";

    fail = pub.Verify(message, messageLen-1, buffer);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "checking invalid signature\n";

    pub.Encrypt(rng, message, messageLen, buffer);
    fail = (messageLen!=priv.Decrypt(buffer, buffer));
    fail = fail | memcmp(message, buffer, messageLen);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "encryption and decryption\n";

    return pass;
}

boolean DSAValidate()
{
    cout << "\nDSA validation suite running...\n\n";

    DSAPrivateKey priv(FileSource("dsakey.dat", TRUE, new HexDecoder()));
    DSAPublicKey pub(priv);

    byte seed[]="\xd5\x01\x4e\x4b\x60\xef\x2b\xa8\xb6\x21\x1b\x40\x62\xba\x32\x24\xe0\x42\x7d\xbd";
    bignum p("d411a4a0e393f6aab0f08b14d18458665b3e4dbdce254454"
             "3fe365cf71c8622412db6e7dd02bbe13d88c58d7263e9023"
             "6af17ac8a9fe5f249cc81f427fc543f7H");
    bignum q("b20db0b101df0c6624fc1392ba55f77d577481e5H");
    bignum k("79577ddcaafddc038b865b19f8eb1ada8a2838c6h");
    bignum h("0164b8a914cd2a5e74c4f7ff082c4d97f1edf880h");
    bignum r("9b77,f705,4c81,531c,4e46,a469,2fbf,e0f7,7f7e,bff2h");
    bignum s("95b4,f608,1f8f,890e,4b5a,199e,f10f,fe21,f52b,2d68h");

    bignum pGen, qGen, rOut, sOut;
    boolean pass = TRUE, fail;
    int c;

#ifdef NEW_SHS
    cout << "NEW_SHS defined.  Skipping prime generation test.\n";
#else
    fail = !GenerateDSAPrimes(seed, 160, c, pGen, 512, qGen);
    fail = fail || (pGen != p) || (qGen != q);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "prime generation test\n";
#endif

    priv.RawSign(k, h, rOut, sOut);
    fail = (rOut != r) || (sOut != s);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "signature test\n";

    fail = !pub.RawVerify(h, r, s);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "valid signature verification\n";

    fail = pub.RawVerify(h+1, r, s);
    pass = pass && !fail;

    cout << (fail ? "FAILED    " : "PASSED    ");
    cout << "invalid signature verification\n";

    return pass;
}

boolean ZKValidate()
{
    cout << "\nZero Knowledge validation suite running...\n\n";

    SimpleGraph g1(FileSource("graph.dat", TRUE, new HexDecoder));
    cout << "g1:\n" << g1;

    word16 g1_to_g2[20]={1, 19, 9, 0, 8, 11, 17, 5, 2, 7, 3, 14, 13, 16, 6, 4, 12, 18, 10, 15};
    cout << "g1_to_g2:";
    for (int j=0; j<20; j++)
        cout << j << '-' << g1_to_g2[j] << ' ';
    cout << endl;

    SimpleGraph g2(g1);
    g2.Permutate(g1_to_g2);
    cout << "g2:\n" << g2;

    LC_RNG rng(93753L);
    ZK_IsomorphismProver prover(g1, g1_to_g2, rng);
    ZK_IsomorphismVerifier verifier(g1, g2, rng);

    SimpleGraph h;
    word16 gi_to_h[20];
    int i;
    boolean pass=TRUE;

    for (j=0; j<40; j++)
    {
        prover.Setup(h);
        i=verifier.Setup(h);
        prover.Prove(i, gi_to_h);
        pass = verifier.Verify(gi_to_h) && pass;
    }

    cout << "data from last round:\n";
    cout << "i: " << i << endl;
    cout << "gi_to_h:";
    for (j=0; j<20; j++)
        cout << j << '-' << gi_to_h[j] << ' ';
    cout << endl;
    cout << "h:\n" << h << endl;

    cout << (pass ? "PASSED    " : "FAILED    ") << endl;
    return pass;
}

