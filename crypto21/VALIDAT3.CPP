// validat3.cpp - written and placed in the public domain by Wei Dai

#include "md5.h"
#include "sha.h"
#include "tiger.h"
#include "ripemd.h"
#include "haval.h"

#include "md5mac.h"
#include "hmac.h"
#include "xormac.h"

#include <iostream.h>
#include <iomanip.h>
#include <string.h>

struct HashTestTuple
{
	HashTestTuple(const char *input, const char *output, unsigned int repeatTimes=1)
		: input((byte *)input), output((byte *)output), inputLen(strlen(input)), repeatTimes(repeatTimes) {}
	
	HashTestTuple(const byte *input, const byte *output, unsigned int inputLen, unsigned int repeatTimes)
		: input(input), output(output), inputLen(inputLen), repeatTimes(repeatTimes) {}

	const byte *input, *output;
	unsigned int inputLen, repeatTimes;
};

boolean HashModuleTest(HashModule &md, const HashTestTuple *testSet, unsigned int testSetSize)
{
	boolean pass=TRUE, fail;
	SecByteBlock digest(md.DigestSize());

    for (unsigned int i=0; i<testSetSize; i++)
    {
		unsigned j;

		for (j=0; j<testSet[i].repeatTimes; j++)
			md.Update(testSet[i].input, testSet[i].inputLen);
        md.Final(digest);
        fail = memcmp(digest, testSet[i].output, md.DigestSize());
        pass = pass && !fail;

        cout << (fail ? "FAILED   " : "PASSED   ");
        for (j=0; j<md.DigestSize(); j++)
            cout << setw(2) << setfill('0') << hex << (int)digest[j];
        cout << "   \"" << testSet[i].input << '\"';
		if (testSet[i].repeatTimes != 1)
			cout << " repeated " << dec << testSet[i].repeatTimes << " times";
		cout  << endl;
    }

	return pass;
}

