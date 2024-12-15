/*----------------------------------------------------------------------
 * keysock.c  --  Functions which implement the PF_KEY socket for key and
 *                security association updates.  The key and security
 *                association tables themselves are in another file.
 *                
 * 
 * Copyright 1995 by Bao Phan,  Randall Atkinson, & Dan McDonald,
 * All Rights Reserved.  All Rights have been assigned to the US
 * Naval Research Laboratory (NRL).  The NRL Copyright Notice and
 * License governs distribution and use of this software.
 *
 * Patents are pending on this technology.  NRL grants a license
 * to use this technology at no cost under the terms below with
 * the additional requirement that software, hardware, and 
 * documentation relating to use of this technology must include
 * the note that:
 *    	This product includes technology developed at and
 *	licensed from the Information Technology Division, 
 *	US Naval Research Laboratory.
 *
 ----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1a (NRL) 17 August 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL also has copyright on the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

	This product includes software developed at the Information
	Technology Division, US Naval Research Laboratory.

4. Neither the name of the NRL nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THE SOFTWARE PROVIDED BY NRL IS PROVIDED BY NRL AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL NRL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of the US Naval
Research Laboratory (NRL).

----------------------------------------------------------------------*/

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>

#include <net/raw_cb.h>

#include <netinet/in.h>
#include <netinet6/in6.h>
#include <netinet6/ipv6.h>

#include <netinet6/ipsec.h>
#include <netinet6/key.h>

#include <netinet6/in6_debug.h>

extern struct keyso_cb keyso_cb;

/*
 * Definitions of protocols supported in the KEY domain.
 */

struct	sockaddr key_dst = { 2, PF_KEY, };
struct	sockaddr key_src = { 2, PF_KEY, };
struct	sockproto key_proto = { PF_KEY, };

/*
 * Debug routines -- should be moved to in6_debug.c
 */

/*----------------------------------------------------------------------
 * (temporary) Dump an ipsec_assoc structure
 ----------------------------------------------------------------------*/
void
dump_secassoc(seca)
     struct ipsec_assoc *seca;
{
  u_int8 *p;
  int i;

  if (seca) {
    printf("secassoc_len=%u ", seca->len);
    printf("secassoc_type=%d ", seca->type);
    printf("secassoc_state=0x%x\n", seca->state);
    printf("secassoc_label=%u ", seca->label);
    printf("secassoc_spi=0x%x ", seca->spi);
    printf("secassoc_keylen=%u\n", seca->keylen);
    printf("secassoc_ivlen=%u ", seca->ivlen);
    printf("secassoc_algorithm=%u ", seca->algorithm);
    printf("secassoc_lifetype=%u\n", seca->lifetype);
    printf("secassoc_iv=0x%x:\n", seca->iv);
    p = (u_int8 *)(seca->iv);
    for (i = 0 ; i < seca->ivlen; i++)
      printf("0x%x ", *(p + i));
    printf("secassoc_key=0x%x:\n", seca->key);
    p = (u_int8 *)(seca->key);
    for (i = 0 ; i < seca->keylen; i++)
      printf("0x%x ", *(p + i));
    printf("secassoc_lifetime1=%u ", seca->lifetime1);
    printf("secassoc_lifetime2=%u\n", seca->lifetime2);
    dump_smart_sockaddr(&(seca->src));
    dump_smart_sockaddr(&(seca->dst));
    dump_smart_sockaddr(&(seca->from));
  } else
    printf("can't dump null secassoc pointer!\n");
}


/*----------------------------------------------------------------------
 * (temporary) Dump a key_msghdr structure
 ----------------------------------------------------------------------*/
void
dump_keymsghdr(km)
     struct key_msghdr *km;
{
  if (km) {
    printf("key_msglen=%d\n", km->key_msglen);
    printf("key_msgvers=%d\n", km->key_msgvers);
    printf("key_msgtype=%d\n", km->key_msgtype);    
    printf("key_pid=%d\n", km->key_pid);
    printf("key_seq=%d\n", km->key_seq);
    printf("key_errno=%d\n", km->key_errno);
    printf("type=0x%x\n", km->type);
    printf("state=0x%x\n", km->state);
    printf("label=0x%x\n", km->label);
    printf("spi=0x%x\n", km->spi);
    printf("keylen=%d\n", km->keylen);
    printf("ivlen=%d\n", km->ivlen);
    printf("algorithm=%d\n", km->algorithm);
    printf("lifetype=0x%x\n", km->lifetype);
    printf("lifetime1=%u\n", km->lifetime1);
    printf("lifetime2=%u\n", km->lifetime2);
  } else
    printf("key_msghdr pointer is NULL!\n");
}


