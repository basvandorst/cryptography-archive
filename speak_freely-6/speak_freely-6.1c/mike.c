/*

			Speak Freely for Unix
		  Network sound transmission program

	Designed and implemented in July of 1991 by John Walker

*/

#include "speakfree.h"

/*  Destination host descriptor.  */

struct destination {
    struct destination *dnext;	      /* Next destination in list */
    char *server;		      /* Host name identifier string */
    struct sockaddr_in name;	      /* Internet address */
    struct sockaddr_in ctrl;	      /* RTCP port address */
    unsigned char time_to_live;       /* Multicast scope specification */
    char deskey[9];		      /* Destination DES key, if any */
    char rtpdeskey[9];		      /* Destination RTP DES key, if any */
    char vatdeskey[9];		      /* Destination VAT DES key, if any */
    char ideakey[17];		      /* Destination IDEA key, if any */
    char pgpkey[17];		      /* Destination PGP key, if any */
    char *otp;			      /* Key file */
};

static char *progname;		      /* Program name */
static int sock;		      /* Communication socket */
static struct destination *dests = NULL, *dtail;
static int compressing = FALSE;       /* Compress sound: simple 2X */
static int gsmcompress = TRUE;	      /* GSM compress buffers */
static int lpccompress = FALSE;       /* LPC compress buffers */
static int lpc10compress = FALSE;     /* LPC-10 compress buffers */
static int adpcmcompress = FALSE;     /* ADPCM compress buffers */
static int toasted = FALSE;	      /* Sending already-compressed file ? */
static int robust = 1;		      /* Robust duplicate packets mode */
static int rseq = 0;		      /* Robust mode packet sequence number */

#define DEFAULT_SQUELCH 4096	      /* Default squelch level */
static int squelch = 0; 	      /* Squelch level if > 0 */
static int sqdelay = 12000;	      /* Samples to delay before squelch */
static int sqwait = 0;		      /* Squelch delay countdown */
#ifdef PUSH_TO_TALK
static int push = TRUE; 	      /* Push to talk mode */
static int talking = FALSE;	      /* Push to talk button state */
static int rawmode = FALSE;	      /* Is terminal in raw mode ? */
#endif
static int ring = FALSE;	      /* Force speaker & level on next pkt ? */
static int rtp = FALSE; 	      /* Use Internet Real-Time Protocol */
static int vat = FALSE; 	      /* Use VAT protocol */
static int agc = FALSE; 	      /* Automatic gain control active ? */
static int rgain = 33;		      /* Current recording gain level */
static int spurt = TRUE;	      /* True for first packet of talk spurt */
static int debugging = FALSE;	      /* Debugging enabled here and there ? */
static int havesound = FALSE;	      /* True if we've acquired sound input */
static char hostname[20];	      /* Host name to send with packets */
static int loopback = FALSE;	      /* Remote loopback mode */
static gsm gsmh;		      /* GSM handle */
static struct adpcm_state adpcm = {0, 0}; /* ADPCM compression state */

static char *vatid = NULL;	      /* VAT ID packet */
static int vatidl;		      /* VAT ID packet length */

static unsigned long ssrc;	      /* RTP synchronisation source identifier */
static unsigned long timestamp;       /* RTP packet timestamp */
static unsigned short seq;	      /* RTP packet sequence number */
static unsigned long rtpdesrand;      /* RTP DES random RTCP prefix */

static char *sdes = NULL;	      /* RTP SDES packet */
static int sdesl;		      /* RTP SDES packet length */

static char curkey[9] = "";	      /* Current DES key if curkey[0] != 0 */
static char currtpkey[9] = "";	      /* Current RTP DES key if currtpkey[0] != 0 */
static char curvatkey[9] = "";	      /* Current VAT DES key if currtpkey[0] != 0 */
static char curideakey[17] = "";      /* Current IDEA key if curideakey[0] != 0 */
static char curpgpkey[17] = "";       /* Current PGP key if curpgpkey[0] != 0 */
static char *curotp = NULL;	      /* Key file buffer */
#ifdef TINY_PACKETS
static int sound_packet;	      /* Ideal samples/packet */
#endif
static struct soundbuf *pgpsb = NULL; /* PGP key sound buffer, if any */
static LONG pgpsbl;		      /* Length of PGP key sound buffer data */
#ifdef HALF_DUPLEX
static struct in_addr localhost;      /* Our internet address */
static int halfDuplexMuted = FALSE;   /* Muted by half-duplex transmission */
#endif
static int hasFace = FALSE;	      /* Is a face image available ? */

#ifdef sgi
static long usticks;		      /* Microseconds per clock tick */
#endif

#define TimerStep   (7 * 1000000L)    /* Alarm interval in microseconds */

#ifdef HEWLETT_PACKARD
extern int HPTermHookChar;
#endif

#define ucase(x)    (islower(x) ? toupper(x) : (x))

/*  ULARM  --  Wrapper for setitimer() that looks like alarm()
	       but accepts a time in microseconds.  */

static void ularm(t)
  long t;
{
    struct itimerval it;

    it.it_value.tv_sec = t / 1000000L;
    it.it_value.tv_usec = t % 1000000L;
    it.it_interval.tv_sec = it.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &it, NULL);
}

/*  GSMCOMP  --  Compress the contents of a sound buffer using GSM.  */

static void gsmcomp(sb)
  struct soundbuf *sb;
{
    gsm_signal src[160];
    gsm_frame dst;
    int i, j, l = 0;
    char *dp = ((char *) sb->buffer.buffer_val) + sizeof(short);

    sb->compression |= fCompGSM;
    for (i = 0; i < sb->buffer.buffer_len; i += 160) {
	for (j = 0; j < 160; j++) {
	    if ((i + j) < sb->buffer.buffer_len) {
		src[j] = audio_u2s(sb->buffer.buffer_val[i + j]);
	    } else {
		src[j] = 0;
	    }
	}
	gsm_encode(gsmh, src, dst);
	bcopy(dst, dp, sizeof dst);
	dp += sizeof dst;
	l += sizeof dst;
    }

    /* Hide original uncompressed buffer length in first 2 bytes of buffer. */

    *((short *) sb->buffer.buffer_val) = sb->buffer.buffer_len;
    sb->buffer.buffer_len = l + sizeof(short);
}

/*  ADPCMCOMP  --  Compress the contents of a sound buffer using ADPCM.  */

static void adpcmcomp(sb)
  struct soundbuf *sb;
{
    unsigned char *dp = (unsigned char *) sb->buffer.buffer_val;
    struct adpcm_state istate;

    istate = adpcm;
    sb->compression |= fCompADPCM;
    adpcm_coder_u(dp, (char *) dp, sb->buffer.buffer_len, &adpcm);
    sb->buffer.buffer_len /= 2;

    /* Hide the ADPCM encoder state at the end of this buffer.
       The shifting and anding makes the code byte-order
       insensitive. */

    dp += sb->buffer.buffer_len;
    *dp++ = ((unsigned int) istate.valprev) >> 8;
    *dp++ = istate.valprev & 0xFF;
    *dp = istate.index;
    sb->buffer.buffer_len += 3;
}

/*  LPCCOMP  --  Compress the contents of a sound buffer using LPC.  */

static void lpccomp(sb)
  struct soundbuf *sb;
{
    int i, l = 0;
    char *dp = ((char *) sb->buffer.buffer_val) + sizeof(short);
    unsigned char *src = ((unsigned char *) sb->buffer.buffer_val);
    lpcparams_t lp;

    sb->compression |= fCompLPC;
    for (i = 0; i < sb->buffer.buffer_len; i += LPC_FRAME_SIZE) {
	lpc_analyze(src + i, &lp);
	bcopy(&lp, dp, LPCRECSIZE);
	dp += LPCRECSIZE;
	l += LPCRECSIZE;
    }

    /* Hide original uncompressed buffer length in first 2 bytes of buffer. */

    *((short *) sb->buffer.buffer_val) = htons((short) sb->buffer.buffer_len);
    sb->buffer.buffer_len = l + sizeof(short);
}