boolean MD5Validate()
{
	HashTestTuple testSet[] = 
	{
		HashTestTuple("", "\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e"),
		HashTestTuple("a", "\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61"),
		HashTestTuple("abc", "\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72"),
		HashTestTuple("message digest", "\xf9\x6b\x69\x7d\x7c\xb7\x93\x8d\x52\x5a\x2f\x31\xaa\xf1\x61\xd0"),
		HashTestTuple("abcdefghijklmnopqrstuvwxyz", "\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "\xd1\x74\xab\x98\xd2\x77\xd9\xf5\xa5\x61\x1c\x2c\x9f\x41\x9d\x9f"),
		HashTestTuple("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "\x57\xed\xf4\xa2\x2b\xe3\xc9\x55\xac\x49\xda\x2e\x21\x07\xb6\x7a")
	};

    MD5 md5;

    cout << "\nMD5 validation suite running...\n\n";
	return HashModuleTest(md5, testSet, sizeof(testSet)/sizeof(testSet[0]));
}

boolean SHAValidate()
{
#ifdef NEW_SHA
	HashTestTuple testSet[] = 
	{
		HashTestTuple("abc", "\xA9\x99\x3E\x36\x47\x06\x81\x6A\xBA\x3E\x25\x71\x78\x50\xC2\x6C\x9C\xD0\xD8\x9D"),
		HashTestTuple("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "\x84\x98\x3E\x44\x1C\x3B\xD2\x6E\xBA\xAE\x4A\xA1\xF9\x51\x29\xE5\xE5\x46\x70\xF1"),
		HashTestTuple("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "\x34\xAA\x97\x3C\xD4\xC4\xDA\xA4\xF6\x1E\xEB\x2B\xDB\xAD\x27\x31\x65\x34\x01\x6F", 15625)
	};
#else
	HashTestTuple testSet[] = 
	{
		HashTestTuple("abc", "\x01\x64\xB8\xA9\x14\xCD\x2A\x5E\x74\xC4\xF7\xFF\x08\x2C\x4D\x97\xF1\xED\xF8\x80"),
		HashTestTuple("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "\xD2\x51\x6E\xE1\xAC\xFA\x5B\xAF\x33\xDF\xC1\xC4\x71\xE4\x38\x44\x9E\xF1\x34\xC8"),
		HashTestTuple("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "\x32\x32\xAF\xFA\x48\x62\x8A\x26\x65\x3B\x5A\xAA\x44\x54\x1F\xD9\x0D\x69\x06\x03", 15625)
	};
#endif

    SHA sha;

    cout << "\nSHA validation suite running...\n\n";
	return HashModuleTest(sha, testSet, sizeof(testSet)/sizeof(testSet[0]));
}

boolean TigerValidate()
{
    cout << "\nTiger validation suite running...\n\n";

#ifdef WORD64_AVAILABLE
	HashTestTuple testSet[] =
	{
		HashTestTuple("", "\x44\x41\xBE\x75\xF6\x01\x87\x73\xC2\x06\xC2\x27\x45\x37\x4B\x92\x4A\xA8\x31\x3F\xEF\x91\x9F\x41"),
		HashTestTuple("abc", "\xF6\x8D\x7B\xC5\xAF\x4B\x43\xA0\x6E\x04\x8D\x78\x29\x56\x0D\x4A\x94\x15\x65\x8B\xB0\xB1\xF3\xBF"),
		HashTestTuple("Tiger", "\xFE\x40\x79\x8B\x8E\xB9\x37\xFD\x97\x76\x08\x93\x05\x48\xD6\xA8\x94\xC2\x0B\x04\xCB\xEF\x7A\x42"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-", "\x15\x9B\x38\x0A\xB7\x92\x94\xE0\xDA\x19\xF1\x62\x82\xCE\x6D\xCE\x0F\x84\xD3\x4F\x72\x9D\xBE\xA3"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz+0123456789", "\x92\xD1\x2E\xB5\x6D\xCC\x0A\x94\xF1\xF6\x05\xD3\xBD\x00\x39\x11\x1B\xEB\xDC\x28\x77\x57\x77\xC9"),
		HashTestTuple("Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham", "\xF0\x55\x89\xB5\xB8\x97\xC0\xF8\x1F\x16\x26\x6F\x79\xE3\x62\x5E\x0D\x4D\x4A\x4E\x34\x3E\x5E\xF9"),
		HashTestTuple("Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham, proceedings of Fast Software Encryption 3, Cambridge.", "\xD3\xA2\x54\x48\x92\xA5\xAE\xC5\x7F\x03\x85\x62\x94\xA8\x22\x0A\x07\xE7\xFA\x11\xA1\x61\x8E\x69"),
		HashTestTuple("Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham, proceedings of Fast Software Encryption 3, Cambridge, 1996.", "\x9A\x45\xE7\x7B\xEE\x2F\x4D\x44\xA7\x51\xCB\x36\xD5\xBA\x40\xC0\x02\x6D\xFD\xFF\x2A\x71\x67\xE0"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-", "\x01\xEF\x91\x0B\x9B\xB2\xCB\x4C\x6C\x47\x49\x5C\x86\xB3\x64\x1A\xFF\x14\xFB\xF7\x79\x40\x9C\x0E")
	};

    Tiger tiger;

	return HashModuleTest(tiger, testSet, sizeof(testSet)/sizeof(testSet[0]));
#else
    cout << "word64 not available, skipping Tiger validation." << endl;
    return TRUE;
#endif
}

boolean RIPEMDValidate()
{
	HashTestTuple testSet[] = 
	{
		HashTestTuple("", "\x9c\x11\x85\xa5\xc5\xe9\xfc\x54\x61\x28\x08\x97\x7e\xe8\xf5\x48\xb2\x25\x8d\x31"),
		HashTestTuple("a", "\x0b\xdc\x9d\x2d\x25\x6b\x3e\xe9\xda\xae\x34\x7b\xe6\xf4\xdc\x83\x5a\x46\x7f\xfe"),
		HashTestTuple("abc", "\x8e\xb2\x08\xf7\xe0\x5d\x98\x7a\x9b\x04\x4a\x8e\x98\xc6\xb0\x87\xf1\x5a\x0b\xfc"),
		HashTestTuple("message digest", "\x5d\x06\x89\xef\x49\xd2\xfa\xe5\x72\xb8\x81\xb1\x23\xa8\x5f\xfa\x21\x59\x5f\x36"),
		HashTestTuple("abcdefghijklmnopqrstuvwxyz", "\xf7\x1c\x27\x10\x9c\x69\x2c\x1b\x56\xbb\xdc\xeb\x5b\x9d\x28\x65\xb3\x70\x8d\xbc"),
		HashTestTuple("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "\x12\xa0\x53\x38\x4a\x9c\x0c\x88\xe4\x05\xa0\x6c\x27\xdc\xf4\x9a\xda\x62\xeb\x2b"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "\xb0\xe2\x0b\x6e\x31\x16\x64\x02\x86\xed\x3a\x87\xa5\x71\x30\x79\xb2\x1f\x51\x89"),
		HashTestTuple("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "\x9b\x75\x2e\x45\x57\x3d\x4b\x39\xf4\xdb\xd3\x32\x3c\xab\x82\xbf\x63\x32\x6b\xfb"),
		HashTestTuple("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "\x52\x78\x32\x43\xc1\x69\x7b\xdb\xe1\x6d\x37\xf9\x7f\x68\xf0\x83\x25\xdc\x15\x28", 15625)
	};

    RIPEMD160 md;

    cout << "\nRIPEMD-160 validation suite running...\n\n";
	return HashModuleTest(md, testSet, sizeof(testSet)/sizeof(testSet[0]));
}

boolean HAVALValidate()
{
	HashTestTuple testSet[] = 
	{
		HashTestTuple("", "\x1B\xDC\x55\x6B\x29\xAD\x02\xEC\x09\xAF\x8C\x66\x47\x7F\x2A\x87"),
		HashTestTuple("a", "\x5E\x16\x10\xFC\xED\x1D\x3A\xDB\x0B\xB1\x8E\x92\xAC\x2B\x11\xF0\xBD\x99\xD8\xED"),
		HashTestTuple("HAVAL", "\x74\xAA\x31\x18\x2F\xF0\x9B\xCC\xE4\x53\xA7\xF7\x1B\x5A\x7C\x5E\x80\x87\x2F\xA9\x0C\xD9\x3A\xE4"),
		HashTestTuple("0123456789", "\x14\x4C\xB2\xDE\x11\xF0\x5D\xF7\xC3\x56\x28\x2A\x3B\x48\x57\x96\xDA\x65\x3F\x6B\x70\x28\x68\xC7\xDC\xF4\xAE\x76"),
		HashTestTuple("abcdefghijklmnopqrstuvwxyz", "\x1A\x1D\xC8\x09\x9B\xDA\xA7\xF3\x5B\x4D\xA4\xE8\x05\xF1\xA2\x8F\xEE\x90\x9D\x8D\xEE\x92\x01\x98\x18\x5C\xBC\xAE\xD8\xA1\x0A\x8D"),
		HashTestTuple("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "\xC5\x64\x7F\xC6\xC1\x87\x7F\xFF\x96\x74\x2F\x27\xE9\x26\x6B\x68\x74\x89\x4F\x41\xA0\x8F\x59\x13\x03\x3D\x9D\x53\x2A\xED\xDB\x39")
	};

	boolean pass=TRUE;

    cout << "\nHAVAL validation suite running...\n\n";
	{
		HAVAL3 md(16);
		pass = HashModuleTest(md, testSet+0, 1) && pass;
	}
	{
		HAVAL3 md(20);
		pass = HashModuleTest(md, testSet+1, 1) && pass;
	}
	{
		HAVAL4 md(24);
		pass = HashModuleTest(md, testSet+2, 1) && pass;
	}
	{
		HAVAL4 md(28);
		pass = HashModuleTest(md, testSet+3, 1) && pass;
	}
	{
		HAVAL5 md(32);
		pass = HashModuleTest(md, testSet+4, 1) && pass;
	}
	{
		HAVAL5 md(32);
		pass = HashModuleTest(md, testSet+5, 1) && pass;
	}

	return pass;
}

boolean MD5MACValidate()
{
	const byte keys[2][MD5MAC::KEYLENGTH]={
		{0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff},
		{0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10}};

	const char *TestVals[7]={
		"",
		"a",
		"abc",
		"message digest",
		"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890"};

	const byte output[2][7][MD5MAC::DIGESTSIZE]={
		{{0x1f,0x1e,0xf2,0x37,0x5c,0xc0,0xe0,0x84,0x4f,0x98,0xe7,0xe8,0x11,0xa3,0x4d,0xa8},
		{0x7a,0x76,0xee,0x64,0xca,0x71,0xef,0x23,0x7e,0x26,0x29,0xed,0x94,0x52,0x73,0x65},
		{0xe8,0x01,0x3c,0x11,0xf7,0x20,0x9d,0x13,0x28,0xc0,0xca,0xa0,0x4f,0xd0,0x12,0xa6},
		{0xc8,0x95,0x53,0x4f,0x22,0xa1,0x74,0xbc,0x3e,0x6a,0x25,0xa2,0xb2,0xef,0xd6,0x30},
		{0x91,0x72,0x86,0x7e,0xb6,0x00,0x17,0x88,0x4c,0x6f,0xa8,0xcc,0x88,0xeb,0xe7,0xc9},
		{0x3b,0xd0,0xe1,0x1d,0x5e,0x09,0x4c,0xb7,0x1e,0x35,0x44,0xac,0xa9,0xb8,0xbf,0xa2},
		{0x93,0x37,0x16,0x64,0x44,0xcc,0x95,0x35,0xb7,0xd5,0xb8,0x0f,0x91,0xe5,0x29,0xcb}},
		{{0x2f,0x6e,0x73,0x13,0xbf,0xbb,0xbf,0xcc,0x3a,0x2d,0xde,0x26,0x8b,0x59,0xcc,0x4d},
		{0x69,0xf6,0xca,0xff,0x40,0x25,0x36,0xd1,0x7a,0xe1,0x38,0x03,0x2c,0x0c,0x5f,0xfd},
		{0x56,0xd3,0x2b,0x6c,0x34,0x76,0x65,0xd9,0x74,0xd6,0xf7,0x5c,0x3f,0xc6,0xf0,0x40},
		{0xb8,0x02,0xb2,0x15,0x4e,0x59,0x8b,0x6f,0x87,0x60,0x56,0xc7,0x85,0x46,0x2c,0x0b},
		{0x5a,0xde,0xf4,0xbf,0xf8,0x04,0xbe,0x08,0x58,0x7e,0x94,0x41,0xcf,0x6d,0xbd,0x57},
		{0x18,0xe3,0x49,0xa5,0x24,0x44,0xb3,0x0e,0x5e,0xba,0x5a,0xdd,0xdc,0xd9,0xf1,0x8d},
		{0xf2,0xb9,0x06,0xa5,0xb8,0x4b,0x9b,0x4b,0xbe,0x95,0xed,0x32,0x56,0x4e,0xe7,0xeb}}};

    byte digest[MD5MAC::DIGESTSIZE];
    boolean pass=TRUE, fail;

    cout << "\nMD5MAC validation suite running...\n";

	for (int k=0; k<2; k++)
	{
		MD5MAC mac(keys[k]);
		cout << "\nKEY: ";
		for (int j=0;j<MD5MAC::KEYLENGTH;j++)
			cout << setw(2) << setfill('0') << hex << (int)keys[k][j];
		cout << endl << endl;
		for (int i=0;i<7;i++)
		{
			mac.Update((byte *)TestVals[i], strlen(TestVals[i]));
			mac.Final(digest);
			fail = memcmp(digest, output[k][i], MD5MAC::DIGESTSIZE)
			     || !mac.VerifyMAC(output[k][i], (byte *)TestVals[i], strlen(TestVals[i]));
			pass = pass && !fail;
			cout << (fail ? "FAILED   " : "PASSED   ");
			for (int j=0;j<MD5MAC::DIGESTSIZE;j++)
				cout << setw(2) << setfill('0') << hex << (int)digest[j];
			cout << "   \"" << TestVals[i] << '\"' << endl;
		}
	}

    return pass;
}

boolean HMACValidate()
{
	typedef HMAC<MD5> HMAC_MD5;

	const byte keys[2][HMAC_MD5::KEYLENGTH]={
		{0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff},
		{0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10}};

	const char *TestVals[7]={
		"",
		"a",
		"abc",
		"message digest",
		"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890"};

	const byte output[2][7][HMAC_MD5::DIGESTSIZE]={
		{{0xf8,0xe1,0x71,0x93,0xbd,0x33,0xe5,0x5e,0x1d,0x1d,0xa5,0x43,0x0f,0x44,0x56,0x8b},
		{0xaa,0x72,0x7b,0xf1,0xfe,0xc3,0x53,0xc5,0xa3,0x25,0xc2,0x6a,0x9f,0x06,0x18,0x1b},
		{0xe4,0x6f,0xac,0xf7,0x3e,0x28,0x2b,0xb3,0x45,0x44,0x8f,0x85,0x7a,0xc0,0x55,0x0e},
		{0x34,0x00,0xb7,0xd9,0x8c,0x5c,0x3b,0xf4,0x15,0xcb,0x7a,0xed,0xb8,0xea,0xc7,0x85},
		{0x62,0x72,0x29,0x26,0x36,0xe2,0xd4,0x3e,0x3c,0x6c,0xff,0x17,0xc4,0xbe,0x4f,0xb0},
		{0xd8,0xe5,0x27,0xf8,0x13,0xd0,0x03,0x15,0x1b,0x91,0x89,0x1c,0x97,0x53,0x5c,0xee},
		{0x9c,0x16,0xf6,0xff,0x14,0xc2,0x91,0x8b,0xc9,0x4a,0xd0,0x9c,0x8b,0x82,0x4b,0x85}},
		{{0xdd,0xff,0x1b,0xce,0x22,0xf3,0x07,0xc5,0x9b,0xdf,0xd8,0x00,0x5d,0x2d,0x88,0xbe},
		{0x58,0x87,0x90,0xa6,0x3a,0x0b,0xe0,0xcd,0xc5,0x9d,0x96,0x2d,0x54,0x5f,0x64,0x64},
		{0x84,0x6c,0xef,0x41,0x60,0xf8,0xc7,0x16,0xd9,0x6e,0x49,0x2f,0x48,0xc1,0x19,0x6a},
		{0x21,0xc7,0x8d,0xb5,0xea,0x22,0x18,0x23,0xac,0xce,0xf0,0xb7,0x87,0x76,0xeb,0x47},
		{0x3d,0x95,0xf4,0x91,0x3f,0xa4,0x09,0xf5,0x15,0x3a,0x9c,0x1b,0x26,0xe2,0x42,0x89},
		{0x0b,0x82,0x45,0xeb,0x26,0xeb,0x24,0xdd,0x2c,0x52,0x87,0xf0,0x76,0x65,0x8f,0xc6},
		{0x5e,0xfd,0x03,0x6e,0x89,0x4d,0x96,0x40,0x0e,0xe2,0xb5,0x7a,0x3c,0xf3,0x7d,0xc9}}};

    byte digest[HMAC_MD5::DIGESTSIZE];
    boolean pass=TRUE, fail;

    cout << "\nHMAC/MD5 validation suite running...\n";

	for (int k=0; k<2; k++)
	{
		HMAC_MD5 mac(keys[k]);
		cout << "\nKEY: ";
		for (int j=0;j<HMAC_MD5::KEYLENGTH;j++)
			cout << setw(2) << setfill('0') << hex << (int)keys[k][j];
		cout << endl << endl;
		for (int i=0;i<7;i++)
		{
			mac.Update((byte *)TestVals[i], strlen(TestVals[i]));
			mac.Final(digest);
			fail = memcmp(digest, output[k][i], HMAC_MD5::DIGESTSIZE)
			     || !mac.VerifyMAC(output[k][i], (byte *)TestVals[i], strlen(TestVals[i]));
			pass = pass && !fail;
			cout << (fail ? "FAILED   " : "PASSED   ");
			for (int j=0;j<HMAC_MD5::DIGESTSIZE;j++)
				cout << setw(2) << setfill('0') << hex << (int)digest[j];
			cout << "   \"" << TestVals[i] << '\"' << endl;
		}
	}

    return pass;
}

boolean XMACCValidate()
{
	typedef XMACC<MD5> XMACC_MD5;

	const byte keys[2][XMACC_MD5::KEYLENGTH]={
		{0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb},
		{0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98}};

	const word32 counters[2]={0xccddeeff, 0x76543210};

	const char *TestVals[7]={
		"",
		"a",
		"abc",
		"message digest",
		"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890"};

	const byte output[2][7][XMACC_MD5::DIGESTSIZE]={
		{{0xcc,0xdd,0xef,0x00,0xfa,0x89,0x54,0x92,0x86,0x32,0xda,0x2a,0x3f,0x29,0xc5,0x52,0xa0,0x0d,0x05,0x13},
		{0xcc,0xdd,0xef,0x01,0xae,0xdb,0x8b,0x7b,0x69,0x71,0xc7,0x91,0x71,0x48,0x9d,0x18,0xe7,0xdf,0x9d,0x5a},
		{0xcc,0xdd,0xef,0x02,0x5e,0x01,0x2e,0x2e,0x4b,0xc3,0x83,0x62,0xc2,0xf4,0xe6,0x18,0x1c,0x44,0xaf,0xca},
		{0xcc,0xdd,0xef,0x03,0x3e,0xa9,0xf1,0xe0,0x97,0x91,0xf8,0xe2,0xbe,0xe0,0xdf,0xf3,0x41,0x03,0xb3,0x5a},
		{0xcc,0xdd,0xef,0x04,0x2e,0x6a,0x8d,0xb9,0x72,0xe3,0xce,0x9f,0xf4,0x28,0x45,0xe7,0xbc,0x80,0xa9,0xc7},
		{0xcc,0xdd,0xef,0x05,0x1a,0xd5,0x40,0x78,0xfb,0x16,0x37,0xfc,0x7a,0x1d,0xce,0xb4,0x77,0x10,0xb2,0xa0},
		{0xcc,0xdd,0xef,0x06,0x13,0x2f,0x11,0x47,0xd7,0x1b,0xb5,0x52,0x36,0x51,0x26,0xb0,0x96,0xd7,0x60,0x81}},
		{{0x76,0x54,0x32,0x11,0xe9,0xcb,0x74,0x32,0x07,0x93,0xfe,0x01,0xdd,0x27,0xdb,0xde,0x6b,0x77,0xa4,0x56},
		{0x76,0x54,0x32,0x12,0xcd,0x55,0x87,0x5c,0xc0,0x35,0x85,0x99,0x44,0x02,0xa5,0x0b,0x8c,0xe7,0x2c,0x68},
		{0x76,0x54,0x32,0x13,0xac,0xfd,0x87,0x50,0xc3,0x8f,0xcd,0x58,0xaa,0xa5,0x7e,0x7a,0x25,0x63,0x26,0xd1},
		{0x76,0x54,0x32,0x14,0xe3,0x30,0xf5,0xdd,0x27,0x2b,0x76,0x22,0x7f,0xaa,0x90,0x73,0x6a,0x48,0xdb,0x00},
		{0x76,0x54,0x32,0x15,0xfc,0x57,0x00,0x20,0x7c,0x9d,0xf6,0x30,0x6f,0xbd,0x46,0x3e,0xfb,0x8a,0x2c,0x60},
		{0x76,0x54,0x32,0x16,0xfb,0x0f,0xd3,0xdf,0x4c,0x4b,0xc3,0x05,0x9d,0x63,0x1e,0xba,0x25,0x2b,0xbe,0x35},
		{0x76,0x54,0x32,0x17,0xc6,0xfe,0xe6,0x5f,0xb1,0x35,0x8a,0xf5,0x32,0x7a,0x80,0xbd,0xb8,0x72,0xee,0xae}}};

    byte digest[XMACC_MD5::DIGESTSIZE];
    boolean pass=TRUE, fail;

    cout << "\nXMACC/MD5 validation suite running...\n";

	for (int k=0; k<2; k++)
	{
		XMACC_MD5 mac(keys[k], counters[k]);
		cout << "\nKEY: ";
		for (int j=0;j<XMACC_MD5::KEYLENGTH;j++)
			cout << setw(2) << setfill('0') << hex << (int)keys[k][j];
		cout << "    COUNTER: 0x" << hex << counters[k] << endl << endl;
		for (int i=0;i<7;i++)
		{
			mac.Update((byte *)TestVals[i], strlen(TestVals[i]));
			mac.Final(digest);
			fail = memcmp(digest, output[k][i], XMACC_MD5::DIGESTSIZE)
			     || !mac.VerifyMAC(output[k][i], (byte *)TestVals[i], strlen(TestVals[i]));
			pass = pass && !fail;
			cout << (fail ? "FAILED   " : "PASSED   ");
			for (int j=0;j<XMACC_MD5::DIGESTSIZE;j++)
				cout << setw(2) << setfill('0') << hex << (int)digest[j];
			cout << "   \"" << TestVals[i] << '\"' << endl;
		}
	}

    return pass;
}

