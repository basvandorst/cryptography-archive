/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */

extern int com_update(struct skipcache *c);
extern int com_remove(u_char *IPaddr);
extern int com_attach(u_char *IPaddr);
extern int com_detach(u_char *IPaddr);
extern int com_register(void);
extern int com_getrequests(u_char *buf, int len);
extern int com_flush(void);
extern int com_stat(ipsp_stat_t *ipspstat, skipcache_stat_t *cachestat);

extern int com_init(void);
extern int com_exit(void);
