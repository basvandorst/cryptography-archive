// the code for polynomial evaluation and interpolation
// came from Hal Finney's secsplit.c

#include "misc.h"
#include "secshare.h"

Polynomial::Polynomial(unsigned int degree)
    : degree(degree), coefficients(degree+1)
{
}

Polynomial::Polynomial(RandomNumberGenerator &rng, unsigned int d, unsigned int message)
    : degree(d), coefficients(degree+1)
{
    coefficients[0]=message;
    for (unsigned int i=1; i<=degree; i++)
        coefficients[i] = rng.GetShort(0, SECSHARE_PRIME-1);
}

// reconstruct polynomial using Lagrange interpolation
Polynomial::Polynomial(const word16 *x, const word16 *y, int n)
    : degree(n-1), coefficients(n)
{
    SecBlock<unsigned long> alpha(n);
    CalculateAlphas(alpha, x, y, n);

    coefficients[0] = (unsigned int) alpha[n-1];
    for (int i=1; i<n; i++)
        coefficients[i] = 0;

    for (int j=n-2; j>=0; --j)
    {
        unsigned int xj = SECSHARE_PRIME-x[j];

        for (i=n-j-1; i>0; i--)
            coefficients[i] = (unsigned int)(((unsigned long)coefficients[i]*xj + coefficients[i-1]) % SECSHARE_PRIME);

        coefficients[0] = (unsigned int)(((unsigned long)coefficients[0]*xj + alpha[j]) % SECSHARE_PRIME);
    }

    while (degree && coefficients[degree]==0)
        degree--;
}

unsigned int Polynomial::Evaluate(int x) const
{
    unsigned long prod = coefficients[degree];
    for (int j=degree-1; j>=0; j--)
    {
    	prod *= x;
    	prod += coefficients[j];
    	prod %= SECSHARE_PRIME;
    }
    return (unsigned int) prod;
}

static unsigned int CalculateInverse(unsigned int x)
{
    unsigned int g[3]={SECSHARE_PRIME, x};
    long v[3]={0, 1};
    unsigned int y;

#define iplus1  ( i==2 ? 0 : i+1 )	// used by Euclid algorithms
#define iminus1 ( i==0 ? 2 : i-1 )	// used by Euclid algorithms
    for (int i=1; !!g[i]; i = iplus1)
    {
        y = g[iminus1] / g[i];
        g[iplus1] = g[iminus1] % g[i];
        v[iplus1] = v[iminus1] - (y * v[i]);
    }

    if (v[iminus1] < 0)
        v[iminus1] += SECSHARE_PRIME;

    return (unsigned int)v[iminus1];
}

