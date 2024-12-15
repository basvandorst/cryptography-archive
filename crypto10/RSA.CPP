#include "misc.h"
#include "queue.h"
#include "asn.h"
#include "rsa.h"
#include "nbtheory.h"
#include <assert.h>

RSAPublicKey::RSAPublicKey(const bignum &n, const bignum &e)
    : n(n), e(e), modulusLen(n.ByteCount())
{
}

RSAPublicKey::RSAPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    n.BERDecode(seq);
    modulusLen = n.ByteCount();
    e.BERDecode(seq);
}

void RSAPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    n.DEREncode(seq);
    e.DEREncode(seq);
}

void RSAPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *input, unsigned int inputLen, byte *output)
{
    assert (inputLen <= MaxPlainTextLength());

    SecByteBlock pkcsBlock(modulusLen);

    pkcsBlock[0] = 0;
    pkcsBlock[1] = 2;  // block type 2

    for (int i = 2; i < modulusLen - inputLen - 1; i++)
    {
        // Find nonzero random byte.
        do
            pkcsBlock[i] = rng.GetByte();
        while (pkcsBlock[i] == 0);
    }

    pkcsBlock[i++] = 0;     // separator
    memcpy(pkcsBlock+i, input, inputLen);

    bignum c;
    RawEncrypt(bignum(pkcsBlock, modulusLen), c);
    c.Encode(output, modulusLen);
}

unsigned int RSAPublicKey::Decrypt(const byte *input, byte *output)
{
    bignum m;
    RawEncrypt(bignum(input, modulusLen), m);

    SecByteBlock pkcsBlock(modulusLen);
    m.Encode(pkcsBlock, modulusLen);

    // Require block type 1.
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))
        return 0;

    for (int i = 2; i < modulusLen-1; i++)
        if (pkcsBlock[i] != 0xff)
            break;

    // check separator
    if (pkcsBlock[i++] != 0)
        return 0;

    unsigned int outputLen = modulusLen - i;
    if (outputLen > MaxPlainTextLength())
        return 0;

    memcpy (output, pkcsBlock+i, outputLen);
    return outputLen;
}

boolean RSAPublicKey::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
    assert(messageLen <= MaxMessageLength());
    SecByteBlock m(MaxMessageLength());
    unsigned int mLen = RSAPublicKey::Decrypt(signature, m);
    return (mLen==messageLen && memcmp(message, m, mLen)==0);
}

void RSAPublicKey::RawEncrypt(const bignum &in, bignum &out) const
{
    out = a_exp_b_mod_c(in, e, n);
}

// *****************************************************************************
// private key operations:

RSAPrivateKey::RSAPrivateKey(const bignum &nIn, const bignum &eIn, const bignum &dIn,
                             const bignum &pIn, const bignum &qIn,
                             const bignum &dpIn, const bignum &dqIn, const bignum &uIn)
    : RSAPublicKey(nIn, eIn)
{
    d=dIn;
    if (pIn < qIn)
    {
        p=pIn;
        q=qIn;
        dp=dpIn;
        dq=dqIn;
    }
    else
    {
        p=qIn;
        q=pIn;
        dp=dqIn;
        dq=dpIn;
    }
    u=uIn;
}

// generate a random private key
RSAPrivateKey::RSAPrivateKey(RandomNumberGenerator &rng, int keybits, bignum eStart)
{
    p = bignum(rng, keybits/2, PRIME);
    const bignum minQ = ((bignum(1) << (keybits-1)) / p) + 1;
    const bignum maxQ = (bignum(1) << keybits) / p;

    do
    {
        bignum temp(rng, minQ, maxQ, PRIME);
        if (p>temp && p.BitCount()-(p-temp).BitCount() < 7)
        {
            q=p;
            p=temp;
            break;
        }
        if (p<temp && temp.BitCount()-(temp-p).BitCount() < 7)
        {
            q=temp;
            break;
        }
    } while (1);

    // ASSERT: q-p >= q/128

    bignum phi = (p-1)*(q-1);
    for (e = eStart; Gcd(e, phi)!=1; ++e, ++e);
    d = Inverse(e, phi/Gcd(p-1, q-1));
    dp = d % (p-1);
    dq = d % (q-1);
    u = Inverse(p, q);
    n = p * q;
    modulusLen = n.ByteCount();
}

RSAPrivateKey::RSAPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);

    bignum version(seq);
    if (!!version)  // make sure version is 0
        BERDecodeError();

    n.BERDecode(seq);
    modulusLen = n.ByteCount();
    e.BERDecode(seq);
    d.BERDecode(seq);
    p.BERDecode(seq);
    q.BERDecode(seq);
    dp.BERDecode(seq);
    dq.BERDecode(seq);
    u.BERDecode(seq);

    if (p>q)    // make sure q is the bigger prime
    {
        swap(p, q);
        swap(dp, dq);
    }
}

void RSAPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);

    byte version[] = {INTEGER, 1, 0};
    seq.Put(version, sizeof(version));
    n.DEREncode(seq);
    e.DEREncode(seq);
    d.DEREncode(seq);
    q.DEREncode(seq);    // encode the bigger prime first
    p.DEREncode(seq);
    dq.DEREncode(seq);
    dp.DEREncode(seq);
    u.DEREncode(seq);
}

void RSAPrivateKey::Encrypt(const byte *input, unsigned int inputLen, byte *output)
{
    assert (inputLen <= MaxPlainTextLength());

    SecByteBlock pkcsBlock(modulusLen);

    pkcsBlock[0] = 0;
    pkcsBlock[1] = 1;  // block type 1

    for (int i = 2; i < modulusLen - inputLen - 1; i++)
        pkcsBlock[i] = 0xff;

    pkcsBlock[i++] = 0; // separator
    memcpy(pkcsBlock+i, input, inputLen);

    bignum c;
    RawEncrypt(bignum(pkcsBlock, modulusLen), c);
    c.Encode(output, modulusLen);
}

unsigned int RSAPrivateKey::Decrypt(const byte *input, byte *output)
{
    bignum m;
    RawEncrypt(bignum(input, modulusLen), m);

    SecByteBlock pkcsBlock(modulusLen);
    m.Encode(pkcsBlock, modulusLen);

    // Require block type 2.
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 2))
        return 0;

    for (int i = 2; i < modulusLen-1; i++)
        if (pkcsBlock[i] == 0)  // separator
            break;

    if (++i >= modulusLen)
        return 0;

    unsigned int outputLen = modulusLen - i;

    if (outputLen > MaxPlainTextLength())
        return 0;

    memcpy (output, pkcsBlock+i, outputLen);
    return outputLen;
}

void RSAPrivateKey::RawEncrypt(const bignum &in, bignum &out) const
{
    out = a_exp_b_mod_pq(in, dp, dq, p, q, u);
}

