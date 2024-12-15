#include <stdlib.h>
#include <sys/param.h>
#include <sys/file.h>
#include "../../sys/sys/socket.h"
#include "../../sys/sys/socketvar.h"
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipsec.h>
#include <netinet6/key.h>

void
dump_in_addr6(in_addr6)
     struct in_addr6 *in_addr6;
{
  u_short *shorts = (u_short *)in_addr6;
  int i = 0;
 
  printf("(conv. for printing) ");
  while (i < 7)
    printf("%4x:",htons(shorts[i++]));
  printf("%4x\n",htons(shorts[7]));
}
 
void
dump_in_addr(in_addr)
     struct in_addr *in_addr;
{
  u_char *chars = (u_char *)in_addr;
  int i = 0;
 
  while (i < 3)
    printf("%d.",chars[i++]);
  printf("%d\n",chars[3]);
}

void
dump_sockaddr_in6(sin6)
     struct sockaddr_in6 *sin6;
{
  printf("sin6_len = %d, sin6_family = %d, sin6_port = %d (0x%x)\n",
         sin6->sin6_len,sin6->sin6_family, htons(sin6->sin6_port),
         htons(sin6->sin6_port));
  printf("sin6_flowinfo = 0x%x\n",sin6->sin6_flowinfo);
  printf("sin6_addr = ");
  dump_in_addr6(&sin6->sin6_addr);
}



void
dump_sockaddr_in(sin)
     struct sockaddr_in *sin;
{
  int i;
 
  printf("sin_len = %d, sin_family = %d, sin_port (conv.) = %d (0x%x)\n",
         sin->sin_len,sin->sin_family, htons(sin->sin_port),
         htons(sin->sin_port));
  printf("sin_addr = ");
  dump_in_addr(&sin->sin_addr);
  printf("sin_zero == ");
  for(i=0;i<8;i++)
    printf("0x%2x ",sin->sin_zero[i]);
  printf("\n");
}
 
void
dump_sockaddr(sa)
     struct sockaddr *sa;
{
  int i;
 
  printf("sa_len = %d, sa_family = %d, remaining bytes are:\n",
         sa->sa_len, sa->sa_family);
  for (i = 0; i <sa->sa_len - 2; i++)
    printf("0x%2x ",(unsigned char)sa->sa_data[i]);
  printf("\n");
}
 

void
dump_smart_sockaddr(sa)
     struct sockaddr *sa;
{
  if (sa == NULL) {
    printf("Can't print NULL sockaddr.\n");
    return;
  }
 
  switch (sa->sa_family)
    {
    case AF_INET6:
      dump_sockaddr_in6((struct sockaddr_in6 *)sa);
      break;
    case AF_INET:
      dump_sockaddr_in((struct sockaddr_in *)sa);
      break;
    default:
      dump_sockaddr(sa);
      break;
    }
}

