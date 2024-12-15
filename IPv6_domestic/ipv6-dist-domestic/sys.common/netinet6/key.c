/*----------------------------------------------------------------------
  key.c :         Key Management Engine for BSD

  Copyright 1995 by Bao Phan,  Randall Atkinson, & Dan McDonald,
  All Rights Reserved.  All Rights have been assigned to the US
  Naval Research Laboratory (NRL).  The NRL Copyright Notice and
  License governs distribution and use of this software.

  Patents are pending on this technology.  NRL grants a license
  to use this technology at no cost under the terms below with
  the additional requirement that software, hardware, and 
  documentation relating to use of this technology must include
  the note that:
     	This product includes technology developed at and
	licensed from the Information Technology Division, 
	US Naval Research Laboratory.

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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <net/raw_cb.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipsec.h>
#include <netinet6/key.h>
#include <netinet6/in6_debug.h>

#define MAXHASHKEYLEN (2 * sizeof(int) + 2 * sizeof(struct sockaddr_in6))

/*
 *  Not clear whether these values should be 
 *  tweakable at kernel config time.
 */
#define KEYTBLSIZE 61
#define KEYALLOCTBLSIZE 61
#define SO2SPITBLSIZE 61

/*
 *  These values should be tweakable...
 *  perhaps by using sysctl
 */

#define MAXLARVALTIME 240;   /* Lifetime of a larval key table entry */ 
#define MAXKEYACQUIRE 1;     /* Max number of key acquire messages sent */
                             /*   per destination address               */
#define MAXACQUIRETIME 15;   /* Lifetime of acquire message */

/*
 *  Key engine tables and global variables
 */

struct key_tblnode keytable[KEYTBLSIZE];
struct key_allocnode keyalloctbl[KEYALLOCTBLSIZE];
struct key_so2spinode so2spitbl[SO2SPITBLSIZE];

struct keyso_cb keyso_cb;
struct key_tblnode nullkeynode  = { 0, 0, 0, 0, 0 };
struct key_registry *keyregtable;
struct key_acquirelist *key_acquirelist;
u_long maxlarvallifetime = MAXLARVALTIME;
int maxkeyacquire = MAXKEYACQUIRE;
u_long maxacquiretime = MAXACQUIRETIME;

extern void dump_secassoc();


/*----------------------------------------------------------------------
 * (temporary) Dump a data buffer 
 ----------------------------------------------------------------------*/

void
dump_buf(buf, len) 
     char *buf;
     int len;
{
  int i;

  printf("buf=0x%x len=%d:\n", buf, len);
  for (i = 0; i < len; i++) {
    printf("0x%x ", (u_int8)*(buf+i));
  }
  printf("\n");
}


/*----------------------------------------------------------------------
 * (temporary) Dump a key_tblnode structrue
 ----------------------------------------------------------------------*/

void
dump_keytblnode(ktblnode) 
     struct key_tblnode *ktblnode;
{
  if (!ktblnode) {
    printf("NULL key table node pointer!\n");
    return;
  }
  printf("solist=0x%x ", ktblnode->solist);
  printf("secassoc=0x%x ", ktblnode->secassoc);
  printf("next=0x%x\n", ktblnode->next);
}


/*----------------------------------------------------------------------
 * key_secassoc2msghdr(): 
 *      Copy info from a security association into a key message buffer.
 *      Assume message buffer is sufficiently large to hold all security
 *      association information including src, dst, from, key and iv.
 ----------------------------------------------------------------------*/
int
key_secassoc2msghdr(secassoc, km, keyinfo)
     struct ipsec_assoc *secassoc;
     struct key_msghdr *km;
     struct key_msgdata *keyinfo;
{
  char *cp;
  DPRINTF(IDL_GROSS_EVENT, ("Entering key_secassoc2msghdr\n"));

  if ((km == 0) || (keyinfo == 0) || (secassoc == 0))
    return(-1);

  km->type = secassoc->type;
  km->state = secassoc->state;
  km->label = secassoc->label;
  km->spi = secassoc->spi;
  km->keylen = secassoc->keylen;
  km->ivlen = secassoc->ivlen;
  km->algorithm = secassoc->algorithm;
  km->lifetype = secassoc->lifetype;
  km->lifetime1 = secassoc->lifetime1;
  km->lifetime2 = secassoc->lifetime2;

  /*
   *  Stuff src/dst/from/key/iv in buffer after
   *  the message header.
   */
  cp = (char *)(km + 1);

#define ROUNDUP(a) \
  ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) \
    { x += ROUNDUP(n); } 

  DPRINTF(IDL_FINISHED, ("sa2msghdr: 1\n"));
  keyinfo->src = (struct sockaddr *)cp;
  if (secassoc->src.sin6_len) {
    bcopy((char *)&(secassoc->src), cp, secassoc->src.sin6_len);
    ADVANCE(cp, secassoc->src.sin6_len);
  } else {
    bzero(cp, sizeof(struct sockaddr_in6));
    ADVANCE(cp, sizeof(struct sockaddr_in6));
  }
  DPRINTF(IDL_FINISHED, ("sa2msghdr: 2\n"));

  keyinfo->dst = (struct sockaddr *)&(secassoc->dst);
  if (secassoc->dst.sin6_len) {
    bcopy((char *)&(secassoc->dst), cp, secassoc->dst.sin6_len);
    ADVANCE(cp, secassoc->dst.sin6_len);
  } else {
    bzero(cp, sizeof(struct sockaddr_in6));
    ADVANCE(cp, sizeof(struct sockaddr_in6));
  }
  DPRINTF(IDL_FINISHED, ("sa2msghdr: 3\n"));

  keyinfo->from = (struct sockaddr *)cp;
  if (secassoc->from.sin6_len) {
    bcopy((char *)&(secassoc->from), cp, secassoc->from.sin6_len);
    ADVANCE(cp, secassoc->from.sin6_len);
  } else {
    bzero(cp, sizeof(struct sockaddr_in6));
    ADVANCE(cp, sizeof(struct sockaddr_in6));
  }
  DPRINTF(IDL_FINISHED, ("sa2msghdr: 4\n"));

  keyinfo->key = cp;
  keyinfo->keylen = secassoc->keylen;
  if (secassoc->keylen) {
    bcopy((char *)(secassoc->key), cp, secassoc->keylen);
    ADVANCE(cp, secassoc->keylen);
  }

  DPRINTF(IDL_FINISHED, ("sa2msghdr: 5\n"));
  keyinfo->iv = cp;
  keyinfo->ivlen = secassoc->ivlen;
  if (secassoc->ivlen) {
    bcopy((char *)(secassoc->iv), cp, secassoc->ivlen);
    ADVANCE(cp, secassoc->ivlen);
  }

  DDO(IDL_FINISHED,printf("msgbuf(len=%d):\n",(char *)cp - (char *)km));
  DDO(IDL_FINISHED,dump_buf((char *)km, (char *)cp - (char *)km));
  DPRINTF(IDL_FINISHED, ("sa2msghdr: 6\n"));
  return(0);
}


/*----------------------------------------------------------------------
 * key_msghdr2secassoc():
 *      Copy info from a key message buffer into an ipsec_assoc 
 *      structure
 ----------------------------------------------------------------------*/
int 
key_msghdr2secassoc(secassoc, km, keyinfo)
     struct ipsec_assoc *secassoc;
     struct key_msghdr *km;
     struct key_msgdata *keyinfo;
{
  DPRINTF(IDL_GROSS_EVENT, ("Entering key_msghdr2secassoc\n"));

  if ((km == 0) || (keyinfo == 0) || (secassoc == 0))
    return(-1);

