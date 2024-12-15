// secshare.cpp - written and placed in the public domain by Wei Dai
// part of the code for polynomial evaluation and interpolation
// came from Hal Finney's secsplit.c

#include "misc.h"
#include "queue.h"
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
	int i;
    for (i=1; i<n; i++)
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
	int i;

#define iplus1  ( i==2 ? 0 : i+1 )	// used by Euclid algorithms
#define iminus1 ( i==0 ? 2 : i-1 )	// used by Euclid algorithms
    for (i=1; !!g[i]; i = iplus1)
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
        0U, 1U, 32761U, 43681U, 49141U, 52417U, 54601U, 56161U,
        57331U, 58241U, 58969U, 11913U, 60061U, 60481U, 60841U, 61153U,
        61426U, 42396U, 61881U, 6897U, 62245U, 62401U, 38717U, 11395U,
        62791U, 49796U, 63001U, 41254U, 63181U, 58743U, 63337U, 25363U,
        30713U, 3971U, 21198U, 63649U, 63701U, 54896U, 36209U, 63841U,
        63883U, 43148U, 63961U, 6095U, 52119U, 64065U, 38458U, 43216U,
        64156U, 8023U, 24898U, 14132U, 64261U, 4945U, 20627U, 28591U,
        64351U, 2299U, 62132U, 21100U, 64429U, 27927U, 45442U, 64481U,
        48117U, 64513U, 34746U, 26404U, 10599U, 47479U, 64585U, 5537U,
        64611U, 27824U, 27448U, 38439U, 50865U, 11062U, 64681U, 41469U,
        64702U, 57432U, 21574U, 48154U, 64741U, 60896U, 35808U, 19581U,
        58820U, 50061U, 64793U, 64801U, 19229U, 52135U, 21608U, 40692U,
        32078U, 52687U, 36772U, 23164U, 12449U, 53844U, 7066U, 60432U,
        64891U, 64897U, 35233U, 15921U, 43074U, 5410U, 47056U, 40139U,
        64936U, 3479U, 33910U, 2279U, 31066U, 64961U, 10550U, 34137U,
        64975U, 1083U, 46724U, 36223U, 22721U, 62376U, 65001U, 53655U,
        56819U, 23872U, 65017U, 53017U, 17373U, 47786U, 13202U, 21355U,
        38060U, 43043U, 56500U, 3771U, 65053U, 58086U, 35529U, 41237U,
        65066U, 37957U, 13912U, 46355U, 13724U, 47052U, 51980U, 40354U,
        58193U, 26551U, 5531U, 31281U, 65101U, 1252U, 53495U, 45329U,
        32351U, 10988U, 28716U, 39393U, 10787U, 53211U, 24077U, 16086U,
        65131U, 44973U, 30448U, 44447U, 17904U, 29920U, 42551U, 25834U,
        29410U, 50714U, 57791U, 18668U, 65157U, 362U, 65161U, 9309U,
        42375U, 63396U, 58828U, 27680U, 10804U, 43334U, 20346U, 57288U,
        16039U, 13240U, 59104U, 65185U, 18386U, 54878U, 11582U, 64204U,
        38985U, 52482U, 26922U, 17752U, 3533U, 34838U, 30216U, 59507U,
        65206U, 627U, 65209U, 5900U, 50377U, 23686U, 40721U, 1219U,
        21537U, 50424U, 2705U, 31115U, 23528U, 53662U, 52830U, 39959U,
        32468U, 12813U, 34500U, 10391U, 16955U, 60657U, 33900U, 47368U,
        15533U, 55960U, 65241U, 61060U, 5275U, 13823U, 49829U, 54281U,
        65248U, 19031U, 33302U, 19144U, 23362U, 27813U, 50872U, 30771U,
        44121U, 59732U, 31188U, 6526U, 65261U, 54644U, 59588U, 42139U
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
    poly.SetCoefficient(bufCount++, message);

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
    while (n>=2)
    {
	    SecBlock<word16> y(numberOfPorts);
		int i;

        for (i=0; i<numberOfPorts; i++)
            inPorts[i]->GetShort(y[i]);

        Polynomial poly(x, y, numberOfPorts);

        for (i=0; i<threshold; i++)
            Output(poly.Coefficient(i));

        n -= 2;
    }
}
