// validat2.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#include "blumshub.h"
#include "rsa.h"
#include "rsarefcl.h"
#include "elgamal.h"
#include "dsa.h"
#include "dh.h"
#include "luc.h"
#include "rabin.h"
#include "blumgold.h"
#include "eccrypto.h"
#include "ecp.h"
#include "ec2n.h"
// #include "zeroknow.h"
#include "asn.h"
#include "rng.h"
#include "files.h"
#include "hex.h"
#include "forkjoin.h"

#include "iostream"
#include "iomanip"
USING_NAMESPACE(std)

#include "validate.h"

bool BBSValidate()
{
	cout << "\nBlumBlumShub validation suite running...\n\n";

	Integer p("212004934506826557583707108431463840565872545889679278744389317666981496005411448865750399674653351");
	Integer q("100677295735404212434355574418077394581488455772477016953458064183204108039226017738610663984508231");
	Integer seed("63239752671357255800299643604761065219897634268887145610573595874544114193025997412441121667211431");
	BlumBlumShub bbs(p, q, seed);
	bool pass = true, fail;
	int j;

	const byte output1[] = {
		0x49,0xEA,0x2C,0xFD,0xB0,0x10,0x64,0xA0,0xBB,0xB9,
		0x2A,0xF1,0x01,0xDA,0xC1,0x8A,0x94,0xF7,0xB7,0xCE};
	const byte output2[] = {
		0x74,0x45,0x48,0xAE,0xAC,0xB7,0x0E,0xDF,0xAF,0xD7,
		0xD5,0x0E,0x8E,0x29,0x83,0x75,0x6B,0x27,0x46,0xA1};

	byte buf[20];

	bbs.GetBlock(buf, 20);
	fail = memcmp(output1, buf, 20) != 0;
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	for (j=0;j<20;j++)
		cout << setw(2) << setfill('0') << hex << (int)buf[j];
	cout << endl;

	bbs.Seek(10);
	bbs.GetBlock(buf, 10);
	fail = memcmp(output1+10, buf, 10) != 0;
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	for (j=0;j<10;j++)
		cout << setw(2) << setfill('0') << hex << (int)buf[j];
	cout << endl;

	bbs.Seek(1234567);
	bbs.GetBlock(buf, 20);
	fail = memcmp(output2, buf, 20) != 0;
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	for (j=0;j<20;j++)
		cout << setw(2) << setfill('0') << hex << (int)buf[j];
	cout << endl;

	return pass;
}

bool RSAValidate()
{
	cout << "\nRSA validation suite running...\n\n";

	byte *plain = (byte *)
		"\x30\x20\x30\x0c\x06\x08\x2a\x86\x48\x86\xf7\x0d\x02\x02\x05\x00\x04"
		"\x10\x1d\x32\xde\x00\x9f\x9c\x56\xea\x46\x36\xd3\x9a\xaf\xfd\xae\xa1";
	byte *signature = (byte *)
		"\x05\xfa\x6a\x81\x2f\xc7\xdf\x8b\xf4\xf2\x54\x25\x09\xe0\x3e\x84"
		"\x6e\x11\xb9\xc6\x20\xbe\x20\x09\xef\xb4\x40\xef\xbc\xc6\x69\x21"
		"\x69\x94\xac\x04\xf3\x41\xb5\x7d\x05\x20\x2d\x42\x8f\xb2\xa2\x7b"
		"\x5c\x77\xdf\xd9\xb1\x5b\xfc\x3d\x55\x93\x53\x50\x34\x10\xc1\xe1";
	byte out[100], outPlain[100];
	unsigned int outLen;
	LC_RNG rng(765);
	bool pass = true, fail;

#ifdef THROW_EXCEPTIONS
	try
#endif
	{
		FileSource keys("rsa512a.dat", true, new HexDecoder);
		RSAPrivateKey rsaPriv(keys);
		RSAPublicKey rsaPub(rsaPriv);

		rsaPriv.Sign(rng, plain, 34, out);
		fail = memcmp(signature, out, 64) != 0;
		pass = pass && !fail;

		cout << (fail ? "FAILED    " : "PASSED    ");
		cout << "signature\n";

		fail = !rsaPub.Verify(plain, 34, out);
		pass = pass && !fail;

		cout << (fail ? "FAILED    " : "PASSED    ");
		cout << "valid signature verification\n";

		out[10]++;
		fail = rsaPub.Verify(plain, 34, out);
		pass = pass && !fail;

		cout << (fail ? "FAILED    " : "PASSED    ");
		cout << "invalid signature verification\n";

		rsaPub.Encrypt(rng, plain, 34, out);
		memset(outPlain, 0, 34);
		outLen = rsaPriv.Decrypt(out, outPlain);
		fail = (outLen!=34) || memcmp(plain, outPlain, 34);
		pass = pass && !fail;

		cout << (fail ? "FAILED    " : "PASSED    ");
		cout << "encryption and decryption\n";
	}
#ifdef THROW_EXCEPTIONS
	catch (BERDecodeErr)
	{
		cout << "FAILED    Error decoding RSA key\n";
		pass = false;
	}
#endif

#ifdef RSAREF_RSA_AVAILABLE

#ifdef THROW_EXCEPTIONS
	try
#endif
	{
		FileSource keys("rsa512a.dat", true, new HexDecoder);
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
		pass = false;
	}
#endif

#endif  // RSAREF_RSA_AVAILABLE

	return pass;
}

