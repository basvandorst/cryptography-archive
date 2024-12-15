/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * TODO:
 *  - compression
 *  - ip reassembly on input
 */

#include "config.h"

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include "skip_defs.h"
#include "memblk.h"
#include "skipcache.h"
#include "ipsp.h"
#include "ipsum.h"
#include "sequence.h"
#include "crypt.h"
#include "sign.h"
#include "random.h"

#define ALIGN4(len) (((len) + 3) & ~3)

ipsp_stat_t ipsp_stat;


/*
 * the first segment must contain the complete ip header + start of payload
 * and there must be ipsp_maxheadergrowth() bytes of memory in front
 * of the original ip header, ipsp_maxtrailergrowth() behind packet
 */
int ipsp_ip2ipsp(struct skipcache *c, struct memblk *ipblk)
{
  struct ip *ip = (struct ip *)BLKSTART(ipblk);
  int hdr_len = SKIP_HDRSIZE;	/* 8 bytes of IPSP header + next proto field */
  int trl_len = 0;	/* trailer length (padding + ICV) */
  int encaps;		/* encapsulated or transport mode */
  int payload_len;
  struct memblk payload;/* payload mem block (incl. ip hdr in encaps mode) */
  struct memblk digest;	/* pointer to beginning of authenticated data */
  u_char *padding_len;	/* pointer to padding length storage */
  struct memblk data;
  int Kp_Kij_len = 0, MI_len = 0, Kp_len = 0;
  u_char *MI = NULL;

  /* we do not enskip IPSP packets! */
  if (ip->ip_p == IPPROTO_IPSP)
    return 0;

  /* we send it encapsulated if it's configured or if it is a fragment */
  encaps = (c->flags & SKIP_ENCAPS) || (SKIP_NTOHS(ip->ip_off) & ~IP_DF);

  /* add 8 bytes to header if we are sequencing */
  if (c->enskip_SAID[0] & SAID_SEQ)
    hdr_len += sizeof(c->enskip_seqno);

  /* add length of encrypted Kp */
  if (c->enskip_SAID[0] & (SAID_CRYPT | SAID_AUTH))
  {
    int Kij_blk_len = crypt_blocklen(c->enskip_Kij_alg);

    Kp_len = crypt_keylen(c->enskip_Kp_alg);

    /* calculated length of encrypted Kp */
    Kp_Kij_len = Kp_len;
    if ((Kp_len % Kij_blk_len) != 0)
      Kp_Kij_len += Kij_blk_len - (Kp_len % Kij_blk_len);

    hdr_len += Kp_Kij_len;
  }

  /* add MI length (algorithm depending) */
  if (c->enskip_SAID[0] & SAID_CRYPT)
  {
    MI_len = crypt_MIlen(c->enskip_Kp_alg);
    hdr_len += MI_len;
  }

  payload = *ipblk;

  /* in encapsulated mode add an ip hdr */
  if (encaps)
  {
    ipsp_stat.encaps_out++;
    /* TODO: should we really throw the ip options away? */
    hdr_len += sizeof(struct ip);
  }
  else
  {
    /* skip ip header */
    BLKINC(&payload, (ip->ip_hl * 4));
  }

  /* Maximum bytes for this packet key reached, i.e. need to change key? */
  if ((c->enskip_Kp_maxbytes && (c->enskip_Kp_bytes >= c->enskip_Kp_maxbytes))
      || (c->enskip_Kp_ttl < 0))
  {
    random_randomKp(c);
  }
  c->enskip_Kp_bytes += payload.len;

  data = *ipblk;
  /* we have no need for the length field */
  BLKDEC(&data, hdr_len);

  /* copy or move the IP header */
  ip = (struct ip *)BLKSTART(&data);
  BLKADD(&data, BLKSTART(ipblk), sizeof(struct ip));

  /* add IPSP SAID field */
  BLKADD(&data, c->enskip_SAID, sizeof(c->enskip_SAID));

  /* add algrithm identifiers */
  BLKADDCHAR(&data, c->enskip_Kij_alg);
  BLKADDCHAR(&data, c->enskip_Kp_alg);
  BLKADDCHAR(&data, c->enskip_ICV_alg);
  BLKADDCHAR(&data, c->enskip_Comp_alg);

  /* add encrypted Kp */
  if (c->enskip_SAID[0] & (SAID_CRYPT | SAID_AUTH))
    BLKADD(&data, &c->data[c->enskip_Kp_Kij], Kp_Kij_len);


  /* add MI */
  if (c->enskip_SAID[0] & SAID_CRYPT)
  {
    MI = BLKSTART(&data);
    BLKINC(&data, MI_len);
  }

  /* next protocol field and three zero bytes */
  digest = data;
  digest.len = 4 + payload.len;
  BLKADDCHAR(&data, encaps ? IPPROTO_IP : ip->ip_p);
  padding_len = BLKSTART(&data);  /* Pos of padding len, filled in later... */
  BLKADDCHAR(&data, 0);
  BLKADDCHAR(&data, 0);
  BLKADDCHAR(&data, 0);

  /* add sequencing numbers if needed */
  if (c->enskip_SAID[0] & SAID_SEQ)
  {
    ipsp_stat.seq_out++;
    BLKADD(&data, c->enskip_seqno, sizeof(c->enskip_seqno));
    sequence_inc(c->enskip_seqno);
    digest.len += sizeof(c->enskip_seqno);
  }

  /* compression */
  if (c->enskip_SAID[0] & SAID_COMP)
  {
    /* TODO */
    ipsp_stat.comp_out++;
  }

  if (c->enskip_SAID[0] & SAID_CRYPT)
  {
    int Kp_blk_len = crypt_blocklen(c->enskip_Kp_alg);

    /* adding padding due to cipher block size */
    if ((payload.len % Kp_blk_len) > 0)
    {
      *padding_len = Kp_blk_len - (payload.len % Kp_blk_len);

      payload.len += *padding_len;
      digest.len += *padding_len;
    }

    payload_len = payload.len;

    /* After encrypt, payload will point behind actual payload */
    if (crypt_encrypt(c->enskip_Kp_alg, &c->data[c->encryptstate],
                      &c->data[c->enskip_Kp], MI, &c->data[c->enskip_MI],
                      &payload) < 0)
    {
      ipsp_stat.badKpalg++;
      return -1;
    }
    ipsp_stat.crypt_out++;
  }
  else
    payload_len = payload.len;

  /* add ICV */
  if (c->enskip_SAID[0] & SAID_AUTH)
  {
    int icvlen = sign_icvlen(c->enskip_ICV_alg);
    u_char ICV[MAXBLOCKLEN];

    trl_len += icvlen;




    if (sign_sign(c->enskip_ICV_alg, &c->data[c->enskip_Kp], Kp_len, 
                  &digest, ICV) == 0)
    { 
      /* Note: digest is incremented by sign, space for ICV in 1 memseg */
      MEMCPY(BLKSTART(&digest), ICV, icvlen);
    }
    else
    {
      ipsp_stat.badICValg++;
      return -1;
    }


    ipsp_stat.auth_out++;
  }

  /* fix clear IP header */
  ip->ip_hl = sizeof(struct ip) / 4;  /* normal header len (without options) */
  ip->ip_p = IPPROTO_IPSP;	/* IPSP packet */
  ip->ip_off = 0;		/* not fragmented */
  ip->ip_len = SKIP_HTONS(hdr_len + (encaps ? 0 : sizeof(struct ip)) + 
                          payload_len + trl_len);  /* packet length */
  ip->ip_sum = SKIP_HTONS(ipsum(ip));	/* calculate IP checksum */

  /* Adjust ipblk to reflect changes we made */
  BLKDEC(ipblk, hdr_len);
  ipblk->len = SKIP_NTOHS(ip->ip_len);


  ipsp_stat.out++;
  return 0;
}

