/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "skip_defs.h"
#include "memblk.h"
#include "crypt.h"
#include "skipcache.h"
#include "iaddr.h"
#include "random.h"

#define PARSE_COMMENT	"##\n" \
			"## SKIP Daemon configuration file\n" \
			"##\n\n"

#define ENTRYSEPARATOR	\
"#########################################################################\n\n"

#define IGNORE			"ignore"

#define IS_EOL(x)		(((x) == '\n') || ((x) == '\r'))
#define IS_BLANK(x)		(((x) == ' ') || ((x) == '\t'))
#define IS_SEPARATOR(x)		(IS_BLANK(x) || ((x) == ':') || ((x) == ';') || ((x) == ','))

#define ALIGN4(len) (((len) + 3) & ~3)

static u_char defaultIPaddr[IPADDRSIZE] = { 0xff, 0xff, 0xff, 0xff,
					    0xff, 0xff, 0xff, 0xff,
					    0xff, 0xff, 0xff, 0xff,
					    0xff, 0xff, 0xff, 0xff };

static u_char ignoreIPaddr[IPADDRSIZE];

static u_char data[8192], *dataptr;

struct parseentry
{
  char *keyword;
  int (*req)(struct skipcache *cnf, char *val);
  int (*add)(struct skipcache *cnf, char *optname, FILE *f);
};

static u_char defaultdata[8192];

static struct skipcache defaultconfig =
{
  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },  /* IPaddr */
  NULL, NULL,                                          /* next, prev */
  60, 60,                                              /* ttl, maxttl */
  0,                                                   /* ref */
  SKIP_NOKEY,                                          /* flags */
  -1,                                                  /* fd */
  2,                                                   /* DH base */
  0, 0,                                                /* DH modulus */
  0, 0,                                                /* DH secret key */
  0, 0,                                                /* DH public key */
  0, 0,                                                /* lookup hosts */
  0,                                                   /* Kij offset */
  { FILTER_NONE, FILTER_NONE },                        /* filter_before */
  { FILTER_NONE, FILTER_NONE },                        /* filter_after */
  { ENSKIPMODE_ALL, ENSKIPMODE_ALL },                  /* enskip_mode */
  { 0x00, 0x00, 0x00, 0x00 },                          /* enskip_SAID */
  1, 1, 1, 0,                                          /* Kij,Kp,ICV,Comp alg */
  0, 512*1024,					       /* Kp_bytes, maxbytes */
  0, 30,					       /* Kp_ttl, maxttl */
  0, 0, 0,                                             /* Kp,Kp_Kij,MI offset */
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  /* enskip_seqno */
  { DESKIPMODE_DESKIP, DESKIPMODE_DESKIP },            /* deskip_mode */
  { { 0x00, 0x00, 0x00, 0x00 },                        /* deskip_policy[0] */
    { 0x00, 0x00, 0x00, 0x00 } },                      /* deskip_policy[1] */
  0, 0,                                                /* Kij,Kp alg */
  0, 0,                                                /* Kp,Kp_Kij offset */
  { { 0 }, 0, 10 },                                    /* deskip_seqhistory */
  0,                                                   /* deskip_seqhistbuf */
  0, 0, 0,                                             /* state offsets */
  defaultdata, 0,                                      /* data ptr, len */
};

static int linecnt;

static char *cryptalgs[] = { "Unknown", "DES", "RC2", "RC4 40", "RC4 128", "3DES 2", "3DES 3", "IDEA", "Unknown", "Unknown", "SIMPLE"}; 
static char *signalgs[] = { "Unknown", "MD5" };
static char *filter[] = { "NONE", "IP", "IPSP" };
static char *said[] = { "NONE", "CRYPT", "AUTH", "SEQ", "COMP" };
static char *enskipmode[] = { "NONE", "TCP", "UDP", "ICMP", "OTHER", "ALL" };
static char *deskipmode[] = { "NONE", "DESKIP" };
static char *flags[] = { "NONE", "NO_KEY", "ENCAPS", "VALID_KEY" };

static int allocdata(struct skipcache *cnf, u_char **buf, int len)
{
  int result = 0;

  if (len)
  {
    if (buf)
      *buf = dataptr;
    MEMZERO(dataptr, ALIGN4(len));
    result = dataptr - cnf->data;
    dataptr += ALIGN4(len);
  }

  return result;
}