bool ZKValidate()
{
	return true;
#if 0
	int i;
	int j;

	cout << "\nZero Knowledge validation suite running...\n\n";

	FileSource f("graph.dat", true, new HexDecoder);
	SimpleGraph g1(f);
	cout << "g1:\n" << g1;

	word16 g1_to_g2[20]={1, 19, 9, 0, 8, 11, 17, 5, 2, 7, 3, 14, 13, 16, 6, 4, 12, 18, 10, 15};
	cout << "g1_to_g2:";
	for (j=0; j<20; j++)
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
	bool pass=true;

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
#endif
}

bool SignatureValidate(PK_Signer &priv, PK_Verifier &pub)
{
	LC_RNG rng(9374);
	const byte *message = (byte *)"test message";
	const int messageLen = 12;
	byte buffer[512];
	bool pass = true, fail;

	memset(buffer, 0, sizeof(buffer));
	priv.Sign(rng, message, messageLen, buffer);
	fail = !pub.Verify(message, messageLen, buffer);
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	cout << "signature and verification\n";

	++buffer[0];
	fail = pub.Verify(message, messageLen, buffer);
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	cout << "checking invalid signature" << endl;

	return pass;
}

bool CryptoSystemValidate(PK_Decryptor &priv, PK_Encryptor &pub)
{
	LC_RNG rng(9375);
	const byte *message = (byte *)"test message";
	const int messageLen = 12;
	SecByteBlock ciphertext(priv.CipherTextLength(messageLen));
	SecByteBlock plaintext(priv.MaxPlainTextLength(ciphertext.size));
	bool pass = true, fail;

	pub.Encrypt(rng, message, messageLen, ciphertext);
	fail = (messageLen!=priv.Decrypt(ciphertext, priv.CipherTextLength(messageLen), plaintext));
	fail = fail || memcmp(message, plaintext, messageLen);
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	cout << "encryption and decryption\n";

	return pass;
}

bool KeyAgreementValidate(KeyAgreementProtocol &a, KeyAgreementProtocol &b)
{
	if (a.PublicValueLength() != b.PublicValueLength() || a.AgreedKeyLength() != b.AgreedKeyLength())
	{
		cout << "FAILED    incompatible protocols" << endl;
		return false;
	}

	SecByteBlock pub1(a.PublicValueLength()), pub2(b.PublicValueLength());
	SecByteBlock key1(a.AgreedKeyLength()), key2(b.AgreedKeyLength());

	LC_RNG rng(5234);
	a.Setup(rng, pub1);
	b.Setup(rng, pub2);

	memset(key1.ptr, 0x10, key1.size);
	memset(key2.ptr, 0x11, key2.size);

	a.Agree(pub2, key1);
	b.Agree(pub1, key2);

	if (memcmp(key1.ptr, key2.ptr, a.AgreedKeyLength()))
	{
		cout << "FAILED    keys not equal" << endl;
		return false;
	}
	else
	{
		cout << "PASSED    keys agreed" << endl;
		return true;
	}
}