  secassoc->len = sizeof(*secassoc);
  secassoc->type = km->type;
  secassoc->state = km->state;
  secassoc->label = km->label;
  secassoc->spi = km->spi;
  secassoc->keylen = km->keylen;
  secassoc->ivlen = km->ivlen;
  secassoc->algorithm = km->algorithm;
  secassoc->lifetype = km->lifetype;
  secassoc->lifetime1 = km->lifetime1;
  secassoc->lifetime2 = km->lifetime2;

  if (keyinfo->src)
    bcopy((char *)(keyinfo->src), (char *)&(secassoc->src),
	  keyinfo->src->sa_len);

  if (keyinfo->dst)
    bcopy((char *)(keyinfo->dst), (char *)&(secassoc->dst),
	  keyinfo->dst->sa_len);

  if (keyinfo->from)
    bcopy((char *)(keyinfo->from), (char *)&(secassoc->from),
	  keyinfo->from->sa_len);

  /*
   *  Make copies of key and iv
   */
  if (secassoc->ivlen) {
    K_Malloc(secassoc->iv, caddr_t, secassoc->ivlen);
    if (secassoc->iv == 0) {
      DPRINTF(IDL_CRITICAL,("msghdr2secassoc: can't allocate mem for iv\n"));
      return(-1);
    }
    bcopy((char *)keyinfo->iv, (char *)secassoc->iv, secassoc->ivlen);
  } else
    secassoc->iv = NULL;
	     
  if (secassoc->keylen) {
    K_Malloc(secassoc->key, caddr_t, secassoc->keylen);
    if (secassoc->key == 0) {
      DPRINTF(IDL_CRITICAL,("msghdr2secassoc: can't allocate mem for key\n"));
      if (secassoc->iv)
	KFree(secassoc->iv);
      return(-1);
    }
    bcopy((char *)keyinfo->key, (char *)secassoc->key, secassoc->keylen);
  } else
    secassoc->key = NULL;
  return(0);
}


/*----------------------------------------------------------------------
 * addrpart_equal():
 *      Determine if the address portion of two sockaddrs are equal.
 *      Currently handles only AF_INET and AF_INET6 address families.
 ----------------------------------------------------------------------*/
int
addrpart_equal(sa1, sa2)
     struct sockaddr *sa1;
     struct sockaddr *sa2;
{

  if ((sa1->sa_family == sa2->sa_family)) 
    switch(sa1->sa_family) {
    case AF_INET:
      if (((struct sockaddr_in *)sa1)->sin_addr.s_addr == 
	  ((struct sockaddr_in *)sa2)->sin_addr.s_addr)
	return(1);
      break;
    case AF_INET6:
      if (IN6_ADDR_EQUAL(((struct sockaddr_in6 *)sa1)->sin6_addr, 
			 ((struct sockaddr_in6 *)sa2)->sin6_addr))
	return(1);
      break;
    }
  return(0);
}


/*----------------------------------------------------------------------
 * my_addr():
 *      Determine if an address belongs to one of my configured interfaces.
 *      Currently handles only AF_INET and AF_INET6 addresses.
 ----------------------------------------------------------------------*/
int
my_addr(sa)
     struct sockaddr *sa;
{
  extern struct in6_ifaddr *in6_ifaddr;
  extern struct in_ifaddr *in_ifaddr;
  struct in6_ifaddr *i6a = 0;
  struct in_ifaddr *ia = 0;

  switch(sa->sa_family) {
  case AF_INET6:
    for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next) {
      if (IN6_ADDR_EQUAL(((struct sockaddr_in6 *)sa)->sin6_addr, 
			 i6a->i6a_addr.sin6_addr))
	return(1);
    }
    break;
  case AF_INET:
    for (ia = in_ifaddr; ia; ia = ia->ia_next) {
      if (((struct sockaddr_in *)sa)->sin_addr.s_addr == 
	   ia->ia_addr.sin_addr.s_addr) 
	return(1);
    }
    break;
  }
  return(0);
}


/*----------------------------------------------------------------------
 * key_inittables():
 *      Allocate space and initialize key engine tables
 ----------------------------------------------------------------------*/
void
key_inittables()
{
  struct key_tblnode *keynode;
  int i;

  K_Malloc(keyregtable, struct key_registry *, sizeof(struct key_registry));
  if (keyregtable == 0)
    panic("key_inittables");
  bzero((char *)keyregtable, sizeof(struct key_registry));
  K_Malloc(key_acquirelist, struct key_acquirelist *, 
	   sizeof(struct key_acquirelist));
  if (key_acquirelist == 0)
    panic("key_inittables");
  bzero((char *)key_acquirelist, sizeof(struct key_acquirelist));
  for (i = 0; i < KEYTBLSIZE; i++) 
    bzero((char *)&keytable[i], sizeof(struct key_tblnode));
  for (i = 0; i < KEYALLOCTBLSIZE; i++)
    bzero((char *)&keyalloctbl[i], sizeof(struct key_allocnode));
  for (i = 0; i < SO2SPITBLSIZE; i++)
    bzero((char *)&so2spitbl[i], sizeof(struct key_so2spinode));
}


/*----------------------------------------------------------------------
 * key_gethashval():
 *      Determine keytable hash value.
 ----------------------------------------------------------------------*/
int
key_gethashval(buf, len, tblsize) 
     char *buf;
     int len;
     int tblsize;
{
  int i, j = 0;

  /* 
   * Todo: Use word size xor and check for alignment
   *       and zero pad if necessary.  Need to also pick 
   *       a good hash function and table size.
   */
  if (len <= 0) {
    DPRINTF(IDL_CRITICAL,("key_gethashval got bogus len!\n"));
    return(-1);
  }
  for(i = 0; i < len; i++) {
    j ^=  (u_int8)(*(buf + i));
  }
  return (j % tblsize);
}


/*----------------------------------------------------------------------
 * key_createkey():
 *      Create hash key for hash function
 *      key is: type+src+dst if keytype = 1
 *              type+src+dst+spi if keytype = 0
 *      Uses only the address portion of the src and dst sockaddrs to 
 *      form key.  Currently handles only AF_INET and AF_INET6 sockaddrs
 ----------------------------------------------------------------------*/
int
key_createkey(buf, type, src, dst, spi, keytype)
     char *buf;
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     u_int32 spi;
     u_int keytype;
{
  char *cp, *p;

  DPRINTF(IDL_FINISHED,("Entering key_createkey\n"));

  if (!buf || !src || !dst)
    return(-1);

  cp = buf;
  bcopy((char *)&type, cp, sizeof(type));
  cp += sizeof(type);

  /*
   * Assume only IPv4 and IPv6 addresses.
   */
#define ADDRPART(a) \
    ((a)->sa_family == AF_INET6) ? \
    (char *)&(((struct sockaddr_in6 *)(a))->sin6_addr) : \
    (char *)&(((struct sockaddr_in *)(a))->sin_addr)

#define ADDRSIZE(a) \
    ((a)->sa_family == AF_INET6) ? sizeof(struct in_addr6) : \
    sizeof(struct in_addr)  

  DPRINTF(IDL_GROSS_EVENT,("src addr:\n"));
  DDO(IDL_GROSS_EVENT,dump_smart_sockaddr(src));
  DPRINTF(IDL_GROSS_EVENT,("dst addr:\n"));
  DDO(IDL_GROSS_EVENT,dump_smart_sockaddr(dst)); 

  p = ADDRPART(src);
  bcopy(p, cp, ADDRSIZE(src));
  cp += ADDRSIZE(src);

  p = ADDRPART(dst);
  bcopy(p, cp, ADDRSIZE(dst));
  cp += ADDRSIZE(dst);

#undef ADDRPART
#undef ADDRSIZE

  if (keytype == 0) {
    bcopy((char *)&spi, cp, sizeof(spi));
    cp += sizeof(spi);
  }

  DPRINTF(IDL_FINISHED,("hash key:\n"));
  DDO(IDL_FINISHED, dump_buf(buf, cp - buf));
  return(cp - buf);
}


