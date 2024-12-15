#include "des.h"

static	DES	des_z;

int
des_init(desbuf,mode)

DES	*desbuf;
int	mode;

{
  *desbuf = des_z;
}

void
des_done(keybuf)

DES	*keybuf;

{
  keybuf->key = des_z.key;
  keybuf->sched = des_z.sched;
}

void
des_setkey(keybuf,key)

DES	*keybuf;
char	*key;

{
  C_Block	b;

  des_bitrev(key,&b);
  des_set_key(&b,&keybuf->sched);
}

int
des_encrypt(keybuf,block)

DES	*keybuf;
char	*block;

{
  des_bitrev(block,block);
  des_ecb_encrypt(block,block,&keybuf->sched,DES_ENCRYPT);
  des_bitrev(block,block);
}

void
des_decrypt(keybuf,block)

DES	*keybuf;
char	*block;

{
  des_bitrev(block,block);
  des_ecb_encrypt(block,block,&keybuf->sched,DES_DECRYPT);
  des_bitrev(block,block);
}
