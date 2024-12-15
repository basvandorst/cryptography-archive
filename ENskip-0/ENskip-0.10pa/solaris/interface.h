/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
int interface_attach(u_char *IPaddr);
int interface_detach(u_char *IPaddr);
int interface_infeedback(void *feedbackif, mblk_t *mp);
int interface_outfeedback(void *feedbackif, mblk_t *mp);

int interface_init(void);
int interface_exit(void);
