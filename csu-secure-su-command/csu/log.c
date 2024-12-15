/* Copyright 1993 by Scott Gustafson (scott@cadence.com) */

#ifdef DEBUG
#include <stdio.h>	/* for fprintf() */
#endif
#include <stdlib.h>	/* for exit() */
#ifdef SYSLOG
#include <syslog.h>
#endif
#include "log.h"

void setuploging(char *PGMNAME) {
#ifdef SYSLOG
	openlog(PGMNAME, LOG_PID & LOG_CONS, LOG_AUTH);
#endif
}

void log(char *message, char *userid) {
#ifdef SYSLOG
	syslog(LOG_NOTICE, message, userid);
#endif
#ifdef DEBUG
	fprintf(stderr,"%s %s\n", message, userid);
#endif
}

void error(char *message, char *userid) {
        log(message, userid);
#ifdef SYSLOG
        closelog();
#endif
        exit(-1);
}
