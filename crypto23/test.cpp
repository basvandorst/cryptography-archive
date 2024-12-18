// test.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#include "md5.h"
#include "sha.h"
#include "haval.h"

#include "iostream"
USING_NAMESPACE(std)
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <exception>

#include "misc.h"
#include "modes.h"
#include "files.h"
#include "validate.h"
#include "filters.h"
#include "rng.h"
#include "secshare.h"
#include "hex.h"
#include "bench.h"
#include "gzip.h"
#include "default.h"

#if (_MSC_VER >= 1000)
#include <crtdbg.h>		// for the debug heap
#endif

const int MAX_PHRASE_LENGTH=250;

void DigestFile(const char *file);

char *EncryptString(const char *plaintext, const char *passPhrase);
char *DecryptString(const char *ciphertext, const char *passPhrase);

void EncryptFile(const char *in, const char *out, const char *passPhrase);
void DecryptFile(const char *in, const char *out, const char *passPhrase);

void ShareFile(int n, int m, const char *filename);
void AssembleFile(char *outfile, char **infiles, int n);

void GzipFile(const char *in, const char *out, int deflate_level);
void GunzipFile(const char *in, const char *out);

bool Validate(int);

int main(int argc, char *argv[])
{
#ifdef _CRTDBG_LEAK_CHECK_DF
	// Turn on leak-checking
	int tempflag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tempflag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tempflag );
#endif

#ifdef THROW_EXCEPTIONS
	try
	{
#endif
	char command;

	if (argc < 2)
		command = 'h';
	else
		command = argv[1][0];

	switch (command)
	{
	case 'm':
		DigestFile(argv[2]);
		return 0;
	case 't':
	  {
		char passPhrase[MAX_PHRASE_LENGTH], plaintext[1024];
		char *ciphertext, *decrypted;

		cout << "Passphrase: ";
		cin.getline(passPhrase, MAX_PHRASE_LENGTH);

		cout << "\nPlaintext: ";
		cin.getline(plaintext, 1024);

		ciphertext = EncryptString(plaintext, passPhrase);
		cout << "\nCiphertext: " << ciphertext << endl;

		decrypted = DecryptString(ciphertext, passPhrase);
		cout << "\nDecrypted: " << decrypted << endl;

		delete [] ciphertext;
		delete [] decrypted;
		return 0;
	  }
	case 'e':
	case 'd':
	  {
		char passPhrase[MAX_PHRASE_LENGTH];
		cout << "Passphrase: ";
		cin.getline(passPhrase, MAX_PHRASE_LENGTH);
		if (command == 'e')
			EncryptFile(argv[2], argv[3], passPhrase);
		else
			DecryptFile(argv[2], argv[3], passPhrase);
		return 0;
	  }
	case 's':
		ShareFile(atoi(argv[2]), atoi(argv[3]), argv[4]);
		return 0;
	case 'j':
		AssembleFile(argv[2], argv+3, argc-3);
		return 0;
	case 'v':
		return !Validate(argc>2 ? atoi(argv[2]) : 0);
	case 'b':
		if (argc<3)
			BenchMarkAll();
		else
			BenchMarkAll((float)atof(argv[2]));
		return 0;
	case 'g':
		GzipFile(argv[3], argv[4], argv[2][0]-'0');
		return 0;
	case 'u':
		GunzipFile(argv[2], argv[3]);
		return 0;
	default:
		FileSource usage("usage.dat", true, new FileSink(cout));
		return 1;
	}
#ifdef THROW_EXCEPTIONS
	}
	catch(CryptlibException &e)
	{
		cout << "CryptlibException caught: " << e.what() << endl;
		return -1;
	}
	catch(exception &e)
	{
		cout << "exception caught: " << e.what() << endl;
		return -2;
	}
	catch(...)
	{
		cout << "unknown exception caught" << endl;
		return -3;
	}
#endif
}

void DigestFile(const char *filename)
{
	MD5 md5;
	SHA shs;
	HAVAL3 haval;
	BufferedTransformation *outputs[]={new HashFilter(md5), new HashFilter(shs), new HashFilter(haval)};
	FileSource file(filename, true, new Fork(3, outputs));

	cout << "MD5:    ";
	outputs[0]->Attach(new HexEncoder(new FileSink(cout)));
	cout << endl;
	cout << "SHA:    ";
	outputs[1]->Attach(new HexEncoder(new FileSink(cout)));
	cout << endl;
	cout << "HAVAL:  ";
	outputs[2]->Attach(new HexEncoder(new FileSink(cout)));
	cout << endl;
}

char *EncryptString(const char *instr, const char *passPhrase)
{
	unsigned int len=strlen(instr);
	char* outstr;

	DefaultEncryptor encryptor(passPhrase, new HexEncoder());
	encryptor.Put((byte *)instr, len);
	encryptor.Close();

	unsigned int outputLength = encryptor.MaxRetrieveable();
	outstr = new char[outputLength+1];
	encryptor.Get((byte *)outstr, outputLength);
	outstr[outputLength] = 0;
	return outstr;
}