static int freedata(struct skipcache *cnf, int offset)
{
  dataptr = cnf->data + offset;

  return 0;
}

static int scanlonghex(struct skipcache *cnf, u_long *offset, int *len,
		       char *val)
{
  int result = 0, used;
  u_char *buf;
  int l;

  val += 2;
  l = strlen(val);
  if (l & 1)
  {
    fprintf(stderr, "skipd: malformed hex number, length is odd\n");
    l = 0;
  }
  l /= 2;

  *offset = allocdata(cnf, &buf, l);

  for (used = 0; used < l; used++, val += 2)
  {
    static char *hexdigits = "0123456789abcdefABCDEF", *hd1, *hd2;

    if ((hd1 = strchr(hexdigits, val[0])) && (hd2 = strchr(hexdigits, val[1])))
    {
      int d1 = hd1 - hexdigits;
      int d2 = hd2 - hexdigits;
      if (d1 > 15)  /* Uppercase */
	d1 -= 6;
      if (d2 > 15)  /* Uppercase */
	d2 -= 6;
      *buf++ = (d1 << 4) | d2;
    }
    else
    {
      fprintf(stderr, "skipd: malformed hex number at '%s'\n", val);
      freedata(cnf, *offset);
      used = 0;
      break;
    }
  }

  if (used <= 0)
  {
    *offset = 0;
    l = 0;
    result = -1;
  }

  if (len)
    *len = l;

  return result;
}

static int printlonghex(FILE *f, u_char *val, int len)
{
  int i;

  fprintf(f, "0x");
  for (i = 0; i < len; i++)
    fprintf(f, "%02x", *val++);

  return 0;
}

static int calcKij(struct skipcache *cnf)
{
  cnf->flags |= SKIP_VALIDKEY;
  cnf->flags &= ~SKIP_NOKEY;
  return 0;
}

static int idx(char *str, char **arr, int entries)
{
  int walk;

  for (walk = 0; walk < entries; walk++)
  {
    if (arr[walk])
    {
      if (strcmp(str, arr[walk]) == 0)
        return walk;
    }
  }
  return -1;
}

static char *separator(char *val)
{
  char *ret = NULL, *tmp = val;

  while (tmp && *tmp)
  {
    if (IS_SEPARATOR(*tmp))
    {
      *tmp = '\0';
      ret = tmp;
    }
    else
    {
      if (ret)
      {
	ret = tmp;
	break;
      }
    }

    tmp++;
  }

  return (ret && *ret) ? ret : NULL;
}

static int cryptalloc(struct skipcache *cnf)
{
  int MI_len = crypt_MIlen(cnf->enskip_Kp_alg);
  int estatelen = crypt_encryptstatelen(cnf->enskip_Kp_alg);
 
  cnf->enskip_MI = allocdata(cnf, NULL, MI_len);
  cnf->encryptstate = allocdata(cnf, NULL, estatelen);

  return 0;
}

static int keyalloc(struct skipcache *cnf)
{
  int keylen = crypt_keylen(cnf->enskip_Kp_alg);
  int blocklen = crypt_blocklen(cnf->enskip_Kij_alg);
  int padlen;

  if ((padlen = keylen % blocklen))
    padlen = blocklen - padlen;

  cnf->enskip_Kp = allocdata(cnf, NULL, keylen);
  cnf->enskip_Kp_Kij = allocdata(cnf, NULL, keylen + padlen);
 
  return 0;
}

static int deskipalloc(struct skipcache *cnf)
{
  int maxdslen = crypt_decryptstatelen(CRYPTALG_MAX);
  int maxkeylen = crypt_keylen(CRYPTALG_MAX);
  int maxblocklen = crypt_blocklen(CRYPTALG_MAX);
  int maxdsblocklen = crypt_decryptstatelen(CRYPTALG_MAXBLOCK);

  cnf->payload_decryptstate = allocdata(cnf, NULL, maxdslen);
  cnf->deskip_Kp = allocdata(cnf, NULL, maxkeylen);
  cnf->deskip_Kp_Kij = allocdata(cnf, NULL, maxkeylen + maxblocklen - 1);
  cnf->Kp_decryptstate = allocdata(cnf, NULL, maxdsblocklen);

  return 0;
}

