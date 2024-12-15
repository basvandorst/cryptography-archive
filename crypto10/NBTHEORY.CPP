#include "nbtheory.h"

static const int maxPrimeTableSize = 3511;
static int primeTableSize=552;

static word16 primeTable[maxPrimeTableSize] =
    {2, 3, 5, 7, 11, 13, 17, 19,
    23, 29, 31, 37, 41, 43, 47, 53,
    59, 61, 67, 71, 73, 79, 83, 89,
    97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223,
    227, 229, 233, 239, 241, 251, 257, 263,
    269, 271, 277, 281, 283, 293, 307, 311,
    313, 317, 331, 337, 347, 349, 353, 359,
    367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457,
    461, 463, 467, 479, 487, 491, 499, 503,
    509, 521, 523, 541, 547, 557, 563, 569,
    571, 577, 587, 593, 599, 601, 607, 613,
    617, 619, 631, 641, 643, 647, 653, 659,
    661, 673, 677, 683, 691, 701, 709, 719,
    727, 733, 739, 743, 751, 757, 761, 769,
    773, 787, 797, 809, 811, 821, 823, 827,
    829, 839, 853, 857, 859, 863, 877, 881,
    883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997,
    1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
    1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097,
    1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
    1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
    1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
    1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459,
    1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
    1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
    1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619,
    1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
    1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
    1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
    1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
    1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
    1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003,
    2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069,
    2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
    2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203,
    2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267,
    2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311,
    2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
    2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
    2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503,
    2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579,
    2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657,
    2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
    2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
    2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801,
    2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861,
    2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939,
    2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011,
    3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
    3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167,
    3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221,
    3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301,
    3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347,
    3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
    3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491,
    3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541,
    3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
    3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671,
    3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727,
    3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797,
    3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863,
    3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923,
    3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003};

void BuildPrimeTable()
{
    unsigned int p=primeTable[primeTableSize-1];
    for (int i=primeTableSize; i<maxPrimeTableSize; i++)
    {
        int j;
        do
        {
            p+=2;
            for (j=1; j<54; j++)
                if (p%primeTable[j] == 0)
                    break;
        } while (j!=54);
        primeTable[i] = p;
    }
    primeTableSize = maxPrimeTableSize;
}

// *****************************************************************************
// the following functions do not depend on the details of bignum implementation
// and can be used with any bignum class

#define iplus1  ( i==2 ? 0 : i+1 )	// used by Euclid algorithms
#define iminus1 ( i==0 ? 2 : i-1 )	// used by Euclid algorithms

bignum Gcd(const bignum &a, const bignum &n)
{
    bignum g[3]={n, a};

    for (int i=1; !!g[i]; i = iplus1)
        g[iplus1] = g[iminus1] % g[i];

    return g[iminus1];
}

bignum Inverse(const bignum &a, const bignum &n)
{
    bignum g[3]={n, a};
    bignum v[3]={(unsigned long)0, 1};
    bignum y;

    for (int i=1; !!g[i]; i = iplus1)
    {
        // y = g[iminus1] / g[i];
        // g[iplus1] = g[iminus1] % g[i];
        Divide(g[iplus1], y, g[iminus1], g[i]);
        v[iplus1] = v[iminus1] - (v[i] * y);
    }

    if (Negative(v[iminus1]))
        v[iminus1] += n;

    return v[iminus1];
}

boolean IsSmallPrime(const bignum &p)
{
    BuildPrimeTable();

    if (p>primeTable[primeTableSize-1])
        return FALSE;

    for (int i = 0; i<primeTableSize; i++)
    	if (p == primeTable[i])
    	    return TRUE;

    return FALSE;
}

boolean SmallDivisorsTest(const bignum &p)
{
    for (int i = 0; i<primeTableSize; i++)
    	if ((p % primeTable[i]) == 0)
    	    return FALSE;

    return TRUE;
}

boolean FermatTest(const bignum &p, unsigned int rounds)
{
    assert(rounds < primeTableSize);

    for (unsigned int i = 0; i < rounds; i++)
    {
    	// if ((x**(p-1)) mod p) != 1, then p is not prime
    	if (a_exp_b_mod_c(primeTable[i], p-1, p) != 1)
    	    return FALSE;
    }
    return TRUE;
}

boolean RabinMillerTest(RandomNumberGenerator &rng, const bignum &w, unsigned int rounds)
{
    bignum wminus1 = (w-1);
    for (word16 a=0; a<wminus1.MaxBitPrecision(); a++)
        if (wminus1[a])
            break;
    bignum m = wminus1>>a;
    // ASSERT: a == largest power of 2 that divides (w-1)
    // && w == 1 + (2**a) * m

    for (unsigned int i=0; i<rounds; i++)
    {
        bignum b(rng, 2, wminus1);
        bignum z = a_exp_b_mod_c(b, m, w);
        if (z==1 || z==wminus1)
            continue;   // passes this round
        for (int j=1; j<a; j++)
        {
            z = a_exp_b_mod_c(z, 2, w);
            if (z==wminus1)
                break;  // passed this round
            if (z==1)
                return FALSE;
        }
        if (j==a)
            return FALSE;
    }
    return TRUE;
}

