/*
 # $Id: misc.h,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
 # Copyright (C) 1997-1999 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#ifndef MISC_H
#define MISC_H

#define SEG_UNKN	-1
#define SEG_TEXT	0
#define SEG_DATA	1
#define SEG_RODATA	2
#define NSEGS		3

#define isoctal(a)	((a) >= '0' && (a) <= '7')

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) || defined(OS2)
#define f_stricmp		stricmp
#else
#define f_stricmp		strcasecmp
#endif

void		*f_malloc(size_t sz);
void		*f_realloc(void *s, size_t sz);
void		*f_calloc(size_t n, size_t sz);
char		*f_strdup(const char *s);
char		*f_strndup(const char *s, int n);
char		*seg_name(int i);
int		isnumeric(char *s);
int		zread(int fd, char *b, int n, char *msg);
void		zlseek(int fd, off_t off, char *msg);
unsigned short	toShort(unsigned char *p);
unsigned long	toLong(unsigned char *p);
char		*nextToken(char *p);
char		*getProgDir(char *p);
int		unescape(char *src, int len);
void		lftocr(char *s);

#ifdef macintosh
int		strcasecmp(const char *s1, const char *s2);
#endif

#ifdef __cplusplus
}
#endif

#endif
