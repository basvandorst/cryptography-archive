/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef TERMIO
#include <termios.h>
#include <unistd.h>
#endif /* TERMIO */

/* this is the password file and it's permissions */
#define MODE 33152 /* rw------- */

/* the maximum length of any string */
#define MAX_LENGTH 255

#define FALSE 0
#define TRUE 1
