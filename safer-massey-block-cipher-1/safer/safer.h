/*******************************************************************************
*
* FILE:           safer.h
*
* DESCRIPTION:    block-cipher algorithms SAFER K-64 and SAFER K-128
*                 (Secure And Fast Encryption Routine with a user-selected Key
*                 of length 64 bits and 128 bits, respectively)
*
* AUTHOR:         Richard De Moliner (demoliner@isi.ee.ethz.ch)
*                 Signal and Information Processing Laboratory
*                 Swiss Federal Institute of Technology
*                 CH-8092 Zuerich, Switzerland
*
* DATE:           March 23, 1995
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
#define SAFER_MAX_NOF_ROUNDS            13
#define SAFER_BLOCK_LEN                  8
#define SAFER_KEY_LEN     (1 + SAFER_BLOCK_LEN * (1 + 2 * SAFER_MAX_NOF_ROUNDS))

/******************* Assertions ***********************************************/

/******************* Macros ***************************************************/
 
/******************* Types ****************************************************/
typedef unsigned char safer_block_t[SAFER_BLOCK_LEN];
typedef unsigned char safer_key_t[SAFER_KEY_LEN];

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
*                           safer_key_t key)
*
*   expands a user-selected key of length 64 bits or 128 bits to a encryption / *   decryption key. If your user-selected key is of length 64 bits, then give
*   this key to both arguments 'userkey_1' and 'userkey_2', e.g.
*   'Safer_Expand_Userkey(z, z, key)'. Note: SAFER K-64 with a user-selected 
*   key 'z' of length 64 bits is identical to SAFER K-128 with a user-selected
*   key 'z z' of length 128 bits.
*   pre:  'userkey_1'  contains the first 64 bits of user key.
*         'userkey_2'  contains the second 64 bits of user key.
*         'nof_rounds' contains the number of encryption rounds
*                      'nof_rounds' <= 'SAFER_MAX_NOF_ROUNDS'
*   post: 'key'        contains the expanded key.
*
********************************************************************************
* void Safer_Encrypt_Block(safer_block_t block_in, safer_key_t key, 
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
    extern void Safer_Expand_Userkey(safer_block_t userkey_1,
                                     safer_block_t userkey_2,
                                     unsigned int nof_rounds,
                                     safer_key_t key);
    extern void Safer_Encrypt_Block (safer_block_t block_in, safer_key_t key, 
                                     safer_block_t block_out);
    extern void Safer_Decrypt_Block (safer_block_t block_in, safer_key_t key,
                                     safer_block_t block_out);
#else
    Safer_Init_Module();
    Safer_Expand_Userkey();
    Safer_Encrypt_Block();
    Safer_Decrypt_Block();
#endif

/******************************************************************************/
#endif /* SAFER_H */