static int sequencealloc(struct skipcache *cnf)
{
  MEMZERO(cnf->deskip_seqhistory.last, sizeof(cnf->deskip_seqhistory.last));
  cnf->deskip_seqhistory.index = 0;
  cnf->deskip_seqhistorybuf = allocdata(cnf, NULL, cnf->deskip_seqhistory.len);

  return 0;
}

static int outfilterbefore_req(struct skipcache *cnf, char *val)
{
  int f = idx(val, filter, sizeof(filter) / sizeof(filter[0]));

  if (f < 0)
  {
    fprintf(stderr, "skipd: unknown filter '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->filter_before[SKIP_OUTPUT] = f;
    return 0;
  }
}
static int outfilterbefore_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, filter[cnf->filter_before[SKIP_OUTPUT]]);
  return 0;
}

static int infilterbefore_req(struct skipcache *cnf, char *val)
{
  int f = idx(val, filter, sizeof(filter) / sizeof(filter[0]));

  if (f < 0)
  {
    fprintf(stderr, "skipd: unknown filter '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->filter_before[SKIP_INPUT] = f;
    return 0;
  }
}
static int infilterbefore_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, filter[cnf->filter_before[SKIP_INPUT]]);
  return 0;
}

static int outfilterafter_req(struct skipcache *cnf, char *val)
{
  int f = idx(val, filter, sizeof(filter) / sizeof(filter[0]));

  if (f < 0)
  {
    fprintf(stderr, "skipd: unknown filter '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->filter_after[SKIP_OUTPUT] = f;
    return 0;
  }
}
static int outfilterafter_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, filter[cnf->filter_after[SKIP_OUTPUT]]);
  return 0;
}

static int infilterafter_req(struct skipcache *cnf, char *val)
{
  int f = idx(val, filter, sizeof(filter) / sizeof(filter[0]));

  if (f < 0)
  {
    fprintf(stderr, "skipd: unknown filter '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->filter_after[SKIP_INPUT] = f;
    return 0;
  }
}
static int infilterafter_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, filter[cnf->filter_after[SKIP_INPUT]]);
  return 0;
}

static int parseenskipmode(char *val)
{
  char *walk, *next;
  int mode;

  mode = ENSKIPMODE_NONE;
  for (walk = val, next = separator(walk); walk; walk = next, next = separator(walk))
  {
    switch (idx(walk, enskipmode, sizeof(enskipmode) / sizeof(enskipmode[0])))
    {
      case -1:
        fprintf(stderr, "skipd: unknown enskip mode '%s'.\n", val);
	break;
      case 0:
	/* NONE */
	break;
      case 1:
	mode |= ENSKIPMODE_TCP;
	break;
      case 2:
	mode |= ENSKIPMODE_UDP;
	break;
      case 3:
	mode |= ENSKIPMODE_ICMP;
	break;
      case 4:
	mode |= ENSKIPMODE_OTHER;
	break;
      case 5:
	mode |= ENSKIPMODE_ALL;
	break;
    }
  }
  return mode;
}

static int saveenskipmode(int mode, char *optname, FILE *f)
{
  fprintf(f, "  %s = ", optname);

  if (mode == ENSKIPMODE_ALL)
    fprintf(f, "ALL");
  else if (mode == ENSKIPMODE_NONE)
    fprintf(f, "NONE");
  else
  {
    if (mode & ENSKIPMODE_TCP)
      fprintf(f, "TCP ");
    if (mode & ENSKIPMODE_UDP)
      fprintf(f, "UDP ");
    if (mode & ENSKIPMODE_ICMP)
      fprintf(f, "ICMP ");
    if (mode & ENSKIPMODE_OTHER)
      fprintf(f, "OTHER ");
  }
  fprintf(f, "\n");
  return 0;
}

static int outenskipmode_req(struct skipcache *cnf, char *val)
{
  cnf->enskip_mode[SKIP_OUTPUT] = parseenskipmode(val);
  return 0;
}
static int outenskipmode_add(struct skipcache *cnf, char *optname, FILE *f)
{
  return saveenskipmode(cnf->enskip_mode[SKIP_OUTPUT], optname, f);
}
 
static int inenskipmode_req(struct skipcache *cnf, char *val)
{
  cnf->enskip_mode[SKIP_INPUT] = parseenskipmode(val);
  return 0;
}
static int inenskipmode_add(struct skipcache *cnf, char *optname, FILE *f)
{
  return saveenskipmode(cnf->enskip_mode[SKIP_INPUT], optname, f);
}