/*----------------------------------------------------------------------
 * (temporary) Dump a key_msgdata structure
 ----------------------------------------------------------------------*/
void
dump_keymsginfo(kp)
     struct key_msgdata *kp;
{
  int i;

  if (kp) {
    printf("src addr:\n");
    dump_smart_sockaddr(kp->src);
    printf("dest addr:\n");
    dump_smart_sockaddr(kp->dst);
    printf("from addr:\n");
    dump_smart_sockaddr(kp->from);
#define dumpbuf(a, b) \
    { for (i= 0; i < (b); i++) \
      printf("0x%2x%s", (unsigned char)(*((caddr_t)a+i)),((i+1)%16)?" ":"\n");\
      printf("\n"); }
    printf("iv is:\n");
    dumpbuf(kp->iv, kp->ivlen);
    printf("key is:\n");
    dumpbuf(kp->key, kp->keylen);
#undef dumpbuf    
  } else
    printf("key_msgdata point is NULL!\n");
}


/*----------------------------------------------------------------------
 * key_init():
 *      Init routine for key socket and key engine
 ----------------------------------------------------------------------*/
void
key_init()
{
  DPRINTF(IDL_EVENT,("Called key_init().\n"));
  key_inittables();
  bzero((char *)&keyso_cb, sizeof(keyso_cb));
}


/*----------------------------------------------------------------------
 * key_cbinit():
 *      Control block init routine for key socket
 ----------------------------------------------------------------------*/
void
key_cbinit()
{
 /*
  *  This is equivalent to raw_init for the routing socket. 
  *  The key socket uses the same control block as the routing 
  *  socket.
  */
  DPRINTF(IDL_EVENT,("Called key_cbinit().\n"));
}


/*----------------------------------------------------------------------
 * key_xdata():
 *      Parse message buffer for src/dst/from/iv/key if parseflag = 0
 *      else parse for src/dst only.
 ----------------------------------------------------------------------*/
int
key_xdata(km, len, kip, parseflag)
     struct key_msghdr *km;
     int len;
     struct key_msgdata *kip;
     int parseflag;
{
  char *cp, *cpmax;

  if (!km || (len <= 0))
    return (-1);

  cp = (caddr_t)(km + 1);
  cpmax = (caddr_t)km + len;

  /*
   * Assumes user process passes message with 
   * correct word alignment.
   */

#define ROUNDUP(a) \
  ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) \
  { x += ROUNDUP(n); if (cp > cpmax) return(-1); }

  /* 
   * Need to clean up this code later.  
   */

  /* Grab src addr */
  kip->src = (struct sockaddr *)cp;
  if (!kip->src->sa_len) {
    DPRINTF(IDL_MAJOR_EVENT,("key_xdata couldn't parse src addr\n"));
    return(-1);
  }

  ADVANCE(cp, kip->src->sa_len);

  /* Grab dest addr */
  kip->dst = (struct sockaddr *)cp;
  if (!kip->dst->sa_len) {
    DPRINTF(IDL_MAJOR_EVENT,("key_xdata couldn't parse dest addr\n"));
    return(-1);
  }

  ADVANCE(cp, kip->dst->sa_len);
  if (parseflag == 1) {
    kip->from = 0;
    kip->key = kip->iv = 0;
    kip->keylen = kip->ivlen = 0;
    return(0);
  }
 
  /* Grab from addr */
  kip->from = (struct sockaddr *)cp;
  if (!kip->from->sa_len) {
    DPRINTF(IDL_MAJOR_EVENT,("key_xdata couldn't parse from addr\n"));
    return(-1);
  }

  ADVANCE(cp, kip->from->sa_len);
 
  /* Grab key */
  if (kip->keylen = km->keylen) {
    kip->key = cp;
    ADVANCE(cp, km->keylen);
  } else 
    kip->key = 0;

  /* Grab iv */
  if (kip->ivlen = km->ivlen) 
    kip->iv = cp;
  else
    kip->iv = 0;

  return (0);
}


