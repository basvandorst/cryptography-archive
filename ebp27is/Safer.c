/*******************************************************************************
*
* FILE:           safer.c
*
* DESCRIPTION:    block-cipher algorithm SAFER (Secure And Fast Encryption
*                 Routine) in its four versions: SAFER K-64, SAFER K-128,
*                 SAFER SK-64 and SAFER SK-128.
*
* AUTHOR:         Richard De Moliner (demoliner@isi.ee.ethz.ch)
*                 Signal and Information Processing Laboratory
*                 Swiss Federal Institute of Technology
*                 CH-8092 Zuerich, Switzerland
*
* DATE:           September 9, 1995
*
* CHANGE HISTORY: January 1997, EBP - Even Better Privacy
*					by Loefgren/Jarl
*
*******************************************************************************/

/******************* External Headers *****************************************/
#include "usuals.h"
#include "ebp.h"
#include "memory.h"
#include "randpool.h"
#include "string.h"
#include "idea.h"

/******************* Local Headers ********************************************/
#include "safer.h"

/******************* Constants ************************************************/
#define TAB_LEN      256

/******************* Assertions ***********************************************/

/******************* Macros ***************************************************/
#define ROL(x, n)    ((unsigned char)((unsigned int)(x) << (n)\
                                     |(unsigned int)((x) & 0xFF) >> (8 - (n))))
#define EXP(x)       exp_tab[(x) & 0xFF]
#define LOG(x)       log_tab[(x) & 0xFF]
#define PHT(x, y)    { y += x; x += y; }
#define IPHT(x, y)   { x -= y; y -= x; }

/******************* Types ****************************************************/
static unsigned char exp_tab[TAB_LEN];
static unsigned char log_tab[TAB_LEN];

/******************* Module Data **********************************************/
#define NOF_CHARS          ('~' - ' ' + 1) /* number of printable characters  */
#define MAX_SHORT_KEY_STRING_LEN        9  /* maximal length for a 64-bit key */
static int userkey_2_set;       /* 'userkey_2' has been set                   */


/******************* Functions ************************************************/

/******************************************************************************/
#ifndef NOT_ANSI_C
    void Safer_Init_Module(void)
#else
    Safer_Init_Module()
#endif

{   unsigned int i, exp;

    exp = 1;
    for (i = 0; i < TAB_LEN; i++)
    {
        exp_tab[i] = (unsigned char)(exp & 0xFF);
        log_tab[exp_tab[i]] = (unsigned char)i;
        exp = exp * 45 % 257;
    }
} /* Safer_Init_Module */




/*******************************************************************************
* read key
*/
#ifndef NOT_ANSI_C
    void Divide_Key_String(byte const key[16], safer_block_t userkey_1, safer_block_t userkey_2)
#else
    Divide_Key_String(key[16], userkey_1, userkey_2)
#endif

{   unsigned int val;
    int i;

    val = key[0];
	i =0;
    while ( i < SAFER_BLOCK_LEN & val != '\0')
    {
	    userkey_1[i] = (unsigned char)(val & 0xFF);
        val = key[i + 1];
		i++;
	}
	i =0;
    while ( i < SAFER_BLOCK_LEN & val != '\0')
    {
	    userkey_2[i] = (unsigned char)(val & 0xFF);
        val = key[SAFER_BLOCK_LEN + i + 1];
		i++;
	}
} /* Divide_Key_String */




/******************************************************************************/
#ifndef NOT_ANSI_C
    void Safer_Expand_Userkey(safer_block_t userkey_1,
                              safer_block_t userkey_2,
                              unsigned int nof_rounds,
                              int strengthened,
                              safer_key_t key)
#else
    Safer_Expand_Userkey(userkey_1, userkey_2, nof_rounds, strengthened, key)
    safer_block_t userkey_1;
    safer_block_t userkey_2;
    unsigned int nof_rounds;
    int strengthened;
    safer_key_t key;
#endif

