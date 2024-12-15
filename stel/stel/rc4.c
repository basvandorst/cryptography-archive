/*

Alleged RC4 (based on the Usenet posting in Spring-95)

*/

#include <stdio.h>
#include <assert.h>
#include "rc4.h"

void rc4_init(RC4Context *ctx, const unsigned char *key, unsigned int key_len)
{
  unsigned int t, u;
  unsigned int keyindex;
  unsigned int stateindex;
  unsigned char* state;
  unsigned int counter;

  assert(key_len > 0);

  state = &ctx->state[0];
  ctx->x = 0;
  ctx->y = 0;
  for (counter = 0; counter < 256; counter++)
    state[counter] = counter;
  keyindex = 0;
  stateindex = 0;
  for (counter = 0; counter < 256; counter++)
    {
      t = state[counter];
      stateindex = (stateindex + key[keyindex] + t) & 0xff;
      u = state[stateindex];
      state[stateindex] = t;
      state[counter] = u;
      if (++keyindex >= key_len)
	keyindex = 0;
    }
}

inline unsigned int rc4_byte(RC4Context *ctx)
{
  unsigned int x;
  unsigned int y;
  unsigned int sx, sy;
  unsigned char *state;

  state = ctx->state;
  x = (ctx->x + 1) & 0xff;
  sx = state[x];
  y = (sx + ctx->y) & 0xff;
  sy = state[y];
  ctx->x = x;
  ctx->y = y;
  state[y] = sx;
  state[x] = sy;
  return state[(sx + sy) & 0xff];
}

void rc4_encrypt(RC4Context *ctx, unsigned char *dest, 
		 const unsigned char *src, unsigned int len)
{
  unsigned int i;
  for (i = 0; i < len; i++)
    dest[i] = src[i] ^ rc4_byte(ctx);
}

void rc4_decrypt(RC4Context *ctx, unsigned char *dest, 
		 const unsigned char *src, unsigned int len)
{
  rc4_encrypt(ctx, dest, src, len);
}

#ifdef TEST
main(argc, argv)
int	argc;
char	*argv[];
{
	RC4Context	rc4;
	char		buf[1024];
	register	i, len;

	rc4_init(&rc4, argv[1], strlen(argv[1]));

	while ((len = read(0, buf, sizeof(buf))) > 0)  {
		for (i = 0; i < len; i++)
			buf[i] ^= rc4_byte(&rc4);
		if (write(1, buf, len) != len) {
			perror("write()");
			exit(1);
		}
	}
	if (len < 0) {
		perror("read()");
		exit(1);
	}
	exit(0);
}
#endif /* TEST */

