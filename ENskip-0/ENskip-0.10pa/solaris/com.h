/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#include <sys/ioccom.h>

#define SKIPIOC(cmd)       (('s' << 24) | ('k' << 16) | ('i' << 8) | (cmd))
#define SKIPIOCUPDATE      SKIPIOC(1)
#define SKIPIOCREMOVE      SKIPIOC(2)
#define SKIPIOCSTATCACHE   SKIPIOC(3)
#define SKIPIOCSTATIPSP    SKIPIOC(4)
#define SKIPIOCATTACH      SKIPIOC(5)
#define SKIPIOCDETACH      SKIPIOC(6)
#define SKIPIOCREGISTER    SKIPIOC(7)
#define SKIPIOCGETREQUESTS SKIPIOC(8)
#define SKIPIOCFLUSH       SKIPIOC(9)

#ifdef KERNEL
extern int com_register(pid_t pid);
extern int com_signal(int sig);
#endif