static u_char parsesaid(char *val)
{
  char *walk, *next;
  skip_said_t res;

  res[0] = SAID_VERSION;

  for (walk = val, next = separator(walk); walk; walk = next, next = separator(walk))
  {
    switch (idx(walk, said, sizeof(said) / sizeof(said[0])))
    {
      case -1:
        fprintf(stderr, "skipd: unknown said mode '%s'.\n", val);
	break;
      case 0:
	break;
      case 1:
	res[0] |= SAID_CRYPT;
	break;
      case 2:
	res[0] |= SAID_AUTH;
	break;
      case 3:
	res[0] |= SAID_SEQ;
	break;
      case 4:
	res[0] |= SAID_COMP;
	break;
    }
  }
  return res[0];
}

static int enskipsaid_req(struct skipcache *cnf, char *val)
{
  cnf->enskip_SAID[0] = parsesaid(val);
  return 0;
}

static int savesaid(u_char said, char *optname, FILE *f)
{
  if (said)
  {
    fprintf(f, "  %s = ", optname);

    if (said & SAID_CRYPT)
      fprintf(f, "CRYPT ");
    if (said & SAID_AUTH)
      fprintf(f, "AUTH ");
    if (said & SAID_SEQ)
      fprintf(f, "SEQ ");
    if (said & SAID_COMP)
      fprintf(f, "COMP ");
    fprintf(f, "\n");
  }
  else
    fprintf(f, "  %s = NONE\n", optname);

  return 0;
}

static int enskipsaid_add(struct skipcache *cnf, char *optname, FILE *f)
{
  return savesaid(cnf->enskip_SAID[0], optname, f);
}

static int kpalg_req(struct skipcache *cnf, char *val)
{
  int alg = idx(val, cryptalgs, sizeof(cryptalgs) / sizeof(cryptalgs[0])); 

  if (alg <= 0)
  {
    fprintf(stderr, "skipd: unknown Kp algorithm '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->enskip_Kp_alg = alg;
    return 0;
  }
}
static int kpalg_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, cryptalgs[cnf->enskip_Kp_alg]);
  return 0;
}

static int kijalg_req(struct skipcache *cnf, char *val)
{
  int alg = idx(val, cryptalgs, sizeof(cryptalgs) / sizeof(cryptalgs[0])); 

  if (alg <= 0)
  {
    fprintf(stderr, "skipd: unknown Kij algorithm '%s'.\n", val);
    return -1;
  }
  else
  {
    if (crypt_blocklen(alg) != 1)
      cnf->enskip_Kij_alg = alg;
    else
    {
      fprintf(stderr, "skipd: stream ciphers cannot be used to encrypt packet keys\n");
      return -1;
    }
  }

  return 0;
}
static int kijalg_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, cryptalgs[cnf->enskip_Kij_alg]);
  return 0;
}

static int icvalg_req(struct skipcache *cnf, char *val)
{
  int alg = idx(val, signalgs, sizeof(signalgs) / sizeof(signalgs[0])); 

  if (alg <= 0)
  {
    fprintf(stderr, "skipd: unknown ICV algorithm '%s'.\n", val);
    return -1;
  }
  else
  {
    cnf->enskip_ICV_alg = alg;
    return 0;
  }
}
static int icvalg_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, signalgs[cnf->enskip_ICV_alg]);
  return 0;
}

static int outdeskipmode_req(struct skipcache *cnf, char *val)
{
  int m = idx(val, deskipmode, sizeof(deskipmode) / sizeof(deskipmode[0]));

  if (m < 0)
  {
    fprintf(stderr, "skipd: unknown deskip mode '%s'.\n", val);
    return -1;
  }
  else
    cnf->deskip_mode[SKIP_OUTPUT] = m;

  return 0;
}
static int outdeskipmode_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, deskipmode[cnf->deskip_mode[SKIP_OUTPUT]]);
  return 0;
}

