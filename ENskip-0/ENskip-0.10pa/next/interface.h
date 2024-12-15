/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
struct interface_outfbparam
{
  netif_t ifp;
  struct sockaddr_in address;
};

int interface_outfeedback(struct mbuf *m);
int interface_infeedback(struct mbuf *m);

int interface_attach(u_char *IPaddr);
int interface_detach(u_char *IPaddr);

int interface_init(void);
int interface_exit(void);