/*  LPC10COMP  --  Compress the contents of a sound buffer using LPC-10.  */

static void lpc10comp(sb)
  struct soundbuf *sb;
{
    char *dp = (char *) sb->buffer.buffer_val;
    unsigned char *src = ((unsigned char *) sb->buffer.buffer_val);

    sb->compression |= fCompLPC10;
    sb->buffer.buffer_len = lpc10encode(src, dp, sb->buffer.buffer_len);
}

/*  LPC10STUFF	--  Stuff last 16 bytes of LPC10 packet in sendinghost. */

static int lpc10stuff(sb, pktlen)
  struct soundbuf *sb;
  int pktlen;
{
    if ((sb->compression & fCompLPC10) && (sb->buffer.buffer_len > 16)) {
	bcopy(((char *) sb) + (pktlen - (sizeof sb->sendinghost)),
	    sb->sendinghost, sizeof sb->sendinghost);
	pktlen -= sizeof sb->sendinghost;
    }
    return pktlen;
}

/*  SENDRTPCTRL  --  Send RTP or VAT control message, encrypting if
		     necessary.  */

static int sendrtpctrl(d, destaddr, msg, msgl, rtppkt)
  struct destination *d;
  struct sockaddr *destaddr;
  char *msg;
  int msgl, rtppkt;
{
    char aux[1024];

    if ((rtp || vat) && d->rtpdeskey[0]) {
	int vlen;
	des_cblock key;
	des_key_schedule sched;
	des_cblock ivec;

	bzero(ivec, 8);

	if (rtppkt) {

	    /* Encrypted RTCP messages are prefixed with 4 random
	       bytes to prevent known plaintext attacks. */

	    bcopy(&rtpdesrand, aux, 4);
	    bcopy(msg, aux + 4, msgl);
	    msgl += 4;
	} else {
	    bcopy(msg, aux, msgl);
	}

	/* If we're DES encrypting we must round the size of
	   the data to be sent to be a multiple of 8 so that
	   the entire DES frame is sent.  This applies only to
	   VAT, as the code that creates RTCP packets guarantees
	   they're already padded to a multiple of 8 bytes. */

	vlen = msgl;
	msgl = (msgl + 7) & (~7);
	if (msgl > vlen) {
	    bzero(aux + vlen, msgl - vlen);
	}
	if (debugging) {
	    fprintf(stderr, "Encrypting %d VAT/RTP bytes with DES key.\r\n",
		    msgl);
	}
	des_set_key((des_cblock *) ((rtppkt ? d->rtpdeskey : d->vatdeskey) + 1), sched);
	des_ncbc_encrypt((des_cblock *) aux,
	    (des_cblock *) aux, msgl, sched,
	    (des_cblock *) ivec, DES_ENCRYPT);
	msg = aux;
    }
    return sendto(sock, msg, msgl, 0, destaddr, sizeof d->ctrl);
}

/*  ADDEST  --	Add destination host to host list.  */

static int addest(host)
  char *host;
{
    struct destination *d;
    struct hostent *h;
    long naddr;
    unsigned int ttl = 1;
    char *mcs;
    int curport = Internet_Port;

    /* If a multicast scope descriptor appears in the name, scan
       it and lop it off the end.  We'll apply it later if we discover
       this is actually a multicast address. */

    if ((mcs = strrchr(host, '/')) != NULL &&
	((strchr(host, ':') != NULL) || (mcs != strchr(host, '/')))) {
	*mcs = 0;
#ifdef MULTICAST
	ttl = atoi(mcs);
#endif
    }

    /* If a port number appears in the name, scan it and lop
       it off.	We allow a slash as well as the documented colon
       as the delimiter to avoid confusing users familiar with
       VAT. */

    if ((mcs = strrchr(host, ':')) != NULL ||
	(mcs = strrchr(host, '/')) != NULL) {
	*mcs++ = 0;
	curport = atoi(mcs);
    }

    /* If it's a valid IP number, use it.  Otherwise try to look
       up as a host name. */

    if ((naddr = inet_addr(host)) != -1) {
    } else {
	h = gethostbyname(host);
	if (h == 0) {
	    fprintf(stderr, "%s: unknown host\n", host);
	    return FALSE;
	}
	bcopy((char *) h->h_addr, (char *) &naddr, sizeof naddr);
    }

#ifdef MULTICAST
    if (!IN_MULTICAST(naddr)) {
	ttl = 0;
    }
#endif

    d = (struct destination *) malloc(sizeof(struct destination));
    d->dnext = NULL;
    d->server = host;
    bcopy((char *) &naddr, (char *) &(d->name.sin_addr), sizeof naddr);
    bcopy((char *) &naddr, (char *) &(d->ctrl.sin_addr), sizeof naddr);
    bcopy(curkey, d->deskey, 9);
    bcopy(currtpkey, d->rtpdeskey, 9);
    bcopy(curvatkey, d->vatdeskey, 9);
    bcopy(curideakey, d->ideakey, 17);
    bcopy(curpgpkey, d->pgpkey, 17);
    d->otp = curotp;
    d->time_to_live = ttl;
    d->name.sin_family = AF_INET;
    d->name.sin_port = htons(curport);
    d->ctrl.sin_family = AF_INET;
    d->ctrl.sin_port = htons(curport + 1);
    if (dests == NULL) {
	dests = d;
    } else {
	dtail->dnext = d;
    }
    dtail = d;

    /* Send initial status and identity message in
       the correct protocol. */

    if (rtp) {
	if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			sdes, sdesl, TRUE) < 0) {
	    perror("sending initial RTCP SDES packet");
	    return FALSE;
	}
    } else if (vat) {
	if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
		       vatid, vatidl, FALSE) < 0) { 
	    perror("sending initial VAT ID control packet");
	    return FALSE;
	}
    } else {
	char pid = sdes[0];

	/* Set Speak Freely protocol flag in packet */
	sdes[0] = (sdes[0] & 0x3F) | (1 << 6);
	if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			sdes, sdesl, TRUE) < 0) {
	    perror("sending initial Speak Freely SDES control packet");
	    return FALSE;
	}
	sdes[0] = pid;
    }

    if (pgpsb != NULL) {
	int i;

	/* "What I tell you three times is true".  When we change
	   over to a TCP connection bracketing the UDP sound data
	   transmission, we can send this just once, knowing it has
	   arrived safely. */

	for (i = 0; i < 3; i++) {
	    if (sendto(sock, (char *) pgpsb, (sizeof(struct soundbuf) - BUFL) + pgpsbl,
		0, (struct sockaddr *) &(d->name), sizeof d->name) < 0) {
		perror("sending PGP session key");
		break;
	    }
	}
/* No need to sleep once ack from far end confirms key decoded. */
sleep(7);
    }
    return TRUE;
}

/*  TIMERTICK  --  Timer tick signal-catching function.  */

static void timertick()
{
    struct destination *d;

    for (d = dests; d != NULL; d = d->dnext) {
#ifdef MULTICAST
	if (IN_MULTICAST(d->name.sin_addr.s_addr)) {
	    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *) &(d->time_to_live), sizeof d->time_to_live);
	    if (debugging) {
		fprintf(stderr, "Multicasting ID/SDES with scope of %d to %s.\n",
		    d->time_to_live, inet_ntoa(d->name.sin_addr));
	    }
	}
#endif
	if (rtp) {
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			 sdes, sdesl, TRUE) < 0) {
		perror("sending RTCP SDES packet");
	    }
	} else if (vat) {
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			vatid, vatidl, FALSE) < 0) { 
		perror("sending VAT ID control packet");
	    }
	} else {
	    char pid = sdes[0];

	    /* Set Speak Freely protocol flag in packet */
	    sdes[0] = (sdes[0] & 0x3F) | (1 << 6);
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			    sdes, sdesl, TRUE) < 0) {
		perror("sending Speak Freely SDES control packet");
	    }
	    sdes[0] = pid;
	}
    }

    ularm(TimerStep);
    signal(SIGALRM, timertick);       /* Reset signal to handle timeout */
}

/*  SENDPKT  --  Send a message to all active destinations.  */

