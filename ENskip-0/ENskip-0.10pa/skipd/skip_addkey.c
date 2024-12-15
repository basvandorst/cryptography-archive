/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include "config.h"

#include <time.h>

#include "skip_defs.h"
#include "skipcache.h"
#include "memblk.h"
#include "ipsp.h"
#include "com.h"
#include "crypt.h"
#include "sign.h"
#include "iaddr.h"

#define USAGE	\
"Usage: skip_addkey [-ensv] [-t ttl] [-a ICValg] [-c Kpalg] [-i Kijalg]\n" \
"                   [-r seq_range] address [Kij [Kp]]\n"

#define TTL_DEF		 60
#define MAXTTL_DEF	 60
#define ICVALG_DEF	 SIGNALG_MD5
#define KPALG_DEF	 CRYPTALG_DES
#define KIJALG_DEF	 CRYPTALG_DES
#define SEQRANGE_DEF     8
#define FILTERBEFOREOUT_DEF FILTER_NONE
#define FILTERBEFOREIN_DEF  FILTER_NONE
#define FILTERAFTEROUT_DEF  FILTER_NONE
#define FILTERAFTERIN_DEF   FILTER_NONE
#define ENSKIPMODEOUT_DEF   ENSKIPMODE_ALL
#define ENSKIPMODEIN_DEF    ENSKIPMODE_NONE
#define DESKIPMODEOUT_DEF   DESKIPMODE_NONE
#define DESKIPMODEIN_DEF    DESKIPMODE_DESKIP
#define DESKIPPOLICYOUT_DEF 0
#define DESKIPPOLICYIN_DEF  0

#define ALIGN4(len) (((len) + 3) & ~3)

int do_default(struct skipcache *c)
{
  if (c->ttl == 0)
    c->ttl = TTL_DEF;
  if (c->maxttl == 0)
    c->maxttl = MAXTTL_DEF;
  if (c->enskip_ICV_alg == 0)
    c->enskip_ICV_alg = ICVALG_DEF;
  if (c->enskip_Kp_alg == 0)
    c->enskip_Kp_alg = KPALG_DEF;
  if (c->enskip_Kij_alg == 0)
    c->enskip_Kij_alg = KIJALG_DEF;
  if (c->deskip_seqhistory.len == 0)
    c->deskip_seqhistory.len = SEQRANGE_DEF;

  if (c->filter_before[SKIP_OUTPUT] == 0)
    c->filter_before[SKIP_OUTPUT] = FILTERBEFOREOUT_DEF;
  if (c->filter_before[SKIP_INPUT] == 0)
    c->filter_before[SKIP_INPUT] = FILTERBEFOREIN_DEF;
  if (c->filter_after[SKIP_OUTPUT] == 0)
    c->filter_after[SKIP_OUTPUT] = FILTERAFTEROUT_DEF;
  if (c->filter_after[SKIP_INPUT] == 0)
    c->filter_after[SKIP_INPUT] = FILTERAFTERIN_DEF;

  if (c->enskip_mode[SKIP_OUTPUT] == 0)
    c->enskip_mode[SKIP_OUTPUT] = ENSKIPMODEOUT_DEF;
  if (c->enskip_mode[SKIP_INPUT] == 0)
    c->enskip_mode[SKIP_INPUT] = ENSKIPMODEIN_DEF;
  if (c->deskip_mode[SKIP_OUTPUT] == 0)
    c->deskip_mode[SKIP_OUTPUT] = DESKIPMODEOUT_DEF;
  if (c->deskip_mode[SKIP_INPUT] == 0)
    c->deskip_mode[SKIP_INPUT] = DESKIPMODEIN_DEF;
  if (c->deskip_policy[SKIP_OUTPUT] == 0)
    c->deskip_policy[SKIP_OUTPUT][0] = DESKIPPOLICYOUT_DEF;
  if (c->deskip_policy[SKIP_INPUT] == 0)
    c->deskip_policy[SKIP_INPUT][0] = DESKIPPOLICYIN_DEF;

  return 0;
}

int cryptinit(struct skipcache *c, u_char **data)
{
  int maxdslen = crypt_decryptstatelen(CRYPTALG_MAX);
  int MI_len = crypt_MIlen(c->enskip_Kp_alg);

  c->enskip_MI = (*data - c->data);
  MEMZERO(*data, MI_len);		/* for rc4! */
  *data += ALIGN4(MI_len);

  c->encryptstate = (*data - c->data);
  MEMZERO(*data, crypt_encryptstatelen(c->enskip_Kp_alg));
  *data += ALIGN4(crypt_encryptstatelen(c->enskip_Kp_alg));
  c->payload_decryptstate = (*data - c->data);
  MEMZERO(*data, maxdslen);
  *data += ALIGN4(maxdslen);

  return 0;
}

