#ifndef BENCH_H
#define BENCH_H

#include "cryptlib.h"

void BenchMarkAll(float t=1.0);
float BenchMark(BlockTransformation &cipher, float timeTotal);
float BenchMark(StreamCipher &cipher, float timeTotal);
float BenchMark(HashModule &hash, float timeTotal);
float BenchMark(BufferedTransformation &bt, float timeTotal);

float BenchMarkEncryption(PK_Encryptor &key, float timeTotal);
float BenchMarkDecryption(PK_Decryptor &priv, PK_Encryptor &pub, float timeTotal);
float BenchMarkSignature(PK_Signer &key, float timeTotal);
float BenchMarkVerification(PK_Signer &priv, PK_Verifier &pub, float timeTotal);

#endif