/*----------------------------------------------------------------------
 * key_output():
 *      Process outbound pf_key message.
 ----------------------------------------------------------------------*/
int
key_output(m, so)
     register struct mbuf *m;
     struct socket *so;
{
  struct key_msghdr *km = 0;
  struct key_msgdata keyinfo;
  struct ipsec_assoc *secassoc = NULL;
  caddr_t cp, cplimit;
  int len;
  int error = 0;
  int keyerror = 0;

  DPRINTF(IDL_EVENT,("key_output() got a message len=%d.\n", m->m_pkthdr.len));

#define senderr(e) \
  { error = (e); goto flush; }

  if (m == 0 || ((m->m_len < sizeof(long)) && 
		 (m = m_pullup(m, sizeof(long))) == 0)) {
    DPRINTF(IDL_CRITICAL,("key_output can't pullup mbuf\n"));
    return (ENOBUFS);
  }
  if ((m->m_flags & M_PKTHDR) == 0)
    panic("key_output");

  DDO(IDL_FINISHED,dump_mbuf(m));  
  
  len = m->m_pkthdr.len;
  if (len < sizeof(*km) || len != mtod(m, struct key_msghdr *)->key_msglen) {
    DPRINTF(IDL_CRITICAL,("keyout: Invalid length field/length mismatch!\n"));
    senderr(EINVAL); 
  }
  K_Malloc(km, struct key_msghdr *, len); 
  if (km == 0) {
    DPRINTF(IDL_CRITICAL,("keyoutput: Can't malloc memory!\n"));
    senderr(ENOBUFS);
  }

  m_copydata(m, 0, len, (caddr_t)km);
  if (km->key_msgvers != KEY_VERSION) {
    DPRINTF(IDL_CRITICAL,("keyoutput: Unsupported key message version!\n"));
    senderr(EPROTONOSUPPORT);
  }

/*   if (km->type > IPSEC_TYPE_MAX) {
    DPRINTF(IDL_CRITICAL,("keyout: Unsupported security assocation type!\n"));
    senderr(EPROTONOSUPPORT);
  }
*/

  km->key_pid = curproc->p_pid; 

  DDO(IDL_FINISHED, printf("keymsghdr:\n"); dump_keymsghdr(km));

  /*
   * Parse buffer for src addr, dest addr, from addr, key, iv
   */
  bzero((char *)&keyinfo, sizeof(keyinfo));
  if ((km->key_msgtype == KEY_ADD) || (km->key_msgtype == KEY_UPDATE)) {
    if (key_xdata(km, len, &keyinfo, 0) < 0) {
      DPRINTF(IDL_MAJOR_EVENT,("Can't parse src/dst/from/key/iv fields!\n"));
      bzero((char *)&keyinfo, sizeof(keyinfo));
      senderr(EINVAL);
    }
    DDO(IDL_FINISHED, printf("keydata:\n"); dump_keymsginfo(&keyinfo));
  } else if ((km->key_msgtype == KEY_DELETE) || (km->key_msgtype == KEY_GET) ||
	     (km->key_msgtype == KEY_GETSPI)) {
    if (key_xdata(km, len, &keyinfo, 1) < 0) {
      DPRINTF(IDL_MAJOR_EVENT,("Can't parse src/dst fields!\n"));
      bzero((char *)&keyinfo, sizeof(keyinfo));
      senderr(EINVAL);
    }
    DDO(IDL_FINISHED, printf("keydata:\n"); dump_keymsginfo(&keyinfo));
  }