static int indeskipmode_req(struct skipcache *cnf, char *val)
{
  int m = idx(val, deskipmode, sizeof(deskipmode) / sizeof(deskipmode[0]));

  if (m < 0)
  {
    fprintf(stderr, "skipd: unknown deskip mode '%s'.\n", val);
    return -1;
  }
  else
    cnf->deskip_mode[SKIP_INPUT] = m;

  return 0;
}
static int indeskipmode_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %s\n", optname, deskipmode[cnf->deskip_mode[SKIP_INPUT]]);
  return 0;
}

static int outpolicy_req(struct skipcache *cnf, char *val)
{
  cnf->deskip_policy[SKIP_OUTPUT][0] = parsesaid(val);
  return 0;
}
static int outpolicy_add(struct skipcache *cnf, char *optname, FILE *f)
{
  return savesaid(cnf->deskip_policy[SKIP_OUTPUT][0], optname, f);
}

static int inpolicy_req(struct skipcache *cnf, char *val)
{
  cnf->deskip_policy[SKIP_INPUT][0] = parsesaid(val);
  return 0;
}
static int inpolicy_add(struct skipcache *cnf, char *optname, FILE *f)
{
  return savesaid(cnf->deskip_policy[SKIP_INPUT][0], optname, f);
}

static int ttl_req(struct skipcache *cnf, char *val)
{
  cnf->ttl = atoi(val);
  return 0;
}

static int ttl_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %d\n", optname, cnf->ttl);
  return 0;
}

static int maxttl_req(struct skipcache *cnf, char *val)
{
  cnf->maxttl = atoi(val);
  return 0;
}

static int maxttl_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %d\n", optname, cnf->maxttl);
  return 0;
}

static int keychangebytes_req(struct skipcache *cnf, char *val)
{
  cnf->enskip_Kp_maxbytes = atoi(val);
  return 0;
}

static int keychangebytes_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %d\n", optname, cnf->enskip_Kp_maxbytes);
  return 0;
}

static int keychangetime_req(struct skipcache *cnf, char *val)
{
  cnf->enskip_Kp_maxttl = atoi(val);
  return 0;
}

static int keychangetime_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %d\n", optname, cnf->enskip_Kp_maxttl);
  return 0;
}

static int manualsecret_req(struct skipcache *cnf, char *val)
{
  int result = -1;
  u_char *buf;
  int len;

  if (*val == '"')  /* Manual secret format "...." */
  {
    val++;
    for (len = 0; val[len] && (val[len] != '"'); len++)
      ;
      
    if (len)
    {
      int maxkeylen = crypt_keylen(CRYPTALG_MAXBLOCK);

      cnf->flags |= SKIP_VALIDKEY;
      cnf->flags &= ~SKIP_NOKEY;
      cnf->Kij = allocdata(cnf, &buf, maxkeylen);
      random_hash(buf, maxkeylen, val, len);
      result = 0;
    }
    else
      fprintf(stderr, "skipd: empty 'manual secret' value\n");
  }
  else if ((val[0] == '0') && (val[1] == 'x'))  /* Secret format 0x... */
  {
    if (scanlonghex(cnf, &cnf->Kij, NULL, val) == 0)
      result = 0;
  }
  else
    fprintf(stderr, "skipd: wrong 'manual secret' value\n");

  return result;
}

static int manualsecret_add(struct skipcache *cnf, char *optname, FILE *f)
{
  if (cnf->secretkeylen)
  {
    fprintf(f, "  %s = ", optname);
    printlonghex(f, cnf->data + cnf->Kij, crypt_keylen(CRYPTALG_MAXBLOCK));
    fprintf(f, "\n");
  }

  return 0;
}

static int secretkey_req(struct skipcache *cnf, char *val)
{
  int result = -1;

  if ((val[0] == '0') && (val[1] == 'x'))  /* Public format 0x... */
  {
    if (scanlonghex(cnf, &cnf->secretkey, &cnf->secretkeylen, val) == 0)
    {
      if (cnf->publickeylen)
	calcKij(cnf);

      result = 0;
    }
  }
  else
    fprintf(stderr, "skipd: wrong 'secret key' value\n");

  return result;
}

static int secretkey_add(struct skipcache *cnf, char *optname, FILE *f)
{
  if (cnf->secretkeylen)
  {
    fprintf(f, "  %s = ", optname);
    printlonghex(f, cnf->data + cnf->secretkey, cnf->secretkeylen);
    fprintf(f, "\n");
  }

  return 0;
}