{   unsigned int i, j;
    unsigned char ka[SAFER_BLOCK_LEN + 1];
    unsigned char kb[SAFER_BLOCK_LEN + 1];

    if (SAFER_MAX_NOF_ROUNDS < nof_rounds)
        nof_rounds = SAFER_MAX_NOF_ROUNDS;
    *key++ = (unsigned char)nof_rounds;
    ka[SAFER_BLOCK_LEN] = 0;
    kb[SAFER_BLOCK_LEN] = 0;
    for (j = 0; j < SAFER_BLOCK_LEN; j++)
    {
        ka[SAFER_BLOCK_LEN] ^= ka[j] = ROL(userkey_1[j], 5);
        kb[SAFER_BLOCK_LEN] ^= kb[j] = *key++ = userkey_2[j];
    }
    for (i = 1; i <= nof_rounds; i++)
    {
        for (j = 0; j < SAFER_BLOCK_LEN + 1; j++)
        {
            ka[j] = ROL(ka[j], 6);
            kb[j] = ROL(kb[j], 6);
        }
        for (j = 0; j < SAFER_BLOCK_LEN; j++)
            if (strengthened)
                *key++ = (ka[(j + 2 * i - 1) % (SAFER_BLOCK_LEN + 1)]
                                + exp_tab[exp_tab[18 * i + j + 1]]) & 0xFF;
            else
                *key++ = (ka[j] + exp_tab[exp_tab[18 * i + j + 1]]) & 0xFF;
        for (j = 0; j < SAFER_BLOCK_LEN; j++)
            if (strengthened)
                *key++ = (kb[(j + 2 * i) % (SAFER_BLOCK_LEN + 1)]
                                + exp_tab[exp_tab[18 * i + j + 10]]) & 0xFF;
            else
                *key++ = (kb[j] + exp_tab[exp_tab[18 * i + j + 10]]) & 0xFF;
    }
    for (j = 0; j < SAFER_BLOCK_LEN + 1; j++)
        ka[j] = kb[j] = 0;
} /* Safer_Expand_Userkey */

/******************************************************************************/
#ifndef NOT_ANSI_C
    void Safer_Block(safer_block_t block_in, safer_key_t key, 
                             safer_block_t block_out)
#else
    Safer_Block(block_in, key, block_out)
    safer_block_t block_in;
    safer_key_t key;
    safer_block_t block_out;
#endif

{   unsigned char a, b, c, d, e, f, g, h, t;
    unsigned int round;

    a = block_in[0]; b = block_in[1]; c = block_in[2]; d = block_in[3];
    e = block_in[4]; f = block_in[5]; g = block_in[6]; h = block_in[7];
    if (SAFER_MAX_NOF_ROUNDS < (round = *key)) round = SAFER_MAX_NOF_ROUNDS;
    while(round--)
    {
        a ^= *++key; b += *++key; c += *++key; d ^= *++key;
        e ^= *++key; f += *++key; g += *++key; h ^= *++key;
        a = EXP(a) + *++key; b = LOG(b) ^ *++key;
        c = LOG(c) ^ *++key; d = EXP(d) + *++key;
        e = EXP(e) + *++key; f = LOG(f) ^ *++key;
        g = LOG(g) ^ *++key; h = EXP(h) + *++key;
        PHT(a, b); PHT(c, d); PHT(e, f); PHT(g, h);
        PHT(a, c); PHT(e, g); PHT(b, d); PHT(f, h);
        PHT(a, e); PHT(b, f); PHT(c, g); PHT(d, h);
        t = b; b = e; e = c; c = t; t = d; d = f; f = g; g = t;
    }
    a ^= *++key; b += *++key; c += *++key; d ^= *++key;
    e ^= *++key; f += *++key; g += *++key; h ^= *++key;
    block_out[0] = a & 0xFF; block_out[1] = b & 0xFF;
    block_out[2] = c & 0xFF; block_out[3] = d & 0xFF;
    block_out[4] = e & 0xFF; block_out[5] = f & 0xFF;
    block_out[6] = g & 0xFF; block_out[7] = h & 0xFF;
} /* Safer_Block */

/******************************************************************************/
#ifndef NOT_ANSI_C
    void Safer_Decrypt_Block(safer_block_t block_in, safer_key_t key, 
                             safer_block_t block_out)
#else
    Safer_Decrypt_Block(block_in, key, block_out)
    safer_block_t block_in;
    safer_key_t key;
    safer_block_t block_out;
#endif

{   unsigned char a, b, c, d, e, f, g, h, t;
    unsigned int round;

    a = block_in[0]; b = block_in[1]; c = block_in[2]; d = block_in[3];
    e = block_in[4]; f = block_in[5]; g = block_in[6]; h = block_in[7];
    if (SAFER_MAX_NOF_ROUNDS < (round = *key)) round = SAFER_MAX_NOF_ROUNDS;
    key += SAFER_BLOCK_LEN * (1 + 2 * round);
    h ^= *key; g -= *--key; f -= *--key; e ^= *--key;
    d ^= *--key; c -= *--key; b -= *--key; a ^= *--key;
    while (round--)
    {
        t = e; e = b; b = c; c = t; t = f; f = d; d = g; g = t;
        IPHT(a, e); IPHT(b, f); IPHT(c, g); IPHT(d, h);
        IPHT(a, c); IPHT(e, g); IPHT(b, d); IPHT(f, h);
        IPHT(a, b); IPHT(c, d); IPHT(e, f); IPHT(g, h);
        h -= *--key; g ^= *--key; f ^= *--key; e -= *--key;
        d -= *--key; c ^= *--key; b ^= *--key; a -= *--key;
        h = LOG(h) ^ *--key; g = EXP(g) - *--key;
        f = EXP(f) - *--key; e = LOG(e) ^ *--key;
        d = LOG(d) ^ *--key; c = EXP(c) - *--key;
        b = EXP(b) - *--key; a = LOG(a) ^ *--key;
    }
    block_out[0] = a & 0xFF; block_out[1] = b & 0xFF;
    block_out[2] = c & 0xFF; block_out[3] = d & 0xFF;
    block_out[4] = e & 0xFF; block_out[5] = f & 0xFF;
    block_out[6] = g & 0xFF; block_out[7] = h & 0xFF;
} /* Safer_Decrypt_Block */