static int sendpkt(sb)
  struct soundbuf *sb;
{
    struct destination *d;
    int pktlen;

    if (gsmcompress && !toasted) {
	gsmcomp(sb);
    }

    if (adpcmcompress && !toasted) {
	adpcmcomp(sb);
    }

    if (lpccompress && !toasted) {
	lpccomp(sb);
    }

    if (lpc10compress && !toasted) {
	lpc10comp(sb);
    }

    if (hasFace) {
	sb->compression |= fFaceOffer;
    }

    pktlen = sb->buffer.buffer_len + (sizeof(struct soundbuf) - BUFL);

    if (vat) {
	pktlen = vatout(sb, 0L, timestamp, spurt);
	timestamp += sound_packet;
    }

    if (rtp) {
	pktlen = rtpout(sb, ssrc, timestamp, seq, spurt);
	seq++;
	timestamp += sound_packet;
    }

    spurt = FALSE;		      /* Not the start of a talk spurt */

    for (d = dests; d != NULL; d = d->dnext) {
#ifdef MULTICAST
	if (IN_MULTICAST(d->name.sin_addr.s_addr)) {
	    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *) &(d->time_to_live), sizeof d->time_to_live);
	    if (debugging) {
		fprintf(stderr, "Multicasting with scope of %d to %s.\n",
		    d->time_to_live, inet_ntoa(d->name.sin_addr));
	    }
	}
#endif
	if (d->deskey[0] || d->ideakey[0] || d->pgpkey[0] || d->otp != NULL) {
	    soundbuf ebuf;
	    int i;
	    LONG slen, plen = 0;

	    bcopy(sb, &ebuf, pktlen);
	    slen = ebuf.buffer.buffer_len;

	    /* DES encryption. */

	    if (d->deskey[0]) {
		if (rtp || vat) {
		    int vlen = pktlen;
		    des_cblock key;
		    des_key_schedule sched;
		    des_cblock ivec;

		    /* If we're DES encrypting we must round the size of
		       the data to be sent to be a multiple of 8 so that
		       the entire DES frame is sent.  If this is an RTP
		       packet, we may have to set the Pad bit in the
		       header and include a count of pad bytes at the end
		       of the packet. */

		    bzero(ivec, 8);
		    pktlen = (pktlen + 7) & (~7);
		    if (debugging) {
			fprintf(stderr, "Encrypting %d VAT/RTP bytes with DES key.\r\n",
				pktlen);
		    }
		    if (pktlen > vlen) {
			bzero(((char *) &ebuf) + vlen, pktlen - vlen);
		    }
		    if (rtp && (pktlen > vlen)) {
			char *p = (char *) &ebuf;

			p[0] |= 0x20; /* Set pad bytes present bit */
			p[pktlen - 1] = pktlen - vlen; /* Set pad count at end */
		    }
		    des_set_key((des_cblock *) ((rtp ? d->rtpdeskey : d->vatdeskey) + 1), sched);
		    des_ncbc_encrypt((des_cblock *) &ebuf,
			(des_cblock *) &ebuf, pktlen, sched,
			(des_cblock *) ivec, DES_ENCRYPT);
		    if (vat) {
			pktlen = vlen;
		    }
		} else {
		    setkey(d->deskey + 1);

		    /* If we're DES encrypting we must round the size of
		       the data to be sent to be a multiple of 8 so that
		       the entire DES frame is sent. */

		    slen = (slen + 7) & (~7);
		    pktlen = slen + (sizeof(struct soundbuf) - BUFL);
		    if (debugging) {
			fprintf(stderr, "Encrypting %d bytes with DES key.\r\n",
				slen);
		    }
		    for (i = 0; i < slen; i += 8) {

			/* Apply cipher block chaining within the packet. */

			if (i > 0) {
			    int j;

			    for (j = 0; j < 8; j++) {
				ebuf.buffer.buffer_val[(i + j)] ^=
				    ebuf.buffer.buffer_val[(i + j) - 8];
			    }
			}
			endes(ebuf.buffer.buffer_val + i);
		    }
		    ebuf.compression |= fEncDES;
		}
	    }

	    /* IDEA encryption. */

	    if (d->ideakey[0]) {
		unsigned short iv[4];

		bzero(iv, sizeof(iv));
		initcfb_idea(iv, d->ideakey + 1, FALSE);

		/* If we're IDEA encrypting we must round the size of
		   the data to be sent to be a multiple of 8 so that
		   the entire IDEA frame is sent. */

		slen = (slen + 7) & (~7);
		pktlen = slen + (sizeof(struct soundbuf) - BUFL);
		if (debugging) {
		    fprintf(stderr, "Encrypting %d bytes with IDEA key.\r\n",
			    slen);
		}
		ideacfb(ebuf.buffer.buffer_val, slen);
		close_idea();
		ebuf.compression |= fEncIDEA;
	    }

	    /* PGP encryption. */

	    if (d->pgpkey[0]) {
		unsigned short iv[4];

		bzero(iv, sizeof(iv));
		initcfb_idea(iv, d->pgpkey + 1, FALSE);

		/* If we're PGP IDEA encrypting we must round the size of
		   the data to be sent to be a multiple of 8 so that
		   the entire IDEA frame is sent. */

		slen = (slen + 7) & (~7);
		pktlen = slen + (sizeof(struct soundbuf) - BUFL);
		if (debugging) {
		    fprintf(stderr, "Encrypting %d bytes with PGP key.\r\n",
			    slen);
		}
		ideacfb(ebuf.buffer.buffer_val, slen);
		close_idea();
		ebuf.compression |= fEncPGP;
	    }

	    /* Key file encryption. */

	    if (d->otp != NULL) {
		if (debugging) {
		    fprintf(stderr, "Encrypting %d bytes with key file.\r\n",
			    slen);
		}
		for (i = 0; i < slen; i ++) {
		    ebuf.buffer.buffer_val[i] ^= d->otp[i];
		}
		ebuf.compression |= fEncOTP;
	    }

	    if (!rtp && !vat) {
		if (lpc10compress && !toasted) {
		    pktlen = lpc10stuff(&ebuf, pktlen);
		    if (robust > 1) {
			ebuf.compression |= fCompRobust;
			roSeq(&ebuf) = rseq;
			rseq = (rseq + 1) & 0xFF;
		    }
		}
		ebuf.compression = htonl(ebuf.compression);
		ebuf.buffer.buffer_len = htonl(ebuf.buffer.buffer_len);
	    }
	    for (i = 0; i < (lpc10comp ? robust : 1); i++) {
		if (sendto(sock, (char *) &ebuf, pktlen,
		    0, (struct sockaddr *) &(d->name), sizeof d->name) < 0) {
		    perror("sending datagram message");
		    return FALSE;
		}
	    }
	} else {
	    int i, wasf = FALSE;

	    if (!vat && !rtp) {
		wasf = TRUE;
		if (lpc10compress && !toasted) {
		    pktlen = lpc10stuff(sb, pktlen);
		    if (robust > 1) {
			sb->compression |= fCompRobust;
			sb->buffer.buffer_len |= ((long) rseq) << 24;
			rseq = (rseq + 1) & 0xFF;
		    }
		}
		sb->compression = htonl(sb->compression);
		sb->buffer.buffer_len = htonl(sb->buffer.buffer_len);
	    }

	    for (i = 0; i < (lpc10comp ? robust : 1); i++) {
		if (sendto(sock, (char *) sb, pktlen,
		    0, (struct sockaddr *) &(d->name), sizeof d->name) < 0) {
		    perror("sending datagram message");
		    return FALSE;
		}
	    }
	    if (wasf) {
		sb->compression = ntohl(sb->compression);
		sb->buffer.buffer_len = ntohl(sb->buffer.buffer_len);
		if (sb->compression & fCompRobust) {
		    sb->compression &= ~fCompRobust;
		    sb->buffer.buffer_len &= 0xFFFFFFL;
		}
		if ((sb->compression & fCompLPC10) &&
		    (sb->buffer.buffer_len > 16)) {
		    pktlen += sizeof(sb->sendinghost);
		}
	    }
	}
    }
    return TRUE;
}

