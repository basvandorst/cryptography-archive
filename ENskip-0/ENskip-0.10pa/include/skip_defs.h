/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#define IPPROTO_IPSP	79

#define IPADDRSIZE	16

#define SAID_SEQ	0x1
#define SAID_COMP	0x2
#define SAID_AUTH	0x4
#define SAID_CRYPT	0x8

#define SAID_VERSION	(0x1 << 4)

#define SKIP_HDRSIZE	12
#define SKIP_SAIDSIZE	4
#define SKIP_SEQSIZE	8
