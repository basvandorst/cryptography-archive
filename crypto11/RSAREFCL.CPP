#include "cryptlib.h"

#ifdef USE_RSAREF

#include "misc.h"
#include "queue.h"
#include "rsarefcl.h"
#include "asn.h"
#include <assert.h>
#include <memory.h>

static void FillRandomStruct(RandomNumberGenerator &rng, R_RANDOM_STRUCT &randomStruct)
{
    R_RandomInit(&randomStruct);

    unsigned int blocksize;
    R_GetRandomBytesNeeded(&blocksize, &randomStruct);

    SecByteBlock temp(blocksize);
    rng.GetBlock(temp, blocksize);

    R_RandomUpdate(&randomStruct, temp, blocksize);
}

static unsigned int DEREncodeByteArray(BufferedTransformation &bt, const byte *in, unsigned int length)
{
    bt.Put(INTEGER);
    // skip leading zeros
    for (unsigned int i=0; i<length && !in[i]; i++);
    // add a leading zero if highest bit is set or the number is zero
    boolean addZero = (i==length) || (in[i] & 0x80);
    length = length - i + addZero;
    DERLengthEncode(length, bt);
    if (addZero)
        bt.Put(0);
    bt.Put(in+i, length-addZero);
    return length;
}

static unsigned int BERDecodeByteArray(byte *out, BufferedTransformation &bt, unsigned int length)
{
    byte b;
    if (!bt.Get(b) || b != INTEGER)
        BERDecodeError();

    unsigned int bc;
    BERLengthDecode(bt, bc);

    // skip leading zeros
    while (bc > 0 && (bt.Get(b), b==0))
    	bc--;

    if (bc > length)
        BERDecodeError();

    memset(out, 0, length-bc);
    if (bc)
    {
    	out[length-bc] = b;
    	bt.Get(out+length-bc+1, bc-1);
    }

    return bc;
}

