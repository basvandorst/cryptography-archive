/*	BSDI	savefile.c,v 2.1 1995/02/03 18:12:21 polk Exp	*/
/*
 * Copyright (c) 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef lint
static char rcsid[] =
    "@(#)/master/usr.sbin/tcpdump/libpcap/savefile.c,v 2.1 1995/02/03 18:12:21 polk Exp (LBL)";
#endif

/*
 * savefile.c - supports offline use of tcpdump
 *	Extraction/creation by Jeffrey Mogul, DECWRL
 *	Modified by Steve McCanne, LBL.
 *
 * Used to save the received packet headers, after filtering, to
 * a file, and then read them later.
 * The first record in the file contains saved values for the machine
 * dependent values so we can print the dump file on any architecture.
 */

#include <sys/types.h>
#include <sys/time.h>

#ifndef __bsdi__
#include <net/bpf.h>
#endif

#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pcap-int.h"

#define TCPDUMP_MAGIC 0xa1b2c3d4

/*
 * We use the "receiver-makes-right" approach to byte order,
 * because time is at a premium when we are writing the file.
 * In other words, the pcap_file_header and pcap_pkthdr,
 * records are written in host byte order.
 * Note that the packets are always written in network byte order.
 *
 * ntoh[ls] aren't sufficient because we might need to swap on a big-endian
 * machine (if the file was written in little-end order).
 */
#define	SWAPLONG(y) \
((((y)&0xff)<<24) | (((y)&0xff00)<<8) | (((y)&0xff0000)>>8) | (((y)>>24)&0xff))
#define	SWAPSHORT(y) \
	( (((y)&0xff)<<8) | (((y)&0xff00)>>8) )

#define SFERR_TRUNC		1
#define SFERR_BADVERSION	2
#define SFERR_BADF		3
#define SFERR_EOF		4 /* not really an error, just a status */

static int
sf_write_header(FILE *fp, int linktype, int thiszone, int snaplen)
{
	struct pcap_file_header hdr;

	hdr.magic = TCPDUMP_MAGIC;
	hdr.version_major = PCAP_VERSION_MAJOR;
	hdr.version_minor = PCAP_VERSION_MINOR;

	hdr.thiszone = thiszone;
	hdr.snaplen = snaplen;
	hdr.sigfigs = 0;
	hdr.linktype = linktype;

	if (fwrite((char *)&hdr, sizeof(hdr), 1, fp) != 1)
		return (-1);

	return (0);
}

static void
swap_hdr(struct pcap_file_header *hp)
{
	hp->version_major = SWAPSHORT(hp->version_major);
	hp->version_minor = SWAPSHORT(hp->version_minor);
	hp->thiszone = SWAPLONG(hp->thiszone);
	hp->sigfigs = SWAPLONG(hp->sigfigs);
	hp->snaplen = SWAPLONG(hp->snaplen);
	hp->linktype = SWAPLONG(hp->linktype);
}

pcap_t *
pcap_open_offline(char *fname, char *errbuf)
{
	register pcap_t *p;
	register FILE *fp;
	struct pcap_file_header hdr;
	int linklen;

	p = (pcap_t *)malloc(sizeof(*p));
	if (p == NULL) {
		strcpy(errbuf, "out of swap");
		return (NULL);
	}

#ifdef notdef
	bzero(p, sizeof(*p));
#else
	memset(p, 0, sizeof(*p));
#endif
	/*
	 * Set this field so we don't close stdin in pcap_close!
	 */
	p->fd = -1;

	if (fname[0] == '-' && fname[1] == '\0')
		fp = stdin;
	else {
		fp = fopen(fname, "r");
		if (fp == NULL) {
			sprintf(errbuf, "%s: %s", fname, pcap_strerror(errno));
			goto bad;
		}
	}
	if (fread((char *)&hdr, sizeof(hdr), 1, fp) != 1) {
		sprintf(errbuf, "fread: %s", pcap_strerror(errno));
		goto bad;
	}
	if (hdr.magic != TCPDUMP_MAGIC) {
		if (SWAPLONG(hdr.magic) != TCPDUMP_MAGIC) {
			sprintf(errbuf, "bad dump file format");
			goto bad;
		}
		p->sf.swapped = 1;
		swap_hdr(&hdr);
	}
	if (hdr.version_major < PCAP_VERSION_MAJOR) {
		sprintf(errbuf, "archaic file format");
		goto bad;
	}
	p->tzoff = hdr.thiszone;
	p->snapshot = hdr.snaplen;
	p->linktype = hdr.linktype;
	p->sf.rfile = fp;
	p->bufsize = hdr.snaplen;
	/* Align link header as required for proper data alignment */
	linklen = 14;					/* XXX */
	p->sf.base = (u_char *)malloc(p->bufsize + BPF_ALIGNMENT);
	p->buffer = p->sf.base + BPF_ALIGNMENT - (linklen % BPF_ALIGNMENT);
	p->sf.version_major = hdr.version_major;
	p->sf.version_minor = hdr.version_minor;

	return (p);
 bad:
	free(p);
	return (NULL);
}