/******************************************************************************/
void saferCfbReinit(struct SaferCfbContext *context, byte const *iv)
{
    if (iv)
	memcpy(context->iv, iv, 8);
    else
	fill0(context->iv, 8);
    context->bufleft = 0;
}

void saferCfbInit(struct SaferCfbContext *context, byte const key[16])
{

	static safer_block_t userkey_1; /* first 64 bits of selected key */
	static safer_block_t userkey_2; /* second 64 bits of selected key */

if (algostruct.Blockcipher == NEWALG)
{
	Safer_Init_Module();
	Divide_Key_String(key, userkey_1, userkey_2);
	Safer_Expand_Userkey(userkey_1, userkey_2, SAFERROUNDS, 1, context->Skey);
}
else
    ideaExpandKey(key, context->key);

    saferCfbReinit(context, 0);
}

void saferCfbDestroy(struct SaferCfbContext *context)
{
    burn(*context);
}

/*
 * Okay, explanation time:
 * Phil invented a unique way of doing CFB that's sensitive to semantic
 * boundaries within the data being encrypted.  One way to phrase
 * CFB en/decryption is to say that you XOR the current 8 bytes with
 * SAFER/IDEA(previous 8 bytes of ciphertext).  Normally, you repeat this
 * at 8-byte intervals, but Phil decided to resync things on the
 * boundaries between elements in the stream being encrypted.
 *
 * That is, the last 4 bytes of a 12-byte field are en/decrypted using
 * the first 4 bytes of SAFER/IDEA(previous 8 bytes of ciphertext), but then
 * the last 4 bytes of that SAFER/IDEA computation are thrown away, and the
 * first 8 bytes of the next field are en/decrypted using
 * SAFER/IDEA(last 8 bytes of ciphertext).  This is equivalent to using a
 * shorter feedback length (if you're familiar with the general CFB
 * technique) briefly, and doesn't weaken the cipher any (using shorter
 * CFB lengths makes it stronger, actually), it just makes it a bit unusual.
 *
 * Anyway, to accomodate this behaviour, every time we do an SAFER/IDEA
 * encrpytion of 8 bytes of ciphertext to get 8 bytes of XOR mask,
 * we remember the ciphertext.  Then if we have to resync things
 * after having processed, say, 2 bytes, we refill the iv buffer
 * with the last 6 bytes of the old ciphertext followed by the
 * 2 bytes of new ciphertext stored in the front of the iv buffer.
 */
void saferCfbSync(struct SaferCfbContext *context)
{
    int bufleft = context->bufleft;

    if (bufleft) {
	memmove(context->iv + bufleft, context->iv, 8 - bufleft);
	memcpy(context->iv, context->oldcipher + 8 - bufleft, bufleft);
	context->bufleft = 0;
    }
}

/*
 * Encrypt a buffer of data, using SAFER in CFB mode.
 * There are more compact ways of writing this, but this is
 * written for speed.
 */
void saferCfbEncrypt(struct SaferCfbContext *context, byte const *src,
		    byte * dest, int count)
{
    int bufleft = context->bufleft;
    byte *bufptr = context->iv + 8 - bufleft;

    /* If there are no more bytes to encrypt that there are bytes
     * in the buffer, XOR them in and return.
     */
    if (count <= bufleft) {
	context->bufleft = bufleft - count;
	while (count--) {
	    *dest++ = *bufptr++ ^= *src++;
	}
	return;
    }
    count -= bufleft;
    /* Encrypt the first bufleft (0 to 7) bytes of the input by XOR
     * with the last bufleft bytes in the iv buffer.
     */
    while (bufleft--) {
	*dest++ = (*bufptr++ ^= *src++);
    }
    /* Encrypt middle blocks of the input by cranking the cipher,
     * XORing 8-byte blocks, and repeating until the count
     * is 8 or less.
     */
    while (count > 8) {
	bufptr = context->iv;
	memcpy(context->oldcipher, bufptr, 8);
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(bufptr, context->Skey, bufptr);
else
	ideaCipher(bufptr, bufptr, context->key);
		
	bufleft = 8;
	count -= 8;
	do {
	    *dest++ = (*bufptr++ ^= *src++);
	} while (--bufleft);
    }
    /* Do the last 1 to 8 bytes */
    bufptr = context->iv;
    memcpy(context->oldcipher, bufptr, 8);
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(bufptr, context->Skey, bufptr);
else
	ideaCipher(bufptr, bufptr, context->key);

   context->bufleft = 8 - count;
    do {
	*dest++ = (*bufptr++ ^= *src++);
    } while (--count);
}