void
dump_keymsghdr(km)
     struct key_msghdr *km;
{
  if (km) {
    printf("key_msglen=%d ", km->key_msglen);
    printf("key_msgvers=%d ", km->key_msgvers);
/*    printf("key_msgtype=%d ", km->key_msgtype);    */
    switch(km->key_msgtype) {
    case KEY_ADD:
      printf("key_msgtype=KEY_ADD ");
      break;
    case KEY_DELETE:
      printf("key_msgtype=KEY_DELETE ");
      break;
    case KEY_UPDATE:
      printf("key_msgtype=KEY_UPDATE ");
      break;
    case KEY_GET:
      printf("key_msgtype=KEY_GET ");
      break;
    case KEY_ACQUIRE:
      printf("key_msgtype=KEY_ACQUIRE ");
      break;
    case KEY_GETSPI:
      printf("key_msgtype=KEY_GETSPI ");
      break;
    case KEY_REGISTER:
      printf("key_msgtype=KEY_REGISTER ");
      break;
    case KEY_EXPIRE:
      printf("key_msgtype=KEY_EXPIRE ");
      break;
    case KEY_DUMP:
      printf("key_msgtype=KEY_DUMP ");
      break;
    case KEY_FLUSH:
      printf("key_msgtype=KEY_FLUSH ");
      break;
    default:
      printf("key_msgtype=%d ",km->key_msgtype);
    }
    printf("key_pid=%d ", km->key_pid);
    printf("key_seq=%d\n", km->key_seq);
    printf("key_errno=%d ", km->key_errno);
    printf("type=0x%x ", km->type);
    printf("state=0x%x ", km->state);
    printf("label=0x%x ", km->label);
    printf("spi=0x%x ", km->spi);
    printf("keylen=%d ", km->keylen);
    printf("ivlen=%d\n", km->ivlen);
    printf("algorithm=%d ", km->algorithm);
    printf("lifetype=0x%x ", km->lifetype);
    printf("lifetime1=%u ", km->lifetime1);
    printf("lifetime2=%u\n", km->lifetime2);
  } else
    printf("key_msghdr pointer is NULL!\n");
}

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
    printf("key is:\n");
    dumpbuf(kp->key, kp->keylen);
    printf("iv is:\n");
    dumpbuf(kp->iv, kp->ivlen);
#undef dumpbuf    
  } else
    printf("key_msgdata pointer is NULL!\n");
}

 

struct m_keymsg {
  struct key_msghdr km;
  char m_buffer[512];
};

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
 