/*
 * the first segment must contain the complete ipsp header + start of payload
 */
int ipsp_ipsp2ip(struct skipcache *c, struct memblk *ipblk, int dir)
{
  struct ip *ip = (struct ip *)BLKSTART(ipblk);
  struct memblk data;
  struct memblk Kpblk;
  struct memblk digest;
  struct memblk payload;
  u_char SAID, next_proto, *MI = NULL, *Kp = NULL;
  int Kij_alg, Kp_alg, ICV_alg, Comp_alg, Kp_Kij_len = 0, Kp_len = 0;
  int hdr_len = SKIP_HDRSIZE, trl_len = 0;
  int ip_len = (SKIP_NTOHS(ip->ip_len) - (ip->ip_hl * 4));
  int padding_len;
  int icvlen;
  u_short ip_sum = SKIP_NTOHS(ip->ip_sum);

  /* if it is no ipsp packet, do nothing */
  if (ip->ip_p != IPPROTO_IPSP)
    return 0;

  /* drop pkt if it is a fragment */
  if (SKIP_NTOHS(ip->ip_off) & ~IP_DF)
  {
    ipsp_stat.fragdrop++;
    return -1;  
  }

  /* is packet too short? (header excl. next proto field, reserved) */
  if (ip_len < (SKIP_HDRSIZE - 4))
  {
    ipsp_stat.badpktlen++;
    return -1;
  }
  ip_len -= (SKIP_HDRSIZE - 4);

  /* check ip checksum */
  if (ip_sum && (ipsum(ip) != ip_sum))
  {
    ipsp_stat.badipsum++;
    return -1;
  }

  data = *ipblk;
  BLKINC(&data, ip->ip_hl * 4);

  /* parse SAID */
  SAID = *BLKSTART(&data);
  BLKINC(&data, sizeof(skip_said_t));

  if ((SAID & c->deskip_policy[dir][0]) != c->deskip_policy[dir][0])
  {
    ipsp_stat.badpolicy++;
    return -1;
  }

  /* parse algorithm fields */
  BLKGETCHAR(&data, Kij_alg);
  BLKGETCHAR(&data, Kp_alg);
  BLKGETCHAR(&data, ICV_alg);
  BLKGETCHAR(&data, Comp_alg);

  /* get packet key Kp */
  if (SAID & (SAID_CRYPT | SAID_AUTH))
  {
    u_char Kij_MI[MAXBLOCKLEN];
    int Kij_blk_len = crypt_blocklen(Kij_alg);

    MEMZERO(Kij_MI, sizeof(Kij_MI));

    Kp_len = crypt_keylen(Kp_alg);

    /* check if key is padded */
    Kp_Kij_len = Kp_len;
    if ((Kp_len % Kij_blk_len) != 0)
      Kp_Kij_len += Kij_blk_len - (Kp_len % Kij_blk_len);

    /* check length */
    if (ip_len < Kp_Kij_len)
    {
      ipsp_stat.badpktlen++;
      return -1;
    }
    ip_len -= Kp_Kij_len;

    Kpblk = data;
    Kpblk.len = Kp_Kij_len;
    Kp = BLKSTART(&Kpblk);
    BLKINC(&data, Kp_Kij_len);
    hdr_len += Kp_Kij_len;

    if ((c->deskip_Kij_alg == Kij_alg) && (c->deskip_Kp_alg == Kp_alg) &&
        (MEMCMP(Kp, &c->data[c->deskip_Kp_Kij], Kp_Kij_len) == 0))
    {
      /* same Kp as last time */
      Kp = &c->data[c->deskip_Kp];
    }
    else
    {
      /* cache encrypted, decrypt & cache decrypted Kp */
      MEMCPY(&c->data[c->deskip_Kp_Kij], Kp, Kp_Kij_len);

      if (crypt_decrypt(Kij_alg, &c->data[c->Kp_decryptstate],
          &c->data[c->Kij], Kij_MI, &Kpblk) < 0)
      {
        ipsp_stat.badKijalg++;
        return -1;
      }

      MEMCPY(&c->data[c->deskip_Kp], Kp, Kp_len);
      c->deskip_Kij_alg = Kij_alg;
      c->deskip_Kp_alg = Kp_alg;
    }
  }

  /* get MI */
  if (SAID & SAID_CRYPT)
  {
    int MI_len = crypt_MIlen(Kp_alg);

    /* length check */
    if (ip_len < MI_len)
    {
      ipsp_stat.badpktlen++;
      return -1;
    }
    ip_len -= MI_len;

    MI = BLKSTART(&data);
    BLKINC(&data, MI_len);
    hdr_len += MI_len;
  }

  /* parse next protocol field */
  if (ip_len < 4)
  {
    ipsp_stat.badpktlen++;
    return -1;
  }
  ip_len -= 4;

  digest = data;
  BLKGETCHAR(&data, next_proto);
  BLKGETCHAR(&data, padding_len);
  BLKINC(&data, 2);  /* Skip 2 reserved bytes */

  /* check sequence numbers */
  if (SAID & SAID_SEQ)
  {
    if (ip_len < sizeof(skip_seq_t))
    {
      ipsp_stat.badpktlen++;
      return -1;
    }
    ip_len -= sizeof(skip_seq_t);

    if (sequence_check(&c->deskip_seqhistory,
                       &c->data[c->deskip_seqhistorybuf], BLKSTART(&data)) < 0)
    {
      ipsp_stat.badseq++;
      return -1;
    }

    BLKINC(&data, sizeof(skip_seq_t));
    hdr_len += sizeof(skip_seq_t);
    ipsp_stat.seq_in++;
  }

  icvlen = 0;
  if (SAID & SAID_AUTH)
  {
    icvlen = sign_icvlen(ICV_alg);
    trl_len += icvlen;
  }

  payload = data;
  payload.len -= trl_len;

  /* final length check */
  if (payload.len <= 0)
  {
    ipsp_stat.badpktlen++;
    return -1;
  }


  if (SAID & SAID_AUTH)
  {
    u_char ICV[32];  /* ICV is assumed to be <= 256 bit :-) */

    digest.len = (BLKSTART(&payload) - BLKSTART(&digest)) + payload.len;


    if (sign_sign(ICV_alg, Kp, Kp_len, &digest, ICV) == 0)
    {
      u_char *icv = BLKSTART(&digest);  /* ICV is behind actual payload */
      struct memseg *ms = digest.ms;
      int i, l;


      for (i = 0, l = ms->len - digest.offset; i < icvlen; i++)
      {
        if (*icv++ != ICV[i])
	{
	  ipsp_stat.badICV++;
	  return -1;
	}

        if (--l == 0)  /* End of segment? */
        {
	  ms++;
	  icv = ms->ptr;
	  l = ms->len;
        }
      }

      ipsp_stat.auth_in++;
    }
    else
    {
      ipsp_stat.badICV++;
      return -1;
    }
  }



  /* New IP packet will be returned in ipblk */
  *ipblk = payload;


  if (SAID & SAID_CRYPT)
  {

    if ((payload.len % crypt_blocklen(Kp_alg)) != 0)
    {
      ipsp_stat.badblklen++;
      return -1;
    }

    if (crypt_decrypt(Kp_alg, &c->data[c->payload_decryptstate], Kp, MI, 
                      &payload) == 0)
    {
      ipsp_stat.crypt_in++;
    }
    else
    {
      ipsp_stat.badKpalg++;
      return -1;
    }
  }


  if (SAID & SAID_COMP)
  {
    /* TODO: decompress payload */
     ipsp_stat.comp_in++;
  }

  ipblk->len -= padding_len;

  if (next_proto != IPPROTO_IP)
  {
    /* Copy old ip header including options */
    BLKDEC(ipblk, ip->ip_hl * 4);
    MEMCPY(BLKSTART(ipblk), ip, ip->ip_hl * 4);
    ip = (struct ip *)BLKSTART(ipblk);
    ip->ip_len = SKIP_HTONS(ipblk->len);
    ip->ip_p = next_proto;
    ip->ip_sum = SKIP_HTONS(ipsum(ip));
  }
  else
  {
    ipsp_stat.encaps_in++;  /* ipblk->len == NTOHS(ip->ip_len) */
    ip = (struct ip *)BLKSTART(ipblk);
    if ((ip->ip_v != IPVERSION) || (ipsum(ip) != SKIP_NTOHS(ip->ip_sum)))
    {
      ipsp_stat.badipsum++;
      return -1;  /* Minimal sanity checks failed, drop */
    }
  }

  ipsp_stat.in++;


  return 0;
}

int ipsp_maxheadergrowth()
{
  return ALIGN4(sizeof(struct ip) + SKIP_HDRSIZE + sizeof(skip_seq_t) +
         crypt_MIlen(CRYPTALG_MAX) + crypt_keylen(CRYPTALG_MAX));
}
int ipsp_maxtrailergrowth()
{
  return (crypt_blocklen(CRYPTALG_MAX) - 1 + sign_icvlen(SIGNALG_MAX));
}

int ipsp_init()
{
  MEMZERO(&ipsp_stat, sizeof(ipsp_stat));
  return 0;
}

int ipsp_exit()
{
  return 0;
}