int keyinit(struct skipcache *c, u_char **data, u_char *Kij, int Kij_len, u_char *Kp, int Kp_len)
{
  int maxkeylen = crypt_keylen(CRYPTALG_MAX);
  int maxdslen = crypt_decryptstatelen(CRYPTALG_MAXBLOCK);
  u_char MI[MAXBLOCKLEN];
  u_char decryptstate[8192];
  struct memblk mblk;
  struct memseg mseg;

  MEMZERO(MI, sizeof(MI));
  MEMZERO(decryptstate, sizeof(decryptstate));

  c->Kij = (*data - c->data);
  MEMZERO(*data, maxkeylen);
  MEMCPY(*data, Kij, (Kij_len > maxkeylen) ? maxkeylen : Kij_len);
  *data += ALIGN4(maxkeylen);
  c->enskip_Kp = (*data - c->data);
  MEMZERO(*data, maxkeylen);
  MEMCPY(*data, Kp, (Kp_len > maxkeylen) ? maxkeylen : Kp_len);
  *data += ALIGN4(maxkeylen);
  c->enskip_Kp_Kij = (*data - c->data);
  MEMZERO(*data, maxkeylen);
  MEMCPY(*data, Kp, (Kp_len > maxkeylen) ? maxkeylen : Kp_len);
  mblk.ms = &mseg;
  mblk.len = crypt_keylen(c->enskip_Kp_alg);
  mblk.offset = 0;
  mseg.ptr = *data;
  mseg.len = mblk.len;
  if (crypt_encrypt(c->enskip_Kij_alg, decryptstate, &c->data[c->Kij], MI, MI, &mblk) < 0)
    return -1;

  *data += ALIGN4(maxkeylen);

  c->deskip_Kp = (*data - c->data);
  *data += ALIGN4(maxkeylen);
  c->deskip_Kp_Kij = (*data - c->data);
  *data += ALIGN4(maxkeylen);
  c->Kp_decryptstate = (*data - c->data);
  *data += ALIGN4(maxdslen);

  return 0;
}

static int seqhistoryinit(struct skipcache *c, u_char **data)
{
  MEMZERO(c->deskip_seqhistory.last, sizeof(c->deskip_seqhistory.last));
  c->deskip_seqhistory.index = 0;
  c->deskip_seqhistorybuf = (*data - c->data);
  MEMZERO(*data, c->deskip_seqhistory.len);
  *data += ALIGN4(c->deskip_seqhistory.len);

  return 0;
}

int main(int argc, char *argv[], char *envp[])
{
  extern int optind;
  extern char *optarg;
  u_char buffer[4096];
  u_char *data = (buffer + ALIGN4(sizeof(struct skipcache)));
  u_long addr;
  struct skipcache *c = (struct skipcache *)buffer;
  int ch, verbose = 0, errflg = 0;
  int flags = SKIP_VALIDKEY;

  if (com_init() < 0)
  {
    fprintf(stderr, "%s: SKIP module not found\n", argv[0]);
    return 1;
  }

  crypt_init();

  MEMZERO(c, sizeof(*c));
  c->enskip_SAID[0] = SAID_VERSION;

  while ((ch = getopt(argc, argv, "a:c:ei:nr:st:v")) != EOF)
  {
    switch (ch)
    {
      case 'a':
        c->enskip_SAID[0] |= SAID_AUTH;
        c->enskip_ICV_alg = atoi(optarg);
        break;

      case 'e':
        c->flags |= SKIP_ENCAPS;
        break;

      case 'c':
        c->enskip_SAID[0] |=  SAID_CRYPT;
        c->enskip_Kp_alg = atoi(optarg);
        break;

      case 'i':
        c->enskip_Kij_alg = atoi(optarg);
        break;

      case 'n':
        flags = SKIP_NOKEY;
        break;

      case 'r':
        c->deskip_seqhistory.len = atoi(optarg);
        break;

      case 's':
        c->enskip_SAID[0] |= SAID_SEQ;
        break;

      case 't':
        c->ttl = atoi(optarg);
        break;

      case 'v':
        verbose++;
        break;

      default:
        errflg++;
        break;
    }
  }

  if (errflg || (optind == argc))
  {
    fprintf(stderr, USAGE);
    crypt_exit();
    return 1;
  }

  do_default(c);

  MEMZERO(c->IPaddr, sizeof(c->IPaddr));
  addr = iaddr(argv[optind++]);
  MEMCPY(c->IPaddr, &addr, sizeof(addr));

  c->data = (((u_char *)c) + sizeof(*c));

  seqhistoryinit(c, &data);

  /* Kij? -> keyalloc */
  if (((argc - optind) > 0) && (c->enskip_SAID[0] & (SAID_CRYPT | SAID_AUTH)))
  {
    u_char *Kp, rnd[1024];
    int Kp_len;

    if ((argc - optind) > 1)
    {
      Kp = (u_char *)argv[optind + 1];
      Kp_len = strlen(Kp);
    }
    else
    {
      int walk;

      srand(time(0));
      for (walk = 0; walk < sizeof(rnd); walk++)
        rnd[walk] = (u_char)(rand() & 0xff);

      Kp = rnd;
      Kp_len = sizeof(rnd);
    }

    keyinit(c, &data, argv[optind], strlen(argv[optind]), Kp, Kp_len);
    c->flags |= SKIP_VALIDKEY;

    if (c->enskip_SAID[0] & SAID_CRYPT)
      cryptinit(c, &data);

  }
  else
    c->flags |= flags;

  /* Sanity checks */
  if ((c->enskip_SAID[0] & (SAID_AUTH | SAID_SEQ)) == SAID_SEQ)
    fprintf(stderr, "ALERT: Sequencing without authentication is considered harmful.\n");

  c->datalen = (data - c->data);

  if (com_update(c) < 0)
  {
    printf("Update failed.\n");
    return 1;
  }
  else
  {
    printf("Successful update.\n");
    return 0;
  }
}