class RemainderTable
{
public:
    RemainderTable(const bignum &p);
    boolean HasZero() const;
    void Increment();
    void IncrementBy(unsigned int i);
    void IncrementBy(const RemainderTable &rtQ);

private:
    SecBlock<word16> table;
};

RemainderTable::RemainderTable(const bignum &p)
    : table((BuildPrimeTable(), primeTableSize))
{
    for (unsigned int i=0; i<primeTableSize; i++)
        table[i] = p%primeTable[i];
}

boolean RemainderTable::HasZero() const
{
    for (unsigned int i=0; i<primeTableSize; i++)
        if (!table[i])
            break;

    return (i!=primeTableSize);
}

void RemainderTable::Increment()
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i]++;
        if (table[i]==primeTable[i])
            table[i] = 0;
    }
}

void RemainderTable::IncrementBy(unsigned int increment)
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i] += increment;
        while (table[i]>=primeTable[i])
            table[i]-=primeTable[i];
    }
}

void RemainderTable::IncrementBy(const RemainderTable &rtQ)
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i] += rtQ.table[i];
        if (table[i]>=primeTable[i])
            table[i]-=primeTable[i];
    }
}

boolean NextPrime(bignum &p, const bignum &max, boolean blumInt)
{
    ++p;

    if (!p[0])
        ++p;

    if (blumInt && !p[1])
        {++p; ++p;}

    if (p>max)
        return FALSE;

    RemainderTable rt(p);

    while (rt.HasZero() || !IsPrime(p))
    {
        rt.IncrementBy(blumInt ? 4 : 2);
        ++p; ++p;
        if (blumInt)
            {++p; ++p;}

        if (p>max)
            return FALSE;
    }

    return TRUE;
}

bignum a_exp_b_mod_pq(const bignum &a, const bignum &ep, const bignum &eq,
                      const bignum &p, const bignum &q, const bignum &u)
{
    bignum p2 = a_exp_b_mod_c((a % p), ep, p);
    bignum q2 = a_exp_b_mod_c((a % q), eq, q) - p2;
    if (Negative(q2))
        q2 += q;
    return p2 + (p * ((u * q2) % q));
}

// generate random prime of pbits (with maximal subprime) and primitive g
// warning: this is slow!
PrimeAndGenerator::PrimeAndGenerator(RandomNumberGenerator &rng, unsigned int pbits)
{
    bignum minQ = (bignum(1) << (pbits-2));
    bignum maxQ = ((bignum(1) << (pbits-1)) - 1);

    do
    {
        q.Randomize(rng, minQ, maxQ, ODD);
        p = 2*q+1;

        RemainderTable rtQ(q);
        RemainderTable rtP(p);

        while (rtQ.HasZero() || rtP.HasZero() ||
               !FermatTest(q, 1) || !FermatTest(p, 1) ||
               !IsPrime(q) || !IsPrime(p))
        {
            rtQ.IncrementBy(2);
            rtP.IncrementBy(4);
            ++q; ++q;
            ++p; ++p; ++p; ++p;
        }
    } while (q>maxQ);

    g=2;
}

// generate random prime of pbits (with subprime of qbits) and g of order q
// this uses the same algorithm as RSAREF's DH parameter generation code
PrimeAndGenerator::PrimeAndGenerator(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits)
{
    assert(pbits > qbits);

    bignum minQ = (bignum(1) << (qbits-1));
    bignum maxQ = ((bignum(1) << qbits) - 1);
    bignum minP = (bignum(1) << (pbits-1));
    bignum maxP = ((bignum(1) << pbits) - 1);

    while (1)
    {
        q.Randomize(rng, minQ, maxQ, PRIME);
        bignum q2 = 2*q;
        RemainderTable rtq2(q2);

        // make p-1 a random multiple of 2*q
        p.Randomize(rng, minP, maxP, ANY);
        p -= p%q2;
        ++p;
        p += q2;
        RemainderTable rtp(p);

        while (p<=maxP)
        {
            if (rtp.HasZero() || !IsPrime(p))
            {
                p += q2;
                rtp.IncrementBy(rtq2);
            }
            else
            {   // let g = 2**((p-1)/q) mod p
                // g should have order q
                g = a_exp_b_mod_c(2, (p-1)/q, p);
                return;
            }
        }
    }
}

