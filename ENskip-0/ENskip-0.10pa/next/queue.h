/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */

extern int queue_enqueuein(struct skipcache *c, struct mbuf *header, struct mbuf * m);
extern int queue_enqueueout(struct skipcache *c, struct mbuf *header, struct mbuf *m);

extern int queue_free(void *c);
extern int queue_feed(void *c);

