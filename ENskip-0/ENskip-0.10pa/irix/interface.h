/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
struct interface_fbparam
{
  struct ifnet *ifp;
  struct sockaddr_in address;
};

extern int interface_attach(u_char *IPaddr);
extern int interface_detach(u_char *IPaddr);
extern int interface_outfeedback(struct mbuf *m);
extern int interface_infeedback(struct mbuf *m);

extern int interface_init(void);
extern int interface_exit(void);