/*
#ifdef USE_BIGNUM_WITH_RSAREF
RSAREFPublicKey::RSAREFPublicKey(const bignum &n, const bignum &e)
    : keyBlock(1), key(keyBlock)
{
    key->bits = n.BitCount();
    n.Encode(key->modulus, MAX_RSA_MODULUS_LEN);
    e.Encode(key->exponent, MAX_RSA_MODULUS_LEN);
}
#endif

RSAREFPublicKey::RSAREFPublicKey(const RSAREFPrivateKey &privateKey)
    : keyBlock(1), key(keyBlock)
{
    key->bits = privateKey.key->bits;
    memcpy(key->modulus, privateKey.key->modulus, MAX_RSA_MODULUS_LEN);
    memcpy(key->exponent, privateKey.key->publicExponent, MAX_RSA_MODULUS_LEN);
}

RSAREFPublicKey::RSAREFPublicKey(BufferedTransformation &bt)
    : keyBlock(1), key(keyBlock)
{
    BERSequenceDecoder seq(bt);
    unsigned int modulusLen = BERDecodeByteArray(key->modulus, seq, MAX_RSA_MODULUS_LEN);
    key->bits = BitPrecision(key->modulus[MAX_RSA_MODULUS_LEN-modulusLen]) + 8*modulusLen - 8;
    BERDecodeByteArray(key->exponent, seq, MAX_RSA_MODULUS_LEN);
}

void RSAREFPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    DEREncodeByteArray(seq, key->modulus, MAX_RSA_MODULUS_LEN);
    DEREncodeByteArray(seq, key->exponent, MAX_RSA_MODULUS_LEN);
}

void RSAREFPublicKey::Encrypt(RandomNumberGenerator &rng, const byte *input, unsigned int inputLen, byte *output)
{
    assert (inputLen <= MaxPlainTextLength());

    R_RANDOM_STRUCT randomStruct;
    FillRandomStruct(rng, randomStruct);

    if (input != output)
        memcpy(output, input, inputLen);    // copy it first for the "const"

    unsigned int outputLen;
    int status = RSAPublicEncrypt(output, &outputLen, output, inputLen, key, &randomStruct);

    assert(status==0);
    assert(outputLen==CipherTextLength());

    R_RandomFinal(&randomStruct);	// Clean up RSAREF's RNG
}

unsigned int RSAREFPublicKey::Decrypt(const byte *input, byte *output)
{
    if (input != output)
        memcpy(output, input, CipherTextLength());    // copy it first for the "const"

    unsigned int outputLen;
    int status = RSAPublicDecrypt(output, &outputLen, output, CipherTextLength(), key);

    if (status)
        return 0;

    assert(outputLen<=MaxPlainTextLength());
    return outputLen;
}

boolean RSAREFPublicKey::Verify(const byte *message, unsigned int messageLen, const byte *signature)
{
    assert(messageLen <= MaxMessageLength());
    SecByteBlock m(MaxMessageLength());
    unsigned int mLen = RSAREFPublicKey::Decrypt(signature, m);
    return (mLen==messageLen && memcmp(message, m, mLen)==0);
}

// *****************************************************************************
// private key operations:

#ifdef USE_BIGNUM_WITH_RSAREF
RSAREFPrivateKey::RSAREFPrivateKey(const bignum &n, const bignum &e, const bignum &d,
                             const bignum &p, const bignum &q,
                             const bignum &dp, const bignum &dq, const bignum &u)
    : keyBlock(1), key(keyBlock)
{
    key->bits = n.BitCount();
    n.Encode(key->modulus, MAX_RSA_MODULUS_LEN);
    e.Encode(key->publicExponent, MAX_RSA_MODULUS_LEN);
    d.Encode(key->exponent, MAX_RSA_MODULUS_LEN);
    if (p>q)
    {
        p.Encode(key->prime[0], MAX_RSA_PRIME_LEN);
        q.Encode(key->prime[1], MAX_RSA_PRIME_LEN);
        dp.Encode(key->primeExponent[0], MAX_RSA_PRIME_LEN);
        dq.Encode(key->primeExponent[1], MAX_RSA_PRIME_LEN);
    }
    else
    {
        q.Encode(key->prime[0], MAX_RSA_PRIME_LEN);
        p.Encode(key->prime[1], MAX_RSA_PRIME_LEN);
        dq.Encode(key->primeExponent[0], MAX_RSA_PRIME_LEN);
        dp.Encode(key->primeExponent[1], MAX_RSA_PRIME_LEN);
    }
    u.Encode(key->coefficient, MAX_RSA_PRIME_LEN);
}
#endif

// generate a random private key
RSAREFPrivateKey::RSAREFPrivateKey(RandomNumberGenerator &rng, int keybits, boolean F4)
    : keyBlock(1), key(keyBlock)
{
    assert(keybits >= MIN_RSA_MODULUS_BITS && keybits <= MAX_RSA_MODULUS_BITS);

    R_RSA_PROTO_KEY protoKey;
    protoKey.bits = keybits;
    protoKey.useFermat4 = F4;

    R_RANDOM_STRUCT randomStruct;
    FillRandomStruct(rng, randomStruct);

    R_RSA_PUBLIC_KEY temp;
    int status;
    do
        status = R_GeneratePEMKeys(&temp, key, &protoKey, &randomStruct);
    while (status == RE_DATA);   // retry if prime generation failed;
    assert(status==0);

    R_RandomFinal(&randomStruct);	// Clean up RSAREF's RNG
}

RSAREFPrivateKey::RSAREFPrivateKey(BufferedTransformation &bt)
    : keyBlock(1), key(keyBlock)
{
    BERSequenceDecoder seq(bt);

    // extract and discard the version # first
    BERDecodeByteArray(key->modulus, seq, MAX_RSA_MODULUS_LEN);

    unsigned int modulusLen = BERDecodeByteArray(key->modulus, seq, MAX_RSA_MODULUS_LEN);
    key->bits = BitPrecision(key->modulus[MAX_RSA_MODULUS_LEN-modulusLen]) + 8*modulusLen - 8;

    BERDecodeByteArray(key->publicExponent, seq, MAX_RSA_MODULUS_LEN);
    BERDecodeByteArray(key->exponent, seq, MAX_RSA_MODULUS_LEN);
    BERDecodeByteArray(key->prime[0], seq, MAX_RSA_PRIME_LEN);
    BERDecodeByteArray(key->prime[1], seq, MAX_RSA_PRIME_LEN);
    BERDecodeByteArray(key->primeExponent[0], seq, MAX_RSA_PRIME_LEN);
    BERDecodeByteArray(key->primeExponent[1], seq, MAX_RSA_PRIME_LEN);
    BERDecodeByteArray(key->coefficient, seq, MAX_RSA_PRIME_LEN);
}

void RSAREFPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);

    byte version[] = {INTEGER, 1, 0};
    seq.Put(version, sizeof(version));

    DEREncodeByteArray(seq, key->modulus, MAX_RSA_MODULUS_LEN);
    DEREncodeByteArray(seq, key->publicExponent, MAX_RSA_MODULUS_LEN);
    DEREncodeByteArray(seq, key->exponent, MAX_RSA_MODULUS_LEN);
    DEREncodeByteArray(seq, key->prime[0], MAX_RSA_PRIME_LEN);
    DEREncodeByteArray(seq, key->prime[1], MAX_RSA_PRIME_LEN);
    DEREncodeByteArray(seq, key->primeExponent[0], MAX_RSA_PRIME_LEN);
    DEREncodeByteArray(seq, key->primeExponent[1], MAX_RSA_PRIME_LEN);
    DEREncodeByteArray(seq, key->coefficient, MAX_RSA_PRIME_LEN);
}

void RSAREFPrivateKey::Encrypt(const byte *input, unsigned int inputLen, byte *output)
{
    assert (inputLen <= MaxPlainTextLength());

    if (input != output)
        memcpy(output, input, CipherTextLength());    // copy it first for the "const"

    unsigned int outputLen;
    int status = RSAPrivateEncrypt(output, &outputLen, output, inputLen, key);

    assert(status==0);
    assert(outputLen==CipherTextLength());
}

unsigned int RSAREFPrivateKey::Decrypt(const byte *input, byte *output)
{
    if (input != output)
        memcpy(output, input, CipherTextLength());    // copy it first for the "const"

    unsigned int outputLen;
    int status = RSAPrivateDecrypt(output, &outputLen, output, CipherTextLength(), key);

    if (status)
        return 0;

    assert(outputLen<=MaxPlainTextLength());
    return outputLen;
}
*/

