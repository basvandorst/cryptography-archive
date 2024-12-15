#ifndef SECSHARE_H
#define SECSHARE_H

#include "forkjoin.h"
#include "misc.h"

const unsigned int SECSHARE_PRIME = 65521U;

class Polynomial
{
public:
    Polynomial(unsigned int degree);
    Polynomial(RandomNumberGenerator &rng, unsigned int degree, unsigned int message);
    Polynomial(const word16 *x, const word16 *y, int n);

    unsigned int Degree() const {return degree;}
    unsigned int Coefficient(unsigned int power) {return coefficients[power];}
    void SetCoefficient(unsigned int power, unsigned int value) {coefficients[power] = value;}

    unsigned int Evaluate(int x) const;
    static unsigned int Interpolate (int i, const word16 *x, const word16 *y, int n);

private:
    static unsigned int Inverse (int x);
    static void CalculateAlphas (unsigned long *alpha, const word16 *x, const word16 *y, int n);

    unsigned int degree;
    SecBlock<unsigned int> coefficients;
};

class ShareFork : public Fork
{
public:
    ShareFork(RandomNumberGenerator &rng, int m, int n);
    ShareFork(RandomNumberGenerator &rng, int m, int n,
              BufferedTransformation *const *outports);

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);
    virtual void InputFinished();

protected:
    void Process(unsigned int message);
    virtual void Share(unsigned int message);

    RandomNumberGenerator &rng;
    const int threshold;
    byte lastByte;
    boolean leftOver;
};

class ShareJoin : public Join
{
public:
    ShareJoin(int n, BufferedTransformation *outQ = NULL);

	boolean EOFReached() const {return eofReached;}

    void NotifyInput(int interfaceId, unsigned int length);

protected:
    void ReadIndex();
    virtual void Assemble(unsigned long);
    void Output(unsigned int);

    word16 threshold;
    SecBlock<word16> x;

    // state flags
    boolean indexRead, continued, eofReached;
};

class DisperseFork : public ShareFork
{
public:
    DisperseFork(int m, int n);
    DisperseFork(int m, int n, BufferedTransformation *const *outports);

    virtual void InputFinished();

protected:
    virtual void Share(unsigned int message);

    Polynomial poly;
    int bufCount;
};

class DisperseJoin : public ShareJoin
{
public:
    DisperseJoin(int n, BufferedTransformation *outQ = NULL);

protected:
    virtual void Assemble(unsigned long);
};

#endif

