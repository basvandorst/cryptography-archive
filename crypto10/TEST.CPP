#include "md5.h"
#include "shs.h"

#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
#include "rc4.h"
#include "default.h"

const int MAX_PHRASE_LENGTH=250;

void DigestFile(const char *file);

void EncryptFile(const char *in, const char *out, const char *passPhrase);
void DecryptFile(const char *in, const char *out, const char *passPhrase);

void ShareFile(int n, int m, const char *filename);
void AssembleFile(char *outfile, char **infiles, int n);

void GzipFile(const char *in, const char *out, int deflate_level);
void GunzipFile(const char *in, const char *out);

int main(int argc, char *argv[])
{
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
    case 'e':
    case 'd':
      {
        SecBlock<char> passPhrase(MAX_PHRASE_LENGTH);
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
        return !ValidateAll();
    case 'b':
        if (argc<3)
            BenchMarkAll();
        else
            BenchMarkAll(atol(argv[2]));
        return 0;
    case 'g':
        GzipFile(argv[3], argv[4], argv[2][0]-'0');
        return 0;
    case 'u':
        GunzipFile(argv[2], argv[3]);
        return 0;
    default:
        FileSource usage("usage.dat", TRUE, new FileSink(cout));
        return 1;
    }
}

void DigestFile(const char *filename)
{
    MD5 md5;
    SHS shs;
    BufferedTransformation *outputs[]={new HashFilter(md5), new HashFilter(shs)};
    FileSource file(filename, TRUE, new Fork(2, outputs));

    cout << "MD5:    ";
    outputs[0]->Attach(new HexEncoder(new FileSink(cout)));
    cout << endl;
    cout << "SHS:    ";
    outputs[1]->Attach(new HexEncoder(new FileSink(cout)));
    cout << endl;
}

void EncryptFile(const char *in, const char *out, const char *passPhrase)
{
    FileSource f(in, TRUE, new DefaultEncryptor(passPhrase, new FileSink(out)));
}

void DecryptFile(const char *in, const char *out, const char *passPhrase)
{
    DefaultDecryptor *cipher = new DefaultDecryptor(passPhrase);
    FileSource file(in, FALSE, cipher);
    file.Pump(256);
    if (cipher->CurrentState()!=DefaultDecryptor::KEY_GOOD)
    {
        cerr << "Bad passphrase.\n";
        return;
    }

    file.Attach(new FileSink(out));
    file.Close();
}

void ShareFile(int n, int m, const char *filename)
{
    assert(n<=100);

    SecByteBlock key(16);

    {   // use braces to force file to close
        FileSource file(filename, TRUE, new HashFilter(MD5()));
        file.Get(key, 16);
    }

    X917RNG rng(new Default_ECB_Encryption(key), key);
    rng.GetBlock(key, 16);
    ShareFork pss(rng, m, n);
    pss.Put(key, 16);

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

    FileSource file(filename, TRUE,
                    new StreamCipherFilter(RC4(key, MD5::DIGESTSIZE),
                    new DisperseFork(m, n, outFiles)));
}

void AssembleFile(char *out, char **filenames, int n)
{
    assert(n<=100);

    FileSource *inFiles[100];
    ShareJoin pss(n);

    for (int i=0; i<n; i++)
    {
        inFiles[i] = new FileSource(filenames[i]);
        inFiles[i]->Attach(pss.SelectInterface(i));
    }

    while (pss.MaxRetrieveable() < 16 && inFiles[0]->Pump(1))
        for (i=1; i<n; i++)
            inFiles[i]->Pump(1);

    SecByteBlock key(16);
    pss.Get(key, 16);
    RC4 rc4(key, 16);

    DisperseJoin j(n, new StreamCipherFilter(rc4, new FileSink(out)));

    for (i=0; i<n; i++)
        inFiles[i]->Detach(j.SelectInterface(i));

    while (inFiles[0]->Pump(256))
        for (i=1; i<n; i++)
            inFiles[i]->Pump(256);

    for (i=0; i<n; i++)
    {
        inFiles[i]->Close();
        delete inFiles[i];
    }
}

void GzipFile(const char *in, const char *out, int deflate_level)
{
    FileSource(in, TRUE, new Gzip(deflate_level, new FileSink(out)));
}

void GunzipFile(const char *in, const char *out)
{
    FileSource(in, TRUE, new Gunzip(new FileSink(out)));
}