/*
 * Read sf_readfile and return the next packet.  Return the header in hdr
 * and the contents in buf.  Return 0 on success, SFERR_EOF if there were
 * no more packets, and SFERR_TRUNC if a partial packet was encountered.
 */
static int
sf_next_packet(pcap_t *p, struct pcap_pkthdr *hdr, u_char *buf, int buflen)
{
	FILE *fp = p->sf.rfile;

	/* read the stamp */
	if (fread((char *)hdr, sizeof(struct pcap_pkthdr), 1, fp) != 1) {
		/* probably an EOF, though could be a truncated packet */
		return (1);
	}

	if (p->sf.swapped) {
		/* these were written in opposite byte order */
		hdr->caplen = SWAPLONG(hdr->caplen);
		hdr->len = SWAPLONG(hdr->len);
		hdr->ts.tv_sec = SWAPLONG(hdr->ts.tv_sec);
		hdr->ts.tv_usec = SWAPLONG(hdr->ts.tv_usec);
	}
	/*
	 * We interchanged the caplen and len fields at version 2.3,
	 * in order to match the bpf header layout.  But unfortunately
	 * some files were written with version 2.3 in their headers
	 * but without the interchanged fields.
	 */
	if (p->sf.version_minor < 3 ||
	    (p->sf.version_minor == 3 && hdr->caplen > hdr->len)) {
		int t = hdr->caplen;
		hdr->caplen = hdr->len;
		hdr->len = t;
	}

	if (hdr->caplen > buflen) {
		sprintf(p->errbuf, "bad dump file format");
		return (-1);
	}

	/* read the packet itself */

	if (fread((char *)buf, hdr->caplen, 1, fp) != 1) {
		sprintf(p->errbuf, "truncated dump file");
		return (-1);
	}
	return (0);
}

/*
 * Print out packets stored in the file initialized by sf_read_init().
 * If cnt > 0, return after 'cnt' packets, otherwise continue until eof.
 */
int
pcap_offline_read(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
{
	struct bpf_insn *fcode = p->fcode.bf_insns;
	int status = 0;
	int n = 0;

	while (status == 0) {
		struct pcap_pkthdr h;

		status = sf_next_packet(p, &h, p->buffer, p->bufsize);
		if (status)
			return (-1);

		if (fcode == NULL ||
		    bpf_filter(fcode, p->buffer, h.len, h.caplen)) {
			(*callback)(user, &h, p->buffer);
			if (++n >= cnt && cnt > 0)
				break;
		}
	}
	/*XXX this breaks semantics tcpslice expects */
	return (n);
}

/*
 * Output a packet to the initialized dump file.
 */
void
pcap_dump(u_char *user, const struct pcap_pkthdr *h, const u_char *sp)
{
	FILE * f = (FILE *)user;
	(void)fwrite((char *)h, sizeof(*h), 1, f);
	(void)fwrite((char *)sp, h->caplen, 1, f);
}

/*
 * Initialize so that sf_write() will output to the file named 'fname'.
 */
pcap_dumper_t *
pcap_dump_open(pcap_t *p, char *fname)
{
	FILE *f;
	if (fname[0] == '-' && fname[1] == '\0')
		f = stdout;
	else {
		f = fopen(fname, "w");
		if (f == NULL) {
			sprintf(p->errbuf, "%s: %s",
			    fname, pcap_strerror(errno));
			return (NULL);
		}
	}
	(void)sf_write_header(f, p->linktype, p->tzoff, p->snapshot);
	return ((pcap_dumper_t *)f);
}

void
pcap_dump_close(pcap_dumper_t *p)
{
	fclose((FILE *)p);
}