/*
 * Decrypt a buffer of data, using SAFER in CFB mode.
 * There are more compact ways of writing this, but this is
 * written for speed.
 */
void saferCfbDecrypt(struct SaferCfbContext *context, byte const *src,
		    byte * dest, int count)
{
    int bufleft = context->bufleft;
    static byte *bufptr;
    byte t;

    bufptr = context->iv + (8 - bufleft);
    if (count <= bufleft) {
	context->bufleft = bufleft - count;
	while (count--) {
	    t = *bufptr;
	    *dest++ = t ^ (*bufptr++ = *src++);
	}
	return;
    }
    count -= bufleft;
    while (bufleft--) {
	t = *bufptr;
	*dest++ = t ^ (*bufptr++ = *src++);
    }
    while (count > 8) {
	bufptr = context->iv;
	memcpy(context->oldcipher, bufptr, 8);
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(bufptr, context->Skey, bufptr);
else
	ideaCipher(bufptr, bufptr, context->key);

	bufleft = 8;
	count -= 8;
	do {
	    t = *bufptr;
	    *dest++ = t ^ (*bufptr++ = *src++);
	} while (--bufleft);
    }
    bufptr = context->iv;
    memcpy(context->oldcipher, bufptr, 8);
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(bufptr, context->Skey, bufptr);
else
	ideaCipher(bufptr, bufptr, context->key);

    context->bufleft = 8 - count;
    do {
	t = *bufptr;
	*dest++ = t ^ (*bufptr++ = *src++);
    } while (--count);
}

/********************************************************************/

/*
 * Cryptographically strong pseudo-random-number generator.
 * The design is from Appendix C of ANSI X9.17, "Financial
 * Institution Key Management (Wholesale)", with SAFER
 * substituted for the DES.
 */

/*
 * Initialize a cryptographic random-number generator.
 * key and seed should be arbitrary.
 */
void saferRandInit(struct SaferRandContext *context, byte const key[16],
		  byte const seed[8])
{
	static safer_block_t userkey_1; /* first 64 bits of selected key */
	static safer_block_t userkey_2; /* second 64 bits of selected key */

if (algostruct.Blockcipher == NEWALG)
{
	Safer_Init_Module();
	Divide_Key_String(key, userkey_1, userkey_2);
	Safer_Expand_Userkey(userkey_1, userkey_2, SAFERROUNDS, 1, context->Skey);
}
else
    ideaExpandKey(key, context->key);

    context->bufleft = 0;
    memcpy(context->internalbuf, seed, 8);
}


/*
 * Read out the RNG's state.
 */
void saferRandState(struct SaferRandContext *context, byte key[16], byte seed[8])
{
    int i;

    memcpy(seed, context->internalbuf, 8);
    for (i = 0; i < 8; i++) {
	key[2 * i] = context->Skey[i] >> 8;
	key[2 * i + 1] = context->Skey[i];
    }

}

/*
 * Encrypt the RNG's state with the given CFB encryptor.
 */
void saferRandWash(struct SaferRandContext *context, struct SaferCfbContext *cfb)
{
    byte keyseed[16 + 8];

    saferRandState(context, keyseed, keyseed + 16);
    saferCfbEncrypt(cfb, keyseed, keyseed, 16 + 8);
    saferRandInit(context, keyseed, keyseed + 16);

    memset(keyseed, 0, 16 + 8);
}

/*
 * Cryptographic pseudo-random-number generator, used for generating
 * session keys.
 */
byte
saferRandByte(struct SaferRandContext *c)
{
    int i;

    if (!c->bufleft) {
	byte timestamp[8];

	/* Get some true-random noise to help */
	randPoolGetBytes(timestamp, sizeof(timestamp));

	/* Compute next 8 bytes of output */
	for (i = 0; i < 8; i++)
	    c->outbuf[i] = c->internalbuf[i] ^ timestamp[i];
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(c->outbuf, c->Skey, c->outbuf);
else
	ideaCipher(c->outbuf, c->outbuf, c->key);

	/* Compute new seed vector */
	for (i = 0; i < 8; i++)
	    c->internalbuf[i] = c->outbuf[i] ^ timestamp[i];
if (algostruct.Blockcipher == NEWALG)
	Safer_Block(c->internalbuf, c->Skey, c->internalbuf);
else
	ideaCipher(c->internalbuf, c->internalbuf, c->key);

	burn(timestamp);
	c->bufleft = 8;
    }
    return c->outbuf[--c->bufleft];
}

