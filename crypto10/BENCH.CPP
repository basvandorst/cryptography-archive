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

#include <time.h>

#include "bench.h"
#include "modes.h"
#include "mdc.h"
#include "lubyrack.h"
#include "sapphire.h"

#ifndef CLK_TCK
#ifdef CLOCKS_PER_SEC
#define CLK_TCK (float)CLOCKS_PER_SEC
#else
#define CLK_TCK 1000000.0
#endif
#endif

void BenchMarkAll(long length)
{
    byte *const key=(byte *)"0123456789abcdef000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    cout << "Cipher              Total Bytes    Time   Bytes/S\n\n";
    cout << "MD5                ";
    BenchMark(MD5(), length*2048);
    cout << "SHS                ";
    BenchMark(SHS(), length*2048);
    cout << "MDC/MD5 (raw)      ";
    BenchMark(MDC<MD5>(key), length*1024);
    cout << "MDC/MD5 (CFB)      ";
    BenchMark(CFBEncryption(MDC<MD5>(key), key), length*1024);
    cout << "MDC/SHS (raw)      ";
    BenchMark(MDC<SHS>(key), length*1024);
    cout << "MDC/SHS (CFB)      ";
    BenchMark(CFBEncryption(MDC<SHS>(key), key), length*1024);
    cout << "Luby-Rackoff/MD5   ";
    BenchMark(LREncryption<MD5>(key, 78), length*256);
    cout << "Luby-Rackoff/SHS   ";
    BenchMark(LREncryption<SHS>(key, 70), length*256);
    cout << "DES                ";
    BenchMark(DESEncryption(key), length*1024);
    cout << "IDEA               ";
    BenchMark(IDEAEncryption(key), length*512);
    cout << "RC5 (r=12)         ";
    BenchMark(RC5Encryption(key), length*1024);
    cout << "Blowfish           ";
    BenchMark(BlowfishEncryption(key), length*1024);
    cout << "Diamond (r=10)     ";
    BenchMark(DiamondEncryption(key), length*512);
    cout << "Diamond Lite (r=8) ";
    BenchMark(DiamondLiteEncryption(key), length*512);
    cout << "RC4                ";
    BenchMark(RC4(key), length*2048);
    cout << "WAKE               ";
    BenchMark(WAKEEncryption(key, new BitBucket), length*2048);
    cout << "Sapphire           ";
    BenchMark(SapphireEncryption(key), length*512);

    cout << "BBS (n is 653 bits)";
    bignum p("212004934506826557583707108431463840565872545889679278744389317666981496005411448865750399674653351");
    bignum q("100677295735404212434355574418077394581488455772477016953458064183204108039226017738610663984508231");
    bignum seed("63239752671357255800299643604761065219897634268887145610573595874544114193025997412441121667211431");
    BenchMark(BlumBlumShub(p, q, seed), ((length+255)/256)*128);
}

float BenchMark(BlockTransformation &cipher, long length)
{
    const int BUF_SIZE = cipher.BlockSize();
    byte *buf = new byte[BUF_SIZE];
    clock_t start = clock();
    for (long i=0; i<length/BUF_SIZE; i++)
        cipher.ProcessBlock(buf);
    float timeTaken = float(clock() - start) / CLK_TCK;
    float kbs = length / timeTaken;
    cout << setw(10) << length;
    cout << setw(10) << timeTaken;
    cout << setw(10) << (long)kbs << endl;
    delete [] buf;
    return kbs;
}

float BenchMark(StreamCipher &cipher, long length)
{
    const int BUF_SIZE=128; // encrypt 128 bytes at a time
    byte *buf = new byte[BUF_SIZE];
    clock_t start = clock();
    for (long i=0; i<length/BUF_SIZE; i++)
        cipher.ProcessString(buf, BUF_SIZE);
    float timeTaken = float(clock() - start) / CLK_TCK;
    float kbs = length / timeTaken;
    cout << setw(10) << length;
    cout << setw(10) << timeTaken;
    cout << setw(10) << (long)kbs << endl;
    delete [] buf;
    return kbs;
}

float BenchMark(HashModule &hash, long length)
{
    const int BUF_SIZE=128; // update 128 bytes at a time
    byte *buf = new byte[BUF_SIZE];
    clock_t start = clock();
    for (long i=0; i<length/BUF_SIZE; i++)
        hash.Update(buf, BUF_SIZE);
    float timeTaken = float(clock() - start) / CLK_TCK;
    float kbs = length / timeTaken;
    cout << setw(10) << length;
    cout << setw(10) << timeTaken;
    cout << setw(10) << (long)kbs << endl;
    delete [] buf;
    return kbs;
}

float BenchMark(BufferedTransformation &bt, long length)
{
    const int BUF_SIZE=128; // update 128 bytes at a time
    byte *buf = new byte[BUF_SIZE];
    clock_t start = clock();
    for (long i=0; i<length/BUF_SIZE; i++)
        bt.Put(buf, BUF_SIZE);
    float timeTaken = float(clock() - start) / CLK_TCK;
    float kbs = length / timeTaken;
    cout << setw(10) << length;
    cout << setw(10) << timeTaken;
    cout << setw(10) << (long)kbs << endl;
    delete [] buf;
    return kbs;
}