/*  GETAUDIO  --  Open audio input.  If audio hardware is half duplex,
		  we may have to mute output in progress.  */

static int getaudio()
{
    int i;

#ifdef HALF_DUPLEX
    struct soundbuf hdreq;
    struct sockaddr_in name;

    /* If we're afflicted with half-duplex hardware, make several
       attempts to mute the output device and open audio input.
       We send a mute even if the output isn't active as a courtesy
       to the output program; if a later attempt to acquire output
       fails, he'll know it was a result of being muted. */

    for (i = 5; i > 0; i--) {
	hdreq.compression = htonl(fProtocol | fHalfDuplex | fHalfDuplexMute);
	strcpy(hdreq.sendinghost, hostname);
	hdreq.buffer.buffer_len = 0;

	bcopy((char *) &localhost, (char *) &(name.sin_addr), sizeof localhost);
	name.sin_family = AF_INET;
	name.sin_port = htons(Internet_Port);
	if (debugging) {
	    fprintf(stderr,"Sending fHalfDuplex | fHalfDuplexMute Request\n");
	}
	if (sendto(sock, &hdreq, (sizeof(struct soundbuf) - BUFL),
		   0, (struct sockaddr *) &name, sizeof name) < 0) {
	    perror("sending half-duplex mute request");
	}
	sf_usleep(50000L);
	if (soundinit(O_RDONLY)) {
	    halfDuplexMuted = TRUE;
	    break;
	}
    }

    /* If we failed to initialise, send a resume just in case
       one of our mute requests made it through anyway. */

    if (i <= 0) {
	hdreq.compression = htonl(fProtocol | fHalfDuplex | fHalfDuplexResume);
	if (sendto(sock, &hdreq, (sizeof(struct soundbuf) - BUFL),
		   0, (struct sockaddr *) &name, sizeof name) < 0) {
	    perror("sending half-duplex resume request");
	}
    }
#else
    i = soundinit(O_RDONLY);
#endif

    if (i <= 0) {
	    fprintf(stderr, "%s: unable to initialise audio.\n", progname);
	    return FALSE;
    }
    havesound = TRUE;
    if (agc) {
	soundrecgain(rgain);	  /* Set initial record level */
    }
    return TRUE;
}

/*  FREEAUDIO  --  Release sound input device, sending a resume to
		   the output device if we've muted it due to
		   half-duplex hardware.  */

static void freeaudio()
{
    if (havesound) {
	if (debugging) {
	    fprintf(stderr, "Restoring audio modes at release.\n");
	}
	soundterm();
	havesound = FALSE;
    }

#ifdef HALF_DUPLEX

    /* When bailing out, make sure we don't leave the output
       muted. */

    if (halfDuplexMuted) {
	struct soundbuf hdreq;
	struct sockaddr_in name;

	hdreq.compression = htonl(fProtocol | fHalfDuplex | fHalfDuplexResume);
	strcpy(hdreq.sendinghost, hostname);
	hdreq.buffer.buffer_len = 0;

	bcopy((char *) &localhost, (char *) &(name.sin_addr), sizeof localhost);
	name.sin_family = AF_INET;
	name.sin_port = htons(Internet_Port);
	if (sendto(sock, &hdreq, (sizeof(struct soundbuf) - BUFL),
		   0, (struct sockaddr *) &name, sizeof name) < 0) {
	    perror("sending half-duplex resume request");
	}
	halfDuplexMuted = FALSE;
    }
#endif
}

/*  EXITING  --  Catch as many program termination signals as
		 possible and restore initial audio modes before
		 we exit.  */

static void exiting()
{
    struct destination *d;

#ifdef PUSH_TO_TALK
    if (rawmode) {
	fprintf(stderr, "\r	 \r");
	fcntl(fileno(stdin), F_SETFL, 0);
	nocbreak();
	echo();
	endwin();
    }
#endif

    if (rtp) {
	for (d = dests; d != NULL; d = d->dnext) {
	    char v[1024];
	    int l;

	    l = rtp_make_bye(v, ssrc, "Exiting Speak Freely", TRUE);
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			 v, l, TRUE) < 0) {
		perror("sending RTCP BYE packet");
	    }
	}
    } else if (vat) {
	for (d = dests; d != NULL; d = d->dnext) {
	    char v[16];
	    int l;

	    l = makevatdone(v, 0L);
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			 v, l, FALSE) < 0) {
		perror("sending VAT DONE control packet");
	    }
	}
    } else {
	for (d = dests; d != NULL; d = d->dnext) {
	    char v[1024];
	    int l;

	    l = rtp_make_bye(v, ssrc, "Exiting Speak Freely", TRUE);
	    v[0] = (v[0] & 0x3F) | (1 << 6);
	    if (sendrtpctrl(d, (struct sockaddr *) &(d->ctrl),
			 v, l, TRUE) < 0) {
		perror("sending Speak Freely BYE packet");
	    }
	}
    }

    freeaudio();
    exit(0);
}

/*  TERMCHAR  --  Check for special characters from console when
		  in raw mode.	*/

static void termchar(ch)    
  int ch;
{
#define Ctrl(x) ((x) - '@')
    if (ch == 'q' || ch == 'Q' || ch == 27 ||
	ch == Ctrl('C') || ch == Ctrl('D')) {
	exiting();
    }
}

/*  SENDFILE  --  Send a file or, if the file name is NULL or a
		  single period, send real-time sound input. */

