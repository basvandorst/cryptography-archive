// bench.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#include "md5.h"
#include "md5mac.h"
#include "hmac.h"
#include "xormac.h"
#include "sha.h"
#include "haval.h"
#include "tiger.h"
#include "ripemd.h"
#include "idea.h"
#include "des.h"
// #include "rc4.h"
#include "rc5.h"
#include "blowfish.h"
#include "diamond.h"
#include "wake.h"
#include "3way.h"
#include "safer.h"
#include "gost.h"
#include "shark.h"
#include "cast.h"
#include "square.h"
#include "seal.h"
#include "blumshub.h"
#include "rsa.h"
#include "rsarefcl.h"
#include "elgamal.h"
#include "dsa.h"
#include "luc.h"
#include "rabin.h"
#include "blumgold.h"
#include "eccrypto.h"
#include "ecp.h"
#include "ec2n.h"
#include "asn.h"
#include "rng.h"
#include "files.h"
#include "hex.h"

#include "iostream"
#include "iomanip"
USING_NAMESPACE(std)

#include <time.h>

#include "bench.h"
#include "modes.h"
#include "mdc.h"
#include "lubyrack.h"
#include "sapphire.h"
#include "tea.h"

#ifdef CLOCKS_PER_SEC
const float CLOCK_TICKS_PER_SECOND = (float)CLOCKS_PER_SEC;
#elif defined(CLK_TCK)
const float CLOCK_TICKS_PER_SECOND = (float)CLK_TCK;
#else
const float CLOCK_TICKS_PER_SECOND = 1000000.0;
#endif