/*----------------------------------------------------------------------
 * key_sosearch():
 *      Search the so2spi table for the security association allocated to 
 *      the socket.  Returns pointer to a struct key_so2spinode which can
 *      be used to locate the security association entry in the keytable.
 ----------------------------------------------------------------------*/
struct key_so2spinode *
key_sosearch(type, src, dst, so)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     struct socket *so;
{
  struct key_so2spinode *np = 0;

  if (!(src && dst)) {
    DPRINTF(IDL_CRITICAL,("key_sosearch: got null src or dst pointer!\n"));
    return(NULL);
  }

  for (np = so2spitbl[((u_int32)so) % SO2SPITBLSIZE].next; np; np = np->next) {
    if ((so == np->socket) && (type == np->keynode->secassoc->type) 
	&& addrpart_equal(src, 
			  (struct sockaddr *)&(np->keynode->secassoc->src)) 
	&& addrpart_equal(dst, 
			  (struct sockaddr *)&(np->keynode->secassoc->dst))) 
      return(np);
  }  
  return(NULL);
}


/*----------------------------------------------------------------------
 * key_sodelete():
 *      Delete entries from the so2spi table.
 *        flag = 1  purge all entries
 *        flag = 0  delete entries with socket pointer matching socket  
 ----------------------------------------------------------------------*/
void
key_sodelete(socket, flag) 
     struct socket *socket;
     int flag;
{
  struct key_so2spinode *prevnp, *np;
  int s = splnet();

  DPRINTF(IDL_MAJOR_EVENT,("Entering keysodelete w/so=0x%x flag=%d\n",socket,flag));
  if (flag) {
    int i;

    for (i = 0; i < SO2SPITBLSIZE; i++)
      for(np = so2spitbl[i].next; np; np = np->next) {
	KFree(np);
      }
    splx(s);
    return;
  }

  prevnp = &so2spitbl[((u_int32)socket) % SO2SPITBLSIZE];
  for(np = prevnp->next; np; np = np->next) {
    if (np->socket == socket) {
      struct socketlist *socklp, *prevsocklp;

      (np->keynode->alloc_count)--;

      /* 
       * If this socket maps to a unique secassoc,
       * we go ahead and delete the secassoc, since it
       * can no longer be allocated or used by any other 
       * socket.
       */
      if (np->keynode->secassoc->state & K_UNIQUE) {
	if (key_delete(np->keynode->secassoc) != 0)
	  panic("key_sodelete");
	np = prevnp;
	continue;
      }

      /*
       * We traverse the socketlist and remove the entry
       * for this socket
       */
      DPRINTF(IDL_FINISHED,("keysodelete: deleting from socklist..."));
      prevsocklp = np->keynode->solist;
      for (socklp = prevsocklp->next; socklp; socklp = socklp->next) {
	if (socklp->socket == socket) {
	  prevsocklp->next = socklp->next;
	  KFree(socklp);
	  break;
	}
	prevsocklp = socklp;
      }
      DPRINTF(IDL_FINISHED,("done\n"));
      prevnp->next = np->next;
      KFree(np);
      np = prevnp;
    }
    prevnp = np;  
  }
  splx(s);
}


/*----------------------------------------------------------------------
 * key_deleteacquire():
 *      Delete an entry from the key_acquirelist
 ----------------------------------------------------------------------*/
void
key_deleteacquire(type, target)
     u_int type;
     struct sockaddr *target;
{
  struct key_acquirelist *ap, *prev;

  prev = key_acquirelist;
  for(ap = key_acquirelist->next; ap; ap = ap->next) {
    if (addrpart_equal(target, (struct sockaddr *)&(ap->target)) &&
	(type == ap->type)) {
      DPRINTF(IDL_MAJOR_EVENT,("Deleting entry from acquire list!\n"));
      prev->next = ap->next;
      KFree(ap);
      ap = prev;
    }
    prev = ap;
  }
}


/*----------------------------------------------------------------------
 * key_search():
 *      Search the key table for an entry with same type, src addr, dest
 *      addr, and spi.  Returns a pointer to struct key_tblnode if found
 *      else returns null.
 ----------------------------------------------------------------------*/
struct key_tblnode *
key_search(type, src, dst, spi, indx, prevkeynode)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     u_int32 spi;
     int indx;
     struct key_tblnode **prevkeynode;
{
  struct key_tblnode *keynode, *prevnode;

  if (indx > KEYTBLSIZE || indx < 0)
    return (NULL);
  if (!(&keytable[indx]))
    return (NULL);

#define sec_type keynode->secassoc->type
#define sec_spi keynode->secassoc->spi
#define sec_src keynode->secassoc->src
#define sec_dst keynode->secassoc->dst

  prevnode = &keytable[indx];
  for (keynode = keytable[indx].next; keynode; keynode = keynode->next) {
    if ((type == sec_type) && (spi == sec_spi) && 
	addrpart_equal(src, (struct sockaddr *)&(sec_src))
	&& addrpart_equal(dst, (struct sockaddr *)&(sec_dst)))
      break;
    prevnode = keynode;
  }
  *prevkeynode = prevnode;
  return(keynode);
}


/*----------------------------------------------------------------------
 * key_addnode():
 *      Insert a key_tblnode entry into the key table.  Returns a pointer 
 *      to the newly created key_tblnode.
 ----------------------------------------------------------------------*/
struct key_tblnode *
key_addnode(indx, secassoc)
     int indx;
     struct ipsec_assoc *secassoc;
{
  struct key_tblnode *keynode;

  DPRINTF(IDL_GROSS_EVENT,("Entering key_addnode w/indx=%d secassoc=0x%x\n",indx, (u_int32)secassoc));

  if (!(&keytable[indx]))
    return(NULL);
  if (!secassoc) {
    panic("key_addnode: Someone passed in a null secassoc!\n");
  }

  K_Malloc(keynode, struct key_tblnode *, sizeof(struct key_tblnode));
  if (keynode == 0)
    return(NULL);
  bzero((char *)keynode, sizeof(struct key_tblnode));

  K_Malloc(keynode->solist, struct socketlist *, sizeof(struct socketlist));
  if (keynode->solist == 0) {
    KFree(keynode);
    return(NULL);
  }
  bzero((char *)(keynode->solist), sizeof(struct socketlist));

  keynode->secassoc = secassoc;
  keynode->solist->next = NULL;
  keynode->next = keytable[indx].next;
  keytable[indx].next = keynode;
  return(keynode);
}


/*----------------------------------------------------------------------
 * key_add():
 *      Add a new security association to the key table.  Caller is
 *      responsible for allocating memory for the struct ipsec_assoc as  
 *      well as the buffer space for the key and iv.  Assumes the security 
 *      association passed in is well-formed.
 ----------------------------------------------------------------------*/
int
key_add(secassoc)
     struct ipsec_assoc *secassoc;
{
  char buf[MAXHASHKEYLEN];
  int len, indx;
  int inbound = 0;
  int outbound = 0;
  struct key_tblnode *keynode, *prevkeynode;
  struct key_allocnode *np;
  int s;

  DPRINTF(IDL_GROSS_EVENT, ("Entering key_add w/secassoc=0x%x\n",secassoc));

  if (!secassoc) {
    panic("key_add: who the hell is passing me a null pointer");
  }

  /*
   *  For storage purposes, the two esp modes are
   *  treated the same.
   */
  if (secassoc->type == SS_ENCRYPTION_NETWORK)
    secassoc->type = SS_ENCRYPTION_TRANSPORT;

  /*
   * Should we allow a null key to be inserted into the table ? 
   * or can we use null key to indicate some policy action...
   */

