#ifndef BENCH_H
#define BENCH_H

#include "cryptlib.h"

void BenchMarkAll(long length=1024L);
float BenchMark(BlockTransformation &cipher, long length);
float BenchMark(StreamCipher &cipher, long length);
float BenchMark(HashModule &hash, long length);
float BenchMark(BufferedTransformation &bt, long length);

#endif

