/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
typedef u_char *skipcache_t;
typedef u_char *buffer_t;
typedef u_char IPaddr_t[IPADDRSIZE];

#ifdef KERNEL
extern int com_registerpid(port_t port, port_t priv, int pid);
extern int com_update(port_t port, port_t priv, u_char *arg, u_int len);
extern int com_remove(port_t port, port_t priv, u_char *IPaddr);
extern int com_signal(int sig);
#endif /* KERNEL */