static int sendfile(f)
  char *f;
{ 
    soundbuf netbuf;
#define buf netbuf.buffer.buffer_val
    int bread, nread, lread;
    FILE *afile = NULL;
    static int firstTime = TRUE;

    if (firstTime) {
	firstTime = FALSE;
	signal(SIGHUP, exiting);     /* Set signal to handle termination */
	signal(SIGINT, exiting);     /* Set signal to handle termination */
	signal(SIGTERM, exiting);    /* Set signal to handle termination */
	signal(SIGALRM, timertick);  /* Set signal to handle timer */
	ularm(TimerStep);
    }

#ifdef TINY_PACKETS

    /* Compute the number of sound samples needed to fill a
       packet of TINY_PACKETS bytes. */

    if (rtp) {
	sound_packet = (gsmcompress | lpccompress | adpcmcompress) ? (160 * 4)
		       : 320;
    } else if (vat) {
	sound_packet = (gsmcompress | lpccompress | adpcmcompress) ? (160 * 4)
		       : 320;
    } else {
	sound_packet = ((TINY_PACKETS - ((sizeof(soundbuf) - BUFL))) *
			(compressing ? 2 : 1));
	if (gsmcompress) {
	    sound_packet = compressing ? 3200 : 1600;
	} else if (adpcmcompress) {
	    sound_packet *= 2;
	    sound_packet -= 4;		  /* Leave room for state at the end */
	} else if (lpccompress) {
	    sound_packet = 10 * LPC_FRAME_SIZE;
	} else if (lpc10compress) {
	    sound_packet = compressing ? 3600 : 1800;
	}
    }

#ifdef SHOW_PACKET_SIZE
    printf("Samples per packet = %d\n", sound_packet);
#endif
#endif
    lread = sound_packet;

    strcpy(netbuf.sendinghost, hostname);
    if (f != NULL && (strcmp(f, ".") != 0)) {
	char magic[4];

	afile = fopen(f, "r");
	if (afile == NULL) {
	    fprintf(stderr, "Unable to open sound file %s.\n", f);
	    return 2;
	}

	toasted = FALSE;
	if ((strlen(f) > 4) && (strcmp(f + (strlen(f) - 4), ".gsm") == 0)) {
	    toasted = TRUE;
	    lread = 33 * ((rtp || vat) ? 4 : 10);
	    sound_packet = (160 * lread) / 33;
	} else {

	    /* If the file has a Sun .au file header, skip it.
	       Note that we still blithely assume the file is
	       8-bit ISDN u-law encoded at 8000 samples per
	       second. */

	    fread(magic, 1, sizeof(long), afile);
	    if (bcmp(magic, ".snd", 4) == 0) {
		long startpos;

		fread(&startpos, sizeof(long), 1, afile);
		fseek(afile, ntohl(startpos), 0);
	    } else {
		fseek(afile, 0L, 0);
	    }
	}
    }

    /* Send a file */

    if (afile) {
	int tlast = FALSE;
#ifdef BSD_like
	struct timeb t1, t2, tl;
#else
	struct timeval t1, t2, tl;
#endif
	long et, corr = 0;

	while (
#ifdef BSD_like
	    ftime(&t1),
#else
	    gettimeofday(&t1, NULL),
#endif

	    (bread = nread =
		fread(buf + (toasted ? 2 : 0), 1,
#ifdef TINY_PACKETS
				  lread,
#else
				  (gsmcompress ? 1600 : BUFL),
#endif
				  afile)) > 0) {
	    netbuf.compression = fProtocol | (ring ? (fSetDest | fDestSpkr) : 0);
	    ring = FALSE;
	    netbuf.compression |= debugging ? fDebug : 0;
	    netbuf.compression |= loopback ? fLoopBack : 0;
	    if (toasted) {
		*((short *) netbuf.buffer.buffer_val) = bread = (160 * nread) / 33;
		netbuf.buffer.buffer_len = lread + sizeof(short);
		netbuf.compression |= fCompGSM;
	    } else {
		if (compressing) {
#ifdef OLD2X
		    int i;

		    nread /= 2;
		    for (i = 1; i < nread; i++) {
			buf[i] = buf[i * 2];
		    }
#else
		    int is = nread, os = nread / 2;

		    rate_flow(buf, buf, &is, &os);
		    nread = os;
#endif
		    netbuf.compression |= fComp2X;
		}
		netbuf.buffer.buffer_len = nread;
	    }
	    if (!sendpkt(&netbuf)) {
		fclose(afile);
		toasted = FALSE;
		exiting();
	    }

	    /* The following code is needed because when we're reading
	       sound from a file, as opposed to receiving it in real
	       time from the CODEC, we must meter out the samples
	       at the rate they will actually be played by the destination
	       machine.  For a 8000 samples per second, this amounts
	       to 125 microseconds per sample, minus the time we spent
	       compressing the data (which is substantial for GSM) and
	       encrypting the data.

	       What we're trying to accomplish here is to maintain a
	       predictable time between packets equal to the sample
	       rate.  This is, of course, very difficult since system
	       performance and instantaneous load changes the relationship
	       between the compute time per packet and the length of
	       the sound it represents.  What the code below does is
	       measure the actual time between packets, compare it to
	       the desired time, then use the error signal as input
	       to an exponentially smoothed moving average with P=0.1
	       which adjusts the time, in microseconds, we wait between
	       packets.  This quickly adapts to the correct wait time
	       based on system performance and options selected, while
	       minimsing perturbations due to momentary changes in
	       system load.  Due to incompatibilities between System V
	       and BSD-style gettimeofday(), we do this differently
	       for the two styles of system.  If the system conforms
	       to neither convention, we just guess based on the number
	       of samples in the packet and a fudge factor. */

#ifdef TINY_PACKETS
#define kOverhead   8000
#else
#define kOverhead   3000
#endif

#ifdef BSD_like
	    ftime(&t2);
	    if (tlast) {
		long dt =  1000 * (((t2.time - tl.time) * 1000) +
				   (t2.millitm - tl.millitm)),
			   atime = bread * 125, error;
		error = atime - dt;
		if (error > atime / 2) {
		    error = atime / 2;
		} else if (error < -(atime / 2)) {
		    error = -(atime / 2);
		}
		corr = corr - 0.1 * error;
		et = atime - corr;
#ifdef DEBUG_SOUNDFILE_WAIT
printf("Packet time %d Delta t = %d Error = %d, Corr = %d, Wait = %d\n",
	atime, dt, error, corr, et);
#endif
	    } else {
#ifdef __FreeBSD__
		corr = 80000;
#else
		corr = 8000;
#endif
		et = ((bread * 125) -
		      1000 * (((t2.time - t1.time) * 1000) +
			       (t2.millitm - t1.millitm))) - kOverhead;
	    }
	    tl = t2;
	    tlast = TRUE;
#else
	    gettimeofday(&t2, NULL);
	    if (tlast) {
		long dt = (t2.tv_sec - tl.tv_sec) * 1000000 +
				(t2.tv_usec - tl.tv_usec),
		     atime = bread * 125, error;
		error = atime - dt;
		if (error > atime / 2) {
		    error = atime / 2;
		} else if (error < -(atime / 2)) {
		    error = -(atime / 2);
		}
		corr = corr - 0.1 * error;
		et = atime - corr;
#ifdef DEBUG_SOUNDFILE_WAIT
printf("Packet time %d Delta t = %d Error = %d, Corr = %d, Wait = %d\n",
	atime, dt, error, corr, et);
#endif
	    } else {
		et = ((bread * 125) -
			      (t2.tv_sec - t1.tv_sec) * 1000000 +
			      (t2.tv_usec - t1.tv_usec)) - kOverhead;
	    }
	    tl = t2;
	    tlast = TRUE;
#endif

	    if (et > 0) {
		sf_usleep(et);
	    }
	    if (debugging && !vat && !rtp) {
		fprintf(stderr, "Sent %d samples from %s in %d bytes.\r\n",
			bread, f, ntohl(netbuf.buffer.buffer_len));
	    }
	}

	if (debugging) {
	    fprintf(stderr, "Sent sound file %s.\r\n", f);
	}
	fclose(afile);
	toasted = FALSE;
    } else {

	    /* Send real-time audio. */

#ifdef PUSH_TO_TALK
	if (push) {
	    char c;
	    int l;

	    fprintf(stderr,
		"Space bar switches talk/pause, Esc or \"q\" to quit\nPause: ");
	    fflush(stderr);
	    initscr();
	    noecho();
	    cbreak();
	    rawmode = TRUE;
	    while (TRUE) {
		if (l = read(fileno(stdin), &c, 1) == 1 || errno != EINTR) {
		    break;
		}
	    }	
	    if (l != 1) {
		perror("waiting for first Pause/Talk character");
	    }
	    termchar(c);
/*	    fprintf(stderr, "\rTalk:  "); */
	    fprintf(stderr, squelch > 0 ? "\rQuiet:  " : "\rTalk:   ");
	    fcntl(fileno(stdin), F_SETFL, O_NDELAY);
	    talking = TRUE;
	    spurt = TRUE;
	}
#endif

	/* Send real-time sound. */

#ifdef Solaris
	setaubufsize(
#ifdef TINY_PACKETS
	    sound_packet
#else
	    sizeof buf
#endif
		    );
#endif

	if (!getaudio()) {
	    fprintf(stderr, "Unable to initialise audio.\n");
	    return 2;
	}
	while (TRUE) {
	    int soundel = 0;
	    unsigned char *bs = (unsigned char *) buf;

	    if (havesound) {
		soundel = soundgrab(buf,
#ifdef TINY_PACKETS
					 sound_packet
#else
					 sizeof buf
#endif
				   );
	    }

#ifdef PUSH_TO_TALK
	    while (push) {
		char c;
		int rlen;

		if ((rlen = read(fileno(stdin), &c, 1)) > 0
#ifdef HEWLETT_PACKARD
		    || HPTermHookChar
#endif
		   ) {
#ifdef HEWLETT_PACKARD
		    c = HPTermHookChar;
#endif
		    termchar(c);
		    talking = !talking;
		    fflush(stderr);
#ifdef HALF_DUPLEX

		    /* For half-duplex, acquire and release the
		       audio device at each transition.  This lets
		       us mute the output only while in Talk mode. */

		    if (talking) {
			if (!getaudio()) {
			    fprintf(stderr, "Audio device busy.\n");
			    talking = FALSE;
			}
		    } else {
			freeaudio();
		    }
#endif
		    fprintf(stderr, talking ? (squelch > 0 ?
			    "\rQuiet: " : "\rTalk:   ") : "\rPause: ");
		    fcntl(fileno(stdin), F_SETFL, talking ? O_NDELAY : 0);
		    if (talking) {
			spurt = TRUE;
			/* Discard all backlog sound input. */
			soundflush();
		    } else {
			sqwait = 0;   /* Reset squelch delay */
		    }
		    break;
		} else {
/*
if (rlen == -1) {
perror("Read poll"); fflush(stderr);
}
*/
		    if (rlen == -1 && errno == EINTR) {
			continue;
		    }
		    break;
		}
	    }
#endif

	    if (soundel > 0) {
		register unsigned char *start = bs;
		register int j;
		int squelched = (squelch > 0), osl = soundel;

		/* If entire buffer is less than squelch, ditch it.  If
		   we haven't received sqdelay samples since the last
		   squelch event, continue to transmit. */

		if (sqdelay > 0 && sqwait > 0) {
		    if (debugging) {
			printf("Squelch countdown: %d samples left.\n",
			    sqwait);
		    }
		    sqwait -= soundel;
		    squelched = FALSE;
#ifdef PUSH_TO_TALK
		    if (sqwait <= 0 && push) {
			fprintf(stderr, "\rQuiet: ");
		    }
#endif
		} else if (squelch > 0) {
		    for (j = 0; j < soundel; j++) {
#ifdef USQUELCH
			if (((*start++ & 0x7F) ^ 0x7F) > squelch)
#else
			int samp = audio_u2s(*start++);

			if (samp < 0) {
			    samp = -samp;
			}
			if (samp > squelch)
#endif
			{
			    squelched = FALSE;
#ifdef PUSH_TO_TALK
			    if (sqwait <= 0 && push) {
			       fprintf(stderr, "\rTalk:  ");
			    }
#endif
			    sqwait = sqdelay;
			    break;
			}
		    }
		}

		if (squelched) {
		    if (debugging) {
			printf("Entire buffer squelched.\n");
		    }
		    spurt = TRUE;
		} else {
		    netbuf.compression = fProtocol | (ring ? (fSetDest | fDestSpkr) : 0);
		    netbuf.compression |= debugging ? fDebug : 0;
		    netbuf.compression |= loopback ? fLoopBack : 0;

		    /* If automatic gain control is enabled,
		       ride the gain pot to fill the dynamic range
		       optimally. */

		    if (agc) {
			register unsigned char *start = bs;
			register int j;
			long msamp = 0;

			for (j = 0; j < soundel; j++) {
			    int s = audio_u2s(*start++);

			    msamp += (s < 0) ? -s : s;
			}
			msamp /= soundel;
			if (msamp < 6000) {
			    if (rgain < 100) {
				soundrecgain(++rgain);
			    }
			} else if (msamp > 7000) {
			    if (rgain > 1) {
				soundrecgain(--rgain);
			    }
			}
		    }

		    ring = FALSE;
		    if (compressing) {
#ifdef OLD2X
			int i;

			soundel /= 2;
			for (i = 1; i < soundel; i++) {
			    buf[i] = buf[i * 2];
			}
#else
			int is = soundel, os = soundel / 2;

			rate_flow(buf, buf, &is, &os);
			soundel = os;
#endif
			netbuf.compression |= fComp2X;
		    }
		    netbuf.buffer.buffer_len = soundel;
		    if (!sendpkt(&netbuf)) {
			exiting();
		    }
		    if (debugging && !vat && !rtp) {
			fprintf(stderr, "Sent %d audio samples in %d bytes.\r\n",
				osl, soundel);
		    }
		}
	    } else {
		sf_usleep(100000L);  /* Wait for some sound to arrive */
	    }
	}
    }
    return 0;
}