  switch (km->key_msgtype) {
  case KEY_ADD:
    DPRINTF(IDL_EVENT,("key_output got KEY_ADD msg\n"));

    /*
     * Allocate the secassoc structure to insert 
     * into key table here.
     */
    K_Malloc(secassoc, struct ipsec_assoc *, sizeof(struct ipsec_assoc)); 
    if (secassoc == 0) {
      DPRINTF(IDL_CRITICAL,("keyoutput: No more memory!\n"));
      senderr(ENOBUFS);
    }

    if (key_msghdr2secassoc(secassoc, km, &keyinfo) < 0) {
      DPRINTF(IDL_CRITICAL,("keyoutput: key_msghdr2secassoc failed!\n"));
      KFree(secassoc);
      senderr(EINVAL);
    }
    DPRINTF(IDL_EVENT,("secassoc to add:\n"));
    DDO(IDL_EVENT,dump_secassoc(secassoc));

    if ((keyerror = key_add(secassoc)) != 0) {
      DPRINTF(IDL_CRITICAL,("keyoutput: key_add failed\n"));
      if (secassoc->key)
	KFree(secassoc->key);
      if (secassoc->iv)
	KFree(secassoc->iv);
      KFree(secassoc);
      if (keyerror == -2) {
	senderr(EEXIST);
      } else {
	senderr(ENOBUFS);
      }
    }
    break;
  case KEY_DELETE:
    DPRINTF(IDL_EVENT,("key_output got KEY_DELETE msg\n"));
    K_Malloc(secassoc, struct ipsec_assoc *, sizeof(struct ipsec_assoc)); 
    if (secassoc == 0) {
      senderr(ENOBUFS);
    }
    if (key_msghdr2secassoc(secassoc, km, &keyinfo) < 0) {
      KFree(secassoc);
      senderr(EINVAL);
    }
    if (key_delete(secassoc) != 0) {
      if (secassoc->iv)
	KFree(secassoc->iv);
      if (secassoc->key)
	KFree(secassoc->key);
      KFree(secassoc);
      senderr(ESRCH);
    }
    if (secassoc->iv)
      KFree(secassoc->iv);
    if (secassoc->key)
      KFree(secassoc->key);
    KFree(secassoc);
    break;
  case KEY_UPDATE:
    DPRINTF(IDL_EVENT,("key_output got KEY_UPDATE msg\n"));
    K_Malloc(secassoc, struct ipsec_assoc *, sizeof(struct ipsec_assoc)); 
    if (secassoc == 0) {
      senderr(ENOBUFS);
    }
    if (key_msghdr2secassoc(secassoc, km, &keyinfo) < 0) {
      KFree(secassoc);
      senderr(EINVAL);
    }
    if ((keyerror = key_update(secassoc)) != 0) {
      DPRINTF(IDL_CRITICAL,("Error updating key entry\n"));
      if (secassoc->iv)
	KFree(secassoc->iv);
      if (secassoc->key)
	KFree(secassoc->key);
      KFree(secassoc);
      senderr(keyerror);
    }
    KFree(secassoc);
    break;
  case KEY_GET:
    DPRINTF(IDL_EVENT,("key_output got KEY_GET msg\n"));

    if (key_get(km->type, (struct sockaddr *)keyinfo.src, 
		(struct sockaddr *)keyinfo.dst, 
		km->spi, &secassoc) != 0) {
      DPRINTF(IDL_EVENT,("keyoutput: can't get key\n"));
      senderr(ESRCH);
    }

    if (secassoc) {
      int newlen;

      DPRINTF(IDL_EVENT,("keyoutput: Found secassoc!\n"));
      newlen = sizeof(struct key_msghdr) + ROUNDUP(secassoc->src.sin6_len) + 
	ROUNDUP(secassoc->dst.sin6_len) + ROUNDUP(secassoc->from.sin6_len) +
	  ROUNDUP(secassoc->keylen) + ROUNDUP(secassoc->ivlen);
      DPRINTF(IDL_EVENT,("keyoutput: newlen=%d\n", newlen));
      if (newlen > len) {
	struct key_msghdr *newkm;

	DPRINTF(IDL_EVENT,("keyoutput: Allocating new buffer!\n"));
	K_Malloc(newkm, struct key_msghdr *, newlen); 
	if (newkm == 0) {
	  senderr(ENOBUFS);
	}
	bcopy((char *)km, (char *)newkm, len);
	DPRINTF(IDL_FINISHED,("keyoutput: 1\n"));
	KFree(km);
	km = newkm; 
	km->key_msglen = len = newlen;
      }
      DPRINTF(IDL_FINISHED,("keyoutput: 2\n"));
      if (key_secassoc2msghdr(secassoc, km, &keyinfo)) {
	DPRINTF(IDL_CRITICAL,("keyoutput: Can't create msghdr!\n"));
	senderr(EINVAL);
      }
      DPRINTF(IDL_FINISHED,("keyoutput: 3\n"));
    }
    break;
  case KEY_GETSPI:
    DPRINTF(IDL_EVENT,("key_output got KEY_GETSPI msg\n"));
    if (key_getspi(km->type, keyinfo.src, keyinfo.dst, &(km->spi)) != 0) {
      DPRINTF(IDL_CRITICAL,("getspi failed\n"));
      senderr(EINVAL);
    }
    break;
  case KEY_REGISTER:
    DPRINTF(IDL_EVENT,("key_output got KEY_REGISTER msg\n"));
    key_register(so, km->type);
    break;
  case KEY_DUMP:
    DPRINTF(IDL_EVENT,("key_output got KEY_DUMP msg\n"));
    error = key_dump(so);
    if (km)
      KFree(km);
    m_freem(m);
    return(error);
    break;
  case KEY_FLUSH:
    DPRINTF(IDL_EVENT,("key_output got KEY_FLUSH msg\n"));
    key_flush();
    break;
  default:
    DPRINTF(IDL_CRITICAL,("key_output got unsupported msg type=%d\n", 
			     km->key_msgtype));
    senderr(EOPNOTSUPP);
  }

flush:  
  if (km) {
    if (error) {
      DPRINTF(IDL_MAJOR_EVENT, ("key_output exiting with error=%d\n", error));
      km->key_errno = error;
    }
    else
      km->key_errno = 0;
  }