  /*
   *  For esp using des-cbc or tripple-des we call 
   * des_set_odd_parity.
   */
  if (secassoc->key && (secassoc->type == SS_ENCRYPTION_TRANSPORT) && 
      ((secassoc->algorithm == IPSEC_ALGTYPE_ESP_DES_CBC) ||
       (secassoc->algorithm == IPSEC_ALGTYPE_ESP_3DES)))
    des_set_odd_parity(secassoc->key);

  /*
   *  Check if secassoc with same spi exists before adding
   */
  bzero((char *)&buf, sizeof(buf));
  len = key_createkey((char *)&buf, secassoc->type, 
		      (struct sockaddr *)&(secassoc->src), 
		      (struct sockaddr *)&(secassoc->dst), 
		      secassoc->spi, 0);
  indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
  DPRINTF(IDL_GROSS_EVENT,("keyadd: keytbl hash position=%d\n", indx));
  keynode = key_search(secassoc->type, (struct sockaddr *)&(secassoc->src),
		       (struct sockaddr *)&(secassoc->dst), 
		       secassoc->spi, indx, &prevkeynode);
  if (keynode) {
    DPRINTF(IDL_MAJOR_EVENT,("keyadd: secassoc already exists!\n"));
    return(-2);
  }

  inbound = my_addr((struct sockaddr *)&(secassoc->dst));
  outbound = my_addr((struct sockaddr *)&(secassoc->src));
  DPRINTF(IDL_FINISHED,("inbound=%d outbound=%d\n", inbound, outbound));

  /*
   * We allocate mem for an allocation entry if needed.
   * This is done here instead of in the allocaton code 
   * segment so that we can easily recover/cleanup from a 
   * memory allocation error.
   */
  if (outbound || (!inbound && !outbound)) {
    K_Malloc(np, struct key_allocnode *, sizeof(struct key_allocnode));
    if (np == 0) {
      DPRINTF(IDL_CRITICAL,("keyadd: can't allocate allocnode!\n"));
      return(-1);
    }
  }

  s = splnet();

  if ((keynode = key_addnode(indx, secassoc)) == NULL) {
    DPRINTF(IDL_CRITICAL,("keyadd: key_addnode failed!\n"));
    if (np)
      KFree(np);
    splx(s);
    return(-1);
  }
  DPRINTF(IDL_EVENT,("Added new keynode:\n"));
  DDO(IDL_GROSS_EVENT, dump_keytblnode(keynode));
  DDO(IDL_GROSS_EVENT, dump_secassoc(keynode->secassoc));
 
  /*
   *  We add an entry to the allocation table for
   *  this secassoc if the interfaces are up and
   *  the secassoc is outbound.  In the case 
   *  where the interfaces are not up, we go ahead
   *  and do it anyways.  This wastes an allocation
   *  entry if the secassoc later turned out to be
   *  inbound when the interfaces are ifconfig up.
   */
  if (outbound || (!inbound && !outbound)) {
    len = key_createkey((char *)&buf, secassoc->type,
			(struct sockaddr *)&(secassoc->src),
			(struct sockaddr *)&(secassoc->dst),
			0, 1);
    indx = key_gethashval((char *)&buf, len, KEYALLOCTBLSIZE);
    DPRINTF(IDL_GROSS_EVENT,("keyadd: keyalloc hash position=%d\n", indx));
    np->keynode = keynode;
    np->next = keyalloctbl[indx].next;
    keyalloctbl[indx].next = np;
  }
  if (inbound)
    secassoc->state |= K_INBOUND;
  if (outbound)
    secassoc->state |= K_OUTBOUND;

  key_deleteacquire(secassoc->type, (struct sockaddr *)&(secassoc->dst));

  splx(s);
  return 0;
}


/*----------------------------------------------------------------------
 * key_get():
 *      Get a security association from the key table.
 ----------------------------------------------------------------------*/
int
key_get(type, src, dst, spi, secassoc)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     u_int32 spi;
     struct ipsec_assoc **secassoc;
{
  char buf[MAXHASHKEYLEN];
  struct key_tblnode *keynode, *prevkeynode;
  int len, indx;

  /*
   *  For storage purposes, the two esp modes are
   *  treated the same.
   */
  if (type == SS_ENCRYPTION_NETWORK)
    type = SS_ENCRYPTION_TRANSPORT;

  bzero(&buf, sizeof(buf));
  *secassoc = NULL;
  len = key_createkey((char *)&buf, type, src, dst, spi, 0);
  indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
  DPRINTF(IDL_GROSS_EVENT,("keyget: indx=%d\n",indx));
  keynode = key_search(type, src, dst, spi, indx, &prevkeynode);
  if (keynode) {
    DPRINTF(IDL_EVENT,("keyget: found it! keynode=0x%x",keynode));
    *secassoc = keynode->secassoc;
    return(0);
  } else
    return(-1);  /* Not found */
}


/*----------------------------------------------------------------------
 * key_dump():
 *      Dump all valid entries in the keytable to a pf_key socket.  Each
 *      security associaiton is sent one at a time in a pf_key message.  A
 *      message with seqno = 0 signifies the end of the dump transaction.
 ----------------------------------------------------------------------*/
int
key_dump(so)
     struct socket *so;
{
  int len, i;
  int seq = 1;
  struct mbuf *m;
  struct key_msgdata keyinfo;
  struct key_msghdr *km;
  struct key_tblnode *keynode;
  extern struct sockaddr key_src;
  extern struct sockaddr key_dst;

  /*
   * Routine to dump the key table to a routing socket
   * Use for debugging only!
   */

  DPRINTF(IDL_GROSS_EVENT,("Entering key_dump()"));
  /* 
   * We need to speed this up later.  Fortunately, key_dump 
   * messages are not sent often.
   */
  for (i = 0; i < KEYTBLSIZE; i++) {
    for (keynode = keytable[i].next; keynode; keynode = keynode->next) {

      /*
       * We exclude dead/larval/zombie security associations for now
       * but it may be useful to also send these up for debugging purposes
       */
      if (keynode->secassoc->state & (K_DEAD | K_LARVAL | K_ZOMBIE))
	continue;

      len = (sizeof(struct key_msghdr) + 
	     ROUNDUP(keynode->secassoc->src.sin6_len) + 
	     ROUNDUP(keynode->secassoc->dst.sin6_len) +
	     ROUNDUP(keynode->secassoc->from.sin6_len) +
	     ROUNDUP(keynode->secassoc->keylen) + 
	     ROUNDUP(keynode->secassoc->ivlen));
      K_Malloc(km, struct key_msghdr *, len);
      if (km == 0)
	return(ENOBUFS);
      if (key_secassoc2msghdr(keynode->secassoc, km, &keyinfo) != 0)
	panic("key_dump");
      km->key_msglen = len;
      km->key_msgvers = KEY_VERSION;
      km->key_msgtype = KEY_DUMP;
      km->key_pid = curproc->p_pid;
      km->key_seq = seq++;
      km->key_errno = 0;
      MGETHDR(m, M_WAIT, MT_DATA);
      m->m_len = m->m_pkthdr.len = 0;
      m->m_next = 0;
      m->m_nextpkt = 0;
      m->m_pkthdr.rcvif = 0;
      m_copyback(m, 0, len, (caddr_t)km);
      KFree(km);
      if (sbappendaddr(&so->so_rcv, &key_src, m, (struct mbuf *)0) == 0)
	m_free(m);
      else 
	sorwakeup(so);
    }
  }
  K_Malloc(km, struct key_msghdr *, sizeof(struct key_msghdr));
  if (km == 0)
    return(ENOBUFS);
  bzero((char *)km, sizeof(struct key_msghdr));
  km->key_msglen = sizeof(struct key_msghdr);
  km->key_msgvers = KEY_VERSION;
  km->key_msgtype = KEY_DUMP;
  km->key_pid = curproc->p_pid;
  km->key_seq = 0;
  km->key_errno = 0;
  MGETHDR(m, M_WAIT, MT_DATA);
  m->m_len = m->m_pkthdr.len = 0;
  m->m_next = 0;
  m->m_nextpkt = 0;
  m->m_pkthdr.rcvif = 0;
  m_copyback(m, 0, km->key_msglen, (caddr_t)km);  
  KFree(km);
  if (sbappendaddr(&so->so_rcv, &key_src, m, (struct mbuf *)0) == 0)
    m_free(m);
  else 
    sorwakeup(so);
  DPRINTF(IDL_GROSS_EVENT,("Leaving key_dump()\n"));  
  return(0);
}

