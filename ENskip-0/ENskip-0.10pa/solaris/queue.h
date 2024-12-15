/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
extern int queue_enqueuein(struct skipcache *c, mblk_t *m);
extern int queue_enqueueout(struct skipcache *c, mblk_t *header, mblk_t *m);

extern int queue_init(void);
extern int queue_exit(void);