  {
    struct rawcb *rp = 0;

    if ((so->so_options & SO_USELOOPBACK) == 0) {
      if (keyso_cb.any_count <= 1) {
	if (km)
	  KFree(km);
	m_freem(m);
	return (error);
      }
      rp = sotorawcb(so);
    }

    key_proto.sp_protocol = keyinfo.dst ? keyinfo.dst->sa_family : 0;

    if (km) {
      m_copyback(m, 0, km->key_msglen, (caddr_t) km);
      KFree(km);
    }

    if (rp)
      rp->rcb_proto.sp_family = 0;   /* Prevent us from receiving message */

    raw_input(m, &key_proto, &key_src, &key_dst);

    if (rp)
      rp->rcb_proto.sp_family = PF_KEY;
  }
  return (error);
}


/*----------------------------------------------------------------------
 * key_usrreq():
 *      Handles PRU_* for pf_key sockets.
 ----------------------------------------------------------------------*/
int
key_usrreq(so, req, m, nam, control)
	register struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
  register int error = 0;
  register struct rawcb *rp = sotorawcb(so);
  int s;

  DPRINTF(IDL_EVENT,("Entering key_usrreq, req = %d.\n",req));

  if (req == PRU_ATTACH) {
    MALLOC(rp, struct rawcb *, sizeof(*rp), M_PCB, M_WAITOK);
    if (so->so_pcb = (caddr_t)rp)
      bzero(so->so_pcb, sizeof(*rp));
  }

  if (req == PRU_DETACH && rp) {
    int af = rp->rcb_proto.sp_protocol;
    if (af == AF_INET)
      keyso_cb.ip4_count--;
    else if (af == AF_INET6)
      keyso_cb.ip6_count--;
    keyso_cb.any_count--;
  }
  s = splnet();
  error = raw_usrreq(so, req, m, nam, control);
  rp = sotorawcb(so);

  if (req == PRU_ATTACH && rp) {
    int af = rp->rcb_proto.sp_protocol;
    if (error) {
      free((caddr_t)rp, M_PCB);
      splx(s);
      return error;
    }
    if (af == AF_INET)
      keyso_cb.ip4_count++;
    else if (af == AF_INET6)
      keyso_cb.ip6_count++;
    keyso_cb.any_count++;
    rp->rcb_faddr = &key_src;
    soisconnected(so);   /* Key socket, like routing socket, must be
			    connected. */

    /* Possibly set other needed flags/options at creation time in here. */
    so->so_options |= SO_USELOOPBACK; /* Like routing socket, we turn this */
                                      /* on by default                     */
  }
  splx(s);
  return error;
}


/*
 * Protoswitch entry for pf_key 
 */

extern	struct domain keydomain;		/* or at least forward */

struct protosw keysw[] = {
{ SOCK_RAW,	&keydomain,	0,		PR_ATOMIC|PR_ADDR,
  raw_input,	key_output,	raw_ctlinput,	0,
  key_usrreq,
  key_cbinit,	0,		0,		0,
  0,
}
};

struct domain keydomain =
    { PF_KEY, "key", key_init, 0, 0,
      keysw, &keysw[sizeof(keysw)/sizeof(keysw[0])] };