static int publickey_req(struct skipcache *cnf, char *val)
{
  int result = -1;

  if ((val[0] == '0') && (val[1] == 'x'))  /* Public format 0x... */
  {
    if (scanlonghex(cnf, &cnf->publickey, &cnf->publickeylen, val) == 0)
    {
      if (cnf->secretkeylen)
	calcKij(cnf);

      result = 0;
    }
  }
  else
    fprintf(stderr, "skipd: wrong 'public key' value\n");

  return result;
}

static int publickey_add(struct skipcache *cnf, char *optname, FILE *f)
{
  if (cnf->publickeylen)
  {
    fprintf(f, "  %s = ", optname);
    printlonghex(f, cnf->data + cnf->publickey, cnf->publickeylen);
    fprintf(f, "\n");
  }

  return 0;
}

static int lookuphosts_req(struct skipcache *cnf, char *val)
{
  int result = -1;

  cnf->lookuphosts = dataptr - cnf->data;
  cnf->lookuphostslen = 0;

  while (val && *val)
  {
    u_long ip;
    char *next = separator(val);
    
    if ((ip = iaddr(val)))
    {
      MEMZERO(dataptr, IPADDRSIZE);
      dataptr[0] = ip >> 24;
      dataptr[1] = (ip >> 16) & 0xff;
      dataptr[2] = (ip >> 8) & 0xff;
      dataptr[3] = ip  & 0xff;
      cnf->lookuphostslen += IPADDRSIZE;
      dataptr += IPADDRSIZE;
    }
    else
      fprintf(stderr, "skipd: malformed entry in 'lookuphosts' value at %s\n", val);

    val = next;
  }

  if (cnf->lookuphostslen)
    result = 0;
  else
  {
    cnf->lookuphosts = 0;
    fprintf(stderr, "skipd: empty 'lookuphosts' entry\n");
  }

  return result;
}

static int lookuphosts_add(struct skipcache *cnf, char *optname, FILE *f)
{
  int i;

  if (cnf->lookuphostslen)
  {
    fprintf(f, "  %s =", optname);

    for (i = 0; i < cnf->lookuphostslen; i += IPADDRSIZE)
    {
      fprintf(f, " %u.%u.%u.%u",
	      cnf->data[cnf->lookuphosts + i],
	      cnf->data[cnf->lookuphosts + i + 1],
	      cnf->data[cnf->lookuphosts + i + 2],
	      cnf->data[cnf->lookuphosts + i + 3]);
    }

    fprintf(f, "\n");
  }

  return 0;
}

static int base_req(struct skipcache *cnf, char *val)
{
  cnf->base = atoi(val);

  return 0;
}

static int base_add(struct skipcache *cnf, char *optname, FILE *f)
{
  fprintf(f, "  %s = %d\n", optname, cnf->base);

  return 0;
}

static int modulus_req(struct skipcache *cnf, char *val)
{
  int result = -1;

  if ((val[0] == '0') && (val[1] == 'x'))
    result = scanlonghex(cnf, &cnf->modulus, &cnf->moduluslen, val);
  else
    fprintf(stderr, "skipd: wrong 'modulus' value\n");

  return result;
}

static int modulus_add(struct skipcache *cnf, char *optname, FILE *f)
{
  if (cnf->moduluslen)
  {
    fprintf(f, "  %s = ", optname);
    printlonghex(f, cnf->data + cnf->modulus, cnf->moduluslen);
    fprintf(f, "\n");
  }

  return 0;
}

static int flags_req(struct skipcache *cnf, char *val)
{
  char *walk, *next;

  for (walk = val, next = separator(walk); walk; walk = next, next = separator(walk))
  {
    switch (idx(walk, flags, sizeof(flags) / sizeof(flags[0])))
    {
      case -1:
        fprintf(stderr, "skipd: unknown flag '%s'.\n", val);
	break;
      case 0:
	break;
      case 1:
	cnf->flags |= SKIP_NOKEY;
	cnf->flags &= ~SKIP_VALIDKEY;
	break;
      case 2:
	cnf->flags |= SKIP_ENCAPS;
	break;
      case 3:
	if (cnf->Kij == 0)
	{
	  fprintf(stderr, "skipd: no valid key specified.\n");
	  return -1;
	}
	cnf->flags |= SKIP_VALIDKEY;
	cnf->flags &= ~SKIP_NOKEY;
	break;
    }
  }

  return 0;
}
static int flags_add(struct skipcache *cnf, char *optname, FILE *f)
{
  if (cnf->flags == 0)
    fprintf(f, "  %s = NONE\n", optname);
  else
  {
    fprintf(f, "  %s = ", optname);
    if (cnf->flags & SKIP_VALIDKEY)
      fprintf(f, "VALID_KEY ");
    if (cnf->flags & SKIP_NOKEY)
      fprintf(f, "NO_KEY ");
    if (cnf->flags & SKIP_ENCAPS)
      fprintf(f, "ENCAPS ");
    fprintf(f, "\n");
  }
  return 0; 
}