unsigned int Polynomial::Inverse (int x)
{
    // Multiplicative inverses of 1-255 mod SECSHARE_PRIME
    static const unsigned int invtab[] = {
        0, 1, 32761, 43681, 49141, 52417, 54601, 56161,
        57331, 58241, 58969, 11913, 60061, 60481, 60841, 61153,
        61426, 42396, 61881, 6897, 62245, 62401, 38717, 11395,
        62791, 49796, 63001, 41254, 63181, 58743, 63337, 25363,
        30713, 3971, 21198, 63649, 63701, 54896, 36209, 63841,
        63883, 43148, 63961, 6095, 52119, 64065, 38458, 43216,
        64156, 8023, 24898, 14132, 64261, 4945, 20627, 28591,
        64351, 2299, 62132, 21100, 64429, 27927, 45442, 64481,
        48117, 64513, 34746, 26404, 10599, 47479, 64585, 5537,
        64611, 27824, 27448, 38439, 50865, 11062, 64681, 41469,
        64702, 57432, 21574, 48154, 64741, 60896, 35808, 19581,
        58820, 50061, 64793, 64801, 19229, 52135, 21608, 40692,
        32078, 52687, 36772, 23164, 12449, 53844, 7066, 60432,
        64891, 64897, 35233, 15921, 43074, 5410, 47056, 40139,
        64936, 3479, 33910, 2279, 31066, 64961, 10550, 34137,
        64975, 1083, 46724, 36223, 22721, 62376, 65001, 53655,
        56819, 23872, 65017, 53017, 17373, 47786, 13202, 21355,
        38060, 43043, 56500, 3771, 65053, 58086, 35529, 41237,
        65066, 37957, 13912, 46355, 13724, 47052, 51980, 40354,
        58193, 26551, 5531, 31281, 65101, 1252, 53495, 45329,
        32351, 10988, 28716, 39393, 10787, 53211, 24077, 16086,
        65131, 44973, 30448, 44447, 17904, 29920, 42551, 25834,
        29410, 50714, 57791, 18668, 65157, 362, 65161, 9309,
        42375, 63396, 58828, 27680, 10804, 43334, 20346, 57288,
        16039, 13240, 59104, 65185, 18386, 54878, 11582, 64204,
        38985, 52482, 26922, 17752, 3533, 34838, 30216, 59507,
        65206, 627, 65209, 5900, 50377, 23686, 40721, 1219,
        21537, 50424, 2705, 31115, 23528, 53662, 52830, 39959,
        32468, 12813, 34500, 10391, 16955, 60657, 33900, 47368,
        15533, 55960, 65241, 61060, 5275, 13823, 49829, 54281,
        65248, 19031, 33302, 19144, 23362, 27813, 50872, 30771,
        44121, 59732, 31188, 6526, 65261, 54644, 59588, 42139
    };

    static const int tablesize = sizeof(invtab)/sizeof(invtab[0]);

    if (x < 0)
        if (-x < tablesize)
    	    return SECSHARE_PRIME - invtab[-x];
        else
            return SECSHARE_PRIME - CalculateInverse(-x);
    else
        if (x < tablesize)
    	    return invtab[x];
        else
            return CalculateInverse(x);
}

void Polynomial::CalculateAlphas (unsigned long *alpha, const word16 *x, const word16 *y, int n)
{
    int j, k;

    for (j=0; j<n; ++j)
    	alpha[j] = y[j];

    for (k=1; k<n; ++k)
    {
        for (j=n-1; j>=k; --j)
        {
            if (alpha[j] > alpha[j-1])
                alpha[j] = alpha[j] - alpha[j-1];
            else
                alpha[j] = alpha[j] - alpha[j-1] + SECSHARE_PRIME;
            alpha[j] *= Inverse (x[j] - x[j-k]);
            alpha[j] %= SECSHARE_PRIME;
        }
    }
}

// do polynomial interpolation at point x=i
unsigned int Polynomial::Interpolate (int i, const word16 *x, const word16 *y, int n)
{
    SecBlock<unsigned long> alpha(n);
    CalculateAlphas(alpha, x, y, n);

    unsigned long prod = alpha[n-1];
    for (int j=n-2; j>=0; --j)
    {
        if (i < x[j])
            prod *= i-x[j]+SECSHARE_PRIME;
        else
            prod *= i-x[j];

        prod += alpha[j];
        prod %= SECSHARE_PRIME;
    }
    return (unsigned int) prod;
}

// ***********************************************************

ShareFork::ShareFork(RandomNumberGenerator &inRng, int m, int n)
    : Fork(n), rng(inRng), threshold(m)
{
    leftOver=FALSE;
    for (int i=0; i<numberOfPorts; i++)
    {
        outPorts[i]->PutShort(threshold);
        outPorts[i]->PutShort(i+1);
    }
}

ShareFork::ShareFork(RandomNumberGenerator &rng, int m, int n, BufferedTransformation *const *outports)
    : Fork(n, outports), rng(rng), threshold(m)
{
    leftOver=FALSE;
    for (int i=0; i<numberOfPorts; i++)
    {
        outPorts[i]->PutShort(threshold);
        outPorts[i]->PutShort(i+1);
    }
}

void ShareFork::Put(byte inByte)
{
    if (leftOver)
        Process((lastByte << 8) | inByte);
    else
        lastByte = inByte;

    leftOver = (!leftOver);
}

void ShareFork::Put(const byte *inString, unsigned int length)
{
    if (leftOver && length)
    {
        Process((lastByte << 8) | inString[0]);
        length--;
        inString++;
        leftOver = FALSE;
    }

    while (length>1)
    {
        Process((inString[0] << 8) | inString[1]);
        length-=2;
        inString+=2;
    }

    if (length)
    {
        lastByte = inString[0];
        leftOver = TRUE;
    }
}

