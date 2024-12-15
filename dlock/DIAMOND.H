/* diamond.h -- program interface to the Diamond and Diamond Lite block
   ciphers.  This file is in the Public Domain.  Written by Mike Johnson.*/

extern void set_diamond_key(byte *external_key, /* Variable length key */
                        uint key_size,      /* Length of key in bytes */
                        uint rounds,        /* Number of rounds to use (5 to 15
                                           for Diamond, 4 to 30 for Diamond Lite) */
                        boolean invert,     /* true if mpj_decrypt may be called. */
                        int block_size);    /* 16 for Diamond; 8 for Diamond Lite. */
/* Call before the first call to diamond_encrypt_block() or diamond_decrypt_block */

extern void diamond_encrypt_block(byte *x, byte *y);
/* Call set_diamond_key() with a block_size of 16 before first calling
   diamond_encrypt_block().   x is input, y is output.
*/

extern void diamond_decrypt_block(byte *x, byte *y);
/* Call set_diamond_key() with a block_size of 16 before first calling
   diamond_decrypt_block().   x is input, y is output.
*/

extern void lite_encrypt_block(byte *x, byte *y);
/* Call set_diamond_key() with a block_size of 8 before first calling
   lite_encrypt_block().   x is input, y is output.
*/

void lite_decrypt_block(byte *x, byte *y);
/* Call set_diamond_key() with a block_size of 8 before first calling
   lite_decrypt_block().   x is input, y is output.
*/

extern void diamond_done(void);
/* Clears internal keys.  Call after the last call to
diamond_encrypt_block() or diamond_decrypt_block() with a given key.  */