/*
 * parse core
 */
static struct parseentry pt[] =
{
  { "output filter before", outfilterbefore_req, outfilterbefore_add },
  { "input filter before", infilterbefore_req, infilterbefore_add },
  { "output filter after", outfilterafter_req, outfilterafter_add },
  { "input filter after", infilterafter_req, infilterafter_add },
  { "output enskip mode", outenskipmode_req, outenskipmode_add },
  { "input enskip mode", inenskipmode_req, inenskipmode_add },
  { "enskip said", enskipsaid_req, enskipsaid_add },
  { "Kp algorithm", kpalg_req, kpalg_add },
  { "Kij algorithm", kijalg_req, kijalg_add },
  { "ICV algorithm", icvalg_req, icvalg_add },
  { "output deskip mode", outdeskipmode_req, outdeskipmode_add },
  { "input deskip mode", indeskipmode_req, indeskipmode_add },
  { "output deskip policy", outpolicy_req, outpolicy_add },
  { "input deskip policy", inpolicy_req, inpolicy_add },
  { "ttl", ttl_req, ttl_add },
  { "maxttl", maxttl_req, maxttl_add },
  { "key change bytes", keychangebytes_req, keychangebytes_add },
  { "key change time", keychangetime_req, keychangetime_add },
  { "secret key", secretkey_req, secretkey_add },
  { "public key", publickey_req, publickey_add },
  { "manual secret", manualsecret_req, manualsecret_add },
  { "lookuphosts", lookuphosts_req, lookuphosts_add },
  { "base", base_req, base_add },
  { "modulus", modulus_req, modulus_add },
  { "flags", flags_req, flags_add },
};
static struct skipcache *first, *last;

static int parse_addlist(struct skipcache *cnf)
{
  struct skipcache *new;
  
  if (MEMCMP(cnf->IPaddr, ignoreIPaddr, sizeof(ignoreIPaddr)) == 0)
    return 0;

  if (cnf->enskip_SAID[0] & SAID_CRYPT)
    cryptalloc(cnf);
  if (cnf->enskip_SAID[0] & (SAID_AUTH | SAID_CRYPT))
    keyalloc(cnf);
  if (cnf->enskip_SAID[0] & SAID_SEQ)
    sequencealloc(cnf);
  deskipalloc(cnf);

  cnf->datalen = dataptr - cnf->data;

  if ((new = malloc(sizeof(*new) + cnf->datalen)) == NULL)
  {
    fprintf(stderr, "skipd: not enough memory (malloc failed).");
    return -1;
  }

  *new = *cnf;
  new->data = (u_char *)(new + 1);
  new->next = NULL;
  MEMCPY(new->data, cnf->data, cnf->datalen);

  if (first == NULL)
  {
    first = last = new;
  }
  else
  {
    last->next = new;
    last = new;
  }
  return 0;
}

struct skipcache *parse_parse(char *path)
{
  FILE *f;
  char line[1024];
  struct skipcache entry, *lastentry = NULL;

  first = last = NULL;

  if ((f = fopen(path, "r")) == NULL)
  {
    fprintf(stderr, "skipd: cannot open file '%s'\n", path);
    return NULL;
  }