bool DHValidate()
{
	cout << "\nDH validation suite running...\n\n";

	Fork *fork = new Fork(2);
	FileSource f("dhparams.dat", true, new HexDecoder(fork));
	fork->SelectOutPort(0);
	DH dh1(f);
	fork->SelectOutPort(1);
#ifdef USE_RSAREF
	RSAREF_DH dh2(f);
#else
	DH dh2(f);
#endif

	return KeyAgreementValidate(dh1, dh2);
}

bool LUCDIFValidate()
{
	cout << "\nLUCDIF validation suite running...\n\n";

	FileSource f("lucdif.dat", true, new HexDecoder());
	LUCDIF dh1(f);
	LUCDIF dh2(dh1);

	return KeyAgreementValidate(dh1, dh2);
}

bool ElGamalValidate()
{
	cout << "\nElGamal validation suite running...\n\n";
	bool pass = true;

	{
		FileSource f("elgs512.dat", true, new HexDecoder);
		ElGamalSigPrivateKey privS(f);
		ElGamalSigPublicKey pubS(privS);

		pass = SignatureValidate(privS, pubS) && pass;

		FileSource fc("elgc512.dat", true, new HexDecoder);
		ElGamalCryptoPrivateKey privC(fc);
		ElGamalCryptoPublicKey pubC(privC);

		pass = CryptoSystemValidate(privC, pubC) && pass;
	}
	{
		FileSource f("elgs2048.dat", true, new HexDecoder);
		ElGamalSigPrivateKey privS(f);
		privS.Precompute();
		ElGamalSigPublicKey pubS(privS);

		pass = SignatureValidate(privS, pubS) && pass;

		FileSource fc("elgc2048.dat", true, new HexDecoder);
		ElGamalCryptoPrivateKey privC(fc);
		ElGamalCryptoPublicKey pubC(privC);
		pubC.Precompute();

		pass = CryptoSystemValidate(privC, pubC) && pass;
	}

	return pass;
}

bool DSAValidate()
{
	cout << "\nDSA validation suite running...\n\n";

	bool pass = true, fail;
	{
	FileSource fs("dsa512.dat", true, new HexDecoder());
	DSAPrivateKey priv(fs);
	priv.Precompute(16);
	DSAPublicKey pub(priv);

	Integer k("79577ddcaafddc038b865b19f8eb1ada8a2838c6h");
	Integer h("0164b8a914cd2a5e74c4f7ff082c4d97f1edf880h");
	Integer r("9b77,f705,4c81,531c,4e46,a469,2fbf,e0f7,7f7e,bff2h");
	Integer s("95b4,f608,1f8f,890e,4b5a,199e,f10f,fe21,f52b,2d68h");

	Integer pGen, qGen, rOut, sOut;
	int c;

#ifdef NEW_SHA
	byte seed[]={0xd5, 0x01, 0x4e, 0x4b, 0x60, 0xef, 0x2b, 0xa8, 0xb6, 0x21, 
				 0x1b, 0x40, 0x62, 0xba, 0x32, 0x24, 0xe0, 0x42, 0x7d, 0xd3};
	Integer p("8df2a494 492276aa 3d25759b b06869cb eac0d83a fb8d0cf7"
			  "cbb8324f 0d7882e5 d0762fc5 b7210eaf c2e9adac 32ab7aac"
			  "49693dfb f83724c2 ec0736ee 31c80291H");
	Integer q("c773218c 737ec8ee 993b4f2d ed30f48e dace915fH");
#else
	byte seed[]={0xd5, 0x01, 0x4e, 0x4b, 0x60, 0xef, 0x2b, 0xa8, 0xb6, 0x21, 
				 0x1b, 0x40, 0x62, 0xba, 0x32, 0x24, 0xe0, 0x42, 0x7d, 0xbd};
	Integer p("d411a4a0e393f6aab0f08b14d18458665b3e4dbdce254454"
			 "3fe365cf71c8622412db6e7dd02bbe13d88c58d7263e9023"
			 "6af17ac8a9fe5f249cc81f427fc543f7H");
	Integer q("b20db0b101df0c6624fc1392ba55f77d577481e5H");
#endif

	fail = !GenerateDSAPrimes(seed, 160, c, pGen, 512, qGen);
	fail = fail || (pGen != p) || (qGen != q);
	pass = pass && !fail;

	cout << (fail ? "FAILED    " : "PASSED    ");
	cout << "prime generation test\n";

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

	pass = SignatureValidate(priv, pub) && pass;
	}
	FileSource fs("dsa1024.dat", true, new HexDecoder());
	DSAPrivateKey priv(fs);
	priv.LoadPrecomputation(fs);
	DSAPublicKey pub(priv);
	pass = SignatureValidate(priv, pub) && pass;
	return pass;
}