/*  MAKESESSIONKEY  --	Generate session key with optional start
			key.  If mode is TRUE, the key will be
			translated to a string, otherwise it is
			returned as 16 binary bytes.  */

static void makeSessionKey(key, seed, mode)
  char *key, *seed;
  int mode;
{
    int j, k;
    struct MD5Context md5c;
    char md5key[16], md5key1[16];
    char s[1024];

    s[0] = 0;
    if (seed != NULL) {
	strcat(s, seed);
    }

    /* The following creates a seed for the session key generator
       based on a collection of volatile and environment-specific
       information unlikely to be vulnerable (as a whole) to an
       exhaustive search attack.  If one of these items isn't
       available on your machine, replace it with something
       equivalent or, if you like, just delete it. */

    sprintf(s + strlen(s), "%u", getpid());
    sprintf(s + strlen(s), "%u", getppid());
    V getcwd(s + strlen(s), 256);
    sprintf(s + strlen(s), "%u", clock());
    V cuserid(s + strlen(s));
    sprintf(s + strlen(s), "%u", time(NULL));
#ifdef Solaris
    sysinfo(SI_HW_SERIAL,s + strlen(s), 12);
#else
    sprintf(s + strlen(s), "%u", gethostid());
#endif
    getdomainname(s + strlen(s), 256);
    gethostname(s + strlen(s), 256);
    sprintf(s + strlen(s), "%u", getuid());
    sprintf(s + strlen(s), "%u", getgid());
    MD5Init(&md5c);
    MD5Update(&md5c, s, strlen(s));
    MD5Final(md5key, &md5c);
    sprintf(s + strlen(s), "%u", (time(NULL) + 65121) ^ 0x375F);
    MD5Init(&md5c);
    MD5Update(&md5c, s, strlen(s));
    MD5Final(md5key1, &md5c);
    init_idearand(md5key, md5key1, time(NULL));
    if (mode) {
	for (j = k = 0; j < 16; j++) {
	    unsigned char rb = idearand();

#define Rad16(x) ((x) + 'A')
	    key[k++] = Rad16((rb >> 4) & 0xF);
	    key[k++] = Rad16(rb & 0xF);
#undef Rad16
	    if (j & 1) {
		key[k++] = '-';
	    }
	}
	key[--k] = 0;
    } else {
	for (j = 0; j < 16; j++) {
	    key[j] = idearand();
	}
    }
    close_idearand();
}

/*  PROG_NAME  --  Extract program name from argv[0].  */

static char *prog_name(arg)
  char *arg;
{
    char *cp = strrchr(arg, '/');

    return (cp != NULL) ? cp + 1 : arg;
}

/*  USAGE  --  Print how-to-call information.  */

static void usage()
{
    V fprintf(stderr, "%s  --  Speak Freely sound sender.\n", progname);
    V fprintf(stderr, " 	   %s.\n", Relno);
    V fprintf(stderr, "\n");
    V fprintf(stderr, "Usage: %s hostname[:port] [options] [ file1 / . ]...\n", progname);
    V fprintf(stderr, "Options: (* indicates defaults)\n");
#ifdef PUSH_TO_TALK
    V fprintf(stderr, " 	  -A	     Always transmit unless squelched\n");
    V fprintf(stderr, "     *	  -B	     Push to talk using keyboard\n");
#endif
    V fprintf(stderr, " 	  -C	     Compress subsequent sound\n");
    V fprintf(stderr, " 	  -D	     Enable debug output\n");
    V fprintf(stderr, " 	  -E[key]    Emit session key string\n");
    V fprintf(stderr, " 	  -F	     ADPCM compression\n");
    V fprintf(stderr, " 	  -G	     Automatic gain control\n");
    V fprintf(stderr, " 	  -Ikey      IDEA encrypt with key\n");
    V fprintf(stderr, " 	  -Kkey      DES encrypt with key\n");
    V fprintf(stderr, " 	  -L	     Remote loopback\n");
    V fprintf(stderr, " 	  -LPC	     LPC compression\n");
    V fprintf(stderr, " 	  -LPC10Rn   LPC-10 compression, n copies\n");
    V fprintf(stderr, "     *	  -M	     Manual record gain control\n");
    V fprintf(stderr, " 	  -N	     Do not compress subsequent sound\n");
    V fprintf(stderr, " 	  -Ofile     Use file as key file\n");
    V fprintf(stderr, " 	  -Phostname[:port] Party line, add host to list\n");
    V fprintf(stderr, "     *	  -Q	     Disable debug output\n");
    V fprintf(stderr, " 	  -R	     Ring--force volume, output to speaker\n");
    V fprintf(stderr, " 	  -RTP	     Use Internet Real-Time Protocol\n");
    V fprintf(stderr, " 	  -Sn,t      Squelch at level n (0-32767), timeout t milliseconds\n");
    V fprintf(stderr, "     *	  -T	     Telephone (GSM) compression\n");
    V fprintf(stderr, " 	  -U	     Print this message\n");
    V fprintf(stderr, " 	  -VAT	     Use VAT protocol\n");
    V fprintf(stderr, " 	  -Z\"user..\" Send PGP session key for user(s)\n");
    V fprintf(stderr, "\n");
    V fprintf(stderr, "by John Walker\n");
    V fprintf(stderr, "   WWW:	  http://www.fourmilab.ch/\n");
}

