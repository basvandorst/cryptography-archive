/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */

int sequence_inc(u_char *seq);
int sequence_check(struct sequence_history *h, u_char *hbuf, u_char *seq);