bool LUCValidate()
{
	cout << "\nLUC validation suite running...\n\n";

	FileSource f("luc512.dat", true, new HexDecoder);
	LUCPrivateKey priv(f);
	LUCPublicKey pub(priv);

	bool pass = SignatureValidate(priv, pub);
	pass = CryptoSystemValidate(priv, pub) && pass;
	return pass;
}

bool LUCELGValidate()
{
	cout << "\nLUCELG validation suite running...\n\n";

	FileSource f("lucs512.dat", true, new HexDecoder);
	LUCELG_Signer privS(f);
	LUCELG_Verifier pubS(privS);

	bool pass = SignatureValidate(privS, pubS);

	FileSource fc("lucc512.dat", true, new HexDecoder);
	LUCELG_Decryptor privC(fc);
	LUCELG_Encryptor pubC(privC);

	pass = CryptoSystemValidate(privC, pubC) && pass;

	return pass;
}

bool RabinValidate()
{
	cout << "\nRabin validation suite running...\n\n";

	FileSource f("rabi512.dat", true, new HexDecoder);
	RabinPrivateKey priv(f);
	RabinPublicKey pub(priv);

	bool pass = SignatureValidate(priv, pub);
	pass = CryptoSystemValidate(priv, pub) && pass;
	return pass;
}

bool BlumGoldwasserValidate()
{
	cout << "\nBlumGoldwasser validation suite running...\n\n";

	FileSource f("blum512.dat", true, new HexDecoder);
	BlumGoldwasserPrivateKey priv(f);
	BlumGoldwasserPublicKey pub(priv);

	return CryptoSystemValidate(priv, pub);
}

bool ECPValidate()
{
	cout << "\nECP validation suite running...\n\n";

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

	priv.Precompute();
	ByteQueue queue;
	priv.SavePrecomputation(queue);
	ECPublicKey<ECP> pub(ec, P, Q, o);
	pub.LoadPrecomputation(queue);

	bool pass = SignatureValidate(priv, pub);
	pass = CryptoSystemValidate(priv, pub) && pass;

	LC_RNG rng(726590);
	ECPrivateKey<ECP> priv2(rng, ec, P, o);
	ECKEP<ECP> eckep1(priv, priv2);
	ECKEP<ECP> eckep2(priv2, priv);

	pass = KeyAgreementValidate(eckep1, eckep2) && pass;
	return pass;
}

bool EC2NValidate()
{
	cout << "\nEC2N validation suite running...\n\n";

	Integer o("15,223,975,388,863,572,064,621,855,304,781,545,065,589,745,772");
	Integer d("12,065,729,449,256,706,362,097,909,124,274,151,550,853,609,397");
	GF2N gf2n(155, 62, 0);
	byte b[]={0x7, 0x33, 0x8f};
	EC2N ec(gf2n, PolynomialMod2::Zero(), PolynomialMod2(b,3));
	EC2N::Point P(0x7B, 0x1C8);
	EC2N::Point Q(ec.Multiply(d, P));
	ECPrivateKey<EC2N> priv(ec, P, Q, o, d);

	priv.Precompute();
	ByteQueue queue;
	priv.SavePrecomputation(queue);
	ECPublicKey<EC2N> pub(ec, P, Q, o);
	pub.LoadPrecomputation(queue);

	bool pass = SignatureValidate(priv, pub);
	pass = CryptoSystemValidate(priv, pub) && pass;

	LC_RNG rng(726591);
	ECPrivateKey<EC2N> priv2(rng, ec, P, o);
	ECKEP<EC2N> eckep1(priv, priv2);
	ECKEP<EC2N> eckep2(priv2, priv);

	pass = KeyAgreementValidate(eckep1, eckep2) && pass;
	return pass;
}