void BenchMarkAll(float t)
{
#ifdef NEW_SHA
#define SHA_NAME "SHA-1"
#else
#define SHA_NAME "SHA  "
#endif

	const byte *const key=(byte *)"0123456789abcdef000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	cout << "Cipher              Total Bytes    Time   Bytes/S\n\n";
#if 1
	{
		cout << "MD5                ";
		MD5 c;
		BenchMark(c, t);
	}
	{
		cout << "MD5-MAC            ";
		MD5MAC c(key);
		BenchMark(c, t);
	}
	{
		cout << "XMACC/MD5          ";
		XMACC<MD5> c(key, 0);
		BenchMark(c, t);
	}
	{
		cout << "HMAC/MD5           ";
		HMAC<MD5> c(key);
		BenchMark(c, t);
	}
	{
		cout << SHA_NAME "              ";
		SHA c;
		BenchMark(c, t);
	}
	{
		cout << "HAVAL (pass=3)     ";
		HAVAL3 c;
		BenchMark(c, t);
	}
	{
		cout << "HAVAL (pass=4)     ";
		HAVAL4 c;
		BenchMark(c, t);
	}
	{
		cout << "HAVAL (pass=5)     ";
		HAVAL5 c;
		BenchMark(c, t);
	}
#ifdef WORD64_AVAILABLE
	{
		cout << "Tiger              ";
		Tiger c;
		BenchMark(c, t);
	}
#endif
	{
		cout << "RIPE-MD160         ";
		RIPEMD160 c;
		BenchMark(c, t);
	}

	{
		cout << "MDC/MD5            ";
		MDC<MD5> c(key);
		BenchMark(c, t);
	}
	{
		cout << "Luby-Rackoff/MD5   ";
		LREncryption<MD5> c(key, 78);
		BenchMark(c, t);
	}
	{
		cout << "DES                ";
		DESEncryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "IDEA               ";
		IDEAEncryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "RC5 (r=12)         ";
		RC5Encryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "Blowfish (r=16)    ";
		BlowfishEncryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "Diamond2 (r=10)    ";
		Diamond2Encryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "Diamond2 Lite (r=8)";
		Diamond2LiteEncryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "3-WAY (r=11)       ";
		ThreeWayDecryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "TEA                ";
		TEAEncryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "SAFER (r=8)        ";
		SAFER_SK64_Encryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "GOST               ";
		GOSTEncryption c(key);
		BenchMark(c, t);
	}

#ifdef WORD64_AVAILABLE
	{
		cout << "SHARK (r=6)        ";
		SHARKEncryption c(key);
		BenchMark(c, t);
	}
#if 0	// SHARK2 not yet available
	{
		cout << "SHARK2 (r=6)       ";
		SHARK2Encryption c(key);
		BenchMark(c, t);
	}
#endif
#endif

	{
		cout << "CAST-128           ";
		CAST128Encryption c(key);
		BenchMark(c, t);
	}
	{
		cout << "Square             ";
		SquareEncryption c(key);
		BenchMark(c, t);
	}

/*
	{
		cout << "RC4                ";
		RC4 c(key);
		BenchMark(c, t);
	}
*/
	{
		cout << "SEAL               ";
		SEAL c(key);
		BenchMark(c, t);
	}
	{
		cout << "WAKE               ";
		WAKEEncryption c(key, new BitBucket);
		BenchMark(c, t);
	}
	{
		cout << "Sapphire           ";
		SapphireEncryption c(key);
		BenchMark(c, t);
	}

	{
		cout << "BlumBlumShub 512   ";
		Integer p("CB6C,B8CE,6351,164F,5D0C,0C9E,9E31,E231,CF4E,D551,CBD0,E671,5D6A,7B06,D8DF,C4A7h");
		Integer q("FD2A,8594,A132,20CC,4E6D,DE77,3AAA,CF15,CD9E,E447,8592,FF46,CC77,87BE,9876,A2AFh");
		Integer s("63239752671357255800299643604761065219897634268887145610573595874544114193025997412441121667211431");
		BlumBlumShub c(p, q, s);
		BenchMark(c, t);
	}
	{
		cout << "BlumBlumShub 1024  ";
		Integer p("FD2A,8594,A132,20CC,4E6D,DE77,3AAA,CF15,CD9E,E447,8592,FF46,CC77,87BE,9876,9E2C,"
				  "8572,64C3,4CF4,188A,44D4,2130,1135,7982,6FF6,EDD3,26F0,5FAA,BAF4,A81E,7ADC,B80Bh");
		Integer q("C8B9,5797,B349,6BA3,FD72,F2C0,A796,8A65,EE0F,B4BA,272F,4FEE,4DB1,06D5,ECEB,7142,"
				  "E8A8,E5A8,6BF9,A32F,BA37,BACC,8A75,8A6B,2DCE,D6EC,B515,980A,4BB1,08FB,6F2C,2383h");
		Integer s("3578,8F00,2965,71A4,4382,699F,45FD,3922,8238,241B,CEBA,0543,3443,E8D9,12FB,AC46,"
				  "7EC4,8505,EC9E,7EE8,5A23,9B2A,B615,D0C4,9448,F23A,ADEE,E850,1A7A,CA30,0B5B,A408,"
				  "D936,21BA,844E,BDD6,7848,3D1E,9137,CC87,DAA5,773B,D45A,C8BB,5392,1393,108B,6992,"
				  "74E3,C5E2,C235,A321,0111,3BA4,BAB4,1A2F,17EE,C371,DE67,01C9,0F3D,907A,B252,9BDDh");
		BlumBlumShub c(p, q, s);
		BenchMark(c, t);
	}
	{
		cout << "BlumBlumShub 2048  ";
		Integer p("EB56,978A,7BA7,B5D9,1383,4611,94F5,4766,FCEF,CF41,958A,FC41,43D0,839F,C56B,B568,"
				  "4ED3,9E5A,BABB,5ACE,8B11,CEBC,88A2,7C12,FFEE,E6E8,CF0A,E231,5BC2,DEDE,80B7,32F6,"
				  "340E,D8A6,B7DE,C779,7EE5,0E16,9C88,FC9F,2A0E,EE6C,7D47,C5F2,6B06,EB8C,F1C8,2E67,"
				  "5B82,8C28,4FB8,542F,2874,C355,CEEE,7A54,1B06,A8AB,8B66,6A5C,9DB2,72B8,74F3,7BC7h");
		Integer q("EB6B,3645,4591,8343,7331,7CAC,B02E,4BB9,DEF5,8EDC,1772,DB9B,9571,5FAB,1CDD,4FB1,"
				  "7B9A,07CD,E715,D448,F552,CBBD,D387,C037,DE70,6661,F360,D0E8,D42E,292A,9321,DDCB,"
				  "0BF9,C514,BFAC,3F2C,C06E,DF64,A9B8,50D6,AC4F,B9E4,014B,5624,2B40,A0D4,5D0B,6DD4,"
				  "0989,D00E,0268,99AB,21DB,0BB4,DB38,84DA,594F,575F,95AC,1B70,45E4,96C8,C6AD,CE67h");
		Integer s("C75A,8A0D,E231,295F,C08A,1716,8611,D5EC,E9EF,B565,90EC,58C0,57D0,DA7D,C6E6,DB00,"
				  "2282,1CA7,EA31,D64E,768C,0B19,8563,36DF,2226,F4EC,74A4,2844,2E8D,37E8,53DC,0172,"
				  "5F56,8CF9,B444,CA02,78B3,17AF,7C78,D320,16AE,AC3D,B97F,7259,1B8F,9C84,6A16,B878,"
				  "0595,70BB,9C52,18B5,9100,9C1F,E85A,4035,06F3,5F38,7462,F01D,0462,BFBC,A4CD,4A45,"
				  "3A77,E7F8,DED1,D6EF,CEF7,0937,CD3F,3AF1,4F88,932D,6D4B,002C,3735,304C,C5D3,B88A,"
				  "B57B,24B6,5346,9B46,5153,B7ED,B216,C181,B1C6,C52E,CD2B,E0AA,B1BB,0A93,C92E,4F79,"
				  "4931,E303,7C8F,A408,8ACF,56CD,6EC0,76A2,5015,6BA4,4C50,C44D,53B9,E168,5F84,B381,"
				  "2514,10B2,00E5,B4D1,4156,A2FE,0BF6,6F33,0A1B,91C6,31B8,1C90,02F1,FB1F,C494,8B65h");
		BlumBlumShub c(p, q, s);
		BenchMark(c, t);
	}

	cout << "\nOperation                        Iterations  Total Time  Milliseconds/Operation\n";

	{
		FileSource f("rsa512.dat", true, new HexDecoder());
		RSAPrivateKey priv(f);
		RSAPublicKey pub(priv);
		cout << "RSA 512 Encryption            ";
		BenchMarkEncryption(pub, t);
		cout << "RSA 512 Decryption            ";
		BenchMarkDecryption(priv, pub, t);
		cout << "RSA 512 Signature             ";
		BenchMarkSignature(priv, t);
		cout << "RSA 512 Verification          ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("rsa1024.dat", true, new HexDecoder());
		RSAPrivateKey priv(f);
		RSAPublicKey pub(priv);
		cout << "RSA 1024 Encryption           ";
		BenchMarkEncryption(pub, t);
		cout << "RSA 1024 Decryption           ";
		BenchMarkDecryption(priv, pub, t);
		cout << "RSA 1024 Signature            ";
		BenchMarkSignature(priv, t);
		cout << "RSA 1024 Verification         ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("rsa2048.dat", true, new HexDecoder());
		RSAPrivateKey priv(f);
		RSAPublicKey pub(priv);
		cout << "RSA 2048 Encryption           ";
		BenchMarkEncryption(pub, t);
		cout << "RSA 2048 Decryption           ";
		BenchMarkDecryption(priv, pub, t);
		cout << "RSA 2048 Signature            ";
		BenchMarkSignature(priv, t);
		cout << "RSA 2048 Verification         ";
		BenchMarkVerification(priv, pub, t);
	}

  {
		FileSource f("elgc512.dat", true, new HexDecoder());
		ElGamalCryptoPrivateKey priv(f);
		cout << "ElGamal 512 Encryption        ";
		BenchMarkEncryption(priv, t);
		cout << "ElGamal 512 Decryption        ";
		BenchMarkDecryption(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 512 E. w/ precomp.    ";
		BenchMarkEncryption(priv, t);
	}
	{
		FileSource f("elgc1024.dat", true, new HexDecoder());
		ElGamalCryptoPrivateKey priv(f);
		cout << "ElGamal 1024 Encryption       ";
		BenchMarkEncryption(priv, t);
		cout << "ElGamal 1024 Decryption       ";
		BenchMarkDecryption(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 1024 E. w/ precomp.   ";
		BenchMarkEncryption(priv, t);
	}
	{
		FileSource f("elgc2048.dat", true, new HexDecoder());
		ElGamalCryptoPrivateKey priv(f);
		cout << "ElGamal 2048 Encryption       ";
		BenchMarkEncryption(priv, t);
		cout << "ElGamal 2048 Decryption       ";
		BenchMarkDecryption(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 2048 E. w/ precomp.   ";
		BenchMarkEncryption(priv, t);
	}

	{
		FileSource f("elgs512.dat", true, new HexDecoder());
		ElGamalSigPrivateKey priv(f);
		cout << "ElGamal 512 Signature         ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 512 Verification      ";
		BenchMarkVerification(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 512 S. w/ precomp.    ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 512 V. w/ precomp.    ";
		BenchMarkVerification(priv, priv, t);
	}
	{
		FileSource f("elgs1024.dat", true, new HexDecoder());
		ElGamalSigPrivateKey priv(f);
		cout << "ElGamal 1024 Signature        ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 1024 Verification     ";
		BenchMarkVerification(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 1024 S. w/ precomp.   ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 1024 V. w/ precomp.   ";
		BenchMarkVerification(priv, priv, t);
	}
	{
		FileSource f("elgs2048.dat", true, new HexDecoder());
		ElGamalSigPrivateKey priv(f);
		cout << "ElGamal 2048 Signature        ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 2048 Verification     ";
		BenchMarkVerification(priv, priv, t);
		priv.Precompute();
		cout << "ElGamal 2048 S. w/ precomp.   ";
		BenchMarkSignature(priv, t);
		cout << "ElGamal 2048 V. w/ precomp.   ";
		BenchMarkVerification(priv, priv, t);
	}

	{
		FileSource f("luc512.dat", true, new HexDecoder());
		LUCPrivateKey priv(f);
		LUCPublicKey pub(priv);
		cout << "LUC 512 Encryption            ";
		BenchMarkEncryption(pub, t);
		cout << "LUC 512 Decryption            ";
		BenchMarkDecryption(priv, pub, t);
		cout << "LUC 512 Signature             ";
		BenchMarkSignature(priv, t);
		cout << "LUC 512 Verification          ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("luc1024.dat", true, new HexDecoder());
		LUCPrivateKey priv(f);
		LUCPublicKey pub(priv);
		cout << "LUC 1024 Encryption           ";
		BenchMarkEncryption(pub, t);
		cout << "LUC 1024 Decryption           ";
		BenchMarkDecryption(priv, pub, t);
		cout << "LUC 1024 Signature            ";
		BenchMarkSignature(priv, t);
		cout << "LUC 1024 Verification         ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("luc2048.dat", true, new HexDecoder());
		LUCPrivateKey priv(f);
		LUCPublicKey pub(priv);
		cout << "LUC 2048 Encryption           ";
		BenchMarkEncryption(pub, t);
		cout << "LUC 2048 Decryption           ";
		BenchMarkDecryption(priv, pub, t);
		cout << "LUC 2048 Signature            ";
		BenchMarkSignature(priv, t);
		cout << "LUC 2048 Verification         ";
		BenchMarkVerification(priv, pub, t);
	}

	{
		FileSource f("lucc512.dat", true, new HexDecoder());
		LUCELG_Decryptor priv(f);
		cout << "LUCELG 512 Encryption         ";
		BenchMarkEncryption(priv, t);
		cout << "LUCELG 512 Decryption         ";
		BenchMarkDecryption(priv, priv, t);
	}
	{
		FileSource f("lucc1024.dat", true, new HexDecoder());
		LUCELG_Decryptor priv(f);
		cout << "LUCELG 1024 Encryption        ";
		BenchMarkEncryption(priv, t);
		cout << "LUCELG 1024 Decryption        ";
		BenchMarkDecryption(priv, priv, t);
	}
	{
		FileSource f("lucs512.dat", true, new HexDecoder());
		LUCELG_Signer priv(f);
		cout << "LUCELG 512 Signature          ";
		BenchMarkSignature(priv, t);
		cout << "LUCELG 512 Verification       ";
		BenchMarkVerification(priv, priv, t);
	}
	{
		FileSource f("lucs1024.dat", true, new HexDecoder());
		LUCELG_Signer priv(f);
		cout << "LUCELG 1024 Signature         ";
		BenchMarkSignature(priv, t);
		cout << "LUCELG 1024 Verification      ";
		BenchMarkVerification(priv, priv, t);
	}

	{
		FileSource f("rabi512.dat", true, new HexDecoder());
		RabinPrivateKey priv(f);
		RabinPublicKey pub(priv);
		cout << "Rabin 512 Encryption          ";
		BenchMarkEncryption(pub, t);
		cout << "Rabin 512 Decryption          ";
		BenchMarkDecryption(priv, pub, t);
		cout << "Rabin 512 Signature           ";
		BenchMarkSignature(priv, t);
		cout << "Rabin 512 Verification        ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("rabi1024.dat", true, new HexDecoder());
		RabinPrivateKey priv(f);
		RabinPublicKey pub(priv);
		cout << "Rabin 1024 Encryption         ";
		BenchMarkEncryption(pub, t);
		cout << "Rabin 1024 Decryption         ";
		BenchMarkDecryption(priv, pub, t);
		cout << "Rabin 1024 Signature          ";
		BenchMarkSignature(priv, t);
		cout << "Rabin 1024 Verification       ";
		BenchMarkVerification(priv, pub, t);
	}
	{
		FileSource f("rabi2048.dat", true, new HexDecoder());
		RabinPrivateKey priv(f);
		RabinPublicKey pub(priv);
		cout << "Rabin 2048 Encryption         ";
		BenchMarkEncryption(pub, t);
		cout << "Rabin 2048 Decryption         ";
		BenchMarkDecryption(priv, pub, t);
		cout << "Rabin 2048 Signature          ";
		BenchMarkSignature(priv, t);
		cout << "Rabin 2048 Verification       ";
		BenchMarkVerification(priv, pub, t);
	}

	{
		FileSource f("blum512.dat", true, new HexDecoder());
		BlumGoldwasserPrivateKey priv(f);
		cout << "BlumGoldwasser 512 Encryption ";
		BenchMarkEncryption(priv, t);
		cout << "BlumGoldwasser 512 Decryption ";
		BenchMarkDecryption(priv, priv, t);
	}
	{
		FileSource f("blum1024.dat", true, new HexDecoder());
		BlumGoldwasserPrivateKey priv(f);
		cout << "BlumGoldwasser 1024 Encryption";
		BenchMarkEncryption(priv, t);
		cout << "BlumGoldwasser 1024 Decryption";
		BenchMarkDecryption(priv, priv, t);
	}
	{
		FileSource f("blum2048.dat", true, new HexDecoder());
		BlumGoldwasserPrivateKey priv(f);
		cout << "BlumGoldwasser 2048 Encryption";
		BenchMarkEncryption(priv, t);
		cout << "BlumGoldwasser 2048 Decryption";
		BenchMarkDecryption(priv, priv, t);
	}

	{
		FileSource f("dsa512.dat", true, new HexDecoder());
		DSAPrivateKey priv(f);
		cout << "DSA 512 Signature             ";
		BenchMarkSignature(priv, t);
		cout << "DSA 512 Verification          ";
		BenchMarkVerification(priv, priv, t);
		priv.Precompute(16);
		cout << "DSA 512 S. w/ precomputation  ";
		BenchMarkSignature(priv, t);
		cout << "DSA 512 V. w/ precomputation  ";
		BenchMarkVerification(priv, priv, t);
	}
	{
		FileSource f("dsa1024.dat", true, new HexDecoder());
		DSAPrivateKey priv(f);
		cout << "DSA 1024 Signature            ";
		BenchMarkSignature(priv, t);
		cout << "DSA 1024 Verification         ";
		BenchMarkVerification(priv, priv, t);
		priv.Precompute(16);
		cout << "DSA 1024 S. w/ precomputation ";
		BenchMarkSignature(priv, t);
		cout << "DSA 1024 V. w/ precomputation ";
		BenchMarkVerification(priv, priv, t);
	}

	{
		Integer modulus("88d876,2bf324,cd0fa5,87fa5b,0cf929,1a6337,1b32b1h");
		Integer a("659942,b7261b,249174,c86bd5,e2a65b,45fe07,37d110h");
		Integer b("3ece7d,09473d,666000,5baef5,d4e00e,30159d,2df49ah");
		Integer x("25dd61,4c0667,81abc0,fe6c84,fefaa3,858ca6,96d0e8h");
		Integer y("4e2477,05aab0,b3497f,d62b5e,78a531,446729,6c3fach");
		Integer o("88d876,2bf324,cd0fa5,880a69,fb6ac8,000000,00012eh");
		Integer d("1D,0BC8,03EF,D719,B78E,5C66,8EF0,59BD,2ED0,77B5,BF56h");

		ECP ec(modulus, a, b);
		ECP::Point P(x, y);
		ECP::Point Q(ec.Multiply(d, P));
		ECPrivateKey<ECP> priv(ec, P, Q, o, d);

		cout << "EC over GF(p) 168 Encryption  ";
		BenchMarkEncryption(priv, t);
		cout << "EC over GF(p) 168 Decryption  ";
		BenchMarkDecryption(priv, priv, t);
		cout << "EC over GF(p) 168 Signature   ";
		BenchMarkSignature(priv, t);
		cout << "EC over GF(p) 168 Verification";
		BenchMarkVerification(priv, priv, t);

		priv.Precompute();
		cout << "EC over GF(p) 168 E. w/ pc.   ";
		BenchMarkEncryption(priv, t);
		cout << "EC over GF(p) 168 S. w/ pc.   ";
		BenchMarkSignature(priv, t);
		cout << "EC over GF(p) 168 V. w/ pc    ";
		BenchMarkVerification(priv, priv, t);
	}
#endif
	{
		Integer o("15,223,975,388,863,572,064,621,855,304,781,545,065,589,745,772");
		Integer d("12,065,729,449,256,706,362,097,909,124,274,151,550,853,609,397");
		GF2N gf2n(155, 62, 0);
		byte b[]={0x7, 0x33, 0x8f};
		EC2N ec(gf2n, PolynomialMod2::Zero(), PolynomialMod2(b,3));
		EC2N::Point P(0x7B, 0x1C8);
		EC2N::Point Q(ec.Multiply(d, P));
		ECPrivateKey<EC2N> priv(ec, P, Q, o, d);

		cout << "EC over GF(2^n) 155 Encryption";
		BenchMarkEncryption(priv, t);
		cout << "EC over GF(2^n) 155 Decryption";
		BenchMarkDecryption(priv, priv, t);
		cout << "EC over GF(2^n) 155 Signature ";
		BenchMarkSignature(priv, t);
		cout << "EC over GF(2^n) 155 Verificati";
		BenchMarkVerification(priv, priv, t);

		priv.Precompute();
		cout << "EC over GF(2^n) 155 E. w/ pc. ";
		BenchMarkEncryption(priv, t);
		cout << "EC over GF(2^n) 155 S. w/ pc. ";
		BenchMarkSignature(priv, t);
		cout << "EC over GF(2^n) 155 V. w/ pc  ";
		BenchMarkVerification(priv, priv, t);
	}
}

float BenchMark(BlockTransformation &cipher, float timeTotal)
{
	const int BUF_SIZE = cipher.BlockSize();
	SecByteBlock buf(BUF_SIZE);
	clock_t start = clock();

	unsigned long i=0, length=BUF_SIZE;
	float timeTaken;
	do
	{
		length *= 2;
		for (; i<length; i+=BUF_SIZE)
			cipher.ProcessBlock(buf);
		timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND;
	}
	while (timeTaken < 2.0/3*timeTotal);

	float kbs = length / timeTaken;
	cout << setw(10) << length;
	cout << setw(10) << timeTaken;
	cout << setw(10) << (long)kbs << endl;
	return kbs;
}

float BenchMark(StreamCipher &cipher, float timeTotal)
{
	const int BUF_SIZE=128; // encrypt 128 bytes at a time
	SecByteBlock buf(BUF_SIZE);
	clock_t start = clock();

	unsigned long i=0, length=BUF_SIZE;
	float timeTaken;
	do
	{
		length *= 2;
		for (; i<length; i+=BUF_SIZE)
			cipher.ProcessString(buf, BUF_SIZE);
		timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND;
	}
	while (timeTaken < 2.0/3*timeTotal);

	float kbs = length / timeTaken;
	cout << setw(10) << length;
	cout << setw(10) << timeTaken;
	cout << setw(10) << (long)kbs << endl;
	return kbs;
}

float BenchMark(HashModule &hash, float timeTotal)
{
	const int BUF_SIZE=128; // update 128 bytes at a time
	SecByteBlock buf(BUF_SIZE);
	clock_t start = clock();

	unsigned long i=0, length=BUF_SIZE;
	float timeTaken;
	do
	{
		length *= 2;
		for (; i<length; i+=BUF_SIZE)
			hash.Update(buf, BUF_SIZE);
		timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND;
	}
	while (timeTaken < 2.0/3*timeTotal);

	float kbs = length / timeTaken;
	cout << setw(10) << length;
	cout << setw(10) << timeTaken;
	cout << setw(10) << (long)kbs << endl;
	return kbs;
}

float BenchMark(BufferedTransformation &bt, float timeTotal)
{
	const int BUF_SIZE=128; // update 128 bytes at a time
	SecByteBlock buf(BUF_SIZE);
	clock_t start = clock();

	unsigned long i=0, length=BUF_SIZE;
	float timeTaken;
	do
	{
		length *= 2;
		for (; i<length; i+=BUF_SIZE)
			bt.Put(buf, BUF_SIZE);
		timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND;
	}
	while (timeTaken < 2.0/3*timeTotal);

	float kbs = length / timeTaken;
	cout << setw(10) << length;
	cout << setw(10) << timeTaken;
	cout << setw(10) << (long)kbs << endl;
	return kbs;
}

float BenchMarkEncryption(PK_Encryptor &key, float timeTotal)
{
	unsigned int len = 16;
	LC_RNG rng(time(NULL));
	SecByteBlock plaintext(len), ciphertext(key.CipherTextLength(len));
	rng.GetBlock(plaintext, len);

	clock_t start = clock();
	unsigned int i;
	float timeTaken;
	for (timeTaken=(float)0, i=0; timeTaken < timeTotal; timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND, i++)
		key.Encrypt(rng, plaintext, len, ciphertext);
	cout << setw(10) << i;
	cout << setw(12) << setprecision(3) << timeTaken;
	cout << setw(16) << (unsigned int)(1000*timeTaken/i) << endl;
	return timeTaken/i;
}

float BenchMarkDecryption(PK_Decryptor &priv, PK_Encryptor &pub, float timeTotal)
{
	unsigned int len = 16;
	LC_RNG rng(time(NULL));
	SecByteBlock ciphertext(pub.CipherTextLength(len));
	SecByteBlock plaintext(pub.MaxPlainTextLength(ciphertext.size));
	rng.GetBlock(plaintext, len);
	pub.Encrypt(rng, plaintext, len, ciphertext);

	clock_t start = clock();
	unsigned int i;
	float timeTaken;
	for (timeTaken=(float)0, i=0; timeTaken < timeTotal; timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND, i++)
		priv.Decrypt(ciphertext, ciphertext.size, plaintext);
	cout << setw(10) << i;
	cout << setw(12) << setprecision(3) << timeTaken;
	cout << setw(16) << (unsigned int)(1000*timeTaken/i) << endl;
	return timeTaken/i;
}

float BenchMarkSignature(PK_Signer &key, float timeTotal)
{
	unsigned int len = STDMIN(key.MaxMessageLength(), 16U);
	LC_RNG rng(time(NULL));
	SecByteBlock message(len), signature(key.SignatureLength());
	rng.GetBlock(message, len);

	clock_t start = clock();
	unsigned int i;
	float timeTaken;
	for (timeTaken=(float)0, i=0; timeTaken < timeTotal; timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND, i++)
		key.Sign(rng, message, len, signature);
	cout << setw(10) << i;
	cout << setw(12) << setprecision(3) << timeTaken;
	cout << setw(16) << (unsigned int)(1000*timeTaken/i) << endl;
	return timeTaken/i;
}

float BenchMarkVerification(PK_Signer &priv, PK_Verifier &pub, float timeTotal)
{
	unsigned int len = STDMIN(priv.MaxMessageLength(), 16U);
	LC_RNG rng(time(NULL));
	SecByteBlock message(len), signature(pub.SignatureLength());
	rng.GetBlock(message, len);
	priv.Sign(rng, message, len, signature);

	clock_t start = clock();
	unsigned int i;
	float timeTaken;
	for (timeTaken=(float)0, i=0; timeTaken < timeTotal; timeTaken = float(clock() - start) / CLOCK_TICKS_PER_SECOND, i++)
		pub.Verify(message, len, signature);
	cout << setw(10) << i;
	cout << setw(12) << setprecision(3) << timeTaken;
	cout << setw(16) << (unsigned int)(1000*timeTaken/i) << endl;
	return timeTaken/i;
}