char *DecryptString(const char *instr, const char *passPhrase)
{
	unsigned int len=strlen(instr);
	char* outstr;

	HexDecoder decryptor(new DefaultDecryptor(passPhrase));
	decryptor.Put((byte *)instr, len);
	decryptor.Close();

	unsigned int outputLength = decryptor.MaxRetrieveable();
	outstr = new char[outputLength+1];
	decryptor.Get((byte *)outstr, outputLength);
	outstr[outputLength] = 0;
	return outstr;
}

void EncryptFile(const char *in, const char *out, const char *passPhrase)
{
	FileSource f(in, true, new DefaultEncryptor(passPhrase, new FileSink(out)));
}

void DecryptFile(const char *in, const char *out, const char *passPhrase)
{
	DefaultDecryptor *cipher = new DefaultDecryptor(passPhrase);
	FileSource file(in, false, cipher);
	file.Pump(256);
	if (cipher->CurrentState()!=DefaultDecryptor::KEY_GOOD)
	{
		cerr << "Bad passphrase.\n";
		return;
	}

	file.Attach(new FileSink(out));
	file.PumpAll();
	file.Close();
}

void ShareFile(int n, int m, const char *filename)
{
	assert(n<=100);

	SecByteBlock key(16), IV(16);

	{   // use braces to force file to close
		MD5 md5;
		FileSource file(filename, true, new HashFilter(md5));
		file.Get(key, 16);
	}

	X917RNG rng(new Default_ECB_Encryption(key), key);
	rng.GetBlock(key, 16);
	ShareFork pss(rng, m, n);
	pss.Put(key, 16);
	pss.Close();

	char outname[256];
	strcpy(outname, filename);
	int inFilenameLength = strlen(filename);
	outname[inFilenameLength] = '.';

	BufferedTransformation *outFiles[100];
	for (int i=0; i<n; i++)
	{
		outname[inFilenameLength+1]='0'+byte(i/10);
		outname[inFilenameLength+2]='0'+byte(i%10);
		outname[inFilenameLength+3]='\0';
		outFiles[i] = new FileSink(outname);

		pss.SelectOutPort(i);
		pss.TransferTo(*outFiles[i]);
	}

	MD5 md5;
	md5.CalculateDigest(IV, key, 16);

	Default_ECB_Encryption ecb(key);
	CFBEncryption cipher(ecb, IV);

	FileSource file(filename, true,
					new StreamCipherFilter(cipher, 
					new DisperseFork(m, n, outFiles)));
}

void AssembleFile(char *out, char **filenames, int n)
{
	assert(n<=100);

	auto_ptr<FileSource> inFiles[100];
	ShareJoin pss(n);
	int i;

	for (i=0; i<n; i++)
	{
		inFiles[i] = auto_ptr<FileSource>(new FileSource(filenames[i], false, pss.ReleaseInterface(i)));
		inFiles[i]->Pump(28);
		inFiles[i]->Detach();
	}

	SecByteBlock key(16), IV(16);
	inFiles[n-1]->Get(key, 16);
	Default_ECB_Encryption ecb(key);
	MD5 md5;
	md5.CalculateDigest(IV, key, 16);
	CFBDecryption cfb(ecb, IV);
	DisperseJoin j(n, new StreamCipherFilter(cfb, new FileSink(out)));

	for (i=0; i<n; i++)
		inFiles[i]->Attach(j.ReleaseInterface(i));

	while (inFiles[0]->Pump(256))
		for (i=1; i<n; i++)
			inFiles[i]->Pump(256);

	for (i=0; i<n; i++)
	{
		inFiles[i]->PumpAll();
		inFiles[i]->Close();
	}
}

void GzipFile(const char *in, const char *out, int deflate_level)
{
	FileSource(in, true, new Gzip(deflate_level, new FileSink(out)));
}

void GunzipFile(const char *in, const char *out)
{
	FileSource(in, true, new Gunzip(new FileSink(out)));
}

bool Validate(int alg)
{
	switch (alg)
	{
	case 1: return TestSettings();
	case 2: return ZKValidate();
	case 3: return MD5Validate();
	case 4: return SHAValidate();
	case 5: return DESValidate();
	case 6: return IDEAValidate();
	case 7: return RC4Validate();
	case 8: return RC5Validate();
	case 9: return BlowfishValidate();
	case 10: return Diamond2Validate();
	case 11: return ThreeWayValidate();
	case 12: return BBSValidate();
	case 13: return DHValidate();
	case 14: return RSAValidate();
	case 15: return ElGamalValidate();
	case 16: return DSAValidate();
	case 17: return HAVALValidate();
	case 18: return SAFERValidate();
	case 19: return LUCValidate();
	case 20: return RabinValidate();
	case 21: return BlumGoldwasserValidate();
	case 22: return ECPValidate();
	case 23: return EC2NValidate();
	case 24: return MD5MACValidate();
	case 25: return GOSTValidate();
	case 26: return TigerValidate();
	case 27: return RIPEMDValidate();
	case 28: return HMACValidate();
	case 29: return XMACCValidate();
	case 30: return SHARKValidate();
	case 31: return SHARK2Validate();
	case 32: return LUCDIFValidate();
	case 33: return LUCELGValidate();
	case 34: return SEALValidate();
	case 35: return CASTValidate();
	case 36: return SquareValidate();
	default: return ValidateAll();
	}
}