/*----------------------------------------------------------------------
 * key_delete():
 *      Delete a security association from the key table.
 ----------------------------------------------------------------------*/
int
key_delete(secassoc)
     struct ipsec_assoc *secassoc;
{
  char buf[MAXHASHKEYLEN];
  int len, indx;
  struct key_tblnode *keynode = 0;
  struct key_tblnode *prevkeynode = 0;
  struct socketlist *socklp, *deadsocklp;
  struct key_so2spinode *np, *prevnp;
  struct key_allocnode *ap, *prevap;
  int s;

  DPRINTF(IDL_GROSS_EVENT,("Entering key_delete w/secassoc=0x%x\n", secassoc));

  if (secassoc->type == SS_ENCRYPTION_NETWORK)
    secassoc->type = SS_ENCRYPTION_TRANSPORT;

  bzero((char *)&buf, sizeof(buf));
  len = key_createkey((char *)&buf, secassoc->type, 
		      (struct sockaddr *)&(secassoc->src), 
		      (struct sockaddr *)&(secassoc->dst), 
		      secassoc->spi, 0);
  indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
  DPRINTF(IDL_GROSS_EVENT,("keydelete: keytbl hash position=%d\n", indx));
  keynode = key_search(secassoc->type, (struct sockaddr *)&(secassoc->src),
		       (struct sockaddr *)&(secassoc->dst), 
		       secassoc->spi, indx, &prevkeynode); 
 
  if (keynode) {
    s = splnet();
    DPRINTF(IDL_EVENT,("keydelete: found keynode to delete\n"));
    keynode->secassoc->state |= K_DEAD;

    if (keynode->ref_count > 0) {
      DPRINTF(IDL_MAJOR_EVENT,("keydelete: secassoc still held, marking for deletion only!\n"));
      splx(s);
      return(0); 
    }

    prevkeynode->next = keynode->next;
    
    /*
     *  Walk the socketlist and delete the
     *  entries mapping sockets to this secassoc
     *  from the so2spi table.
     */
    DPRINTF(IDL_GROSS_EVENT,("keydelete: deleting socklist..."));
    for(socklp = keynode->solist->next; socklp; ) {
      prevnp = &so2spitbl[((u_int32)(socklp->socket)) % SO2SPITBLSIZE];
      for(np = prevnp->next; np; np = np->next) {
	if ((np->socket == socklp->socket) && (np->keynode == keynode)) {
	  prevnp->next = np->next;
	  KFree(np);
	  break; 
	}
	prevnp = np;  
      }
      deadsocklp = socklp;
      socklp = socklp->next;
      KFree(deadsocklp);
    }
    DPRINTF(IDL_GROSS_EVENT,("done\n"));
    /*
     * If an allocation entry exist for this
     * secassoc, delete it.
     */
    bzero((char *)&buf, sizeof(buf));
    len = key_createkey((char *)&buf, secassoc->type,
			(struct sockaddr *)&(secassoc->src),
			(struct sockaddr *)&(secassoc->dst),
			0, 1);
    indx = key_gethashval((char *)&buf, len, KEYALLOCTBLSIZE);
    DPRINTF(IDL_GROSS_EVENT,("keydelete: alloctbl hash position=%d\n", indx));
    prevap = &keyalloctbl[indx];
    for (ap = prevap->next; ap; ap = ap->next) {
      if (ap->keynode == keynode) {
	prevap->next = ap->next;
	KFree(ap);
	break; 
      }
      prevap = ap;
    }    

    if (keynode->secassoc->iv)
      KFree(keynode->secassoc->iv);
    if (keynode->secassoc->key)
      KFree(keynode->secassoc->key);
    KFree(keynode->secassoc);
    if (keynode->solist)
      KFree(keynode->solist);
    KFree(keynode);
    splx(s);
    return(0);
  }
  return(-1);
}


/*----------------------------------------------------------------------
 * key_flush():
 *      Delete all entries from the key table.
 ----------------------------------------------------------------------*/
void
key_flush(void)
{
  struct key_tblnode *keynode;
  int i;

  /* 
   * This is slow, but simple.
   */
  DPRINTF(IDL_FINISHED,("Flushing key table..."));
  for (i = 0; i < KEYTBLSIZE; i++) {
    while (keynode = keytable[i].next)
      if (key_delete(keynode->secassoc) != 0)
	panic("key_flush");
  }
  DPRINTF(IDL_FINISHED,("done\n"));
}


/*----------------------------------------------------------------------
 * key_getspi():
 *      Get a unique spi value for a key management daemon/program.  The 
 *      spi value, once assigned, cannot be assigned again.
 ----------------------------------------------------------------------*/
int
key_getspi(type, src, dst, spi)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     u_int32 *spi;
{
  struct ipsec_assoc *secassoc;
  struct key_tblnode *keynode, *prevkeynode;
  int count, done, len, indx;
  int maxcount = 1000;
  u_int32 val;
  char buf[MAXHASHKEYLEN];
  int s;

  
  DPRINTF(IDL_MAJOR_EVENT,("Entering getspi w/type=%d\n",type));
  if (!(src && dst))
    return(-1);

  /*
   *  For storage purposes, the two esp modes are
   *  treated the same.
   */
  if (type == SS_ENCRYPTION_NETWORK)
    type = SS_ENCRYPTION_TRANSPORT;

  done = count = 0;
  do {
    count++;
    /* 
     *  Currently, valid spi values are 32 bits wide except for
     *  the value of zero.  This need to change to take into 
     *  account more restrictive spi ranges.
     *
     *  TODO:  Kebe says to allow key mgnt daemon to specify range 
     *         of valid spi to get.
     */
    val = random();
    DPRINTF(IDL_FINISHED,("%u ",val));
    if (val) {
      DPRINTF(IDL_FINISHED,("\n"));
      bzero(&buf, sizeof(buf));
      len = key_createkey((char *)&buf, type, src, dst, val, 0);
      indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
      if(!key_search(type, src, dst, val, indx, &prevkeynode)) {
	s = splnet();
	K_Malloc(secassoc, struct ipsec_assoc *, sizeof(struct ipsec_assoc));
	if (secassoc == 0) {
	  DPRINTF(IDL_CRITICAL,("key_getspi: can't allocate memory\n"));
	  splx(s);
	  return(-1);
	}
	bzero((char *)secassoc, sizeof(struct ipsec_assoc));

	DPRINTF(IDL_FINISHED,("getspi: indx=%d\n",indx));
	secassoc->len = sizeof(struct ipsec_assoc);
	secassoc->type = type;
	secassoc->spi = val;
	secassoc->state |= K_LARVAL;
	if (my_addr((struct sockaddr *)&(secassoc->dst)))
	  secassoc->state |= K_INBOUND;
	if (my_addr((struct sockaddr *)&(secassoc->src)))
	  secassoc->state |= K_OUTBOUND;

	bcopy((char *)src, (char *)&(secassoc->src), src->sa_len);
	bcopy((char *)dst, (char *)&(secassoc->dst), dst->sa_len);
	secassoc->from.sin6_family = AF_INET6;
	secassoc->from.sin6_len = sizeof(struct sockaddr_in6);

	/* 
	 * We need to add code to age these larval key table
	 * entries so they don't linger forever waiting for
	 * a KEY_UPDATE message that may not come for various
	 * reasons.  This is another task that key_reaper can
	 * do once we have it coded.
	 */
	secassoc->lifetime1 = time.tv_sec + maxlarvallifetime;

	if (!(keynode = key_addnode(indx, secassoc))) {
	  DPRINTF(IDL_CRITICAL,("key_getspi: can't add node\n"));
	  splx(s);
	  return(-1);
	} 
	DPRINTF(IDL_FINISHED,("key_getspi: added node 0x%x\n",keynode));
	done++;
	splx(s);
      }
    }
  } while ((count < maxcount) && !done);
  DPRINTF(IDL_FINISHED,("getspi returns w/spi=%u,count=%d\n",val,count));
  if (done) {
    *spi = val;
    return(0);
  } else {
    *spi = 0;
    return(-1);
  }
}