// *****************************************************************************
// Diffie-Hellman:

#ifdef USE_BIGNUM_WITH_RSAREF
RSAREF_DH::RSAREF_DH(const bignum &p, const bignum &g)
    : privateValue(p.ByteCount()-1)
{
    params.primeLen = p.ByteCount();
    params.generatorLen = g.ByteCount();

    params.prime = new byte[params.primeLen + params.generatorLen];
    p.Encode(params.prime, params.primeLen);

    params.generator = params.prime + params.primeLen;
    g.Encode(params.generator, params.generatorLen);
}
#endif

RSAREF_DH::RSAREF_DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits)
    : privateValue(DH_PRIME_LEN(pbits)-1)
{
    assert(pbits > qbits);

    params.prime = new byte[2*DH_PRIME_LEN(pbits)];
    params.generator = params.prime + DH_PRIME_LEN(pbits);

    R_RANDOM_STRUCT randomStruct;
    FillRandomStruct(rng, randomStruct);

    int status;
    do  // keep trying until we succeed
    {
        status = R_GenerateDHParams(&params, pbits, qbits, &randomStruct);
        assert (status == 0 || status == RE_DATA);
    }
    while (status);

    R_RandomFinal(&randomStruct);	// Clean up RSAREF's RNG
}

RSAREF_DH::~RSAREF_DH()
{
    delete [] params.prime;
}

RSAREF_DH::RSAREF_DH(BufferedTransformation &bt)
    : privateValue(BERDecode(bt)-1)
{
}

unsigned int RSAREF_DH::BERDecode(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);

    // RSAREF can't handle more than this many significant bytes
    const int maxBytes=MAX_RSA_MODULUS_LEN;
    byte tempPrime[maxBytes];
    byte tempGenerator[maxBytes];

    params.primeLen = BERDecodeByteArray(tempPrime, seq, maxBytes);
    params.generatorLen = BERDecodeByteArray(tempGenerator, seq, maxBytes);

    params.prime = new byte[params.primeLen + params.generatorLen];
    params.generator = params.prime + params.primeLen;

    memcpy(params.prime, tempPrime+maxBytes-params.primeLen, params.primeLen);
    memcpy(params.generator, tempGenerator+maxBytes-params.generatorLen, params.generatorLen);

    return params.primeLen;
}

void RSAREF_DH::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    DEREncodeByteArray(seq, params.prime, params.primeLen);
    DEREncodeByteArray(seq, params.generator, params.generatorLen);
}

void RSAREF_DH::Setup(RandomNumberGenerator &rng, byte *publicValue)
{
    R_RANDOM_STRUCT randomStruct;
    FillRandomStruct(rng, randomStruct);

    int status = R_SetupDHAgreement(publicValue, privateValue, privateValue.size, &params, &randomStruct);
    R_RandomFinal(&randomStruct);	// Clean up RSAREF's RNG
    assert (status == 0);
}

void RSAREF_DH::Agree(const byte *in, byte *agreedKey)
{
    R_ComputeDHAgreedKey(agreedKey, (byte *)in, privateValue, privateValue.size, &params);
}

#endif // USE_RSAREF
