/*******************************************************************************
*
* FILE:           safer.h
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
* CHANGE HISTORY:
*
*******************************************************************************/
#ifndef SAFER_H
#define SAFER_H

/******************* External Headers *****************************************/

/******************* Local Headers ********************************************/
/******************* Constants ************************************************/


#define SAFER_K64_DEFAULT_NOF_ROUNDS     6
#define SAFER_K128_DEFAULT_NOF_ROUNDS   10
#define SAFER_SK64_DEFAULT_NOF_ROUNDS    8
#define SAFER_SK128_DEFAULT_NOF_ROUNDS  10
#define SAFER_MAX_NOF_ROUNDS            13
#define SAFER_BLOCK_LEN                  8
#define SAFER_KEY_LEN     (1 + SAFER_BLOCK_LEN * (1 + 2 * SAFER_MAX_NOF_ROUNDS))
#define SAFERKEYSIZE                    16
#define SAFERROUNDS						13
#define IDEAROUNDS						 8
#define IDEAKEYLEN (6*IDEAROUNDS+4)
/******************* Assertions ***********************************************/

/******************* Macros ***************************************************/
 
/******************* Types ****************************************************/
typedef unsigned char safer_block_t[SAFER_BLOCK_LEN];
typedef unsigned char safer_key_t[SAFER_KEY_LEN];

struct SaferCfbContext {
	byte oldcipher[8];
	byte iv[8];
	safer_key_t Skey;
	word16 key[IDEAKEYLEN];

	int bufleft;
};

struct SaferRandContext {
	byte outbuf[8];
	safer_key_t Skey;
	word16 key[IDEAKEYLEN];

	int bufleft;
	byte internalbuf[8];
};
/******************* Module Data **********************************************/

/******************* Prototypes ***********************************************/

/*******************************************************************************
* void Safer_Init_Module(void)
*
*   initializes this module.
*
********************************************************************************
* void Safer_Expand_Userkey(safer_block_t userkey_1,
*                           safer_block_t userkey_2,
*                           unsigned int nof_rounds,
*                           int strengthened,
*                           safer_key_t key)
*
*   expands a user-selected key of length 64 bits or 128 bits to a encryption /
*   decryption key. If your user-selected key is of length 64 bits, then give
*   this key to both arguments 'userkey_1' and 'userkey_2', e.g.
*   'Safer_Expand_Userkey(z, z, key)'. Note: SAFER K-64 and SAFER SK-64 with a
*   user-selected key 'z' of length 64 bits are identical to SAFER K-128 and
*   SAFER SK-128 with a user-selected key 'z z' of length 128 bits,
*   respectively.
*   pre:  'userkey_1'  contains the first 64 bits of user key.
*         'userkey_2'  contains the second 64 bits of user key.
*         'nof_rounds' contains the number of encryption rounds
*                      'nof_rounds' <= 'SAFER_MAX_NOF_ROUNDS'
*         'strengthened' is non-zero if the strengthened key schedule should be
*                      used and zero if the original key schedule should be
*                      used.
*   post: 'key'        contains the expanded key.
*
********************************************************************************
* void Safer_Block(safer_block_t block_in, safer_key_t key, 
*                          safer_block_t block_out)
*
*   encryption algorithm.
*   pre:  'block_in'  contains the plain-text block.
*         'key'       contains the expanded key.
*   post: 'block_out' contains the cipher-text block.
*
********************************************************************************
* void Safer_Decrypt_Block(safer_block_t block_in, safer_key_t key,
*                          safer_block_t block_out)
*
*   decryption algorithm.
*   pre:  'block_in'  contains the cipher-text block.
*         'key'       contains the expanded key.
*   post: 'block_out' contains the plain-text block.
*
*******************************************************************************/

#ifndef NOT_ANSI_C
    extern void Safer_Init_Module(void);
    extern void Divide_Key_String(byte const key[16], safer_block_t userkey_1, safer_block_t userkey_2);
	extern void Safer_Expand_Userkey(safer_block_t userkey_1,
                                     safer_block_t userkey_2,
                                     unsigned int nof_rounds,
                                     int strengthened,
                                     safer_key_t key);
    extern void Safer_Block (safer_block_t block_in, safer_key_t key, 
                                     safer_block_t block_out);
    extern void Safer_Decrypt_Block (safer_block_t block_in, safer_key_t key, safer_block_t block_out);

	void saferCfbReinit(struct SaferCfbContext *context, byte const *iv);
	void saferCfbInit(struct SaferCfbContext *context, byte const (key[16]));
	void saferCfbSync(struct SaferCfbContext *context);
	void saferCfbDestroy(struct SaferCfbContext *context);
	void saferCfbEncrypt(struct SaferCfbContext *context,
		    byte const *src, byte *dest, int count);
	void saferCfbDecrypt(struct SaferCfbContext *context,
		    byte const *src, byte *dest, int count);
	void saferRandInit(struct SaferRandContext *context, byte const (key[16]),
		  byte const (seed[8]));
	byte saferRandByte(struct SaferRandContext *c);
	void saferRandWash(struct SaferRandContext *c, struct SaferCfbContext *cfb);
	void saferRandState(struct SaferRandContext *c, byte key[16], byte seed[8]);

#else
    Safer_Init_Module();
    Divide_Key_String();
	Safer_Expand_Userkey();
    Safer_Block();
    Safer_Decrypt_Block();
#endif

/******************************************************************************/
#endif /* SAFER_H */