/*----------------------------------------------------------------------
 * key_update():
 *      Update a keytable entry that has an spi value assigned but is 
 *      incomplete (e.g. no key/iv).
 ----------------------------------------------------------------------*/
int
key_update(secassoc)
     struct ipsec_assoc *secassoc;
{
  struct key_tblnode *keynode, *prevkeynode;
  struct key_allocnode *np = 0;
  u_int8 newstate;
  int len, indx, inbound, outbound;
  char buf[MAXHASHKEYLEN];
  int s;

  /*
   *  For storage purposes, the two esp modes are
   *  treated the same.
   */
  if (secassoc->type == SS_ENCRYPTION_NETWORK)
    secassoc->type = SS_ENCRYPTION_TRANSPORT;
  
  bzero(&buf, sizeof(buf));
  len = key_createkey((char *)&buf, secassoc->type, 
		      (struct sockaddr *)&(secassoc->src), 
		      (struct sockaddr *)&(secassoc->dst), 
		      secassoc->spi, 0);
  indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
  if(!(keynode = key_search(secassoc->type, 
			    (struct sockaddr *)&(secassoc->src), 
			    (struct sockaddr *)&(secassoc->dst), 
			    secassoc->spi, indx, &prevkeynode))) {  
    return(ESRCH);
  }
  if (keynode->secassoc->state & K_DEAD)
    return(ESRCH);

  /* Should we also restrict updating of only LARVAL entries ? */

  s = splnet();

  inbound = my_addr((struct sockaddr *)&(secassoc->dst));
  outbound = my_addr((struct sockaddr *)&(secassoc->src));

  newstate = keynode->secassoc->state;
  newstate &= ~K_LARVAL;
  if (inbound)
    newstate |= K_INBOUND;
  if (outbound)
    newstate |= K_OUTBOUND;

  if (outbound || (!inbound && !outbound)) {
    K_Malloc(np, struct key_allocnode *, sizeof(struct key_allocnode));
    if (np == 0) {
      DPRINTF(IDL_CRITICAL,("keyupdate: can't allocate allocnode!\n"));
      splx(s);
      return(ENOBUFS);
    }
  }

  /*
   *  We now copy the secassoc over. We don't need to copy
   *  the key and iv into new buffers since the calling routine
   *  does that already.  
   */

  *(keynode->secassoc) = *secassoc;
  keynode->secassoc->state = newstate;

  /*
   * Should we allow a null key to be inserted into the table ? 
   * or can we use null key to indicate some policy action...
   */
  
  if (keynode->secassoc->key &&
       (keynode->secassoc->type == SS_ENCRYPTION_TRANSPORT) && 
       ((keynode->secassoc->algorithm == IPSEC_ALGTYPE_ESP_DES_CBC) ||
	(keynode->secassoc->algorithm == IPSEC_ALGTYPE_ESP_3DES)))
      des_set_odd_parity(keynode->secassoc->key);

  /*
   *  We now add an entry to the allocation table for this 
   *  updated key table entry.
   */
  if (outbound || (!inbound && !outbound)) {
    len = key_createkey((char *)&buf, secassoc->type,
			(struct sockaddr *)&(secassoc->src),
			(struct sockaddr *)&(secassoc->dst),
			0, 1);
    indx = key_gethashval((char *)&buf, len, KEYALLOCTBLSIZE);
    DPRINTF(IDL_FINISHED,("keyupdate: keyalloc hash position=%d\n", indx));
    np->keynode = keynode;
    np->next = keyalloctbl[indx].next;
    keyalloctbl[indx].next = np;
  }

  key_deleteacquire(secassoc->type, (struct sockaddr *)&(secassoc->dst));

  splx(s);
  return(0);
}

/*----------------------------------------------------------------------
 * key_register():
 *      Register a socket as one capable of acquiring security associations
 *      for the kernel.
 ----------------------------------------------------------------------*/
int
key_register(socket, type)
     struct socket *socket;
     u_int type;
{
  struct key_registry *p, *new;
  int s = splnet();

  DPRINTF(IDL_MAJOR_EVENT,("Entering key_register w/so=0x%x,type=%d\n",socket,type));

  if (!(keyregtable && socket))
    panic("key_register");
  
  /*
   * Make sure entry is not already in table
   */
  for(p = keyregtable->next; p; p = p->next) {
    if ((p->type == type) && (p->socket == socket)) {
      splx(s);
      return(EEXIST);
    }
  }

  K_Malloc(new, struct key_registry *, sizeof(struct key_registry));  
  if (new == 0) {
    splx(s);
    return(ENOBUFS);
  }
  new->type = type;
  new->socket = socket;
  new->next = keyregtable->next;
  keyregtable->next = new;
  splx(s);
  return(0);
}

/*----------------------------------------------------------------------
 * key_unregister():
 *      Delete entries from the registry list.
 *         allflag = 1 : delete all entries with matching socket
 *         allflag = 0 : delete only the entry matching socket and type
 ----------------------------------------------------------------------*/
void
key_unregister(socket, type, allflag)
     struct socket *socket;
     u_int type;
     int allflag;
{
  struct key_registry *p, *prev;
  int s = splnet();

  DPRINTF(IDL_MAJOR_EVENT,("Entering key_unregister w/so=0x%x,type=%d,flag=%d\n",socket, type, allflag));

  if (!(keyregtable && socket))
    panic("key_register");
  prev = keyregtable;
  for(p = keyregtable->next; p; p = p->next) {
    if ((allflag && (p->socket == socket)) ||
	((p->type == type) && (p->socket == socket))) {
      prev->next = p->next;
      KFree(p);
      p = prev;
    }
    prev = p;
  }
  splx(s);
}


/*----------------------------------------------------------------------
 * key_acquire():
 *      Send a key_acquire message to all registered key mgnt daemons 
 *      capable of acquire security association of type type.
 *
 *      Return: 0 if succesfully called key mgnt. daemon(s)
 *              -1 if not successfull.
 ----------------------------------------------------------------------*/