/*  Main program.  */

main(argc, argv)
  int argc;
  char *argv[];
{
    int i, j, k, l, sentfile = 0;
    FILE *fp;
    struct MD5Context md5c;
    static lpcstate_t lpcc;
    char md5key[16], md5key1[16];
    char s[1024];

    progname = prog_name(argv[0]);
    gethostname(hostname, sizeof hostname);
    if (strlen(hostname) > 15) {
	hostname[15] = 0;
    }
#ifdef LINUX_FPU_FIX
    __setfpucw(_FPU_IEEE);	      /* Mask floating point interrupts to
					 enable standard IEEE processing.
					 Current libc releases do this by
					 default, so this is needed only on
					 older libraries. */
#endif
#ifndef OLD2X
    rate_start(8000, 4000);
#endif
    gsmh = gsm_create();

    /* Initialise LPC decoding. */

    if (!lpc_start()) {
	fprintf(stderr, "Cannot allocate LPC decoding memory.\n");
	return 1;
    }
    lpc_init(&lpcc);
    lpc10init();

#ifdef sgi
    usticks = 1000000 / CLK_TCK;
#endif

#ifdef HALF_DUPLEX
    {
	struct hostent *h;
	char host[512];

	gethostname(host, sizeof host);
	h = gethostbyname(host);
	bcopy((char *) (h->h_addr), (char *) &localhost,
	    sizeof localhost);
#ifdef HDX_DEBUG
	fprintf(stderr, "%s: local host %s = %s\n", progname, host,
		inet_ntoa(localhost));
#endif
    }
#endif

    /* Create the socket used to send data. */

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	perror("opening datagram socket");
	return 1;
    }

    /* Initialise randomised packet counters. */

    makeSessionKey(md5key, NULL, FALSE);
    bcopy(md5key, (char *) &ssrc, sizeof ssrc);
    bcopy(md5key + sizeof ssrc, (char *) &timestamp,
	  sizeof timestamp);
    bcopy(md5key + sizeof ssrc + sizeof timestamp,
	  (char *) &seq, sizeof seq);
    bcopy(md5key + sizeof ssrc + sizeof timestamp +
	  sizeof seq, &rtpdesrand, sizeof rtpdesrand);
    sdesl = rtp_make_sdes(&sdes, ssrc, -1, TRUE);

    /* See if there's a face image file for this user.	If
       so, we'll offer our face in sound packets we send. */

    {	char *cp = getenv("SPEAKFREE_FACE");
	FILE *facefile;

	if (cp != NULL) {
	    if ((facefile = fopen(cp, "r")) == NULL) {
		fprintf(stderr, "%s: cannot open SPEAKFREE_FACE file %s\n",
		    progname, cp);
	    } else {
		fclose(facefile);
		hasFace = TRUE;
	    }
	}
    }

    /*	Process command line options.  */

    for (i = 1; i < argc; i++) {
	char *op, opt;

	op = argv[i];
	if (*op == '-') {
	    opt = *(++op);
	    if (islower(opt)) {
		opt = toupper(opt);
	    }

	    switch (opt) {

#ifdef PUSH_TO_TALK
		case 'A':	      /* -A  --  Always transmit (no push to talk) */
		    push = FALSE;
		    break;

		case 'B':	      /* -B  -- Push to talk (button) */
		    if (isatty(fileno(stdin))) {
			push = TRUE;
		    }
		    break;
#endif

		case 'C':	      /* -C  -- Compress sound samples */
		    compressing = TRUE;
		    break;

		case 'D':	      /* -D  --  Enable debug output  */
		    debugging = TRUE;
		    break;

		case 'E':	      /* -E  --  Emit session key and exit */
		    makeSessionKey(s, op + 1, TRUE);
		    printf("%s\n", s);
		    return 0;

		case 'F':	      /* -F -- ADPCM compression */
		    adpcmcompress = TRUE;
		    gsmcompress = lpccompress = lpc10compress = FALSE;
		    break;

		case 'G':	      /* -G  --  Automatic gain control */
		    agc = TRUE;
		    break;

		case 'I':	      /* -Ikey	--  Set IDEA key */
		    if (strlen(op + 1) == 0) {
			curideakey[0] = FALSE;
		    } else {
			MD5Init(&md5c);
			MD5Update(&md5c, op + 1, strlen(op + 1));
			MD5Final(curideakey + 1, &md5c);
			curideakey[0] = TRUE;
			if (debugging) {
			    fprintf(stderr, "IDEA key:");
			    for (j = 1; j < 17; j++) {
				fprintf(stderr, " %02X", (curideakey[j] & 0xFF));
			    }
			    fprintf(stderr, "\n");
			}
		    }
		    break;

		case 'K':	      /* -Kkey	--  Set DES key */
		    desinit(1);       /* Initialise the DES library */
		    if (strlen(op + 1) == 0) {
			curkey[0] = currtpkey[0] = curvatkey[0] = FALSE;
		    } else {
			char algorithm[16];

			MD5Init(&md5c);
			MD5Update(&md5c, op + 1, strlen(op + 1));
			MD5Final(md5key, &md5c);
			for (j = 0; j < 8; j++) {
			    curkey[j + 1] = (char)
					  ((md5key[j] ^ md5key[j + 8]) & 0x7F);
			}
			if (debugging) {
			    fprintf(stderr, "DES key:");
			    for (j = 0; j < 8; j++) {
				fprintf(stderr, " %02X", (curkey[j + 1] & 0xFF));
			    }
			    fprintf(stderr, "\n");
			}
			curkey[0] = TRUE;
			des_string_to_key(op + 1, (des_cblock *) (curvatkey + 1));
			string_DES_key(op + 1, currtpkey + 1, algorithm);
			if (strcmp(algorithm, "DES-CBC") != 0) {
			    fprintf(stderr, "Unsupported encryption algorithm: %s.  Only DES-CBC is available.\n",
				    algorithm);
			    return 2;
			}
			currtpkey[0] = curvatkey[0] = TRUE;
			if (debugging) {
			    fprintf(stderr, "RTP key:");
			    for (j = 0; j < 8; j++) {
				fprintf(stderr, " %02X", (currtpkey[j + 1] & 0xFF));
			    }
			    fprintf(stderr, "\n");
			}
			if (debugging) {
			    fprintf(stderr, "VAT key:");
			    for (j = 0; j < 8; j++) {
				fprintf(stderr, " %02X", (curvatkey[j + 1] & 0xFF));
			    }
			    fprintf(stderr, "\n");
			}
		    }
		    break;

		case 'L':	      /* -L	 --  Remote loopback */
				      /* -LPC	 --  LPC compress sound */
				      /* -LPC10  --  LPC10 compress sound */
		    if (ucase(op[1]) == 'P') {
			if (strlen(op) > 3 && op[3] == '1') {
			    lpc10compress = TRUE;
			    gsmcompress = adpcmcompress = lpccompress = FALSE;
			    robust = 1;
			    if (strlen(op) > 5 && ucase(op[5]) == 'R') {
				robust = 2;
				if (strlen(op) > 6 && isdigit(op[6])) {
				    robust = atoi(op + 6);
				    if (robust <= 0) {
					robust = 1;
				    } else if (robust > 4) {
					robust = 4;
				    }
				}
			    }
			} else {
			    lpccompress = TRUE;
			    gsmcompress = adpcmcompress = lpc10compress = FALSE;
			}
		    } else {
			loopback = TRUE;
		    }
		    break;

		case 'M':	      /* -M  --  Manual record gain control */
		    agc = FALSE;
		    break;

		case 'N':	      /* -N  --  Do not compress sound samples */
		    compressing = FALSE;
		    gsmcompress = FALSE;
		    lpccompress = FALSE;
		    lpc10compress = FALSE;
		    adpcmcompress = FALSE;
		    break;

		case 'O':	      /* -Ofile -- Use file as key file */
		    if (op[1] == 0) {
			curotp = NULL; /* Switch off key file */
		    } else {
			fp = fopen(op + 1, "r");
			if (fp == NULL) {
			    perror("Cannot open key file");
			    return 2;
			}
			curotp = malloc(BUFL);
			if (curotp == NULL) {
			    fprintf(stderr, "Cannot allocate key file buffer.\n");
			    return 2;
			}
			l = fread(curotp, 1, BUFL, fp);
			if (l == 0) {
			    /* Idiot supplied void key file.  Give 'im
			       what he asked for: no encryption. */
			    curotp[0] = 0;
			    l = 1;
			}
			fclose(fp);
			/* If the file is shorter than the maximum buffer
			   we may need to encrypt, replicate the key until
			   the buffer is filled. */
			j = l;
			k = 0;
			while (j < BUFL) {
			    curotp[j++] = curotp[k++];
			    if (k >= l) {
				k = 0;
			    }
			}
		    }
		    break;

		case 'P':	      /* -Phost  --  Copy output to host  */
		    if (!addest(op + 1)) {
			return 1;
		    }
		    break;

		case 'Q':	      /* -Q  --  Disable debug output  */
		    debugging = FALSE;
		    break;

		case 'R':	      /* -R    --  Ring: divert output to speaker */
				      /* -RTP  --  Use RTP to transmit */
		    if (ucase(op[1]) == 'T') {
			rtp = TRUE;
		    } else {
			ring = TRUE;
		    }
		    break;

		case 'S':	      /* -Sn,d	--  Squelch at level n,
						    delay d milliseconds */
		    if (strlen(op + 1) == 0) {
			squelch = DEFAULT_SQUELCH; /* Default squelch */
		    } else {
			char *cp;

			if (op[1] != ',') {
			    squelch = atoi(op + 1);
			} else {
			    squelch = DEFAULT_SQUELCH;
			}
			cp = strchr(op + 1, ',');
			if (cp != NULL) {
			    sqdelay = 8 * atoi(cp + 1);
			}
		    }
		    break;

		case 'T':	      /* -T -- Telephone (GSM) compression */
		    gsmcompress = TRUE;
		    lpccompress = lpc10compress = adpcmcompress = FALSE;
		    break;

		case 'U':	      /* -U  --  Print usage information */
		case '?':	      /* -?  --  Print usage information */
		    usage();
		    return 0;

		case 'V':	      /* -V  --  Use VAT protocol */
		    vat = TRUE;
		    if (vatid == NULL) {
			vatidl = makeVATid(&vatid, 0L);
		    }
		    break;

		case 'Z':	      /* -Z"user1 user2..."  --  Send PGP
						 encrypted session key to
						 named users */
		    if (op[1] == 0) {
			curpgpkey[0] = FALSE;
		    } else {
			char c[80], f[40];
			FILE *kfile;
			FILE *pipe;
			long flen;

			sprintf(f, "/tmp/.SF_SKEY%d", getpid());
			sprintf(c, "pgp -fe +nomanual +verbose=0 +armor=off %s >%s", op + 1, f);
#ifdef PGP_DEBUG
			fprintf(stderr, "Encoding session key with: %s\n", c);
#endif
			pipe = popen(c, "w");
			if (pipe == NULL) {
			    fprintf(stderr, "Unable to open pipe to: %s\n", c);
			    return 2;
			} else {
			    makeSessionKey(curpgpkey + 1, NULL, FALSE);
#ifdef PGP_DEBUG
			    {	
				int i;

				fprintf(stderr, "Session key:", hostname);
				for (i = 0; i < 16; i++) {
				    fprintf(stderr, " %02X", curpgpkey[i + 1] & 0xFF);
				}
				fprintf(stderr, "\n");
			    }
#endif
			    /* The reason we start things off right with
			       "Special K" is to prevent the session key
			       (which can be any binary value) from
			       triggering PGP's detection of the file as
			       one already processed by PGP.  This causes an
			       embarrassing question when we are attempting
			       to run silent. */
			    curpgpkey[0] = 'K';
			    fwrite(curpgpkey, 17, 1, pipe);
			    curpgpkey[0] = FALSE;
			    fflush(pipe);
			    pclose(pipe);
			}
			kfile = fopen(f, "r");
			if (kfile == NULL) {
			    fprintf(stderr, "Cannot open key file %s\n", f);
			} else {
			    fseek(kfile, 0L, 2);
			    flen = ftell(kfile);
			    rewind(kfile);
#ifdef PGP_DEBUG
			    fprintf(stderr, "PGP key buffer length: %d\n", flen);
#endif
#ifdef TINY_PACKETS
			    if (flen > (TINY_PACKETS -
				    (sizeof(soundbuf) - BUFL))) {
				fprintf(stderr, "Warning: PGP key message exceeds %d packet size.\n", TINY_PACKETS);
			    }
#endif
			    if (pgpsb != NULL) {
				free(pgpsb);
			    }
			    pgpsb = (soundbuf *) malloc(((unsigned) flen) +
				      (TINY_PACKETS - (sizeof(soundbuf) - BUFL)));
			    if (pgpsb == NULL) {
				fprintf(stderr, "Cannot allocate PGP sound buffer.\n");
				fclose(kfile);
				unlink(f);
				return 2;
			    }
			    fread(pgpsb->buffer.buffer_val, (int) flen, 1, kfile);
			    pgpsbl = flen;
			    pgpsb->buffer.buffer_len = htonl(flen);
			    pgpsb->compression = htonl(fProtocol | fKeyPGP);
#ifdef SENDMD5
			    MD5Init(&md5c);
			    MD5Update(&md5c, pgpsb->buffer.buffer_val, pgpsb->buffer.buffer_len);
			    MD5Final(pgpsb->sendinghost, &md5c);
#else
			    strcpy(pgpsb->sendinghost, hostname);
#endif
			    fclose(kfile);
			    curpgpkey[0] = TRUE;
			}
			unlink(f);
		    }
		    break;
	    }
	} else {

	    /*	Check for conflicting options.	*/

	    if (rtp && vat) {
		fprintf(stderr, "Cannot choose both -RTP and -VAT protocols simultaneously.\n");
		return 2;
	    }

	    if (rtp || vat) {
		if (curideakey[0] || curpgpkey[0] || curotp != NULL) {
		    fprintf(stderr, "IDEA, PGP, and Key File encryption cannot be used\n    with -RTP or -VAT protocols.\n");
		    return 2;
		}
#ifdef ZZZZ
		if (vat && curkey[0]) {
		    fprintf(stderr, "DES encryption not implemented for VAT protocol.\n");
		    return 2;
		}
#endif
		if (compressing) {
		    fprintf(stderr, "Simple (-C) compression cannot be used with\n   -RTP or -VAT protocols.\n");
		    return 2;
		}
		if (lpc10compress) {
		    fprintf(stderr, "LPC-10 (-LPC10) compression cannot be used with\n	 -RTP or -VAT protocols.\n");
		    return 2;
		}
		if (ring) {
		    fprintf(stderr, "Warning: ring not implemented in -RTP and -VAT protocols.\n");
		}
	    }

	    if (dests == NULL) {
		if (!addest(op)) {
		    return 1;
		}
	    } else {
		int ok = sendfile(op);
		if (ok != 0)
		    return ok;
		sentfile++;
	    }
	}
    }

    if (dests == NULL) {
	usage();
    } else {
	if (sentfile == 0) {
	    return sendfile(NULL);
	}
    }

    exiting();
    gsm_destroy(gsmh);
    lpc_end();
    desdone();
    return 0;
}