  for (line[0] = '\0', linecnt = 1; fgets(line, sizeof(line), f); line[0] = '\0', linecnt++)
  {
    char *walk, name[128], var[128], val[128];
    int r;

    for (walk = line; *walk && IS_BLANK(*walk); walk++) ;

    if (IS_EOL(walk[0]) || (walk[0] == '\0') || (walk[0] == '#'))
      continue;

    if (strchr(walk, ':') && (strchr(walk, '=') == 0) && (sscanf(walk, "%[^:]:%s\n", name, var) == 1))
    {
      u_long addr;

      if (lastentry)
      {
	if (MEMCMP(lastentry->IPaddr, defaultIPaddr, sizeof(defaultIPaddr)))
          parse_addlist(lastentry);
        else
	{
	  entry.datalen = dataptr - data; 
          defaultconfig = *lastentry;
	  defaultconfig.data = defaultdata;
	  MEMCPY(defaultdata, data, lastentry->datalen);
	}
      }

      /* init default */
      lastentry = &entry;
      entry = defaultconfig;
      MEMCPY(data, defaultdata, defaultconfig.datalen);
      entry.data =  data;
      if (defaultconfig.datalen)
	dataptr = data + defaultconfig.datalen;
      else
	dataptr = data + ALIGN4(1);

      if (!strcmp(name, IGNORE))  /* Ignore entry */
	MEMCPY(entry.IPaddr, ignoreIPaddr, sizeof(ignoreIPaddr));
      else if (strcmp(name, "default"))  /* Not default entry */
      {
	MEMZERO(entry.IPaddr, sizeof(entry.IPaddr));
	addr = iaddr(name);
	MEMCPY(entry.IPaddr, &addr, sizeof(addr));
      }
    }
    else if ((r = sscanf(walk," %[^=] = %[^\n]\n", var, val)) == 2)
    {
      int walk, notfound;

      for (walk = 0, notfound = 1; walk < (sizeof(pt) / sizeof(struct parseentry)); walk++)
      {
        if (pt[walk].keyword && pt[walk].req && ((notfound = strncasecmp(pt[walk].keyword, var, strlen(pt[walk].keyword))) == 0))
        {
	  if (pt[walk].req(&entry, val) != 0)
	    fprintf(stderr, "skipd: error in configuration file '%s' on line %d.\n", path, linecnt);
          break;
        }
      }

      if (notfound)
        fprintf(stderr, "skipd: unknown keyword '%s' in configuration file '%s' on line %d.\n", var, path, linecnt);
    }
    else if (r != 1)
      fprintf(stderr, "skipd: error in configuration file '%s' on line %d.\n", path, linecnt);
  }

  if (lastentry)
  {
    if (MEMCMP(lastentry->IPaddr, defaultIPaddr, sizeof(defaultIPaddr)))
      parse_addlist(lastentry);
    else
    {
      entry.datalen = dataptr - data; 
      defaultconfig = *lastentry;
      defaultconfig.data = defaultdata;
      MEMCPY(defaultdata, data, lastentry->datalen);
    }
  }

  dataptr = defaultconfig.data + defaultconfig.datalen;
  parse_addlist(&defaultconfig);

  return first;
}

int parse_save(struct skipcache *cnf, char *path)
{
  FILE *f;
  struct skipcache *cnfwalk;

  if ((f = fopen(path, "w")) == 0)
    return -1;

#ifdef PARSE_COMMENT
  if (fwrite(PARSE_COMMENT, 1, strlen(PARSE_COMMENT), f) != strlen(PARSE_COMMENT
))
  {
    fclose(f);
    return -1;
  }
#endif

  for (cnfwalk = cnf; cnfwalk; cnfwalk = cnfwalk->next)
  {
    int walk;

    if (MEMCMP(cnfwalk->IPaddr, defaultIPaddr, sizeof(defaultIPaddr)) == 0)
      fprintf(f, "default:\n");
    else
      fprintf(f, "%u.%u.%u.%u:\n", cnfwalk->IPaddr[0], cnfwalk->IPaddr[1], cnfwalk->IPaddr[2], cnfwalk->IPaddr[3]);

    for (walk = 0; walk < (sizeof(pt) / sizeof(struct parseentry)); walk++)
      if (pt[walk].add)
        pt[walk].add(cnfwalk, pt[walk].keyword, f);
      else if (pt[walk].req == NULL)
        fprintf(f, pt[walk].keyword);

    fprintf(f, "\n");
    if (cnfwalk->next)
      fprintf(f, ENTRYSEPARATOR);
  }

  fclose(f);
  return 0;
}

int parse_free(struct skipcache *cnf)
{
  struct skipcache *next;

  for (; cnf; cnf = next)
  {
    next = cnf->next;
    free(cnf);
  }
  return 0;
}
