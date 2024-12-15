/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#define SKIP_QUEUED	1
#define SKIP_PROCESSED	0
#define SKIP_DISCARD	-1

extern int skip_process(void *feedback, struct mbuf **m, int dir);
