/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
extern int random_init(void);
extern int random_exit(void);
extern int random_random(u_char *buf, int len);
extern int random_hash(u_char *buf, int buflen, u_char *val, int vallen);
extern int random_randomKp(struct skipcache *c);