void ShareFork::Process(unsigned int message)
{
    if (message >= SECSHARE_PRIME-1)
    {
        // encode message that is >= SECSHARE_PRIME-1
        Share(SECSHARE_PRIME-1);
        Share(message - (SECSHARE_PRIME-1));
    }
    else
        Share(message);
}

void ShareFork::Share(unsigned int message)
{
    Polynomial poly(rng, threshold-1, message);
    for (int i=0; i<numberOfPorts; i++)
        outPorts[i]->PutShort(poly.Evaluate(i+1));
}

void ShareFork::InputFinished()
{
    Share(SECSHARE_PRIME-1);

    if (leftOver)
        Share(0x8000 | lastByte);   // encode the last odd byte
    else
        Share(0x7fff);  // indicate EOF with even number of bytes
}

// ****************************************************************

ShareJoin::ShareJoin(int n, BufferedTransformation *outQ)
    : Join(n, outQ), x(n)
{
    assert(n>0);

    indexRead=FALSE;
    continued=FALSE;
    eofReached=FALSE;
}

void ShareJoin::NotifyInput(int /* interfaceId */, unsigned int /* length */)
{
    unsigned long n=inPorts[0]->MaxRetrieveable();

    for (int i=1; n && i<numberOfPorts; i++)
        n = min(n, inPorts[i]->MaxRetrieveable());

    if (n>=4 && !indexRead)
    {
        ReadIndex();
        n-=4;
    }

    if (n>=2 && indexRead)
        Assemble(n);
}

void ShareJoin::ReadIndex()
{
    for (int i=0; i<numberOfPorts; i++)
    {
        inPorts[i]->GetShort(threshold);
        inPorts[i]->GetShort(x[i]);
    }

    indexRead = TRUE;
}

void ShareJoin::Assemble(unsigned long n)
{
    SecBlock<word16> y(numberOfPorts);

    while (n>=2)
    {
        for (int i=0; i<numberOfPorts; i++)
            inPorts[i]->GetShort(y[i]);

        Output(Polynomial::Interpolate(0, x, y, numberOfPorts));
        n -= 2;
    }
}

void ShareJoin::Output(unsigned int message)
{
    if (eofReached)
        return;

    if (message == SECSHARE_PRIME-1)
    {
        continued = TRUE;
        return;
    }

    if (continued)
    {
        if (message == 0x7fff)
        {
            eofReached = TRUE;
            return;
        }

        if (message >= 0x8000)  // decode last odd byte
        {
            outQueue->Put((byte)message);
            eofReached = TRUE;
            return;
        }

        // decode message that is >= SECSHARE_PRIME-1
        message += (SECSHARE_PRIME-1);
        continued = FALSE;
    }

    outQueue->PutShort(message);
}

// ************************************************************

DisperseFork::DisperseFork(int m, int n)
    : ShareFork(*(RandomNumberGenerator *)0, m, n),
      poly(m-1)
{
    bufCount = 0;
}

DisperseFork::DisperseFork(int m, int n, BufferedTransformation *const *outports)
    : ShareFork(*(RandomNumberGenerator *)0, m, n, outports),
      poly(m-1)
{
    bufCount = 0;
}

void DisperseFork::Share(unsigned int message)
{
    poly.Coefficient(bufCount++) = message;

    if (bufCount==threshold)
    {
        for (int i=0; i<numberOfPorts; i++)
            outPorts[i]->PutShort(poly.Evaluate(i+1));

        bufCount = 0;
    }
}

void DisperseFork::InputFinished()
{
    ShareFork::InputFinished();

    while (bufCount)    // flush out buffer
        Share(0);
}

DisperseJoin::DisperseJoin(int n, BufferedTransformation *outQ)
    : ShareJoin(n, outQ)
{
}

void DisperseJoin::Assemble(unsigned long n)
{
    SecBlock<word16> y(numberOfPorts);

    while (n>=2)
    {
        for (int i=0; i<numberOfPorts; i++)
            inPorts[i]->GetShort(y[i]);

        Polynomial poly(x, y, numberOfPorts);

        for (i=0; i<threshold; i++)
            Output(poly.Coefficient(i));

        n -= 2;
    }
}