int
key_acquire(type, src, dst)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
{
  struct key_registry *p;
  struct key_acquirelist *ap, *prevap;
  int success = 0, created = 0;
  struct socket *last = 0;
  struct mbuf *m = 0;
  u_int etype;
  extern struct sockaddr key_src;

  DPRINTF(IDL_MAJOR_EVENT,("Entering key_acquire()\n"));

  if (!keyregtable || !src || !dst)
    return (-1);

  /*
   * We first check the acquirelist to see if a key_acquire
   * message has been sent for this destination.
   */
  etype = type;
  if (etype == SS_ENCRYPTION_NETWORK)
    etype = SS_ENCRYPTION_TRANSPORT;
  prevap = key_acquirelist;
  for(ap = key_acquirelist->next; ap; ap = ap->next) {
    if (addrpart_equal(dst, (struct sockaddr *)&(ap->target)) &&
	(etype == ap->type)) {
      DPRINTF(IDL_MAJOR_EVENT,("acquire message previously sent!\n"));
      if (ap->expiretime < time.tv_sec) {
	DPRINTF(IDL_MAJOR_EVENT,("acquire message has expired!\n"));
	ap->count = 0;
	break;
      }
      if (ap->count < maxkeyacquire) {
	DPRINTF(IDL_MAJOR_EVENT,("max acquire messages not yet exceeded!\n"));
	break;
      }
      return(0);
    } else if (ap->expiretime < time.tv_sec) {
      /*
       *  Since we're already looking at the list, we may as
       *  well delete expired entries as we scan through the list.
       *  This should really be done by a function like key_reaper()
       *  but until we code key_reaper(), this is a quick and dirty
       *  hack.
       */
      DPRINTF(IDL_MAJOR_EVENT,("found an expired entry...deleting it!\n"));
      prevap->next = ap->next;
      KFree(ap);
      ap = prevap;
    }
    prevap = ap;
  }

  /*
   * Scan registry and send KEY_ACQUIRE message to 
   * appropriate key management daemons.
   */  
  for(p = keyregtable->next; p; p = p->next) {
    if (p->type != type) 
      continue;

    if (!created) {
      struct key_msghdr *km;
      int len;
      
      len = sizeof(struct key_msghdr) + ROUNDUP(src->sa_len) + 
	ROUNDUP(dst->sa_len);
      K_Malloc(km, struct key_msghdr *, len);
      if (km == 0) {
	DPRINTF(IDL_CRITICAL,("key_acquire: no memory\n"));
	return(-1);
      }
      DPRINTF(IDL_FINISHED,("key_acquire/created: 1\n"));
      bzero((char *)km, len);
      km->key_msglen = len;
      km->key_msgvers = KEY_VERSION;
      km->key_msgtype = KEY_ACQUIRE;
      km->type = type;
      DPRINTF(IDL_FINISHED,("key_acquire/created: 2\n"));
      /*
       * This is inefficient and slow.
       */

      /*
       * We zero out sin_zero here for AF_INET addresses because
       * ip_output() currently does not do it for performance reasons.
       */
      if (src->sa_family == AF_INET)
	bzero((char *)(&((struct sockaddr_in *)src)->sin_zero), 
	      sizeof(((struct sockaddr_in *)src)->sin_zero));
      if (dst->sa_family == AF_INET)
	bzero((char *)(&((struct sockaddr_in *)dst)->sin_zero), 
	      sizeof(((struct sockaddr_in *)dst)->sin_zero));

      bcopy((char *)src, (char *)(km + 1), src->sa_len);
      bcopy((char *)dst, (char *)((int)(km + 1) + ROUNDUP(src->sa_len)),
	    dst->sa_len);
      DPRINTF(IDL_FINISHED,("key_acquire/created: 3\n"));
      MGETHDR(m, M_WAIT, MT_DATA);
      m->m_len = m->m_pkthdr.len = 0;
      m->m_next = 0;
      m->m_nextpkt = 0;
      m->m_pkthdr.rcvif = 0;
      m_copyback(m, 0, len, (caddr_t)km);
      KFree(km);
      DPRINTF(IDL_FINISHED,("key_acquire/created: 4\n"));
      DDO(IDL_FINISHED,dump_mchain(m));
      created++; 
    }
    if (last) {
      struct mbuf *n;
      if (n = m_copy(m, 0, (int)M_COPYALL)) {
	if (sbappendaddr(&last->so_rcv, &key_src, n, (struct mbuf *)0) == 0)
	  m_freem(n);
	else {
	  sorwakeup(last);
	  success++;
	}
      }
      DPRINTF(IDL_FINISHED,("key_acquire/last: 1\n"));
    }
    last = p->socket;
  }
  if (last) {
    if (sbappendaddr(&last->so_rcv, &key_src, m, (struct mbuf *)0) == 0)
      m_freem(m);
    else {
      sorwakeup(last);
      success++;
    }
    DPRINTF(IDL_FINISHED,("key_acquire/last: 2\n"));
  } else
    m_freem(m);

  /*
   *  Update the acquirelist 
   */
  if (success) {
    if (!ap) {
      DPRINTF(IDL_MAJOR_EVENT,("Adding new entry in acquirelist\n"));
      K_Malloc(ap, struct key_acquirelist *, sizeof(struct key_acquirelist));
      if (ap == 0)
	return(success ? 0 : -1);
      bzero((char *)ap, sizeof(struct key_acquirelist));
      bcopy((char *)dst, (char *)&(ap->target), dst->sa_len);
      ap->type = etype;
      ap->next = key_acquirelist->next;
      key_acquirelist->next = ap;
    }
    DPRINTF(IDL_EVENT,("Updating acquire counter and expiration time\n"));
    ap->count++;
    ap->expiretime = time.tv_sec + maxacquiretime;
  }
  DPRINTF(IDL_MAJOR_EVENT,("key_acquire: done! success=%d\n",success));
  return(success ? 0 : -1);
}

/*----------------------------------------------------------------------
 * key_alloc():
 *      Allocate a security association to a socket.  A socket requesting 
 *      unique keying (per-socket keying) is assigned a security assocation
 *      exclusively for its use.  Sockets not requiring unique keying are
 *      assigned the first security association which may or may not be
 *      used by another socket.
 ----------------------------------------------------------------------*/
int
key_alloc(type, src, dst, socket, unique_key, keynodep)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     struct socket *socket;
     u_int  unique_key;
     struct key_tblnode **keynodep;
{
  struct key_tblnode *keynode;
  char buf[MAXHASHKEYLEN];
  struct key_allocnode *np, *prevnp;
  struct key_so2spinode *newnp;
  int len;
  int indx;

  DPRINTF(IDL_GROSS_EVENT,("Entering key_alloc w/type=%u!\n",type));
  if (!(src && dst)) {
    DPRINTF(IDL_CRITICAL,("key_alloc: received null src or dst!\n"));
    return(-1);
  }

  /*
   *  We treat esp-transport mode and esp-tunnel mode 
   *  as a single type in the keytable.  
   */
  if (type == SS_ENCRYPTION_NETWORK)
    type = SS_ENCRYPTION_TRANSPORT;

  /*
   * Search key allocation table
   */
  bzero((char *)&buf, sizeof(buf));
  len = key_createkey((char *)&buf, type, src, dst, 0, 1);
  indx = key_gethashval((char *)&buf, len, KEYALLOCTBLSIZE);  

#define np_type np->keynode->secassoc->type
#define np_state np->keynode->secassoc->state
#define np_src (struct sockaddr *)&(np->keynode->secassoc->src)
#define np_dst (struct sockaddr *)&(np->keynode->secassoc->dst)
  
  prevnp = &keyalloctbl[indx];
  for (np = keyalloctbl[indx].next; np; np = np->next) {
    if ((type == np_type) && addrpart_equal(src, np_src) &&
	addrpart_equal(dst, np_dst) &&
	!(np_state & (K_LARVAL | K_DEAD | K_UNIQUE))) {
      if (!(unique_key))
	break;
      if (!(np_state & K_USED)) 
	break;
    }
    prevnp = np;
  }