#define ROUNDUP(a) \
  ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) \
    { x += ROUNDUP(n); if (cp > cpmax) return(-1); }
 
  /* Need to clean up this code later */
 
  /* Grab src addr */
  kip->src = (struct sockaddr *)cp;
  if (!kip->src->sa_len) {
    return(-1);
  }
  ADVANCE(cp, kip->src->sa_len);

  /* Grab dest addr */
  kip->dst = (struct sockaddr *)cp;
  if (!kip->dst->sa_len) {
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

int
main(argc, argv)
     int argc;
     char **argv;
{
  struct m_keymsg m_keymsg;
  struct m_keymsg im_keymsg;
  struct key_msgdata keyinfo;
  struct sockaddr_in sa;
  struct sockaddr_in6 sa6;
  struct hostent *hp;
  pid_t mypid;
  char iv[4]; 
  char key[8];
  int i, s, l, len;
  char *cp = m_keymsg.m_buffer;
  struct sockaddr_in6 dummyfrom;

  mypid = getpid();
  if (mypid < 0) {
    perror("getpid");
    exit(1);
  }

  if (!(s = socket(PF_KEY, SOCK_RAW, 0))) {
    perror("socket");
    exit(1);
  }
  bzero((char *)&dummyfrom, sizeof(struct sockaddr_in6));
  dummyfrom.sin6_family = AF_INET6;
  dummyfrom.sin6_len = sizeof(struct sockaddr_in6);
 
  bzero((char *)&m_keymsg, sizeof(m_keymsg));
  m_keymsg.km.key_msgvers = KEY_VERSION;
  m_keymsg.km.key_msgtype = KEY_REGISTER;
  m_keymsg.km.key_msglen = sizeof(struct key_msghdr); 

  for (i = 1; i < 4; i++) {
    m_keymsg.km.type = i;
    if ((len = write(s, (char *)&m_keymsg, m_keymsg.km.key_msglen)) < 0) {
      perror("write");
      exit(1);
    }
  }


readit:
  if ((len = read(s, (char *)&im_keymsg, 256)) < 0) {
    perror("read");
  }
  printf("--------------Got key message from kernel!----------\n");
  printf("keymsghdr is:\n");
  dump_keymsghdr(&im_keymsg);
  if ((im_keymsg.km.key_msgtype == KEY_ADD) || 
      (im_keymsg.km.key_msgtype == KEY_UPDATE) ||
      ((im_keymsg.km.key_msgtype == KEY_GET) && (im_keymsg.km.key_pid))) {
    if (key_xdata(&im_keymsg, len, &keyinfo, 0) == 0) {
      printf("keydata is:\n");
      dump_keymsginfo(&keyinfo);
    }
  } else  if ((im_keymsg.km.key_msgtype == KEY_DELETE) || 
	      (im_keymsg.km.key_msgtype == KEY_ACQUIRE) ||
	      (im_keymsg.km.key_msgtype == KEY_GET) ||
	      (im_keymsg.km.key_msgtype == KEY_GETSPI)) {
    if (key_xdata(&im_keymsg, len, &keyinfo, 1) == 0) {
      printf("keydata is:\n");
      dump_keymsginfo(&keyinfo);
    }
  }

  switch(im_keymsg.km.key_msgtype) {
  case KEY_ACQUIRE:
    {
      u_long waittime;

/*      waittime = random() % 10;   */
      waittime = 3; 
      printf("\nKernel begging for key...");
      printf("sleeping for %u seconds before responding!\n", waittime);
      sleep(waittime);
      printf("Sending key_getspi message...");
      cp = m_keymsg.m_buffer;
      bzero((char *)&m_keymsg, sizeof(m_keymsg));
      m_keymsg.km.key_msgvers = KEY_VERSION;
      m_keymsg.km.key_msgtype = KEY_GETSPI;
      m_keymsg.km.key_seq = 1;
      m_keymsg.km.type = im_keymsg.km.type;

#define ADDDATA(a, b) \
  {l = ROUNDUP(b); bcopy((char *)(a), cp, l); cp += l;}

      ADDDATA(keyinfo.src, keyinfo.src->sa_len);
      ADDDATA(keyinfo.dst, keyinfo.dst->sa_len);
      m_keymsg.km.key_msglen = cp - (char *)&m_keymsg;
      if (write(s, (char *)&m_keymsg, m_keymsg.km.key_msglen) < 0) {
	perror("write");
      } else
	printf("done\n");
    }
    break;
  case KEY_GETSPI:
    {
      u_long word1, word2;

      if (im_keymsg.km.key_pid != mypid)
	break;
      if (im_keymsg.km.key_errno) {
	printf("\nKernel returns error for getspi operation\n");
	break;
      }
      printf("\nKernel returns spi, cool!\n");
      printf("Sending key_update message to update key material...");
      cp = m_keymsg.m_buffer;
      bzero((char *)&m_keymsg, sizeof(m_keymsg));
      m_keymsg.km = im_keymsg.km;
      m_keymsg.km.key_msgtype = KEY_UPDATE;
      m_keymsg.km.key_seq = 1;
      if (m_keymsg.km.type == SS_AUTHENTICATION)
	m_keymsg.km.algorithm = IPSEC_ALGTYPE_AUTH_MD5;
      else if (m_keymsg.km.type == SS_ENCRYPTION_TRANSPORT || 
	       m_keymsg.km.type == SS_ENCRYPTION_NETWORK)
	m_keymsg.km.algorithm = IPSEC_ALGTYPE_ESP_DES_CBC;

      m_keymsg.km.keylen = sizeof(key);
      m_keymsg.km.ivlen = sizeof(iv);
      word1 = random();
      word2 = random();
      bcopy((char *)&word1, (char *)&key, sizeof(word1));
      bcopy((char *)&word2, (char *)&key + sizeof(word1), sizeof(word2));
      word1 = random();
      bcopy((char *)&word1, (char *)&iv, sizeof(word1));

      ADDDATA(keyinfo.src, keyinfo.src->sa_len);
      ADDDATA(keyinfo.dst, keyinfo.dst->sa_len);
      ADDDATA(&dummyfrom, dummyfrom.sin6_len);
      ADDDATA(key, m_keymsg.km.keylen);
      ADDDATA(iv, m_keymsg.km.ivlen);
      m_keymsg.km.key_msglen = cp - (char *)&m_keymsg;
      if (write(s, (char *)&m_keymsg, m_keymsg.km.key_msglen) < 0) {
	perror("write");
      } else
	printf("done\n");
    }
    break;
  }
goto readit;
  
}
