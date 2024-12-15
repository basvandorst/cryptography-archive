/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
#define SKIPCMD_UPDATE      1
#define SKIPCMD_REMOVE      2
#define SKIPCMD_STATCACHE   3
#define SKIPCMD_STATIPSP    4
#define SKIPCMD_ATTACH      5
#define SKIPCMD_DETACH      6
#define SKIPCMD_REGISTER    7
#define SKIPCMD_GETREQUESTS 8
#define SKIPCMD_FLUSH       9

#ifdef KERNEL
extern int com_signal(int sig);
#endif