  if (np) {
    struct key_so2spinode *newp;
    struct socketlist *newsp;
    int s = splnet();

    DPRINTF(IDL_MAJOR_EVENT,("key_alloc: found node to allocate\n"));
    keynode = np->keynode;

    K_Malloc(newnp, struct key_so2spinode *, sizeof(struct key_so2spinode));
    if (newnp == 0) {
      DPRINTF(IDL_CRITICAL,("key_alloc: Can't alloc mem for so2spi node!\n"));
      splx(s);
      return(ENOBUFS);
    }
    K_Malloc(newsp, struct socketlist *, sizeof(struct socketlist));
    if (newsp == 0) {
      DPRINTF(IDL_CRITICAL,("key_alloc: Can't alloc mem for socketlist!\n"));
      if (newnp)
	KFree(newnp);
      splx(s);
      return(ENOBUFS);
    }

    /*
     * Add a hash entry into the so2spi table to
     * map socket to allocated secassoc.
     */
    DPRINTF(IDL_GROSS_EVENT,("key_alloc: adding entry to so2spi table..."));
    newnp->keynode = keynode;
    newnp->socket = socket;
    newnp->next = so2spitbl[((u_int32)socket) % SO2SPITBLSIZE].next; 
    so2spitbl[((u_int32)socket) % SO2SPITBLSIZE].next = newnp;
    DPRINTF(IDL_GROSS_EVENT,("done\n"));

    if (unique_key) {
      /*
       * Need to remove the allocation entry
       * since the secassoc is now unique and 
       * can't be allocated to any other socket
       */
      DPRINTF(IDL_MAJOR_EVENT,("key_alloc: making keynode unique..."));
      keynode->secassoc->state |= K_UNIQUE;
      prevnp->next = np->next;
      KFree(np);
      DPRINTF(IDL_MAJOR_EVENT,("done\n"));
    }
    keynode->secassoc->state |= K_USED;
    keynode->secassoc->state |= K_OUTBOUND;
    keynode->alloc_count++;

    /*
     * Add socket to list of socket using secassoc.
     */
    DPRINTF(IDL_GROSS_EVENT,("key_alloc: adding so to solist..."));
    newsp->socket = socket;
    newsp->next = keynode->solist->next;
    keynode->solist->next = newsp;
    DPRINTF(IDL_GROSS_EVENT,("done\n"));
    *keynodep = keynode;
    splx(s);
    return(0);
  } 
  *keynodep = NULL;
  return(0);
}


/*----------------------------------------------------------------------
 * key_free():
 *      Decrement the refcount for a key table entry.  If the entry is 
 *      marked dead, and the refcount is zero, we go ahead and delete it.
 ----------------------------------------------------------------------*/
void
key_free(keynode)
     struct key_tblnode *keynode;
{
  DPRINTF(IDL_MAJOR_EVENT,("Entering key_free w/keynode=0x%x\n",keynode));
  if (!keynode) {
    DPRINTF(IDL_CRITICAL,("Warning: key_free got null pointer\n"));
    return;
  }
  (keynode->ref_count)--;
  if (keynode->ref_count < 0) {
    DPRINTF(IDL_CRITICAL,("Warning: key_free decremented refcount to %d\n",keynode->ref_count));
  }
  if ((keynode->secassoc->state & K_DEAD) && (keynode->ref_count <= 0)) {
    DPRINTF(IDL_MAJOR_EVENT,("key_free: calling key_delete\n"));
    key_delete(keynode->secassoc);
  }
}

/*----------------------------------------------------------------------
 * getassocbyspi():
 *      Get a security association for a given type, src, dst, and spi.
 *
 *      Returns: 0 if sucessfull
 *               -1 if error/not found
 *
 *      Caller must convert spi to host order.  Function assumes spi is  
 *      in host order!
 ----------------------------------------------------------------------*/
int
getassocbyspi(type, src, dst, spi, keyentry)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     u_int32 spi;
     struct key_tblnode **keyentry;
{
  char buf[MAXHASHKEYLEN];
  int len, indx;
  struct key_tblnode *keynode, *prevkeynode = 0;

  DPRINTF(IDL_GROSS_EVENT,("Entering getassocbyspi w/type=%u spi=%u\n",type,spi));

  /*
   *  We treat esp-transport mode and esp-tunnel mode 
   *  as a single type in the keytable.
   */
  if (type == SS_ENCRYPTION_NETWORK) 
    type = SS_ENCRYPTION_TRANSPORT;

  *keyentry = NULL;
  bzero(&buf, sizeof(buf));
  len = key_createkey((char *)&buf, type, src, dst, spi, 0);
  indx = key_gethashval((char *)&buf, len, KEYTBLSIZE);
  DPRINTF(IDL_FINISHED,("getassocbyspi: indx=%d\n",indx));
  DDO(IDL_FINISHED,dump_sockaddr(src);dump_sockaddr(dst));
  keynode = key_search(type, src, dst, spi, indx, &prevkeynode);
  DPRINTF(IDL_GROSS_EVENT,("getassocbyspi: keysearch ret=0x%x\n",keynode));
  if (keynode && !(keynode->secassoc->state & (K_DEAD | K_LARVAL))) {
    DPRINTF(IDL_EVENT,("getassocbyspi: found secassoc!\n"));
    (keynode->ref_count)++;
    *keyentry = keynode;
  } else {
    DPRINTF(IDL_MAJOR_EVENT,("getassocbyspi: secassoc not found!\n"));
    return (-1);
  }
  return(0);
}


/*----------------------------------------------------------------------
 * getassocbysocket():
 *      Get a security association for a given type, src, dst, and socket.
 *      If not found, try to allocate one.
 *      Returns: 0 if successfull
 *              -1 if error condition/secassoc not found (*keyentry = NULL)
 *               1 if secassoc temporarily unavailable (*keynetry = NULL)
 *                 (e.g., key mgnt. daemon(s) called)
 ----------------------------------------------------------------------*/
int
getassocbysocket(type, src, dst, socket, unique_key, keyentry)
     u_int type;
     struct sockaddr *src;
     struct sockaddr *dst;
     struct socket *socket;
     u_int unique_key;
     struct key_tblnode **keyentry;
{
  struct key_tblnode *keynode = 0;
  struct key_so2spinode *np;
  int len, indx;
  u_int32 spi;
  u_int realtype;
 
  DPRINTF(IDL_GROSS_EVENT,("Entering getassocbysocket w/type=%u so=0x%x\n",type,socket));

  /*
   *  We treat esp-transport mode and esp-tunnel mode 
   *  as a single type in the keytable.  This has a side
   *  effect that socket using both esp-transport and
   *  esp-tunnel will use the same security association
   *  for both modes.  Is this a problem?
   */
  realtype = type;
  if (type == SS_ENCRYPTION_NETWORK)
    type = SS_ENCRYPTION_TRANSPORT;

  if (np = key_sosearch(type, src, dst, socket)) {
    if (np->keynode && np->keynode->secassoc && 
	!(np->keynode->secassoc->state & (K_DEAD | K_LARVAL))) {
      DPRINTF(IDL_FINISHED,("getassocbysocket: found secassoc!\n"));
      (np->keynode->ref_count)++;
      *keyentry = np->keynode;
      return(0);
    }
  }

  /*
   * No secassoc has been allocated to socket, 
   * so allocate one, if available
   */
  DPRINTF(IDL_EVENT,("getassocbyso: can't find it, trying to allocate!\n"));
  if (key_alloc(realtype, src, dst, socket, unique_key, &keynode) == 0) {
    if (keynode) {
      DPRINTF(IDL_EVENT,("getassocbyso: key_alloc found secassoc!\n"));
      keynode->ref_count++;
      *keyentry = keynode;
      return(0);
    } else {
      /* 
       * Kick key mgnt. daemon(s) 
       * (this should be done in ipsec_output_policy() instead or
       * selectively called based on a flag value)
       */
      DPRINTF(IDL_FINISHED,("getassocbyso: calling key mgnt daemons!\n"));
      *keyentry = NULL;
      if (key_acquire(realtype, src, dst) == 0)
	return (1);
      else
	return(-1);
    }
  }
  *keyentry = NULL;
  return(-1);
}

