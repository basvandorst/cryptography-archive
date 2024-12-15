/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#include "tc.h"

#if PGP_MACINTOSH
#include <unix.h>
#else
#include <sys/types.h>
#endif

#ifdef TESTING
#include <stdio.h>
#endif /* TESTING */
#include "cms_proto.h"

static const unsigned char BaseChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BaseChar(c) BaseChars[c]

static const unsigned char BaseVals[] = {
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,62, 0xff,0xff,0xff,63,
  52,53,54,55, 56,57,58,59, 60,61,0xff,0xff, 0xff,0xff,0xff,0xff,
  0xff, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
  15,16,17,18, 19,20,21,22, 23,24,25,0xff, 0xff,0xff,0xff,0xff,
  0xff,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
  41,42,43,44, 45,46,47,48, 49,50,51,0xff, 0xff,0xff,0xff,0xff
};

#define BaseVal(c) BaseVals[c]

int tc_encode_base64 (unsigned char **out, size_t *outlen,
		      const unsigned char *in, size_t inlen,
		      TC_CONTEXT *ctx)
{
  unsigned char *curpos;
  unsigned char a, b, c;

  /* determine how long the output will be.  if it is not a multple of 3
   * bytes, increase length so that it is so that the pad characters are
   * accounted for.
   */
  if (inlen % 3 != 0)
    *outlen = inlen + 3 - inlen % 3;
  else
    *outlen = inlen;

  /* base64 encoding creates 4 output chars for every 3 input chars */
  *outlen = 4 * (*outlen) / 3;

  if ((*out = TC_Alloc(ctx->memMgr, *outlen + 1)) == NULL)
    return TC_E_NOMEMORY;
  curpos = *out;

  while (inlen)
  {
    a = *in++;
    inlen--;

    *curpos++ = BaseChar (a >> 2);

    if (inlen)
      b = *in++;
    else
      b = 0;

    *curpos++ = BaseChar (((a & 0x03) << 4) | (b >> 4));

    if (!inlen)
    {
      *curpos++ = '=';
      *curpos++ = '=';
      break;
    }
    inlen--;

    if (inlen)
      c = *in++;
    else
      c = 0;

    *curpos++ = BaseChar (((b & 0x0f) << 2) | (c >> 6));

    if (!inlen)
    {
      *curpos++ = '=';
      break;
    }
    inlen--;

    *curpos++ = BaseChar (c & 0x3f);
  }

  *curpos = 0;

  return 0;
}

int
tc_decode_base64 (unsigned char **out,
		  size_t *outlen,
		  const unsigned char *in,
		  TC_CONTEXT *ctx)
{
  unsigned char *curpos;
  unsigned char a, b;

  *outlen = (3 * strlen ((char *) in)) / 4; /* maximum length */
  if ((*out = TC_Alloc(ctx->memMgr, *outlen)) == NULL)
      return TC_E_NOMEMORY;
  curpos = *out;

  SKIPWS (in);
  while (*in)
  {
    a = *in++;
    SKIPWS (in);
    if (!*in)
      return (TC_E_PARSE);
    b = *in++;
    SKIPWS (in);
    if (!*in)
      return (TC_E_PARSE);

    *curpos++ = (BaseVal (a) << 2) | (BaseVal (b) >> 4);

    a = b;
    b = *in++;
    SKIPWS (in);
    if (!*in)
      return (TC_E_PARSE);

    if (b == '=')
      break;

    *curpos++ = (BaseVal (a) << 4) | (BaseVal (b) >> 2);

    a = b;
    b = *in++;
    SKIPWS (in);

    if (b == '=')
      break;

    *curpos++ = (BaseVal (a) << 6) | BaseVal (b);
  }

  *outlen = curpos - *out; /* real length */

  return 0;
}

/* NOTE: this has NOT been updated to use mem. mgmt changes */
#ifdef TESTING
int main()
{
  unsigned char input[1024];
  unsigned char *enc;
  unsigned char *dec;
  size_t enclen;
  size_t declen;

  fgets ((char *) input, sizeof (input), stdin);

  tc_encode_base64 (&enc, &enclen, input, strlen ((char *) input));
  puts ((char *) enc);
  if (strlen (enc) != enclen)
  {
    puts ("length is bad.");
    exit (0);
  }
  if (tc_decode_base64 (&dec, &declen, enc) != 0)
  {
    puts ("base64 decode failed.");
    exit (0);
  }
  dec[declen] = 0;
  puts ((char *) dec);

  if (strcmp ((char *) dec, (char *) input) == 0)
    puts ("success.");
  else
    puts ("failed.");

  free (enc);
  free (dec);

  exit (0);
}
#endif /* TESTING */